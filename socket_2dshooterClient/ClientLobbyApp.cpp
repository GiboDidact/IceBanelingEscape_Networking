#include "pchclient.h"
#include "ClientLobbyApp.h"

int ClientLobbyApp::initialize()
{
	config::LoadConfigValues();

	graphics_manager.Initialize("ClientLobbyApp", false);

	glfwSetFramebufferSizeCallback(graphics_manager.getWindow(), framebuffer_size_callback);
	glfwSetKeyCallback(graphics_manager.getWindow(), key_callback);
	glfwSetCursorPosCallback(graphics_manager.getWindow(), cursor_position_callback);
	glfwSetMouseButtonCallback(graphics_manager.getWindow(), mouse_button_callback);
	glfwSetScrollCallback(graphics_manager.getWindow(), scroll_callback);

	glfwSetWindowUserPointer(graphics_manager.getWindow(), reinterpret_cast<void*>(this));

	if (StartWSA() == -1)
		return -1;

	bitreader.Allocate(MAX_READ_BUFFER_SIZE);

	return 1;
}

void ClientLobbyApp::cleanUp()
{
	if (cam_main)
		delete cam_main;
	tcp_socket.reset();

	graphics_manager.cleanUp();
	CloseWSA();
}

void ClientLobbyApp::options()
{
	//just render a new menu
}

void ClientLobbyApp::mainLoop()
{
	cam_main = new Camera(glm::vec3(0, 0, 15), glm::vec3(0, 1, 0), 270.0f, 90.0f);
	if (cam_main)
	{
		cam_main->scale.x = 10;
		cam_main->scale.y = 10;
	}

	global_data.init();

	global_data.rect_1 = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(global_data.rect_1).SetTexture(3);
	graphics_manager.GetRenderRect(global_data.rect_1).SetVisibility(0);
	graphics_manager.GetRenderRect(global_data.rect_1).Scale(glm::vec3(9, 4, 2));
	graphics_manager.GetRenderRect(global_data.rect_1).Translate(glm::vec3(0, 1.75, 0));

	global_data.rect_2 = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(global_data.rect_2).SetTexture(3);
	graphics_manager.GetRenderRect(global_data.rect_2).SetVisibility(0);
	graphics_manager.GetRenderRect(global_data.rect_2).Scale(glm::vec3(4.25, 6, 2));
	graphics_manager.GetRenderRect(global_data.rect_2).Translate(glm::vec3(2, .75, 0));

	global_data.rect_3 = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(global_data.rect_3).SetTexture(3);
	graphics_manager.GetRenderRect(global_data.rect_3).SetVisibility(0);
	graphics_manager.GetRenderRect(global_data.rect_3).Scale(glm::vec3(6, 2, 2));
	graphics_manager.GetRenderRect(global_data.rect_3).Translate(glm::vec3(0, -3.5, 0));

	global_data.rect_4 = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(global_data.rect_4).SetTexture(3);
	graphics_manager.GetRenderRect(global_data.rect_4).SetVisibility(0);
	graphics_manager.GetRenderRect(global_data.rect_4).Scale(glm::vec3(6, 2, 2));
	graphics_manager.GetRenderRect(global_data.rect_4).Translate(glm::vec3(0, -3.5, 0));

	while (!glfwWindowShouldClose(graphics_manager.getWindow()) && !global_data.should_leave)
	{
#ifdef PING_EMULATOR
		PingEmulator::update();
#endif
		if (global_data.gracefully_closing_connection)
		{
			handleGracefulDisconnect();
			glfwPollEvents();
		}
		else
		{
			handleTransition();
			handleInput();
			handleLogic();
		}
		handleRendering();	
	}
}

void ClientLobbyApp::handleGracefulDisconnect()
{
	if (!tcp_socket->isShutDown()) {
		tcp_socket->ShutDown(SD_SEND);
		global_data.closing_connection_timer.Begin();
	}

	bitreader.reset();
	int bytesReceived = tcp_socket->Recv(bitreader.getData(), bitreader.getCapacityBytes());
	if (bytesReceived == 0 || global_data.closing_connection_timer.TimeElapsed(5000))
	{
		tcp_socket.reset();
		global_data.gracefully_closing_connection = false;
		global_data.connected_to_server = false;

		if (bytesReceived == 0)
			std::cout << "gracefully closed connection\n";
		else
			std::cout << "gracefully close didnt hear back\n";
	}

	return;
}

