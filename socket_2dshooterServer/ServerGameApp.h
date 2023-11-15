#pragma once
#include "SharedSrc/Sockets/SocketInc.h"
#include "SharedSrc/Game/gameinclude.h"

class ServerGameApp 
{
	struct clientproxy_s
	{
		clientproxy_s()
		{
			ACK_SEND_TIME = 100;
			player_id = 100;
			connected = false;
			memset(&client_addr, 0, sizeof(sockaddr_in));
			ping = 0;
			ping_sent = false;
			switching_levels = false;
			welcoming = 0;
		}

		sockaddr_in client_addr;
		NetworkDeliverySender network_sender;
		NetworkDeliveryReciever network_reciever;
		Timer connection_timer;
		Timer ack_timer;
		float ACK_SEND_TIME; //group acks into less packets for reliability order
		bool connected;
		uint32_t player_id;

		bool switching_levels;
		int welcoming;
		
		float ping;
		Timer ping_timer;
		Timer ping_timer_send;
		bool ping_sent;

		shared::uiability_network_s uiability_network_data;
		shared::input_data_s input_data;
	};


	enum class GROUNDTYPE : uint8_t { GRASS, ICE, REVERSE, FAST, SLIDE, EXIT };

	struct ground
	{
		uint32_t render_id;
		GROUNDTYPE type;
	};

	struct game_object
	{
		game_object() { activated = false; }
		uint32_t render_id;
		bool activated;
		uint32_t gameobj_id;
	};

	struct enemy_strip_s 
	{
		glm::vec2 speed;
		float distance;
		float distance_measure;
	};
	struct enemy_s
	{
		enemy_s()
		{
			simple_patrol = false;
			elipse = false;
			randomness = false;
			strip_walking = false;
		}
		static enemy_s make_simple_patrol(uint32_t _render_id, glm::vec2 _speed, float _distance)
		{
			enemy_s data;
			data.simple_patrol = true;
			data.render_id = _render_id;
			data.speed = _speed;
			data.distance = _distance;
			data.distance_measure = 0.0;
			data.dir = true;

			return data;
		}
		
		static enemy_s make_walk_strip(uint32_t _render_id, std::vector<enemy_strip_s>& _walking_strips, bool _backwards_loop)
		{
			enemy_s data;
			data.render_id = _render_id;

			data.strip_walking = true;
			data.current_strip = 0;
			data.backwards_loop = _backwards_loop;
			data.walking_strips = _walking_strips;

			return data;
		}
		
		static enemy_s make_ellipse(uint32_t _render_id, glm::vec2 _speed, float _h, float _k, float _a, float _b, float _startangle = 0.0f, bool _dir = true)
		{
			enemy_s data;
			data.render_id = _render_id;
			data.speed = _speed;
			data.distance_measure = _startangle;
			data.dir = _dir;
			
			data.elipse = true;
			data.h = _h;
			data.k = _k;
			data.a = _a;
			data.b = _b;
			
			return data;
		}
		
		static enemy_s make_random(uint32_t _render_id, glm::vec2 _speed, float _distance, float _prob, uint32_t _rect_boundary)
		{
			enemy_s data;
			data.render_id = _render_id;
			data.dir = true;
			data.speed = _speed;
			data.distance = _distance;
			data.distance_measure = 0;

			data.randomness = true;
			data.prob = _prob;
			data.random_moving = false;
			data.rect_boundary = _rect_boundary;
			
			return data;
		}

		uint32_t render_id;

		//walk
		bool simple_patrol;
		glm::vec2 speed;
		float distance;
		float distance_measure;
		bool dir;

		//walk_strip
		bool strip_walking;
		int current_strip;
		bool backwards_loop;
		std::vector<enemy_strip_s> walking_strips;
	
		//ellipse
		bool elipse;
		float h, k, a, b;

		//random
		bool randomness;
		float prob;
		uint32_t rect_boundary;
		bool random_moving;
		glm::vec2 new_dir;
	};

	struct level1_info_s
	{
		std::vector<game_object> buttons;
		game_object gate_1;
		glm::vec2 spawn_location;
		uint32_t enemy_rect_boundary;
	};

	struct level2_info_s
	{
		glm::vec2 spawn_location;
		uint32_t enemy_rect_boundary_1;
		uint32_t enemy_rect_boundary_2;
	};

	struct level3_info_s
	{
		glm::vec2 spawn_location;
		uint32_t enemy_rect_boundary_1;
		uint32_t enemy_rect_boundary_2;
	};

	struct level4_info_s
	{
		glm::vec2 spawn_location;
	};

