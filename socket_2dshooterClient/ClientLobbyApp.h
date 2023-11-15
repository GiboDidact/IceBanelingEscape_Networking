#pragma once

#include "SharedSrc/GraphicsManager.h"
#include "SharedSrc/Sockets/SocketInc.h"
#include "SharedSrc/Sockets/BitStream.h"

const int MAX_READ_BUFFER_SIZE = 1000;

class ClientLobbyApp
{

public:
	ClientLobbyApp()
	{
		cam_main = nullptr;
	}
		
	int Run(int& port_id)
	{
		if (!initialize())
		{
			cleanUp();
			port_id = -1;
			return -1;
		}
		mainLoop();
		cleanUp();

		port_id = global_data.port_id;
		return global_data.launch_game;
	}

	Input_Manager& GetInputManager() { return InputManager; }
	GraphicsManager& GetGraphicsManager() { return graphics_manager; }

private:
	GraphicsManager graphics_manager;
	Input_Manager InputManager;
	Camera* cam_main;
	
	//sockets
	std::shared_ptr<TCP_Socket> tcp_socket;
	BitStreamWriter bitwriter;
	BitStreamReader bitreader;

	//state
	enum class MENU_TYPE : uint8_t { MAIN, MATCHMAKING, LOBBY };
	enum class OPTIONS_MAIN : int { JOIN, OPTIONS, QUIT };
	enum class OPTIONS_MATCHMAKING : int { JOIN, CREATE, REFRESH, BACK };
	enum class OPTIONS_LOBBY : int { LEAVE, MSG, READY };

	struct state_global
	{
		state_global() { init(); }
		void init()
		{
			launch_game = false;
			should_leave = false;
			transition_main = false;
			transition_matchmaking = false;
			transition_lobby = false;
			menu_screen = MENU_TYPE::MAIN;
			gracefully_closing_connection = false;
			asking_connection = false;
			connection_validated = false;
			ping = 0;
			connected_to_server = false;
			port_id = 0;
		}

		void mainmenureset()
		{
			asking_connection = false;
			connection_validated = false;
			connected_to_server = false;
			ping = 0;
		}

		std::string username;
		std::string lobbyname;
		std::vector<lobby_info> lobbies_available;
		lobby_fullinfo_client current_lobby;
		bool launch_game;
		uint32_t rect_1, rect_2, rect_3, rect_4;
		bool should_leave;
		int port_id;

		bool transition_main;
		bool transition_matchmaking;
		bool transition_lobby;

		//graceful disconnect
		bool gracefully_closing_connection;
		Timer closing_connection_timer;

		//unexpected disconnect
		Timer connect_check_timer;
		bool asking_connection;
		bool connection_validated;

		Timer ping_timer;
		float ping;

		bool connected_to_server;

		MENU_TYPE menu_screen;
	};

	struct state_main 
	{
		state_main() { init(); }
		void init()
		{
			menu_option = OPTIONS_MAIN::JOIN;
			typing_name = false;
			receiving_matchmakingdata = false;
			connecting_server = false;
		}

		bool isconnecting() { return (receiving_matchmakingdata || connecting_server); }

		OPTIONS_MAIN menu_option;
		bool typing_name;
		bool receiving_matchmakingdata;
		bool connecting_server;
		Timer timer_1;
		Timer timer_2;
	};

	struct state_matchmaking 
	{
		state_matchmaking() { init(); }
		void init() 
		{
			menu_option = OPTIONS_MATCHMAKING::JOIN;
			typing_lobbyname = false;
			picking_lobby = false;
			lobby_selection = -1;
			creating_lobby = false;
			joining_lobby = false;
		}

		bool connecting_to_lobby() { return (creating_lobby || joining_lobby); }

		OPTIONS_MATCHMAKING menu_option;
		bool typing_lobbyname;
		bool picking_lobby;
		int lobby_selection;
		bool creating_lobby;
		bool joining_lobby;
		Timer timer_1;
		Timer timer_2;
	};

	struct state_lobby 
	{
		state_lobby() { init(); }
		void init() 
		{
			menu_option = OPTIONS_LOBBY::MSG;
			typing_msg = false;
			ready_up = false;
		}

		OPTIONS_LOBBY menu_option;
		bool typing_msg;
		bool ready_up;
	};

	state_main main_data;
	state_matchmaking matchmaking_data;
	state_lobby lobby_data;
	state_global global_data;

	struct packet_data {
		PACKETCODE code;
		uint32_t bit_offset;
	};
	std::deque<packet_data> packets_info_this_frame;
	int popandfindpacketdata(PACKETCODE code);
	int findandremovepacketdata(PACKETCODE code);

private:
	int initialize();
	void cleanUp();
	void options();

	void mainLoop();
	void handleGracefulDisconnect();
	int handleUnexpectedDisconnect(int bytesreceived);
	void handleTransition();
	void handleInput();
	void handleLogic();
	void handleRendering();

	int ParseIncomingStream();

	void sendRefreshPacket();
	void sendlobbyreadyPacket(bool isready);
	void sendlobbytypePacket(std::string_view msg);
	void sendleaveLobbyPacket();
	void sendjoinLobbyPacket(int lobby_id);
	void sendCreateLobbyPacket();

	int connecttoServer();
	void LaunchGame();

private:

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		ClientLobbyApp* handler = reinterpret_cast<ClientLobbyApp*>(glfwGetWindowUserPointer(window));
		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		handler->GetGraphicsManager().WindowSizeChanged(w, h);
	}
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		reinterpret_cast<ClientLobbyApp*>(glfwGetWindowUserPointer(window))->GetInputManager().key_callback(window, key, scancode, action, mods);
	}
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		reinterpret_cast<ClientLobbyApp*>(glfwGetWindowUserPointer(window))->GetInputManager().cursor_position_callback(window, xpos, ypos);
	}
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		reinterpret_cast<ClientLobbyApp*>(glfwGetWindowUserPointer(window))->GetInputManager().mouse_button_callback(window, button, action, mods);
	}
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		reinterpret_cast<ClientLobbyApp*>(glfwGetWindowUserPointer(window))->GetInputManager().scroll_callback(window, xoffset, yoffset);
	}
};