void ClientLobbyApp::handleTransition()
{
	if (global_data.transition_main || global_data.transition_matchmaking || global_data.transition_lobby)
	{
		assert((global_data.transition_main ^ global_data.transition_matchmaking) ^ global_data.transition_lobby);

		if (global_data.transition_main)
		{
			global_data.transition_main = false;

			main_data.init();
			global_data.mainmenureset();
			global_data.menu_screen = MENU_TYPE::MAIN;
		}
		else if (global_data.transition_matchmaking)
		{
			global_data.transition_matchmaking = false;

			matchmaking_data.init();
			global_data.menu_screen = MENU_TYPE::MATCHMAKING;
		}
		else if (global_data.transition_lobby)
		{
			global_data.transition_lobby = false;

			lobby_data.init();
			global_data.menu_screen = MENU_TYPE::LOBBY;
		}
	}
}

void ClientLobbyApp::handleInput()
{
	glfwPollEvents();

	if (global_data.menu_screen == MENU_TYPE::MAIN)
	{
		if (!main_data.typing_name && !main_data.isconnecting())
		{
			if (InputManager.GetKeyPressOnce(GLFW_KEY_S) || InputManager.GetKeyPressOnce(GLFW_KEY_DOWN))
				main_data.menu_option = static_cast<OPTIONS_MAIN>(std::min(2, static_cast<int>(main_data.menu_option) + 1));
			else if (InputManager.GetKeyPressOnce(GLFW_KEY_W) || InputManager.GetKeyPressOnce(GLFW_KEY_UP))
				main_data.menu_option = static_cast<OPTIONS_MAIN>(std::max(0, static_cast<int>(main_data.menu_option) - 1));
		}

		if (InputManager.GetKeyPressOnce(GLFW_KEY_BACKSPACE))
		{
			if (main_data.typing_name)
			{
				InputManager.stopandreceiverecordingTmp();
				main_data.typing_name = false;
			}
		}

		if (InputManager.GetKeyPressOnce(GLFW_KEY_ENTER))
		{
			if (main_data.menu_option == OPTIONS_MAIN::JOIN && !main_data.isconnecting())
			{
				if (!main_data.typing_name)
				{
					main_data.typing_name = true;
					InputManager.startrecordingTmp();
				}
				else
				{
					std::vector<int> keys_pressed = InputManager.stopandreceiverecordingTmp();
					main_data.typing_name = false;

					global_data.username.clear();
					for (int i = 0; i < keys_pressed.size(); i++)
					{
						if (keys_pressed[i] != GLFW_KEY_ENTER)
							global_data.username += keys_pressed[i];
					}

					if (global_data.username.length() > 1)
					{
						main_data.connecting_server = true;
						if (tcp_socket.use_count() > 0) {
							std::cout << "BUG tcp socket should not exist at this point, closing socket...\n";
							tcp_socket.reset();
							global_data.connected_to_server = false;
						}
					}
				}
			}
			else if (main_data.menu_option == OPTIONS_MAIN::OPTIONS)
			{
				//options();
			}
			else if (main_data.menu_option == OPTIONS_MAIN::QUIT)
			{
				global_data.should_leave = true;
			}
		}
	}
	else if (global_data.menu_screen == MENU_TYPE::MATCHMAKING)
	{
		if (!matchmaking_data.typing_lobbyname && !matchmaking_data.picking_lobby && !matchmaking_data.connecting_to_lobby())
		{
			if (InputManager.GetKeyPressOnce(GLFW_KEY_S) || InputManager.GetKeyPressOnce(GLFW_KEY_DOWN))
				matchmaking_data.menu_option = static_cast<OPTIONS_MATCHMAKING>(std::min(3, static_cast<int>(matchmaking_data.menu_option) + 1));
			else if (InputManager.GetKeyPressOnce(GLFW_KEY_W) || InputManager.GetKeyPressOnce(GLFW_KEY_UP))
				matchmaking_data.menu_option = static_cast<OPTIONS_MATCHMAKING>(std::max(0, static_cast<int>(matchmaking_data.menu_option) - 1));
		}
		else if (matchmaking_data.picking_lobby) {
			if (InputManager.GetKeyPressOnce(GLFW_KEY_S) || InputManager.GetKeyPressOnce(GLFW_KEY_DOWN))
				matchmaking_data.lobby_selection = std::min((int)global_data.lobbies_available.size() - 1, matchmaking_data.lobby_selection + 1);
			else if (InputManager.GetKeyPressOnce(GLFW_KEY_W) || InputManager.GetKeyPressOnce(GLFW_KEY_UP))
				matchmaking_data.lobby_selection = std::max(0, matchmaking_data.lobby_selection - 1);
		}
		if (InputManager.GetKeyPressOnce(GLFW_KEY_BACKSPACE))
		{
			if (matchmaking_data.typing_lobbyname)
			{
				InputManager.stopandreceiverecordingTmp();
				matchmaking_data.typing_lobbyname = false;
			}
			else if (matchmaking_data.picking_lobby)
			{
				matchmaking_data.lobby_selection = -1;
				matchmaking_data.picking_lobby = false;
			}
		}
		if (InputManager.GetKeyPressOnce(GLFW_KEY_ENTER))
		{
			if (matchmaking_data.menu_option == OPTIONS_MATCHMAKING::JOIN)
			{
				if (!matchmaking_data.joining_lobby)
				{
					if (!matchmaking_data.picking_lobby)
					{
						if (!global_data.lobbies_available.empty()) {
							matchmaking_data.lobby_selection = 0;
							matchmaking_data.picking_lobby = true;
						}
					}
					else
					{
						sendjoinLobbyPacket(matchmaking_data.lobby_selection);
						matchmaking_data.picking_lobby = false;
						matchmaking_data.joining_lobby = true;
						matchmaking_data.timer_2.Begin();
						matchmaking_data.lobby_selection = -1;
					}
				}
			}
			else if (matchmaking_data.menu_option == OPTIONS_MATCHMAKING::CREATE)
			{
				if (!matchmaking_data.creating_lobby)
				{
					if (!matchmaking_data.typing_lobbyname)
					{
						matchmaking_data.typing_lobbyname = true;
						InputManager.startrecordingTmp();
					}
					else
					{
						matchmaking_data.typing_lobbyname = false;
						std::vector<int> keys_pressed = InputManager.stopandreceiverecordingTmp();
						global_data.lobbyname.clear();
						for (int i = 0; i < keys_pressed.size(); i++)
						{
							if (keys_pressed[i] != GLFW_KEY_ENTER)
								global_data.lobbyname += keys_pressed[i];
						}

						if (keys_pressed.size() > 1)
						{
							sendCreateLobbyPacket();
							matchmaking_data.creating_lobby = true;
							matchmaking_data.timer_1.Begin();
						}
					}
				}
			}
			else if (matchmaking_data.menu_option == OPTIONS_MATCHMAKING::REFRESH)
			{
				sendRefreshPacket();
			}
			else if (matchmaking_data.menu_option == OPTIONS_MATCHMAKING::BACK)
			{
				global_data.transition_main = true;
				global_data.gracefully_closing_connection = true;
			}
		}
	}
	else if (global_data.menu_screen == MENU_TYPE::LOBBY)
	{
		if (!lobby_data.typing_msg)
		{
			if (InputManager.GetKeyPressOnce(GLFW_KEY_A) || InputManager.GetKeyPressOnce(GLFW_KEY_LEFT))
				lobby_data.menu_option = static_cast<OPTIONS_LOBBY>(std::max(0, static_cast<int>(lobby_data.menu_option) - 1));
			else if (InputManager.GetKeyPressOnce(GLFW_KEY_D) || InputManager.GetKeyPressOnce(GLFW_KEY_RIGHT))
				lobby_data.menu_option = static_cast<OPTIONS_LOBBY>(std::min(2, static_cast<int>(lobby_data.menu_option) + 1));
		}

		if (InputManager.GetKeyPressOnce(GLFW_KEY_BACKSPACE))
		{
			if (lobby_data.typing_msg)
			{
				InputManager.stopandreceiverecordingTmp();
				lobby_data.typing_msg = false;
			}
		}

		if (InputManager.GetKeyPressOnce(GLFW_KEY_ENTER))
		{
			if (lobby_data.menu_option == OPTIONS_LOBBY::LEAVE)
			{
				sendleaveLobbyPacket();
				global_data.transition_matchmaking = true;
			}
			else if (lobby_data.menu_option == OPTIONS_LOBBY::MSG)
			{
				if (!lobby_data.typing_msg)
				{
					lobby_data.typing_msg = true;
					InputManager.startrecordingTmp();
				}
				else
				{
					lobby_data.typing_msg = false;

					std::vector<int> keys_pressed = InputManager.stopandreceiverecordingTmp();
					std::string msg;
					for (int i = 0; i < keys_pressed.size(); i++)
					{
						if (i >= 20) break;
						if (keys_pressed[i] != GLFW_KEY_ENTER)
							msg += keys_pressed[i];
					}

					if (keys_pressed.size() > 1)
						sendlobbytypePacket(msg);
				}
			}
			else if (lobby_data.menu_option == OPTIONS_LOBBY::READY)
			{
				lobby_data.ready_up = !lobby_data.ready_up;
				sendlobbyreadyPacket(lobby_data.ready_up);
			}
		}
	}

}

