#pragma once

#include "SharedSrc/GraphicsManager.h"

//old standalone game, clientGameApp is main one that works with networking

class ClientGameAppold
{
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		ClientGameAppold* handler = reinterpret_cast<ClientGameAppold*>(glfwGetWindowUserPointer(window));
		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		handler->GetGraphicsManager().WindowSizeChanged(w, h);
	}
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		reinterpret_cast<ClientGameAppold*>(glfwGetWindowUserPointer(window))->GetInputManager().key_callback(window, key, scancode, action, mods);
	}
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		reinterpret_cast<ClientGameAppold*>(glfwGetWindowUserPointer(window))->GetInputManager().cursor_position_callback(window, xpos, ypos);
	}
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		reinterpret_cast<ClientGameAppold*>(glfwGetWindowUserPointer(window))->GetInputManager().mouse_button_callback(window, button, action, mods);
	}
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		reinterpret_cast<ClientGameAppold*>(glfwGetWindowUserPointer(window))->GetInputManager().scroll_callback(window, xoffset, yoffset);
	}

	enum class GROUNDTYPE : uint8_t { GRASS, ICE, REVERSE, FAST, EXIT };

	struct game_object 
	{
		game_object() { activated = false; }
		uint32_t render_id;
		bool activated;
	};

	struct global_info
	{
		global_info() { Init(); }
		void Init()
		{
			current_level = 1;

			mouse_left = false;
			mouse_right = false;
			camera_lock = false;
			msg_on = false;
			player_info_on = false;
		}

		uint32_t background_rectid;
		Timer game_time;
		int current_level;

		bool mouse_left;
		bool mouse_right;
		bool camera_lock;
		bool msg_on;
		bool player_info_on;
	};

	struct level1_info 
	{
		game_object button_1;
		game_object gate_1;
		glm::vec2 spawn_location;
	};

	//class
	struct player
	{
		void Init()
		{
			click_pos = glm::vec2(0, 0);
			moving_to_waypoint = false;
			speed = GRASS_SPEED;
			dir = glm::vec2(0, 0);
			rot_angle = 0.0;

			alive = true;
			current_ground = GROUNDTYPE::GRASS;
			ongoo = false;

			goo_available = true;
			goo_active = false;
			goo_timer = 0;

			disk_avaiable = true;
			disk_timer = 0;

			speed_available = true;
			speed_active = false;
			speed_timer = 0;

			ability_s = false;
			ability_mr = false;
			ability_e = false;
			ability_t = false;
			ability_f = false;
			ability_mx = 0;
			ability_my = 0;

			respawn_on = false;
		}

		uint32_t render_id;
		uint32_t respawn_rectid;
		Timer respawn_animation_timer;
		bool respawn_on;

		glm::vec2 click_pos;
		bool moving_to_waypoint;
		float speed;
		glm::vec2 dir;
		float rot_angle;

		bool alive;
		GROUNDTYPE current_ground;
		bool ongoo;
		
		bool goo_available;
		bool goo_active;
		float goo_timer;

		bool disk_avaiable;
		float disk_timer;
		
		bool speed_available;
		bool speed_active;
		float speed_timer;

		bool ability_s;
		bool ability_mr;
		bool ability_e;
		bool ability_t;
		bool ability_f;
		float ability_mx, ability_my;

		const float GRASS_SPEED = 0.005;
		const float ICE_SPEED = 0.01;
		const float FASTICE_SPEED = 0.02;
		const float RESPAWN_ANIMATION_TIME = 100;

		bool onIce()
		{
			return (current_ground == GROUNDTYPE::ICE || current_ground == GROUNDTYPE::REVERSE || current_ground == GROUNDTYPE::FAST);
		}
	};
	struct enemy 
	{
		uint32_t render_id;

		glm::vec2 speed;
		float distance;
		float distance_measure;
		bool dir;
	};

	struct ground 
	{
		uint32_t render_id;
		GROUNDTYPE type;
	};

	struct goo 
	{
		uint32_t render_id;
		float timer;
	};

	struct disk 
	{
		uint32_t render_id;
		float timer;
		glm::vec2 dir;
	};

	//class?
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
	};

	struct ui_msgdata 
	{
		uint32_t toggle_rectid;
		uint32_t background_rectid;
		//messages
	};

	struct ui_playerdata 
	{
		uint32_t toggle_rectid;
		uint32_t background_rectid;
		//players and ping
	};

public:
	ClientGameAppold();
	int Run();

	Input_Manager& GetInputManager() { return InputManager; }
	GraphicsManager& GetGraphicsManager() { return graphics_manager; }
private:
	GraphicsManager graphics_manager;
	Input_Manager InputManager;
	Camera* cam_main;
	bool launchlobby;

	//game state
	global_info global_state;
	level1_info level1_state;
	player main_player;
	
	std::vector<enemy> current_enemies;
	std::vector<ground> current_ground;
	std::vector<goo> goo_rects;
	std::vector<disk> disk_rects;

	//ui
	ui_bottomdata ui_bottom;
	ui_msgdata ui_msg;
	ui_playerdata ui_player;

	//const
	const uint32_t GOO_LIFETIME = 650;
	const uint32_t DISK_LIFETIME = 350;
	const float GOO_EXTRASPEED = 1.5;
	const float SPEED_EXTRASPEED = 1.5;
	const uint32_t GOOPOWER_ACTIVATION_TIME = 300;
	const uint32_t GOOPOWER_CD_TIME = 600;
	const uint32_t DISKPOWER_CD_TIME = 300;
	const uint32_t SPEEDPOWER_ACTIVATION_TIME = 300;
	const uint32_t SPEEDPOWER_CD_TIME = 600;
	const int GOOSPAWN_INTERVAL = 10;
	const glm::vec2 DISK_SPEED = glm::vec2(.02, .02);

	const float CAM_SPEED = .1;
	const float CAM_PADDING = .1;
	const float MAP_PAN_SIZE = 6;
	const float SCROLL_SPEED = 1;
	const float SCROLL_MIN_SCALE = .5;
	const float SCROLL_MAX_SCALE = 20;

private:
	int initializeGame();
	void initializeData();
	void mainLoopGame();
	void cleanUpGame();

	void SwitchLevels(int new_level, bool first = false);
	void LoadLevel1();
	void unLoadLevel1();
	void unLoadEnemyGround();

	void playerDie();
	void playerRevive();
	void playerAnimation();
	void playerInput();

	void Simulate(float deltaT);
	void simulate_enemies(float deltaT);
	void simulate_objects(float deltaT);
	void simulate_player(float deltaT);
	void Resolve();
	void resolve_enemies();
	void resolve_ground();
	void resolve_objects();

	void handlecamera(float deltaT);
	void handleUI();
	bool collidewithgoo();
	glm::vec2 mousetoworldpos(glm::vec2 mousepos);
	bool rect_collision_test(uint32_t render_id_a, uint32_t render_id_b);
	bool point_in_rect_collision_test(glm::vec2 pos, uint32_t render_id);
};