	struct bomb_s
	{
		uint32_t render_id;
		uint32_t gameobj_id;
		uint32_t current_frame;
		bool activated;
	};

	struct minion_s
	{
		uint32_t render_id;
		uint32_t gameobj_id;
		float distance_measure = 0.0;
		glm::vec2 new_dir;
		bool moving = false;
	};

	struct level5_info_s
	{
		void init()
		{
			boss_triggered = false;
			boss_running_stage_1 = false;
			boss_running_stage_2 = false;
			boss_running_stage_3 = false;
			boss_running_stage_4 = false;
			boss_running_stage_5 = false;

			stage2_count = 0;
			stage2_count2 = 0;
			stage3_count = 0;
			stage4_count = 0;
			minion_delay = 0;
			minion_stage1_count = 0;

			avilo_moving_done = false;
			avilo_chase = false;
			stage2_position = false;
			hammer_hit = false;
			bomb_counter = 0;
			kongfu_count = 0;
			fist_angle = 0;
			hammer_hit_count = 0;
			cop_on = false;
			hammer_created = false;
			atira_created = false;
		}

		glm::vec2 spawn_location;
		std::vector<game_object> buttons;
		std::vector<game_object> decorations;
		game_object gate_1;
		game_object gate_2;
		game_object end_button;

		//boss state logic
		bool boss_triggered = false;
		bool boss_running_stage_1 = false;
		bool boss_running_stage_2 = false;
		bool boss_running_stage_3 = false;
		bool boss_running_stage_4 = false;
		bool boss_running_stage_5 = false;

		std::vector<uint32_t> tanks;

		//avilo
		game_object avilo;
		glm::vec2 avilo_spot;
		bool avilo_moving_done = false;
		bool avilo_chase = false;
		int stage2_count = 0;
		bool stage2_position = false;
		int stage2_count2 = 0;
		int kongfu_count = 0;
		int stage3_count = 0;
		int stage4_count = 0;

		//dynamic objects
		game_object atira; 
		bool atira_created = false;
		std::vector<game_object> fists;
		float fist_angle = 0;
		std::vector<game_object> kongfus;
		game_object button_hammer;
		bool hammer_created = false;
		bool hammer_hit = false;
		int hammer_hit_count = 0;

		std::vector<minion_s> minions_stage1;
		int minion_stage1_count;
		std::vector<minion_s> minions;
		int minion_delay = 0;
		uint32_t minion_rect_boundary;

		//ending
		game_object cop_car;
		bool cop_on = false;

		//bombs
		uint64_t bomb_counter = 0;
		std::vector<bomb_s> bombs;
	};

	struct level6_info_s
	{
		glm::vec2 spawn_location;
		std::vector<game_object> decorations;
		int button_1_count = 0;
		int button_2_count = 0;
	};

	struct goo
	{
		uint32_t render_id;
		uint32_t gameobj_id;
		float timer;
	};

	struct disk
	{
		uint32_t render_id;
		uint32_t gameobj_id;
		float timer;
		glm::vec2 dir;
	};

	struct gameobj_idhandler_s
	{
		gameobj_idhandler_s()
		{
			current_index = shared::dynamic_game_object_start_id;
		}
		uint32_t getAvailableId()
		{
			return current_index++;
		}
		void freeid(uint32_t id)
		{
		}

		uint32_t current_index;
	};
	struct player
	{
		void Init()
		{
			//player_id = 101;
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
		}

		uint32_t player_id;
		uint32_t render_id;

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

		const float GRASS_SPEED = 0.02;
		const float ICE_SPEED = 0.05; //0.04
		const float FASTICE_SPEED = 0.1;

		bool onIce()
		{
			return (current_ground == GROUNDTYPE::ICE || current_ground == GROUNDTYPE::REVERSE || current_ground == GROUNDTYPE::FAST
				|| current_ground == GROUNDTYPE::SLIDE);
		}
	};
public:
	ServerGameApp(int _player_count);
	void Run();

	bool StartUp(int& port_id);
	bool loop();
private:
	//network 
	std::shared_ptr<UDP_Socket> server_socket;
	std::vector<clientproxy_s> client_proxys;

	BitStreamReader bitreader;
	BitStreamWriter bitwriter;

	std::vector<shared::gameobject_network_s> gameobject_network_requests;
	std::vector<shared::gameobject_network_s> gameobject_network_requests_garuntee;
	std::vector<shared::SOUNDS> audio_network_requests;