int ClientLobbyApp::handleUnexpectedDisconnect(int bytesreceived)
{
	//handle fin or error
	if (bytesreceived == 0)
	{
		std::cout << "FIN packet, server disconnecting from us\n";

		tcp_socket->ShutDown(SD_SEND);
		tcp_socket.reset();
		global_data.connected_to_server = false;
		global_data.transition_main = true;

		return -1;
	}
	else if (bytesreceived == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			std::cout << "error from recv " << WSAGetLastError() << " not sure so disconnect connection...\n";

			tcp_socket.reset();
			global_data.connected_to_server = false;
			global_data.transition_main = true;

			return -1;
		}
	}

	//handle no ping for 5 seconds
	if (global_data.asking_connection && global_data.connection_validated && global_data.connect_check_timer.TimeElapsed(100))
	{
		global_data.connection_validated = false;
		global_data.asking_connection = false;
	}
	if (!global_data.asking_connection)
	{
		bitwriter.reset();
		WritePacket_connectioncheck(bitwriter);
		int bytessent = tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());

		global_data.ping_timer.Begin();
		global_data.connect_check_timer.Begin();
		global_data.asking_connection = true;
		global_data.connection_validated = false;
	}
	else if (global_data.asking_connection && !global_data.connection_validated && global_data.connect_check_timer.TimeElapsed(5000))
	{
		std::cout << "havn't heard back from server in 5 seconds, trying to gracefully disconnect\n";
		global_data.gracefully_closing_connection = true;

		//we detected that we havn't heard back from the server and are going to abort, what do we do in each menu?
		global_data.transition_main = true;
		if (global_data.menu_screen == MENU_TYPE::MAIN)
		{

		}
		else if (global_data.menu_screen == MENU_TYPE::MATCHMAKING)
		{

		}
		else if (global_data.menu_screen == MENU_TYPE::LOBBY)
		{
			sendleaveLobbyPacket();
		}

		return -1;
	}

	//look for connection check packet
	if (findandremovepacketdata(PACKETCODE::CONNECTION_CHECK) != -1)
	{
		packets_info_this_frame.front();
		global_data.ping = global_data.ping_timer.getTime();

		global_data.connection_validated = true;
		global_data.connect_check_timer.Begin();
	}

	return 1;
}

