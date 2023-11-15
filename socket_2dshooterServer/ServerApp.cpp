#include "pchserver.h"
#include "ServerApp.h"

//make sure if we crash we note it or error and have to leave we quit all clients
//make sure we check on clients and if they fail. I guess client sends us a message already,
//if client dc's make sure to clean up his proxy and all lobbies

//add debug prints, debug packet sending and receiving to file
//simulation ping, variability
//test a lot of different cases
//typing better
int ServerApp::Init()
{
	config::LoadConfigValues();
	if (StartWSA() == -1)
		return -1;

	char name[100];
	gethostname(name, 100);
	std::cout << "hostname: " << name << std::endl;

	if (CreateListenSocket() == -1)
		return -1;

	bitreader.Allocate(1000);

	for (int i = 0; i < clientproxys.size(); i++)
		clientproxys[i].second = false;

	return 1;
}
void ServerApp::mainLoop()
{
	//our loop runs every 90fps and sleeps if theres extra time
	float SERVER_TICK_RATE = 11.1111111111;
	//get new time - last recorded. If difference is less than 11.111111 sleep until 11.111111
	Timer timer_tick_rate;
	while (running)
	{
		auto beggining_time = std::chrono::high_resolution_clock::now();
#ifdef PING_EMULATOR
		PingEmulator::client = false;
		PingEmulator::update();
#endif
		Host_MatchMaking();
		HostGames();
		

		if (GetKeyState('Q') & 0x8000)
			running = false;


		auto end_time = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(beggining_time).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(end_time).time_since_epoch().count();
		float duration = end - start;

		if (duration < SERVER_TICK_RATE)
		{
			//std::cout << "sleeping for " << SERVER_TICK_RATE - duration << std::endl;
			Sleep(SERVER_TICK_RATE - duration);
		}
	}

	//gracefully close any active clients
	for (int i = 0; i < activeSockets.size(); i++)
	{
		if (activeSockets[i] != listen_socket) {
			DisconnectClient(Findclientproxyid(activeSockets[i]));
			i--;
		}
	}
	while (!disconnectingSockets.empty())
	{
		handleGracefulDisconnects();
	}
}

void ServerApp::handleGracefulDisconnects()
{
	for (int i = 0; i < disconnectingSockets.size(); i++)
	{
		if (!disconnectingSockets[i].first->isShutDown())
		{
			disconnectingSockets[i].first->ShutDown(SD_SEND);
			disconnectingSockets[i].second.Begin();
		}

		bitreader.reset();
		int bytesReceived = disconnectingSockets[i].first->Recv(bitreader.getData(), bitreader.getCapacityBytes());
		if (bytesReceived == 0 || disconnectingSockets[i].second.TimeElapsed(5000))
		{
			disconnectingSockets[i].first.reset();
			disconnectingSockets.erase(disconnectingSockets.begin() + i);
			i--;
			if (bytesReceived == 0)
				std::cout << "gracefully closed connection\n";
			else
				std::cout << "gracefully close didnt hear back\n";
		}
	}
}

void ServerApp::DisconnectClient(int proxy_id, bool gracefully)
{
	std::cout << "disconnecting client ";
	if (gracefully)
		std::cout << "gracefully\n";
	else
		std::cout << "ungracefully\n";

	if (clientproxys[proxy_id].second == false)
	{
		std::cout << "GracefullyDisconnectClient, proxy_id is not valid???\n";
		return;
	}
	
	bool is_host = false;
	int lobby_id = -1;
	int joiner_id = -1;
	findLobbyFromProxyId(proxy_id, is_host, lobby_id, joiner_id);
	if (lobby_id != -1)
	{
		std::cout << "client still in a lobby, kicking them out...\n";
		if (is_host)
		{
			leavelobbyHost(lobby_id);
		}
		else
		{
			leavelobbyJoiner(lobby_id, joiner_id);
		}
	}

	int id = -1;
	for (int i = 0; i < activeSockets.size(); i++)
	{
		if (activeSockets[i] == clientproxys[proxy_id].first.tcp_socket)
		{
			id = i;
			break;
		}
	}
	if (id == -1)
	{
		std::cout << "gracefully disconnecting client, could not find proxy_id in active sockets???";
		return;
	}

	if (gracefully)
		disconnectingSockets.emplace_back(activeSockets[id], Timer());
	else
		activeSockets[id]->ShutDown(SD_SEND);
	
	activeSockets.erase(activeSockets.begin() + id);
	removeclientproxy(proxy_id);
}


