#pragma once
#include "SharedSrc/Util/Input_Manager.h"
#include "SharedSrc/Sockets/SocketInc.h"
#include "SharedSrc/Sockets/BitStream.h"
#include "ServerGameApp.h"

class ServerApp
{
public:
	ServerApp()
	{
		running = true;
	}
	void Run()
	{
		std::cout << "Server Running...\n";
		if (Init())
		{
			mainLoop();
		}
		CleanUp();
		std::cout << "Server closing down...\n";
	}

private:
	bool running;

	//sockets
	std::shared_ptr<TCP_Socket> listen_socket;
	std::vector<std::shared_ptr<TCP_Socket>> activeSockets;
	std::vector<std::shared_ptr<TCP_Socket>> readableSockets;
	std::vector<std::pair<std::shared_ptr<TCP_Socket>, Timer>> disconnectingSockets;

	BitStreamWriter bitwriter;
	BitStreamReader bitreader;

	std::vector<lobbydata> lobbies_active;
	
	//proxys
	struct clientdata
	{
		std::shared_ptr<TCP_Socket> tcp_socket;
		sockaddr_in addr;
		std::string username;
		SYSTEMTIME time_on_connection;
		uint16_t packets_sent;
		Timer last_heard_from;
	};
	std::array<std::pair<clientdata, bool>, 100> clientproxys; //pair(clientdata, inuse)

	void addclientproxy(clientdata& data);
	void removeclientproxy(int proxy_id);
	int Findclientproxyid(const std::shared_ptr<TCP_Socket>& socket);
	void findLobbyFromProxyId(int proxy_id, bool& is_host, int& lobby_id, int& joiner_id);
	
	//game servers
	std::vector<ServerGameApp*> running_games;

private:
	int Init();
	void mainLoop();
	void CleanUp();
	int CreateListenSocket();

	void Host_MatchMaking();
	void HostGames();

	void HandleNewConnection(const std::shared_ptr<TCP_Socket>& socket);
	void HandleIncomingPacket(const std::shared_ptr<TCP_Socket>& socket);
	void HandleFINPacket(const std::shared_ptr<TCP_Socket>& socket);
	void HandlerecError(const std::shared_ptr<TCP_Socket>& socket);
	void HandleUsernamePacket(const std::shared_ptr<TCP_Socket>& socket);
	void HandleRefreshPacket(const std::shared_ptr<TCP_Socket>& socket);
	void HandleEnterlobbyPacket(const std::shared_ptr<TCP_Socket>& socket);
	void HandleConnectionCheckPacket(const std::shared_ptr<TCP_Socket>& socket);
	void HandleLobbyLeavingPacket(const std::shared_ptr<TCP_Socket>& socket);
	void HandleLobbyReadyPacket(const std::shared_ptr<TCP_Socket>& socket);
	void HandleLobbyTypePacket(const std::shared_ptr<TCP_Socket>& socket);

	void DisconnectClient(int proxy_id, bool gracefully = true);
	void handleGracefulDisconnects();
	void CheckClientsConnection();

	void leavelobbyHost(int lobby_id);
	void leavelobbyJoiner(int lobby_id, int joiner_id);
	
	void LaunchGame(int lobby_id);
	void NotifyLobby(int lobby_id, bool reset = true);

	void sendlobbyfailPacket(const std::shared_ptr<TCP_Socket>& socket, uint8_t error_code);
	void sendLobbyInfoPacket(const std::shared_ptr<TCP_Socket>& socket, int lobby_id, bool reset = true);
	void sendMatchMakingPacket(const std::shared_ptr<TCP_Socket>& socket, bool reset = true);
};