int ClientLobbyApp::findandremovepacketdata(PACKETCODE code)
{
	//assuming we only have 1 packet of each type. fine for now
	for (auto itr = packets_info_this_frame.begin(); itr != packets_info_this_frame.end(); itr++)
	{
		if (itr->code == code)
		{
			int offset = itr->bit_offset;
			packets_info_this_frame.erase(itr);
			return offset;
		}
	}
	return -1;
}

int ClientLobbyApp::popandfindpacketdata(PACKETCODE code)
{
	bool found = false;
	for (auto itr = packets_info_this_frame.begin(); itr != packets_info_this_frame.end(); itr++)
	{
		if (itr->code == code)
		{
			found = true;
		}
	}
	if (found)
	{
		while (!packets_info_this_frame.empty())
		{
			if (packets_info_this_frame.front().code == code) {
				uint32_t offset = packets_info_this_frame.front().bit_offset;
				packets_info_this_frame.pop_front();
				return offset;
			}
			packets_info_this_frame.pop_front();
		}
	}
	return -1;
}

//To make it really robust you have to handle synchronizxation, what if a bunch of packets from previous frames all come this frame?
//do you remove all duplicate, run them all, do it different in different situations, make sure 2 packets would never come by stalling?

//what I do is remove all duplicates in the frame, stall joining of matchmaking and lobby until you recieve unique packet, and I also
//remove everything from queue in front of packet I need for most places. Really I just get the most up to date packets, and if I get see
//the packet I want I remove everything in front to get that
int ClientLobbyApp::ParseIncomingStream()
{
	while(!packets_info_this_frame.empty())
		packets_info_this_frame.pop_front();

	bitreader.reset();
	int bytesReceived = tcp_socket->Recvnoerror(bitreader.getData(), bitreader.getCapacityBytes());
	if (bytesReceived > 0)
	{	
		//we are assuming here that the first bit is a code, and that all the packetdata is tightly packed. This should be true
		//if anything goes wrong though it all offsets it, maybe have a tail message to know when it ends
		packet_data info;
		int bits_received = bytesReceived*8;

		while (bitreader.getbithead() < bits_received)
		{
			info.bit_offset = bitreader.getbithead() + 8;//skip ahead of opcode
			bitreader.Read(&info.code);

			if (info.code == PACKETCODE::MATCHMAKING)
			{
				std::vector<lobby_info> li;
				ReadPacket_matchmaking(bitreader, li);
			}
			else if (info.code == PACKETCODE::SERVERLOBBY)
			{
				lobby_fullinfo_client mm;
				ReadPacket_serverlobby(bitreader, mm);
			}
			else if (info.code == PACKETCODE::LOBBYFAIL)
			{
				uint8_t mm;
				ReadPacket_lobbyfail(bitreader, mm);
			}
			else if (info.code == PACKETCODE::LAUNCHGAME)
			{
				int packet_id;
				ReadPacket_launchgame(bitreader, packet_id);
			}
			else if (info.code == PACKETCODE::CONNECTION_CHECK)
			{
				ReadPacket_connectioncheck(bitreader);
			}
			else if (info.code == PACKETCODE::WELCOMETOSERVER)
			{
				ReadPacket_welcometoserver(bitreader);
			}
			else if (info.code == PACKETCODE::WELCOMETOLOBBY)
			{
				ReadPacket_welcometolobby(bitreader);
			}
			else if (info.code == PACKETCODE::LOBBY_DISBANDED)
			{
				ReadPacket_lobbydisbanded(bitreader);
			}
			else
			{
				std::cout << "unknown package received\n";
				continue;
			}

			//if code is already in queue pop it off and push new one on
			for (auto itr = packets_info_this_frame.begin(); itr != packets_info_this_frame.end(); itr++)
			{
				if (itr->code == info.code) {
					packets_info_this_frame.erase(itr);
					break;
				}
			}
			packets_info_this_frame.push_back(info);

			//each packet makes sure it ends on a byte, round to byte
			if (bitreader.getbithead() % 8 != 0)
			{
				int remainder = bitreader.getbithead() % 8;
				bitreader.setbithead(bitreader.getbithead() + (8 - remainder));
			}
		}

		if (bitreader.getbithead() != bits_received)
		{
			std::cout << "warning didn't seem to read all bits we received, are packets messed up?\n";
		}
	}
	
	return bytesReceived;
}