void ServerApp::CleanUp()
{
	activeSockets.clear();
	readableSockets.clear();
	disconnectingSockets.clear();

	listen_socket.reset();

	CloseWSA();
}

int ServerApp::CreateListenSocket()
{
	listen_socket = std::make_shared<TCP_Socket>();

	uint16_t portid = config::ServerPort;
	int tries = 1;
	while (tries <= 50)
	{
		std::cout << "TCP server connecting try... " << tries << std::endl;
		sockaddr listen_addr = createSocketAddr_Mixed(config::ServerIP.c_str(), portid++);
		if (listen_socket->Bind(listen_addr) != SOCKET_ERROR) {
			std::cout << "server bind success!\n";
			break;
		}

		tries++;
	};
	if (tries > 50)
	{
		std::cout << "couldn't bind port number, oh well aborting\n";
		return -1;
	}

	std::cout << "SERVER TCP ADDRESS: ";
	printSockName(listen_socket->getSocket());

	activeSockets.push_back(listen_socket);

	listen_socket->Listen();

	return 1;
}

void ServerApp::Host_MatchMaking()
{
	timeval timeout;
	timeout.tv_sec = 0.0167;
	timeout.tv_usec = 0;

	readableSockets.clear();
	handleGracefulDisconnects();
	CheckClientsConnection();
	if (TCP_Socket::Select(&activeSockets, &readableSockets, nullptr, nullptr, nullptr, nullptr, &timeout))
	{
		for (const std::shared_ptr<TCP_Socket>& socket : readableSockets)
		{
			if (socket == listen_socket)
			{
				HandleNewConnection(socket);
			}
			else
			{
				HandleIncomingPacket(socket);
			}
		}
	}
}

void ServerApp::CheckClientsConnection()
{
	for (int i = 0; i < activeSockets.size(); i++)
	{
		if (activeSockets[i] != listen_socket)
		{
			int proxy_id = Findclientproxyid(activeSockets[i]);
			if (clientproxys[proxy_id].first.last_heard_from.TimeElapsed(5000))
			{
				std::cout << "havn't heard back from client in 5 seconds, disconnecting client...\n";
				DisconnectClient(proxy_id);
				i--;
			}
		}
	}
}

void ServerApp::HandleIncomingPacket(const std::shared_ptr<TCP_Socket>& socket)
{
	bitreader.reset();
	int bytesReceived = socket->Recv(bitreader.getData(), bitreader.getCapacityBytes());
	if (bytesReceived == 0) //FIN PACKET
	{
		HandleFINPacket(socket);
	}
	else if (bytesReceived < 0) //ERROR
	{
		HandlerecError(socket);
	}
	else 
	{
		int bits_received = bytesReceived * 8;
		while (bitreader.getbithead() < bits_received)
		{
			//bytes
			PACKETCODE code;
			bitreader.Read(&code);
			if (code == PACKETCODE::USERNAME)
			{
				HandleUsernamePacket(socket);
			}
			else if (code == PACKETCODE::REFRESH)
			{
				HandleRefreshPacket(socket);
			}
			else if (code == PACKETCODE::ENTERLOBBY)
			{
				HandleEnterlobbyPacket(socket);
			}
			else if (code == PACKETCODE::CONNECTION_CHECK)
			{
				HandleConnectionCheckPacket(socket);
			}
			else if (code == PACKETCODE::LOBBYLEAVING)
			{
				HandleLobbyLeavingPacket(socket);
			}
			else if (code == PACKETCODE::LOBBYREADY)
			{
				HandleLobbyReadyPacket(socket);
			}
			else if (code == PACKETCODE::LOBBYTYPE)
			{
				HandleLobbyTypePacket(socket);
			}

			//each packet makes sure it ends on a byte, round to byte
			if (bitreader.getbithead() % 8 != 0)
			{
				int remainder = bitreader.getbithead() % 8;
				bitreader.setbithead(bitreader.getbithead() + (8 - remainder));
			}
		}
	}
}

void ServerApp::HandleNewConnection(const std::shared_ptr<TCP_Socket>& socket)
{
	sockaddr_in client_addr;
	std::shared_ptr<TCP_Socket> handshake_socket = socket->Accept(client_addr);
	if (handshake_socket == nullptr) 
		return;

	activeSockets.push_back(handshake_socket);

	sockaddr_in addr_check;
	int addr_len = sizeof(sockaddr_in);
	WSA_CHECK(getpeername(handshake_socket->getSocket(), reinterpret_cast<sockaddr*>(&addr_check), &addr_len), "getpeername ");
	std::cout << "accepted new client(peer name): "; printSocketAddr(addr_check);

	clientdata cl_data;
	cl_data.addr = addr_check;
	cl_data.tcp_socket = handshake_socket;
	cl_data.username = "NA";
	cl_data.packets_sent = 0;
	GetSystemTime(&cl_data.time_on_connection);
	cl_data.last_heard_from.Begin();

	addclientproxy(cl_data);
}

