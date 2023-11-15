#pragma once
#include "SharedSrc/GraphicsManager.h"
#include "SharedSrc/Sockets/SocketInc.h"
#include "SharedSrc/Game/gameinclude.h"

class ClientGameApp
{
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		ClientGameApp* handler = reinterpret_cast<ClientGameApp*>(glfwGetWindowUserPointer(window));
		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		handler->GetGraphicsManager().WindowSizeChanged(w, h);
	}
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		reinterpret_cast<ClientGameApp*>(glfwGetWindowUserPointer(window))->GetInputManager().key_callback(window, key, scancode, action, mods);
	}
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		reinterpret_cast<ClientGameApp*>(glfwGetWindowUserPointer(window))->GetInputManager().cursor_position_callback(window, xpos, ypos);
	}
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		reinterpret_cast<ClientGameApp*>(glfwGetWindowUserPointer(window))->GetInputManager().mouse_button_callback(window, button, action, mods);
	}
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		reinterpret_cast<ClientGameApp*>(glfwGetWindowUserPointer(window))->GetInputManager().scroll_callback(window, xoffset, yoffset);
	}

	struct global_data_s {

		global_data_s()
		{
			mouse_left = false;
			mouse_right = false;
			camera_lock = false;
			msg_on = false;
			player_info_on = false;
			typing = false;
			level_switching = false;
			current_level = 1;
		}

		bool mouse_left;
		bool mouse_right;
		bool camera_lock;
		bool msg_on;
		bool player_info_on;

		int current_level;
		Timer timer;

		Timer input_timer;
		shared::input_data_s input_data;

		bool typing;

		Timer level_switching_timer;
		bool level_switching;

		Timer game_time;
	};

	struct ui_bottomdata
	{
		uint32_t background_rectid;
		uint32_t speed_rectid;
		uint32_t speed_cd_rectid;
		uint32_t goo_rectid;
		uint32_t goo_cd_rectid;
		uint32_t disk_rectid;
		uint32_t disk_cd_rectid;

		uint32_t halt_rectid;
		uint32_t lock_rectid;

		uint32_t icon_rectid;
		uint32_t timer_rectid;

		uint32_t topbar_rectid;
		uint32_t chatback_rectid;

		uint32_t highlatency_rectid;
		uint32_t latencyvariation_rectid;
		uint32_t packetloss_rectid;

		bool highlatency_on;
		bool latencyvariation_on;
		bool packetloss_on;
		Timer latency_timer;
		Timer variation_timer;
		Timer packetloss_timer;
	};

	struct ui_msgdata
	{
		uint32_t toggle_rectid;
		uint32_t background_rectid;
		uint32_t exit_rectid;
		//messages
	};

	struct ui_playerdata
	{
		uint32_t toggle_rectid;
		uint32_t background_rectid;
		//players and ping
	};

public:
	ClientGameApp(int _port_id);
	int Run();

	Input_Manager& GetInputManager() { return InputManager; }
	GraphicsManager& GetGraphicsManager() { return graphics_manager; }
private:
	GraphicsManager graphics_manager;
	Input_Manager InputManager;
	Camera* cam_main;
	Timer connection_timer;
	int CONNECTION_TIMEOUT;
	bool connected;

	BitStreamReader bitreader;
	BitStreamWriter bitwriter;
	NetworkDeliverySender network_sender;
	NetworkDeliveryReciever network_reciever;
	sockaddr server_addr;
	uint16_t portid;
	std::shared_ptr<UDP_Socket> clientsocket;

	Timer acktimer;
	const float ACK_SEND_TIME = 100; //group acks into less packets so out of order happens less (though packet loss has more of an impact)
	uint32_t nextexpected_levelswitch_uid;

	//game data
	enum class LOADSTATE : uint8_t { SAYINGHI, WAITINGLAUNCH, LAUNCHED  };
	LOADSTATE load_state;
	global_data_s global_state;
	uint32_t load_screen_rectid;
	uint32_t background_rectid;
	std::vector<uint32_t> current_ground;
	std::vector<uint32_t> current_enemies;
	std::map<uint32_t, uint32_t>  current_level_gameobjects; //gameobj_id --> gameobj_rect
	std::map<uint32_t, bool>  current_level_gameobjects_destroyed;

	uint8_t owned_player_id;
	std::map<uint8_t, uint32_t> current_players; //player_id --> player_rect

	//ui
	float current_ping;
	std::vector<std::string> chat_msgs;
	shared::uiability_network_s uiability_network_data;
	ui_bottomdata ui_bottom;
	ui_msgdata ui_msg;
	ui_playerdata ui_player;

	const float CAM_SPEED = .01;
	const float CAM_PADDING = .1;
	const float MAP_PAN_SIZE_DEFAULT = 16;
	const float MAP_PAN_SIZE_lvl5 = 100;
	float MAP_PAN_SIZE;
	const float SCROLL_SPEED = .25;
	const float SCROLL_MIN_PERCENT = .1;
	const float SCROLL_MAX_PERCENT = 4;
	float current_zoom = 1.0f;

	const float INPUT_SEND_TIME = 100;

private:
	int initializeGame();
	void mainLoopGame();
	void cleanUpGame();

	void initializeData();

	void ParseIncomingStream();

	void handleWelcomepacket(sockaddr_in addr);
	void handleLaunchpacket(sockaddr_in addr);
	void handleacknowledgepacket();
	void handleenemypacket();
	void handleplayer_uniquepacket();
	void handleplayer_eventpacket();
	void handleplayer_dyanmicpacket();
	void handlegameobjectpacket();
	void handleuiabilitypacket();
	void handleaudiopacket();
	void handlelevelloadpacket();
	void handleserverchatpacket();
	void handlepingpacket();

	void handleInput();
	void handlecamera(float deltaT);
	void handleUI();

	void sendHelloPacket();

	glm::vec2 mousetoworldpos(glm::vec2 mousepos);

	void LoadLevel1();
	void LoadLevel2();
	void LoadLevel3();
	void LoadLevel4();
	void LoadLevel5();
	void LoadLevel6();
	void UnLoadGeneral();
	void SwitchLevels(int new_level);

};