void ClientLobbyApp::handleLogic()
{
	if (global_data.connected_to_server)
	{
		int bytesReceived = ParseIncomingStream();
		if (handleUnexpectedDisconnect(bytesReceived) == -1)
			return;
	}

	if (global_data.menu_screen == MENU_TYPE::MAIN)
	{
		if (main_data.connecting_server)
		{
			if (!tcp_socket)
			{
				tcp_socket = std::make_shared<TCP_Socket>();
				tcp_socket->SetNonBlocking(true);
				main_data.timer_1.Begin();
			}
			if (main_data.timer_1.TimeElapsed(6000))
			{
				std::cout << "tcp connection never got accepted by server...\n";
				global_data.transition_main = true;
				tcp_socket.reset();
				return;
			}
			if ((connecttoServer() >= 0))
			{
				main_data.connecting_server = false;
				main_data.receiving_matchmakingdata = true;

				main_data.timer_2.Begin();

				global_data.connected_to_server = true;

				global_data.asking_connection = false;
				global_data.connection_validated = false;
			}
		}
		else if (main_data.receiving_matchmakingdata)
		{
			if (main_data.timer_2.TimeElapsed(6000))
			{
				std::cout << "never received welcometoserver packet to join matchmaking...\n";
				global_data.transition_main = true;
				global_data.gracefully_closing_connection = true;
				return;
			}
			int offset;
			if (offset = popandfindpacketdata(PACKETCODE::WELCOMETOSERVER), offset != -1)
			{
				global_data.transition_matchmaking = true;

				if (offset = popandfindpacketdata(PACKETCODE::MATCHMAKING), offset != -1)
				{
					bitreader.setbithead(offset);
					ReadPacket_matchmaking(bitreader, global_data.lobbies_available);
				}
			}
		}
	}
	else if (global_data.menu_screen == MENU_TYPE::MATCHMAKING)
	{
		int offset;
		if (matchmaking_data.creating_lobby)
		{
			if (matchmaking_data.timer_1.TimeElapsed(6000))
			{
				std::cout << "never got welcometolobby packet when creating lobby...\n";
				matchmaking_data.creating_lobby = false;
			}
			if (offset = popandfindpacketdata(PACKETCODE::WELCOMETOLOBBY), offset != -1)
			{
				std::cout << "server accepted our request, go into new created lobby\n";
				global_data.transition_lobby = true;
			
				if (offset = popandfindpacketdata(PACKETCODE::SERVERLOBBY), offset != -1)
				{
					bitreader.setbithead(offset);
					ReadPacket_serverlobby(bitreader, global_data.current_lobby);
				}
			}
		}
		else if (matchmaking_data.joining_lobby)
		{
			if (matchmaking_data.timer_2.TimeElapsed(6000))
			{
				std::cout << "never got welcometolobby packet when joining lobby...\n";
				matchmaking_data.joining_lobby = false;
			}
			if (offset = popandfindpacketdata(PACKETCODE::WELCOMETOLOBBY), offset != -1)
			{
				std::cout << "server accepted our request, go into new created lobby\n";
				global_data.transition_lobby = true;
				if (offset = popandfindpacketdata(PACKETCODE::SERVERLOBBY), offset != -1)
				{
					bitreader.setbithead(offset);
					ReadPacket_serverlobby(bitreader, global_data.current_lobby);
				}
			}
			else if (offset = popandfindpacketdata(PACKETCODE::LOBBYFAIL), offset != -1)
			{
				matchmaking_data.joining_lobby = false;

				bitreader.setbithead(offset);
				uint8_t error_code;
				ReadPacket_lobbyfail(bitreader, error_code);
				std::cout << "can't join lobby error code: " << (int)error_code << std::endl;
			}
		}
		else if (offset = popandfindpacketdata(PACKETCODE::MATCHMAKING), offset != -1)
		{
			bitreader.setbithead(offset);
			ReadPacket_matchmaking(bitreader, global_data.lobbies_available);
		}
	}
	else if (global_data.menu_screen == MENU_TYPE::LOBBY)
	{
		int offset;
		if (offset = popandfindpacketdata(PACKETCODE::LOBBY_DISBANDED), offset != -1)
		{
			//since we are receiving this message here the host of the lobby left, we are kicked out leave
			global_data.transition_matchmaking = true;

			if (offset = popandfindpacketdata(PACKETCODE::MATCHMAKING), offset != -1)
			{
				std::cout << "matchmaking \n";
				bitreader.setbithead(offset);
				ReadPacket_matchmaking(bitreader, global_data.lobbies_available);
			}
		}
		else if (offset = popandfindpacketdata(PACKETCODE::LAUNCHGAME), offset != -1)
		{
			//we are launching game!
			bitreader.setbithead(offset);
			ReadPacket_launchgame(bitreader, global_data.port_id);

			global_data.launch_game = true;
			
			global_data.should_leave = true;
		}
		else if (offset = popandfindpacketdata(PACKETCODE::SERVERLOBBY), offset != -1)
		{
			bitreader.setbithead(offset);
			ReadPacket_serverlobby(bitreader, global_data.current_lobby);
		}
	}
}