void ServerApp::HandleFINPacket(const std::shared_ptr<TCP_Socket>& socket)
{
	std::cout << "got a FIN Packet close socket\n";
	printPeerName(socket->getSocket());

	int proxy_id = Findclientproxyid(socket);
	if (proxy_id == -1)
	{
		std::cout << "got a FIN packet but cant find client proxy, ignoring...\n";
		return;
	}
	
	DisconnectClient(proxy_id, false);
}
void ServerApp::HandlerecError(const std::shared_ptr<TCP_Socket>& socket)
{
	std::cout << "error receiving from socket... shutting down and dropping\n";
	printPeerName(socket->getSocket());

	int proxy_id = Findclientproxyid(socket);
	DisconnectClient(proxy_id, false);
}
void ServerApp::HandleUsernamePacket(const std::shared_ptr<TCP_Socket>& socket)
{
	std::string username;
	ReadPacket_username(bitreader, username);

	int proxy_id = Findclientproxyid(socket);
	if (proxy_id == -1) {
		std::cout << "HandleUsernamePacket: can't find clientproxy???\n";
		return;
	}

	clientproxys[proxy_id].first.username = username;
	clientproxys[proxy_id].first.packets_sent++;

	bitwriter.reset();
	WritePacket_welcometoserver(bitwriter);
	
	sendMatchMakingPacket(socket, false);

	std::cout << "sending welcometoserver and matchmaking packet!\n";
}
void ServerApp::HandleRefreshPacket(const std::shared_ptr<TCP_Socket>& socket)
{
	int proxy_id = Findclientproxyid(socket);
	clientproxys[proxy_id].first.packets_sent++;

	sendMatchMakingPacket(socket);
}
void ServerApp::HandleEnterlobbyPacket(const std::shared_ptr<TCP_Socket>& socket)
{
	create_info data;
	ReadPacket_enterlobby(bitreader, data);

	if (data.creator)
	{
		int proxy_id = Findclientproxyid(socket);

		lobbydata new_lobby;
		new_lobby.info.info.lobby_name = data.lobby_name;
		new_lobby.info.info.player_count = 1;
		new_lobby.info.openaccess = data.open_access;
		new_lobby.info.info.lobby_host = clientproxys[proxy_id].first.username;
		new_lobby.creator_index = proxy_id;
		new_lobby.info.ready.push_back(0);

		lobbies_active.push_back(new_lobby);
		std::cout << "active lobbies: " << lobbies_active.size() << std::endl;

		bitwriter.reset();
		WritePacket_welcometolobby(bitwriter);

		sendLobbyInfoPacket(socket, lobbies_active.size() - 1, false);
	}
	else
	{
		int proxy_id = Findclientproxyid(socket);
		std::string host_name = data.lobby_name;

		int lobby_id = -1;
		for (int i = 0; i < lobbies_active.size(); i++)
		{
			if (strcmp(lobbies_active[i].info.info.lobby_host.c_str(), host_name.c_str()) == 0)
			{
				lobby_id = i;
				break;
			}
		}
		if (lobby_id == -1)
		{
			std::cout << "couldn't find lobby to join let him know it couldn't connect";
			sendlobbyfailPacket(socket, 0);
		}
		else
		{
			lobbydata& data = lobbies_active[lobby_id];
			int lobby_max_size = 100;
			if (data.joiner_indices.size() + 1 >= lobby_max_size)
			{
				std::cout << "lobby is full, couldn't find seat\n";
				sendlobbyfailPacket(socket, 1);
			}
			else
			{
				data.joiner_indices.push_back(proxy_id);
				data.info.info.player_count++;
				data.info.ready.push_back(0);

				bitwriter.reset();
				WritePacket_welcometolobby(bitwriter);
				socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());

				NotifyLobby(lobby_id);
			}
		}
	}
}
void ServerApp::HandleConnectionCheckPacket(const std::shared_ptr<TCP_Socket>& socket)
{
	int proxy_id = Findclientproxyid(socket);
	clientproxys[proxy_id].first.last_heard_from.Begin();

	bitwriter.reset();
	WritePacket_connectioncheck(bitwriter);
	int bytessent = socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
}
void ServerApp::HandleLobbyLeavingPacket(const std::shared_ptr<TCP_Socket>& socket)
{
	int proxy_id = Findclientproxyid(socket);

	bool is_host = false;
	int lobby_id = -1;
	int joiner_id = -1;
	findLobbyFromProxyId(proxy_id, is_host, lobby_id, joiner_id);

	if (lobby_id == -1)
	{
		std::cout << "couldn't find which lobby client is in for lobby_leaving\n";
	}
	else
	{
		if (is_host)
		{
			leavelobbyHost(lobby_id);
		}
		else
		{
			leavelobbyJoiner(lobby_id, joiner_id);
		}
	}
	sendMatchMakingPacket(socket);
}