	//global
	GraphicsManager graphics_manager; //don't need to initialize, just using for the internal struct functions
	bool leave;
	int player_count;
	bool loading_game;
	bool restarting_game;
	Timer restarting_game_timer;
	int nextup_level;
	int CONNECTION_TIMEOUT;
	int current_level;
	int death_sound_track;
	float spawn_angle;
	std::queue<std::string> chat_msgs;
	Timer launch_delay_timer;
	std::random_device rd;

	//level switch logic
	bool level_switching;
	int players_switched;
	bool force_abilityui_packet;
	uint32_t level_switch_uid;

	//game simulation
	std::vector<ground> current_ground;
	std::vector<enemy_s> current_enemies;
	std::vector<goo> current_goo;
	std::vector<disk> current_disks;
	std::map<uint8_t, player> current_players;

	gameobj_idhandler_s gameobj_idhandler;
	level1_info_s level1_state;
	level2_info_s level2_state;
	level3_info_s level3_state;
	level4_info_s level4_state;
	level5_info_s level5_state;
	level6_info_s level6_state;

	//framerate time
	double MS_PER_UPDATE;
	double MS_MAX_TIME;
	double lag;
	Timer::time_point current_time;
	Timer load_wait_timer;

	//const
	const uint32_t GOO_LIFETIME = 500;
	const uint32_t DISK_LIFETIME = 125;
	const float GOO_EXTRASPEED = 1.5;
	const float SPEED_EXTRASPEED = 1.5;
	const uint32_t GOOPOWER_ACTIVATION_TIME = 150;
	const uint32_t GOOPOWER_CD_TIME = 600;
	const uint32_t DISKPOWER_CD_TIME = 300;
	const uint32_t SPEEDPOWER_ACTIVATION_TIME = 200;
	const uint32_t SPEEDPOWER_CD_TIME = 600;
	const int GOOSPAWN_INTERVAL = 3;
	const glm::vec2 DISK_SPEED = glm::vec2(.06, .06);
private:
	int Init();
	bool mainloop();
	void cleanup();

	void ParseIncomingStream();
	void launchGame();
	void sendLaunchData();

	void KickClient(int client_id);
	int findClientwithAddr(sockaddr_in addr);

	void handleHelloPacket(int client, sockaddr_in addr);
	void handleWelcomePacket(int client, sockaddr_in addr);
	void handleInputPacket(int client, sockaddr_in addr);
	void handleAcknowledgePacket(int client);
	void handlelevelloadpacket(int client);
	void handlechatpacket(int client);
	void handlepingpacket(int client);

	void LoadLevel1Enemies();
	void LoadLevel1Ground();
	void LoadLevel1GameObjects();

	void LoadLevel2Enemies();
	void LoadLevel2Ground();
	void LoadLevel2GameObjects();

	void LoadLevel3Enemies();
	void LoadLevel3Ground();
	void LoadLevel3GameObjects();

	void LoadLevel4Enemies();
	void LoadLevel4Ground();
	void LoadLevel4GameObjects();

	void LoadLevel5Enemies();
	void LoadLevel5Ground();
	void LoadLevel5GameObjects();

	void LoadLevel6Enemies();
	void LoadLevel6Ground();
	void LoadLevel6GameObjects();

	void Spawnplayer(player& main_player);

	void LoadLevel1();
	void UnLoadLevel1();
	void LoadLevel2();
	void UnLoadLevel2();
	void LoadLevel3();
	void UnLoadLevel3();
	void LoadLevel4();
	void UnLoadLevel4();
	void LoadLevel5();
	void UnLoadLevel5();
	void LoadLevel6();
	void UnLoadLevel6();
	void UnLoadGeneral();
	void SwitchLevels(int new_level);
	void LevelSwitchDone();

	void Simulate(float deltaT);
	void simulate_enemies(float deltaT);
	void simulate_player(float deltaT, player& main_player);
	void simulate_gameobjects(float deltaT);
	void simulate_lvl5boss(float deltaT);
	void SendSimulationPackets();

	void handlePlayerAnimation();

	void Resolve();
	void resolve_enemies(player& main_player);
	void resolve_ground(player& main_player);
	void resolve_gameobjects(player& main_player);
	void resolve_gameobjectslvl1(player& main_player);
	void resolve_gameobjectslvl2(player& main_player);
	void resolve_gameobjectslvl3(player& main_player);
	void resolve_gameobjectslvl4(player& main_player);
	void resolve_gameobjectslvl5(player& main_player);
	void resolve_gameobjectslvl5boss(player& main_player);
	void resolve_gameobjectslvl6(player& main_player);
	void resolve_players(player& main_player);
	
	void playerDie(player& main_player);
	void playerRevive(player& main_player);
};