void ClientLobbyApp::handleRendering()
{
	graphics_manager.GetRenderRect(global_data.rect_1).SetVisibility(false);
	graphics_manager.GetRenderRect(global_data.rect_2).SetVisibility(false);
	graphics_manager.GetRenderRect(global_data.rect_3).SetVisibility(false);
	graphics_manager.GetRenderRect(global_data.rect_4).SetVisibility(false);

	glm::vec3 font0, font1, font2, font3;
	glm::vec3 selectcol(0, 0, 0);
	if (global_data.menu_screen == MENU_TYPE::MAIN)
	{
		switch (main_data.menu_option)
		{
		case OPTIONS_MAIN::JOIN:	font0 = selectcol;          font1 = glm::vec3(1, 1, 1); font2 = glm::vec3(1, 1, 1); break;
		case OPTIONS_MAIN::OPTIONS: font0 = glm::vec3(1, 1, 1); font1 = selectcol;          font2 = glm::vec3(1, 1, 1); break;
		case OPTIONS_MAIN::QUIT:	font0 = glm::vec3(1, 1, 1); font1 = glm::vec3(1, 1, 1); font2 = selectcol;			break;
		};
		graphics_manager.AddText("Ice Baneling Escape", -4.5, 2, .02, glm::vec3(0, 0, 1));//Ice Baneling Escape
		graphics_manager.AddText("Join Matchmaking", -2, 0, .01, font0);
		graphics_manager.AddText("Options", -2, -1, .01, font1);
		graphics_manager.AddText("Quit", -2, -2, .01, font2);
		graphics_manager.AddText("(c) Gibo Studios 2023", -1.5, -4.75, .005, glm::vec3(1,1,0));
		if (main_data.receiving_matchmakingdata)
			graphics_manager.AddText("recieving matchmaking data...", -2, -3, .005, font0);
		if (main_data.connecting_server)
			graphics_manager.AddText("connecting...", -2, -3, .005, font0);

		//name popup
		if (main_data.typing_name)
		{
			graphics_manager.GetRenderRect(global_data.rect_3).SetVisibility(true);
			graphics_manager.AddText("Enter Username", -2, -3, .01, glm::vec3(1, 1, 1));
			graphics_manager.AddText("Backspace", 2, -3, .0025, glm::vec3(1, 1, 1));
			std::string typing_msg;
			std::vector<int> keys_pressed = InputManager.peekrecordingTmp();
			for (int i = 0; i < keys_pressed.size(); i++)
			{
				if (keys_pressed[i] != GLFW_KEY_ENTER)
					typing_msg += keys_pressed[i];
			}

			graphics_manager.AddText("username: " + typing_msg, -3, -4, .005, glm::vec3(1, 1, 1));
			graphics_manager.AddText("Enter", 2, -4, .005, glm::vec3(1, 1, 1));
		}
	}
	else if (global_data.menu_screen == MENU_TYPE::MATCHMAKING)
	{
		switch (matchmaking_data.menu_option)
		{
		case OPTIONS_MATCHMAKING::JOIN:    font0 = selectcol;          font1 = glm::vec3(1, 1, 1); font2 = glm::vec3(1, 1, 1); font3 = glm::vec3(1, 1, 1); break;
		case OPTIONS_MATCHMAKING::CREATE:  font0 = glm::vec3(1, 1, 1); font1 = selectcol;          font2 = glm::vec3(1, 1, 1); font3 = glm::vec3(1, 1, 1); break;
		case OPTIONS_MATCHMAKING::REFRESH: font0 = glm::vec3(1, 1, 1); font1 = glm::vec3(1, 1, 1); font2 = selectcol;			font3 = glm::vec3(1, 1, 1); break;
		case OPTIONS_MATCHMAKING::BACK:    font0 = glm::vec3(1, 1, 1); font1 = glm::vec3(1, 1, 1); font2 = glm::vec3(1, 1, 1); font3 = selectcol;			break;
		};
		graphics_manager.GetRenderRect(global_data.rect_1).SetVisibility(true);
		graphics_manager.AddText("Match Making Lobbies", -4.0, 4, .015, glm::vec3(0, 0, 1));
		for (int i = 0; i < global_data.lobbies_available.size(); i++)
		{
			std::string info = (i == matchmaking_data.lobby_selection) ? "--->" : "";
			info += std::string("Lobby Name:  ") + global_data.lobbies_available[i].lobby_name +
				std::string("  Host Name:   ") + global_data.lobbies_available[i].lobby_host +
				std::string("  players:   ") + std::to_string((int)global_data.lobbies_available[i].player_count) + std::string("/6");
			graphics_manager.AddText(info, -4, 3.5 - i * .3, .004, glm::vec3(1, 1, 1));
		}
		graphics_manager.AddText("Join Lobby", -2, -1, .01, (matchmaking_data.picking_lobby) ? glm::vec3(1, 1, 0) : font0);
		graphics_manager.AddText("Create Lobby", -2, -2, .01, font1);
		graphics_manager.AddText(global_data.username, 3.5, -4.5, .005, glm::vec3(0, 0, 0));

		if (matchmaking_data.typing_lobbyname)
		{
			graphics_manager.GetRenderRect(global_data.rect_4).SetVisibility(true);
			graphics_manager.AddText("Enter Lobby Name", -2.5, -3, .01, glm::vec3(1, 1, 1));
			graphics_manager.AddText("Backspace", 2, -3, .0025, glm::vec3(1, 1, 1));
			std::string typing_msg;
			std::vector<int> keys_pressed = InputManager.peekrecordingTmp();
			for (int i = 0; i < keys_pressed.size(); i++)
			{
				if (keys_pressed[i] != GLFW_KEY_ENTER)
					typing_msg += keys_pressed[i];
			}

			graphics_manager.AddText("lobbyname: " + typing_msg, -3, -4, .005, glm::vec3(1, 1, 1));
			graphics_manager.AddText("Enter", 2, -4, .005, glm::vec3(1, 1, 1));
		}
		else
		{
			graphics_manager.AddText("Refresh", -2, -3, .01, font2);
			graphics_manager.AddText("Back", -2, -4, .01, font3);
			if (matchmaking_data.creating_lobby || matchmaking_data.joining_lobby)
				graphics_manager.AddText("connecting to lobby...", -2, -4.5, .005, glm::vec3(0, 0, 0));
		}
	}
	else if (global_data.menu_screen == MENU_TYPE::LOBBY)
	{
		graphics_manager.GetRenderRect(global_data.rect_2).SetVisibility(true);

		glm::vec3 font_0(0, 0, 0);
		glm::vec3 font_1(1, 1, 1);
		glm::vec3 font_2(1, 1, 0);

		//render chat
		graphics_manager.AddText("Chat", 1, 4, .01, font_0);
		graphics_manager.AddText("-------", 1, 3.7, .01, font_0);
		for (int i = 0; i < global_data.current_lobby.info.messages.size(); i++)
		{
			if (global_data.current_lobby.info.messages[i][0] != '\0')
				graphics_manager.AddText(global_data.current_lobby.info.messages[i], 0, 3.3 - .3 * i, .005, font_1);
		}

		//render players
		graphics_manager.AddText("Players", -4.6, 4, .01, font_0);
		graphics_manager.AddText("------------", -4.6, 3.7, .01, font_0);
		for (int i = 0; i < global_data.current_lobby.names.size(); i++)
		{
			std::string info = global_data.current_lobby.names[i];// +std::to_string(global_data.current_lobby.info.ready[i]);
			if (global_data.current_lobby.info.ready[i])
				info += " | ready";
			else
				info += " | not ready";

			graphics_manager.AddText(info, -4.6, 3.5 - .5 * i, .005, font_0);
		}

		//ready lobby info
		graphics_manager.AddText("Lobby Info", -4.6, -0.5, .01, font_0);
		graphics_manager.AddText("---------------", -4.6, -0.8, .01, font_0);
		graphics_manager.AddText(global_data.current_lobby.info.info.lobby_name, -4.6, -1, .005, font_0);
		graphics_manager.AddText(std::string("Host: ") + global_data.current_lobby.info.info.lobby_host, -4.6, -1.5, .005, font_0);
		graphics_manager.AddText(std::string("players: ") + std::to_string((int)global_data.current_lobby.info.info.player_count),
			-4.6, -2, .005, font_0);
		graphics_manager.AddText((global_data.current_lobby.info.openaccess) ? "open party" : "closed party", -4.6, -2.5, .005, font_0);

		//render input options
		graphics_manager.AddText("Leave", -4.6, -4, .01, (lobby_data.menu_option == OPTIONS_LOBBY::LEAVE) ? font_1 : font_0);
		std::string typing_msg;
		if (lobby_data.typing_msg) {
			std::vector<int> keys_pressed = InputManager.peekrecordingTmp();
			for (int i = 0; i < keys_pressed.size(); i++)
			{
				if (keys_pressed[i] != GLFW_KEY_ENTER)
					typing_msg += keys_pressed[i];
			}
		}
		graphics_manager.AddText("type: " + typing_msg, 0, -3, .005, (lobby_data.typing_msg) ? font_2 : (lobby_data.menu_option == OPTIONS_LOBBY::MSG) ? font_1 : font_0);
		graphics_manager.AddText("Ready", 3.3, -4, .01, (lobby_data.ready_up) ? font_2 : (lobby_data.menu_option == OPTIONS_LOBBY::READY) ? font_1 : font_0);
	}

	if(global_data.connected_to_server)
		graphics_manager.AddText("ping: " + std::to_string(global_data.ping).substr(0,4), 3.8, 4.5, .005, glm::vec3(0, 0, 0));
	if (global_data.gracefully_closing_connection)
	{
		graphics_manager.AddText("disconnecting...", -4.5, -4.5, .0025, glm::vec3(0, 0, 0));
	}

	graphics_manager.Render(cam_main);
	glfwSwapBuffers(graphics_manager.getWindow());
}