void ServerApp::HandleLobbyReadyPacket(const std::shared_ptr<TCP_Socket>& socket)
{
	bool isready;
	ReadPacket_lobbyready(bitreader, isready);

	int proxy_id = Findclientproxyid(socket);

	bool is_host = false;
	int lobby_id = -1;
	int joiner_id = -1;
	findLobbyFromProxyId(proxy_id, is_host, lobby_id, joiner_id);
	if (lobby_id != -1)
	{
		if (is_host)
			lobbies_active[lobby_id].info.ready[0] = isready;
		else
			lobbies_active[lobby_id].info.ready[joiner_id + 1] = isready;

		int ready_to_go = 0;
		for (int i = 0; i < lobbies_active[lobby_id].info.ready.size(); i++)
		{
			if (lobbies_active[lobby_id].info.ready[i])
				ready_to_go++;
		}

		if (ready_to_go == lobbies_active[lobby_id].info.info.player_count)
			LaunchGame(lobby_id);
		else
			NotifyLobby(lobby_id);
	}
}
void ServerApp::HandleLobbyTypePacket(const std::shared_ptr<TCP_Socket>& socket)
{
	std::string msg;
	ReadPacket_lobbytype(bitreader, msg);

	int proxy_id = Findclientproxyid(socket);

	bool is_host = false;
	int lobby_id = -1;
	int joiner_id = -1;
	findLobbyFromProxyId(proxy_id, is_host, lobby_id, joiner_id);
	if (lobby_id != -1)
	{
		std::string lobbymsg_str;
		//add name
		if (is_host)
			lobbymsg_str = clientproxys[lobbies_active[lobby_id].creator_index].first.username;
		else
			lobbymsg_str = clientproxys[lobbies_active[lobby_id].joiner_indices[joiner_id]].first.username;

		//add msg
		lobbymsg_str += ": ";
		lobbymsg_str += msg;

		lobbies_active[lobby_id].info.messages.push_back(lobbymsg_str);

		NotifyLobby(lobby_id);
	}
}

void ServerApp::leavelobbyHost(int lobby_id)
{
	//kick everyone out, and destroy lobby
	std::vector<int> joiner_ids = lobbies_active[lobby_id].joiner_indices;

	bitwriter.reset();
	WritePacket_lobbydisbanded(bitwriter);
	clientproxys[lobbies_active[lobby_id].creator_index].first.tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());

	lobbies_active.erase(lobbies_active.begin() + lobby_id);
	for (int i = 0; i < joiner_ids.size(); i++)
	{
		int joiner_id = joiner_ids[i];

		clientproxys[joiner_id].first.tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());

		sendMatchMakingPacket(clientproxys[joiner_id].first.tcp_socket, false);
	}
}
void ServerApp::leavelobbyJoiner(int lobby_id, int joiner_id)
{
	lobbies_active[lobby_id].joiner_indices.erase(lobbies_active[lobby_id].joiner_indices.begin() + joiner_id);
	lobbies_active[lobby_id].info.ready.erase(lobbies_active[lobby_id].info.ready.begin() + joiner_id + 1);
	lobbies_active[lobby_id].info.info.player_count--;
	NotifyLobby(lobby_id);
}

void ServerApp::HostGames()
{
	for (int i = 0; i < running_games.size(); i++)
	{
		if (!running_games[i]->loop())
		{
			delete running_games[i];
			running_games.erase(running_games.begin() + i);
			i--;
		}
	}
}