//PACKETS
void ClientLobbyApp::sendRefreshPacket()
{
	bitwriter.reset();
	WritePacket_refresh(bitwriter);
	int bytesSent = tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
}

void ClientLobbyApp::sendlobbyreadyPacket(bool isready)
{
	bitwriter.reset();
	WritePacket_lobbyready(bitwriter, isready);

	int sentBytes = tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
}

void ClientLobbyApp::sendlobbytypePacket(std::string_view msg)
{
	bitwriter.reset();
	WritePacket_lobbytype(bitwriter, msg);

	int sentBytes = tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
}

//CONNECTIONS
void ClientLobbyApp::sendleaveLobbyPacket()
{
	bitwriter.reset();
	WritePacket_lobbyleaving(bitwriter);

	int bytesSent = tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
}

void ClientLobbyApp::sendjoinLobbyPacket(int lobby_id)
{
	create_info info;
	info.creator = false;
	info.lobby_name = global_data.lobbies_available[lobby_id].lobby_host;
	bitwriter.reset();
	WritePacket_enterlobby(bitwriter, info);

	int bytesSent = tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
}

void ClientLobbyApp::sendCreateLobbyPacket()
{
	create_info info;
	info.creator = true;
	info.open_access = true;

	info.lobby_name = global_data.lobbyname;
	bitwriter.reset();
	WritePacket_enterlobby(bitwriter, info);

	int bytesSent = tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
}

//create TCP socket and try to connect to server, if success send a global_data.username packet
int ClientLobbyApp::connecttoServer()
{
	sockaddr addr_of_accept;
	uint16_t portid = config::ServerPort;	
	addr_of_accept = createSocketAddr_Mixed(config::ServerIP.c_str(), portid++);
	printSocketAddr(*reinterpret_cast<sockaddr_in*>(&addr_of_accept));
	tcp_socket->SetNonBlocking(false);
	//if server called listen() it will implicitly accept these without actually calling accept
	int result = tcp_socket->Connect_noerr(addr_of_accept);
	int err = WSAGetLastError();
	if (result != SOCKET_ERROR || err == WSAEISCONN)//|| err == WSAEISCONN
	{
		std::cout << "SERVER TCP ADDRESS"; printSocketAddr(*reinterpret_cast<sockaddr_in*>(&addr_of_accept));

		bitwriter.reset();
		WritePacket_username(bitwriter, global_data.username);
		int bytesSent = tcp_socket->Send((const char*)bitwriter.getData(), bitwriter.getBytesUsed());
		tcp_socket->SetNonBlocking(true);
		return 1;
	}
	else
	{
		if(err != WSAEWOULDBLOCK)
			std::cout << "error in connect: " << "  err: " << err << std::endl;
		tcp_socket->SetNonBlocking(true);
		return -1;
	}
}

void ClientLobbyApp::LaunchGame()
{
	//set up all the new connections

	//then shutdown this client and startup game
	//global_data.launch_game = true;
}