void ServerApp::LaunchGame(int lobby_id)
{
	//connect everyone to new game, setup new udp connections
	int player_count = lobbies_active[lobby_id].joiner_indices.size() + 1;
	int port_id = -1;
	ServerGameApp* new_game = new ServerGameApp(player_count);
	if (new_game->StartUp(port_id))
	{
		std::cout << "starting new game! port: " << port_id << std::endl;
		running_games.push_back(new_game);

		//send everyone a launchgame packet with the port_id
		bitwriter.reset();
		WritePacket_launchgame(bitwriter, port_id);
		for (int i = 0; i < lobbies_active[lobby_id].joiner_indices.size(); i++)
		{
			if (lobbies_active[lobby_id].joiner_indices[i] != -1)
				clientproxys[lobbies_active[lobby_id].joiner_indices[i]].first.tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
		}
		clientproxys[lobbies_active[lobby_id].creator_index].first.tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
	
		//destroy lobby
		lobbies_active.erase(lobbies_active.begin() + lobby_id);
	}
	else
	{
		std::cout << "failed to startup game!\n";
		delete new_game;

		leavelobbyHost(lobby_id);
	}
}

void ServerApp::NotifyLobby(int lobby_id, bool reset)
{
	//sends a serverlobby packet to everyone in the lobby to update information
	for (int i = 0; i < lobbies_active[lobby_id].joiner_indices.size(); i++)
	{
		if (lobbies_active[lobby_id].joiner_indices[i] != -1)
			sendLobbyInfoPacket(clientproxys[lobbies_active[lobby_id].joiner_indices[i]].first.tcp_socket, lobby_id, reset);
	}
	sendLobbyInfoPacket(clientproxys[lobbies_active[lobby_id].creator_index].first.tcp_socket, lobby_id, reset);
}

void ServerApp::sendlobbyfailPacket(const std::shared_ptr<TCP_Socket>& socket, uint8_t error_code)
{
	bitwriter.reset();
	WritePacket_lobbyfail(bitwriter, error_code);

	int bytesSent = socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
}

void ServerApp::sendLobbyInfoPacket(const std::shared_ptr<TCP_Socket>& socket, int lobby_id, bool reset)
{
	//find correct lobby
	lobbydata& data = lobbies_active[lobby_id];

	//lobby info
	lobby_fullinfo_client info;
	info.info.info.lobby_host = data.info.info.lobby_host;
	info.info.info.lobby_name = data.info.info.lobby_name;
	info.info.info.player_count = data.info.info.player_count;
	info.info.openaccess = data.info.openaccess;

	info.names.push_back(clientproxys[data.creator_index].first.username.c_str());
	for (int i = 0; i < data.joiner_indices.size(); i++)
	{
		if (data.joiner_indices[i] != -1)
			info.names.push_back(clientproxys[data.joiner_indices[i]].first.username.c_str());
	}

	info.info.ready = data.info.ready;
	info.info.messages = data.info.messages;

	if(reset)
		bitwriter.reset();
	WritePacket_serverlobby(bitwriter, info);

	int bytesSent = socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
}

void ServerApp::sendMatchMakingPacket(const std::shared_ptr<TCP_Socket>& socket, bool reset)
{
	if(reset)
		bitwriter.reset();
	WritePacket_matchmaking(bitwriter, lobbies_active);

	int bytesSent = socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
}

//CLIENT PROXY
void ServerApp::addclientproxy(clientdata& data)
{
	for (int i = 0; i < clientproxys.size(); i++)
	{
		if (clientproxys[i].second == false)
		{
			clientproxys[i].first = data;
			clientproxys[i].second = true;
			break;
		}
	}
}
void ServerApp::removeclientproxy(int proxy_id)
{
	clientproxys[proxy_id].first.tcp_socket.reset();
	clientproxys[proxy_id].second = false;
}

int ServerApp::Findclientproxyid(const std::shared_ptr<TCP_Socket>& socket)
{
	int id = -1;
	for (int i = 0; i < clientproxys.size(); i++)
	{
		if (clientproxys[i].second == true && clientproxys[i].first.tcp_socket == socket)
		{
			id = i;
			break;
		}
	}
	return id;
}

void ServerApp::findLobbyFromProxyId(int proxy_id, bool& is_host, int& lobby_id, int& joiner_id)
{
	for (int i = 0; i < lobbies_active.size(); i++)
	{
		if (proxy_id == lobbies_active[i].creator_index) {
			is_host = true;
			lobby_id = i;
			break;
		}
		for (int j = 0; j < lobbies_active[i].joiner_indices.size(); j++)
		{
			if (proxy_id == lobbies_active[i].joiner_indices[j])
			{
				is_host = false;
				lobby_id = i;
				joiner_id = j;
				break;
			}
		}
	}
}