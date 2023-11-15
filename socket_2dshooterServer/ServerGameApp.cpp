#include "pchserver.h"
#include "ServerGameApp.h"

ServerGameApp::ServerGameApp(int _player_count)
{
	config::LoadConfigValues();
	//network
	client_proxys.reserve(player_count);

	//global
	player_count = _player_count;
	current_level = 0;
	leave = false;
    restarting_game = false;
	nextup_level = 1;

	loading_game = true;
	CONNECTION_TIMEOUT = 7000;
	death_sound_track = 0;
	spawn_angle = 0;
	
	//level switching logic
	level_switching = false;
	players_switched = 0;
	force_abilityui_packet = false;
	level_switch_uid = 0;

	//game simulation
	
	//framerate
	//simulation tick is 1/60th second. Should apply for simulation and message sending
	MS_PER_UPDATE = 16.667;
	//This is the max time in milliseconds that we will simulate per frame. If computer is super slow and reaches this, it will go out of sync.
	MS_MAX_TIME = 1000.0;
	lag = 0.0f;
	current_time = std::chrono::high_resolution_clock::now();
}

void ServerGameApp::Run()
{
	std::cout << "server running\n";
	if (Init() != -1)
	{
		while (!mainloop());
	}
	cleanup();
	std::cout << "server closing down\n";
}

bool ServerGameApp::StartUp(int& port_id)
{
	std::cout << "server running\n";
	port_id = Init();
	if (port_id == -1)
	{
		cleanup();
		return false;
	}
	return true;
}
bool ServerGameApp::loop()
{
	if (mainloop())
	{
		cleanup();
		return false;
	}
	return true;
}

int ServerGameApp::Init()
{
	if (config::STANDALONE_GAME)
	{
		std::cout << "standlone\n";
		if (StartWSA() == -1)
			return -1;
	}

	char name[100];
	gethostname(name, 100);
	std::cout << "hostname: " << name << std::endl;

	bitreader.Allocate(1000);

	server_socket = std::make_shared<UDP_Socket>();

	uint16_t portid = config::ServerPort;
	int tries = 1;
	while (tries <= 50)
	{
		std::cout << "UDP server connecting try... " << tries << std::endl;
		sockaddr listen_addr = createSocketAddr_Mixed(config::ServerIP.c_str(), portid++);
		if (server_socket->Bind(listen_addr) != SOCKET_ERROR) {
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

	server_socket->SetNonBlocking(true);

	std::cout << "SERVER UDP ADDRESS: ";
	printSockName(server_socket->getSocket());
	printMaxUDPSize(server_socket->getSocket());

	load_wait_timer.Begin();

	return (portid - 1);
}

bool ServerGameApp::mainloop()
{
#ifdef PING_EMULATOR
	PingEmulatorUDP::client = true;
	PingEmulatorUDP::update();
#endif
	if (config::SERVER_LEVEL_SWITCH)
	{
		if (GetKeyState('1') & 0x8000)
		{
			SwitchLevels(1);
		}
		else if (GetKeyState('2') & 0x8000)
		{
			SwitchLevels(2);
		}
		else if (GetKeyState('3') & 0x8000)
		{
			SwitchLevels(3);
		}
		else if (GetKeyState('4') & 0x8000)
		{
			SwitchLevels(4);
		}
		else if (GetKeyState('5') & 0x8000)
		{
			SwitchLevels(5);
		}
		else if (GetKeyState('6') & 0x8000)
		{
			SwitchLevels(6);
		}

		//high object count test
		/*static bool zonce = false;
		static uint32_t starting_id = 0;
		if (GetKeyState('7') & 0x8000)
		{
			if (!zonce)
			{
				for (int i = 0; i < 500; i++)
				{
					shared::gameobject_network_s obj_data;
					obj_data.game_id = gameobj_idhandler.getAvailableId();
					if (!zonce)
					{
						starting_id = obj_data.game_id;
						zonce = true;
					}
					obj_data.operation = shared::OBJECTOPERATION::CREATE;
					obj_data.position = glm::vec3(i * .01, i * .01, 0);
					obj_data.rotation_angle = 0.0;
					obj_data.texture_id = 15;
					obj_data.layer = 2;
					obj_data.visible = true;
					obj_data.scale = glm::vec2(.5, .5);
					gameobject_network_requests_garuntee.push_back(obj_data);
				}
			}
		}
		if (GetKeyState('8') & 0x8000)
		{
			if (zonce)
			{
				zonce = false;
				for (int i = 0; i < 500; i++)
				{
					shared::gameobject_network_s obj_data;
					obj_data.game_id = starting_id + i;
					obj_data.operation = shared::OBJECTOPERATION::DESTROY;
					gameobject_network_requests_garuntee.push_back(obj_data);
				}
			}
		}
		static bool eight_once = false;
		if (GetKeyState('9') & 0x8000)
		{
			static float move_extra = 0;
			uint32_t starting_id2 = starting_id;
			if (!eight_once)
			{
				eight_once = true;
				for (int i = 0; i < 500; i++)
				{
					shared::gameobject_network_s obj_data;
					obj_data.game_id = starting_id2 + i;
					obj_data.operation = shared::OBJECTOPERATION::MOVE;
					obj_data.position = glm::vec3(i * .01 + move_extra, i * .01, 0);
					move_extra += .001;
					gameobject_network_requests.push_back(obj_data);
				}
			}
		}
		if (GetKeyState('0') & 0x8000)
		{
			eight_once = false;
		}
		*/
	}
	//p for pizza
	static bool hit_p = false;
	if (!hit_p && GetKeyState('P') & 0x8000)
	{
		hit_p = true;
		for (auto& _player : current_players)
		{
			graphics_manager.GetRenderRect(_player.second.render_id).SetTexture(53);
		}
	}
	if (hit_p && GetKeyState('O') & 0x8000)
	{
		hit_p = false;
		for (auto& _player : current_players)
		{
			graphics_manager.GetRenderRect(_player.second.render_id).SetTexture(4);
		}
	}


	if (loading_game && launch_delay_timer.TimeElapsed(2000))
	{
		loading_game = false;
	}
	if (restarting_game && restarting_game_timer.TimeElapsed(2000))
	{
		restarting_game = false;
		current_time = std::chrono::high_resolution_clock::now();
		SwitchLevels(nextup_level);
	}
	if (loading_game || level_switching)
		current_time = std::chrono::high_resolution_clock::now();

	if (loading_game && load_wait_timer.TimeElapsed(15000))
	{
		std::cout << "waited 15 seconds and still havn't loading leaving...\n";
		leave = true;
	}
	if (!loading_game && client_proxys.empty())
	{
		std::cout << "no more connected players leaving\n";
		leave = true;
	}
	//std::cout << graphics_manager.GetRenderRectCount() << std::endl;
	ParseIncomingStream();
	if (!loading_game && !level_switching)
	{
		for (int i = 0; i < client_proxys.size(); i++)
		{
			if (!client_proxys[i].ping_sent)
			{
				client_proxys[i].ping_timer.Begin();
				client_proxys[i].ping_timer_send.Begin();
				client_proxys[i].ping_sent = true;
			}
			if (client_proxys[i].connected && client_proxys[i].ping_timer_send.TimeElapsed(100))
			{	
				client_proxys[i].ping_timer_send.Begin();
				bitwriter.reset();
				client_proxys[i].network_sender.labelpacketfake(bitwriter);
				WriteGamePacket_ping(bitwriter, Timer::getTimeSinceEpoch(client_proxys[i].ping_timer.getCurrentTime()), client_proxys[i].ping);
				server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));
			}
		}

		auto new_time = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(current_time).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(new_time).time_since_epoch().count();
		double frameTime = end - start;
		//microseconds to milliseconds
		frameTime /= 1000.0;
		if (frameTime > MS_MAX_TIME) {
			frameTime = MS_MAX_TIME;
			std::cout << "Warning: max time reached. processing " << std::to_string(int(std::round(MS_MAX_TIME / 10.0))) << " update frames\n";
		}
		current_time = new_time;
		lag += frameTime;

		float deltaT = 1.0f; //milliseconds
		bool simulated = false;
		while (lag >= MS_PER_UPDATE)
		{
			simulated = true;
			Simulate(deltaT);
			Resolve();
			
			lag -= MS_PER_UPDATE;
		}

		if (simulated)
		{
			handlePlayerAnimation();
			SendSimulationPackets();

			if (!restarting_game)
			{
				bool any_alive = false;
				for (auto itr = current_players.begin(); itr != current_players.end(); itr++)
				{
					if (itr->second.current_ground == GROUNDTYPE::EXIT) {
						nextup_level = current_level + 1;
						audio_network_requests.push_back(shared::SOUNDS::REVIVE);
						restarting_game = true;
						restarting_game_timer.Begin();
					}
					if (itr->second.alive) {
						any_alive = true;
					}
				}
				if (!any_alive) {
					restarting_game = true;
					restarting_game_timer.Begin();
					nextup_level = 1;

					if (config::INFINITE_LIVES_MODE || current_level == 6)
					{
						nextup_level = current_level;
					}
				}
			}
		}
	}

	return leave;
}

void ServerGameApp::SendSimulationPackets()
{
	//getMaxUDPSize(server_socket->getSocket()
	//every 80 enemies create a new packet so we don't go over size
	int MAX_ENEMY_PER_PACKET = 80;
	std::vector<std::vector<glm::vec3>> enemy_pos_datas;
	std::vector<uint32_t> enemy_pos_starting;
	for (int i = 0; i < current_enemies.size(); i++)
	{
		if (i % MAX_ENEMY_PER_PACKET == 0)
		{
			enemy_pos_datas.resize(enemy_pos_datas.size() + 1);
			enemy_pos_datas.reserve(glm::min(MAX_ENEMY_PER_PACKET, (int)(current_enemies.size() - i)));
			enemy_pos_starting.push_back(i);
		}
		enemy_pos_datas.back().push_back(glm::vec3(graphics_manager.GetRenderRect(current_enemies[i].render_id).getPos(), (float)current_enemies[i].dir));
	}

	std::vector<shared::player_network_data_dynamic_s> player_dynamic_info;
	for (const auto& player : current_players)
	{
		shared::player_network_data_dynamic_s info;
		info.player_id = player.second.player_id;
		info.pos = graphics_manager.GetRenderRect(player.second.render_id).getPos();
		info.angle = graphics_manager.GetRenderRect(player.second.render_id).getangle();
		info.reflect = graphics_manager.GetRenderRect(player.second.render_id).getreflected();
		info.texture_id = graphics_manager.GetRenderRect(player.second.render_id).gettextureid();
		player_dynamic_info.push_back(info);
	}

	//garuntee
	int MAX_GAMEOBJECTPERGARUNTEE_PACKET = 30;
	std::vector<std::vector<shared::gameobject_network_s>> garuntees;
	for (int a = 0; a < gameobject_network_requests_garuntee.size(); a++)
	{
		if (a % MAX_GAMEOBJECTPERGARUNTEE_PACKET == 0)
		{
			garuntees.resize(garuntees.size() + 1);
			garuntees.reserve(glm::min(MAX_GAMEOBJECTPERGARUNTEE_PACKET, (int)(gameobject_network_requests_garuntee.size() - a)));
		}
		garuntees.back().push_back(gameobject_network_requests_garuntee[a]);
	}
	//if (garuntees.size() > 1)
		//std::cout << "garuntees: " << garuntees.size() << std::endl;

	//nongaruntee
	int MAX_GAMEOBJECTPER_PACKET = 70;
	std::vector<std::vector<shared::gameobject_network_s>> gameobject_requests;
	for (int a = 0; a < gameobject_network_requests.size(); a++)
	{
		if (a % MAX_GAMEOBJECTPER_PACKET == 0)
		{
			gameobject_requests.resize(gameobject_requests.size() + 1);
			gameobject_requests.reserve(glm::min(MAX_GAMEOBJECTPER_PACKET, (int)(gameobject_network_requests.size() - a)));
		}
		gameobject_requests.back().push_back(gameobject_network_requests[a]);
	}
//	if (gameobject_requests.size() > 1)
	//	std::cout << "gameobject non-garuntee: " << gameobject_requests.size() << std::endl;

	for (int i = 0; i < client_proxys.size(); i++)
	{
		if (client_proxys[i].connected)
		{
			//send enemy positions
			int enemy_cccc = 0;
			for (auto& enemy_pos : enemy_pos_datas)
			{
				bitwriter.reset();
				client_proxys[i].network_sender.labelpacket(bitwriter);
				WriteGamePacket_enemy(bitwriter, enemy_pos, enemy_pos_starting[enemy_cccc++]);
				int bytes = server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));
				//std::cout << "enemy packet size: " << bytes << std::endl;
			}
			
			//send player positions and rotation
			bitwriter.reset();
			client_proxys[i].network_sender.labelpacket(bitwriter);
			WriteGamePacket_playersdynamic(bitwriter, player_dynamic_info);
			server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));


			//gameobject garuntee
			for (int a = 0; a < garuntees.size(); a++)
			{
				bitwriter.reset();
				client_proxys[i].network_sender.labelpacketGarunteeBegin(bitwriter);
				WriteGamePacket_gameobject(bitwriter, garuntees[a]);
				server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));
				client_proxys[i].network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));
			}
	
			//gameobject non-garuntee
			for (int a = 0; a < gameobject_requests.size(); a++)
			{
				bitwriter.reset();
				client_proxys[i].network_sender.labelpacket(bitwriter);
				WriteGamePacket_gameobject(bitwriter, gameobject_requests[a]);
				int bytes = server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));
			}

			//send abilitycd (calculate per client)
			if (current_players.count(client_proxys[i].player_id) != 0)
			{
				player& main_player = current_players[client_proxys[i].player_id];
				if (main_player.speed_available)
					client_proxys[i].uiability_network_data.speedup_percent = 0.0f;
				else
					client_proxys[i].uiability_network_data.speedup_percent = 1.0 - (main_player.speed_timer / SPEEDPOWER_CD_TIME);

				if (main_player.disk_avaiable)
					client_proxys[i].uiability_network_data.disk_percent = 0.0f;
				else
					client_proxys[i].uiability_network_data.disk_percent = 1.0 - (main_player.disk_timer / DISKPOWER_CD_TIME);

				if (main_player.goo_available)
					client_proxys[i].uiability_network_data.goo_percent = 0.0f;
				else
					client_proxys[i].uiability_network_data.goo_percent = 1.0 - (main_player.goo_timer / GOOPOWER_CD_TIME);

				if (client_proxys[i].uiability_network_data.anyActive() || force_abilityui_packet)
				{
					bitwriter.reset();
					client_proxys[i].network_sender.labelpacket(bitwriter);
					WriteGamePacket_uiability(bitwriter, client_proxys[i].uiability_network_data);
					server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));
				}
			}


			//send Audio
			if (!audio_network_requests.empty())
			{
				bitwriter.reset();
				client_proxys[i].network_sender.labelpacketGarunteeBegin(bitwriter);
				WriteGamePacket_audio(bitwriter, audio_network_requests);
				server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));
				client_proxys[i].network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));
			}

		}
	}
	force_abilityui_packet = false;
	audio_network_requests.clear();
	gameobject_network_requests_garuntee.clear();
	gameobject_network_requests.clear();
}

void ServerGameApp::handlePlayerAnimation()
{
	for (auto itr = current_players.begin(); itr != current_players.end(); itr++)
	{
		player& main_player = itr->second;

		//convert normalized 2d vector to angle (0-360)
		float angle = asin(main_player.dir.y);
		if (isnan(angle))
			angle = 0;

		graphics_manager.GetRenderRect(main_player.render_id).Rotate(glm::vec3(0, 0, 1), glm::degrees(angle));
		main_player.rot_angle = angle;
		if (main_player.dir.x < 0)
			graphics_manager.GetRenderRect(main_player.render_id).Reflect(true);
		else
			graphics_manager.GetRenderRect(main_player.render_id).Reflect(false);

		if (main_player.current_ground == GROUNDTYPE::REVERSE)
		{
			graphics_manager.GetRenderRect(main_player.render_id).Reflect(!graphics_manager.GetRenderRect(main_player.render_id).getreflected());
			
			graphics_manager.GetRenderRect(main_player.render_id).Rotate(glm::vec3(0, 0, 1), glm::degrees(-angle));
			main_player.rot_angle = -angle;
		}
	}
}

void ServerGameApp::Simulate(float deltaT)
{
	//todo: everything has to take into account all players
	simulate_enemies(deltaT);
	simulate_gameobjects(deltaT);
	for (auto itr = current_players.begin(); itr != current_players.end(); itr++)
	{
		simulate_player(deltaT, itr->second);
	}
}

void ServerGameApp::simulate_gameobjects(float deltaT)
{
	//simulate goo
	for (int i = 0; i < current_goo.size(); i++)
	{
		current_goo[i].timer += deltaT;
		if (current_goo[i].timer >= GOO_LIFETIME) {
			graphics_manager.RemoveRenderRect(current_goo[i].render_id);

			shared::gameobject_network_s obj_data;
			obj_data.game_id = current_goo[i].gameobj_id;
			obj_data.operation = shared::OBJECTOPERATION::DESTROY;
			gameobject_network_requests_garuntee.push_back(obj_data);

			gameobj_idhandler.freeid(current_goo[i].gameobj_id);
			current_goo.erase(current_goo.begin() + i);
			i--;
		}
	}

	//simulate disk
	for (int i = 0; i < current_disks.size(); i++)
	{
		current_disks[i].timer += deltaT;
		if (current_disks[i].timer >= DISK_LIFETIME) {
			graphics_manager.RemoveRenderRect(current_disks[i].render_id);

			shared::gameobject_network_s obj_data;
			obj_data.game_id = current_disks[i].gameobj_id;
			obj_data.operation = shared::OBJECTOPERATION::DESTROY;
			gameobject_network_requests_garuntee.push_back(obj_data);

			gameobj_idhandler.freeid(current_disks[i].gameobj_id);
			current_disks.erase(current_disks.begin() + i);
			i--;
		}
		else
		{
			graphics_manager.GetRenderRect(current_disks[i].render_id).Move(glm::vec3(current_disks[i].dir * DISK_SPEED * deltaT, 0));

			shared::gameobject_network_s obj_data;
			obj_data.game_id = current_disks[i].gameobj_id;
			obj_data.operation = shared::OBJECTOPERATION::MOVE;
			obj_data.position = graphics_manager.GetRenderRect(current_disks[i].render_id).getPos();
			gameobject_network_requests.push_back(obj_data);
		}
	}

	if (current_level == 5)
	{
		simulate_lvl5boss(deltaT);
	}

	if (current_level == 6)
	{
		if (level6_state.decorations[3].activated)
		{
			level6_state.button_1_count++;
			if (level6_state.button_1_count >= 2000)
			{
				level6_state.decorations[3].activated = false;

				shared::gameobject_network_s button_data;
				button_data.game_id = 3;
				button_data.operation = shared::OBJECTOPERATION::TEXTURE;
				button_data.texture_id = 8;
				gameobject_network_requests_garuntee.push_back(button_data);
			}
		}
		if (level6_state.decorations[11].activated)
		{
			level6_state.button_2_count++;
			if (level6_state.button_2_count >= 600)
			{
				level6_state.decorations[11].activated = false;

				shared::gameobject_network_s button_data;
				button_data.game_id = 11;
				button_data.operation = shared::OBJECTOPERATION::TEXTURE;
				button_data.texture_id = 8;
				gameobject_network_requests_garuntee.push_back(button_data);
			}
		}
	}
}

void ServerGameApp::simulate_lvl5boss(float deltaT)
{
	//see if we triggered boss
	if (!level5_state.boss_triggered)
	{
		//nothing to simulate
	}
	else if (level5_state.boss_running_stage_1)
	{
		if (true)
		{
			//simulate the bombs 
			int big_chunk_size = 32;
			int small_chunk_size = 15;
			int evensmaller_size = 12;
			int chunks = 0;
			if (level5_state.bomb_counter % 125 == 0)
			{
				chunks += big_chunk_size;
			}
			if (level5_state.bomb_counter % 40 == 0)
			{
				//spawn less
				chunks += small_chunk_size;
			}
			if (level5_state.bomb_counter % 25 == 0)
			{
				//spawn less
				chunks += evensmaller_size;
			}
			if (level5_state.bomb_counter % 100 == 0)
				audio_network_requests.push_back(shared::SOUNDS::TANK_1);
			if (level5_state.bomb_counter % 200 == 0)
				audio_network_requests.push_back(shared::SOUNDS::TANK_2);
			if (level5_state.bomb_counter % 300 == 0)
				audio_network_requests.push_back(shared::SOUNDS::TANK_3);
			
			level5_state.bomb_counter++;

			for (int i = 0; i < chunks; i++)
			{
				//position(10.5,0)   scale(18,18)
				std::mt19937 mt(rd());
				std::uniform_real_distribution<float> dist_1(0.0, 1.0);
				glm::vec2 rand_percent = glm::vec2(dist_1(mt), dist_1(mt));
				glm::vec2 spawn_position = glm::vec2(1.5 + rand_percent.x * 18, -9 + rand_percent.y * 18);

				bomb_s new_bomb;
				new_bomb.current_frame = 0;
				new_bomb.activated = false;
				new_bomb.gameobj_id = gameobj_idhandler.getAvailableId();
				new_bomb.render_id = graphics_manager.CreateRenderRect(glm::vec3(spawn_position, 0));
				graphics_manager.GetRenderRect(new_bomb.render_id).Scale(glm::vec3(.75, .75, 0));
				graphics_manager.GetRenderRect(new_bomb.render_id).SetTexture(45);
				level5_state.bombs.push_back(new_bomb);

				shared::gameobject_network_s obj_data;
				obj_data.game_id = new_bomb.gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::CREATE;
				obj_data.position = graphics_manager.GetRenderRect(new_bomb.render_id).getPos();
				obj_data.rotation_angle = 0;
				obj_data.texture_id = graphics_manager.GetRenderRect(new_bomb.render_id).gettextureid();
				obj_data.layer = 2;
				obj_data.visible = true;
				obj_data.scale = graphics_manager.GetRenderRect(new_bomb.render_id).getScale();
				gameobject_network_requests_garuntee.push_back(obj_data);
			}

			for (int i = 0; i < level5_state.bombs.size(); i++)
			{
				level5_state.bombs[i].current_frame++;
				if (level5_state.bombs[i].current_frame == 75)
				{
					//switch to red color
					shared::gameobject_network_s obj_data;
					obj_data.game_id = level5_state.bombs[i].gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::TEXTURE;
					obj_data.texture_id = 46;
					gameobject_network_requests.push_back(obj_data);
				}
				else if (level5_state.bombs[i].current_frame == 115) {
					//switch to fire color
					shared::gameobject_network_s obj_data;
					obj_data.game_id = level5_state.bombs[i].gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::TEXTURE;
					obj_data.texture_id = 44;
					gameobject_network_requests.push_back(obj_data);

					//activate
					level5_state.bombs[i].activated = true;
				}
				else if (level5_state.bombs[i].current_frame == 135) {
					//delete
					shared::gameobject_network_s obj_data;
					obj_data.game_id = level5_state.bombs[i].gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::DESTROY;
					gameobject_network_requests_garuntee.push_back(obj_data);

					graphics_manager.RemoveRenderRect(level5_state.bombs[i].render_id);

					level5_state.bombs.erase(level5_state.bombs.begin() + i);
					i--;
				}
			}
		}
		/// konguf men running at players, if empty spawn 1 for each player
		if (level5_state.minions_stage1.empty())
		{
			level5_state.minion_stage1_count++;
			if (level5_state.minion_stage1_count == 175)
			{
				for (auto& _player : current_players)
				{
					glm::vec2 avilo_pos = graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos();
					//create minion with players position
					minion_s obj;
					obj.gameobj_id = gameobj_idhandler.getAvailableId();
					obj.render_id = graphics_manager.CreateRenderRect();
					graphics_manager.GetRenderRect(obj.render_id).Scale(glm::vec3(.8, .8, 0));
					graphics_manager.GetRenderRect(obj.render_id).Translate(glm::vec3(avilo_pos, 0));
					graphics_manager.GetRenderRect(obj.render_id).SetTexture(57);
					graphics_manager.GetRenderRect(obj.render_id).SetLayer(3);
					obj.distance_measure = 0;
					obj.moving = false;
					obj.new_dir.x = _player.second.player_id;

					level5_state.minions_stage1.push_back(obj);

					shared::gameobject_network_s obj_data;
					obj_data.game_id = obj.gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::CREATE;
					obj_data.position = graphics_manager.GetRenderRect(obj.render_id).getPos();
					obj_data.rotation_angle = 0;
					obj_data.texture_id = graphics_manager.GetRenderRect(obj.render_id).gettextureid();
					obj_data.layer = 2;
					obj_data.visible = true;
					obj_data.scale = graphics_manager.GetRenderRect(obj.render_id).getScale();
					gameobject_network_requests_garuntee.push_back(obj_data);
				}
			}
		}
		else
		{
			level5_state.minion_stage1_count++;
			if (level5_state.minion_stage1_count >= 475)
			{
				for (int i = 0; i < level5_state.minions_stage1.size(); i++)
				{
					shared::gameobject_network_s obj_data;
					obj_data.game_id = level5_state.minions_stage1[i].gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::DESTROY;
					gameobject_network_requests_garuntee.push_back(obj_data);

					graphics_manager.RemoveRenderRect(level5_state.minions_stage1[i].render_id);
				}
				level5_state.minions_stage1.clear();
				level5_state.minion_stage1_count = 0;
			}
			else
			{
				for (int i = 0; i < level5_state.minions_stage1.size(); i++)
				{
					//run towards the player
					if (current_players.count(level5_state.minions_stage1[i].new_dir.x) != 0)
					{
						glm::vec2 player_pos = graphics_manager.GetRenderRect(current_players[level5_state.minions_stage1[i].new_dir.x].render_id).getPos();
						glm::vec2 kongfu_pos = graphics_manager.GetRenderRect(level5_state.minions_stage1[i].render_id).getPos();
						glm::vec2 dir = glm::normalize(player_pos - kongfu_pos) * glm::vec2(.05, .05) * deltaT;

						graphics_manager.GetRenderRect(level5_state.minions_stage1[i].render_id).Move(glm::vec3(dir, 0));

						shared::gameobject_network_s obj_data;
						obj_data.game_id = level5_state.minions_stage1[i].gameobj_id;
						obj_data.operation = shared::OBJECTOPERATION::MOVE;
						obj_data.position = graphics_manager.GetRenderRect(level5_state.minions_stage1[i].render_id).getPos();
						gameobject_network_requests.push_back(obj_data);
					}
				}
			}
		}

		//simulate avilo going down, then his voice lines
		if (!level5_state.avilo_moving_done)
		{
			graphics_manager.GetRenderRect(level5_state.avilo.render_id).Move(glm::vec3(0, -.03, 0));
			glm::vec2 curr_pos = graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos();
			if (std::abs(curr_pos.y - level5_state.avilo_spot.y) <= .1)
			{
				level5_state.avilo_moving_done = true;
				audio_network_requests.push_back(shared::SOUNDS::AVILO_11);
				graphics_manager.GetRenderRect(level5_state.avilo.render_id).Translate(glm::vec3(level5_state.avilo_spot, 0));
				
				shared::gameobject_network_s obj_data;
				obj_data.game_id = level5_state.avilo.gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::MOVE;
				obj_data.position = level5_state.avilo_spot;
				gameobject_network_requests_garuntee.push_back(obj_data);
			}
			else
			{
				shared::gameobject_network_s obj_data;
				obj_data.game_id = level5_state.avilo.gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::MOVE;
				obj_data.position = curr_pos;
				gameobject_network_requests.push_back(obj_data);
			}
		}
		if (level5_state.avilo_chase)
		{
			//get nearest player
			int closest_player = 0;
			float closest_pos = 100000.0;
			for(const auto& _player : current_players)
			{ 
				glm::vec2 player_pos = graphics_manager.GetRenderRect(_player.second.render_id).getPos();
				glm::vec2 avilo_pos = graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos();

				float dis = glm::distance(player_pos, avilo_pos);
				if (dis < closest_pos)
				{
					closest_pos = dis;
					closest_player = _player.second.player_id;
				}
			}

			//go towards them
			glm::vec2 player_pos = graphics_manager.GetRenderRect(current_players[closest_player].render_id).getPos();
			glm::vec2 avilo_pos = graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos();

			glm::vec2 move = glm::normalize(player_pos - avilo_pos) * glm::vec2(0.04, 0.04) * deltaT;
			graphics_manager.GetRenderRect(level5_state.avilo.render_id).Move(glm::vec3(move, 0));

			shared::gameobject_network_s obj_data;
			obj_data.game_id = level5_state.avilo.gameobj_id;
			obj_data.operation = shared::OBJECTOPERATION::MOVE;
			obj_data.position = graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos();
			gameobject_network_requests.push_back(obj_data);
		}
	}
	else if (level5_state.boss_running_stage_2)
	{
		level5_state.stage2_count++;

		//simulate minions until stage2_position is hit
		if (!level5_state.stage2_position)
		{
			level5_state.minion_delay++;
			for (int i = 0; i < level5_state.minions.size(); i++)
			{
				if (i == 0)
					continue;
				float distance = 7.0;
				glm::vec2 speed(.025, .025);
				float _prob = .010;
				if (!level5_state.minions[i].moving)
				{
					std::mt19937 mt(rd());
					std::uniform_real_distribution<float> dist_1(0.0, 1.0);
					if (dist_1(mt) <= _prob)
					{
						int exit_count = 0;
						glm::vec2 new_pos;
						do
						{
							float rand_angle = 360.0 * dist_1(mt);
							glm::vec2 curr_pos = graphics_manager.GetRenderRect(level5_state.minions[i].render_id).getPos();

							new_pos = glm::vec2(curr_pos.x + distance * cos(glm::radians(rand_angle)),
								curr_pos.y + distance * sin(glm::radians(rand_angle)));

							level5_state.minions[i].moving = true;
							level5_state.minions[i].distance_measure = 0;
							exit_count++;
						} while (!shared::point_in_rect_collision_test(graphics_manager, new_pos, level5_state.minion_rect_boundary, .25) && exit_count < 5);
						if (exit_count >= 5)
							level5_state.minions[i].moving = false;
						else
						{
							glm::vec2 curr_pos = graphics_manager.GetRenderRect(level5_state.minions[i].render_id).getPos();
							glm::vec2 dir = glm::normalize(new_pos - curr_pos);
							level5_state.minions[i].new_dir = dir;
						}
					}
				}
				else
				{
					glm::vec2 distance_step = level5_state.minions[i].new_dir * speed * deltaT;

					graphics_manager.GetRenderRect(level5_state.minions[i].render_id).Move(glm::vec3(distance_step, 0));

					level5_state.minions[i].distance_measure += glm::length(distance_step);
					if (level5_state.minions[i].distance_measure >= distance)
					{
						level5_state.minions[i].moving = false;
						level5_state.minions[i].distance_measure = 0;
					}

					shared::gameobject_network_s obj_data;
					obj_data.game_id = level5_state.minions[i].gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::MOVE;
					obj_data.position = graphics_manager.GetRenderRect(level5_state.minions[i].render_id).getPos();
					gameobject_network_requests.push_back(obj_data);
				}
			}
		}

		if (level5_state.stage2_count == 2200 && !level5_state.stage2_position)
		{
			audio_network_requests.push_back(shared::SOUNDS::AVILO_31);
		}
		if (level5_state.stage2_count > 2500 && !level5_state.stage2_position)
		{
			glm::vec2 new_pos(10.5, 0);

			glm::vec2 avilo_pos = graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos();
			glm::vec2 dir = glm::normalize(new_pos - avilo_pos) * glm::vec2(.05, .05) * deltaT;

			if (glm::distance(new_pos, avilo_pos) <= .05)
			{
				graphics_manager.GetRenderRect(level5_state.avilo.render_id).Translate(glm::vec3(new_pos, 0));
				level5_state.stage2_position = true;

				audio_network_requests.push_back(shared::SOUNDS::AVILO_32);

				graphics_manager.GetRenderRect(level5_state.avilo.render_id).Move(glm::vec3(dir, 0));
				shared::gameobject_network_s obj_data;
				obj_data.game_id = level5_state.avilo.gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::MOVE;
				obj_data.position = graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos();
				gameobject_network_requests_garuntee.push_back(obj_data);
			}
			else
			{
				graphics_manager.GetRenderRect(level5_state.avilo.render_id).Move(glm::vec3(dir, 0));
				shared::gameobject_network_s obj_data;
				obj_data.game_id = level5_state.avilo.gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::MOVE;
				obj_data.position = graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos();
				gameobject_network_requests.push_back(obj_data);
			}
		}
		else if (level5_state.stage2_position)
		{
			level5_state.stage2_count2++;
			if (level5_state.stage2_count2 == 1)
			{
				//destroy minions
				for (int i = 0; i < level5_state.minions.size(); i++)
				{
					shared::gameobject_network_s obj_data;
					obj_data.game_id = level5_state.minions[i].gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::DESTROY;
					gameobject_network_requests_garuntee.push_back(obj_data);

					graphics_manager.RemoveRenderRect(level5_state.minions[i].render_id);
				}
				level5_state.minions.clear();
			}

			if (level5_state.stage2_count2 == 100)
			{
				glm::vec2 new_pos(10.5, 8);
				//create fists game object and send create message
				game_object obj;
				obj.gameobj_id = gameobj_idhandler.getAvailableId();
				obj.render_id = graphics_manager.CreateRenderRect(glm::vec3(10.5 + 2.5, 0 - 2, 0));
				graphics_manager.GetRenderRect(obj.render_id).Scale(glm::vec3(3, 3, 0));
				graphics_manager.GetRenderRect(obj.render_id).SetTexture(49);
				level5_state.fists.push_back(obj);

				shared::gameobject_network_s obj_data;
				obj_data.game_id = obj.gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::CREATE;
				obj_data.position = graphics_manager.GetRenderRect(obj.render_id).getPos();
				obj_data.rotation_angle = 0;
				obj_data.texture_id = graphics_manager.GetRenderRect(obj.render_id).gettextureid();
				obj_data.layer = 2;
				obj_data.visible = true;
				obj_data.scale = graphics_manager.GetRenderRect(obj.render_id).getScale();
				gameobject_network_requests_garuntee.push_back(obj_data);

				obj.gameobj_id = gameobj_idhandler.getAvailableId();
				obj.render_id = graphics_manager.CreateRenderRect(glm::vec3(10.5 - 2.5, 0 - 2, 0));
				graphics_manager.GetRenderRect(obj.render_id).Scale(glm::vec3(3, 3, 0));
				graphics_manager.GetRenderRect(obj.render_id).SetTexture(51);
				level5_state.fists.push_back(obj);

				obj_data.game_id = obj.gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::CREATE;
				obj_data.position = graphics_manager.GetRenderRect(obj.render_id).getPos();
				obj_data.rotation_angle = 0;
				obj_data.texture_id = graphics_manager.GetRenderRect(obj.render_id).gettextureid();
				obj_data.layer = 2;
				obj_data.visible = true;
				obj_data.scale = graphics_manager.GetRenderRect(obj.render_id).getScale();
				gameobject_network_requests_garuntee.push_back(obj_data);

				//button_hammer
				level5_state.hammer_created = true;
				obj.gameobj_id = gameobj_idhandler.getAvailableId();
				obj.render_id = graphics_manager.CreateRenderRect(glm::vec3(10.5, -65, 0));
				graphics_manager.GetRenderRect(obj.render_id).Scale(glm::vec3(2, 2, 0));
				graphics_manager.GetRenderRect(obj.render_id).SetTexture(52);
				level5_state.button_hammer = obj;

				obj_data.game_id = obj.gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::CREATE;
				obj_data.position = graphics_manager.GetRenderRect(obj.render_id).getPos();
				obj_data.rotation_angle = 0;
				obj_data.texture_id = graphics_manager.GetRenderRect(obj.render_id).gettextureid();
				obj_data.layer = 3;
				obj_data.visible = true;
				obj_data.scale = graphics_manager.GetRenderRect(obj.render_id).getScale();
				gameobject_network_requests_garuntee.push_back(obj_data);
			}
			if (level5_state.stage2_count2 >= 200)
			{
				if (level5_state.stage2_count2 % 7 == 0)
				{
					//left kongfu
					game_object obj;
					obj.gameobj_id = gameobj_idhandler.getAvailableId();
					obj.render_id = graphics_manager.CreateRenderRect(glm::vec3(8.5 - .3 *level5_state.kongfu_count, 0, 0));
					graphics_manager.GetRenderRect(obj.render_id).Scale(glm::vec3(.5, .5, 0));
					graphics_manager.GetRenderRect(obj.render_id).SetTexture(50);
					level5_state.kongfus.push_back(obj);

					shared::gameobject_network_s obj_data;
					obj_data.game_id = obj.gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::CREATE;
					obj_data.position = graphics_manager.GetRenderRect(obj.render_id).getPos();
					obj_data.rotation_angle = 0;
					obj_data.texture_id = graphics_manager.GetRenderRect(obj.render_id).gettextureid();
					obj_data.layer = 2;
					obj_data.visible = true;
					obj_data.scale = graphics_manager.GetRenderRect(obj.render_id).getScale();
					gameobject_network_requests_garuntee.push_back(obj_data);

					//right kongfu
					obj.gameobj_id = gameobj_idhandler.getAvailableId();
					obj.render_id = graphics_manager.CreateRenderRect(glm::vec3(12.5 + .3 * level5_state.kongfu_count, 0, 0));
					graphics_manager.GetRenderRect(obj.render_id).Scale(glm::vec3(.5, .5, 0));
					graphics_manager.GetRenderRect(obj.render_id).SetTexture(50);
					level5_state.kongfus.push_back(obj);

					obj_data.game_id = obj.gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::CREATE;
					obj_data.position = graphics_manager.GetRenderRect(obj.render_id).getPos();
					obj_data.rotation_angle = 0;
					obj_data.texture_id = graphics_manager.GetRenderRect(obj.render_id).gettextureid();
					obj_data.layer = 2;
					obj_data.visible = true;
					obj_data.scale = graphics_manager.GetRenderRect(obj.render_id).getScale();
					gameobject_network_requests_garuntee.push_back(obj_data);

					level5_state.kongfu_count++;
				}
			}
			if (level5_state.stage2_count2 >= 350)
			{
				level5_state.boss_running_stage_2 = false;
				level5_state.boss_running_stage_3 = true;

				//open gate
				level5_state.gate_1.activated = false;
				shared::gameobject_network_s gate_data;
				gate_data.game_id = 4;
				gate_data.operation = shared::OBJECTOPERATION::VISIBILITY;
				gate_data.visible = false;
				gameobject_network_requests_garuntee.push_back(gate_data);

				audio_network_requests.push_back(shared::SOUNDS::DOOR);
				
			}
		}
	}
	else if (level5_state.boss_running_stage_3)
	{
		//simulate sounds
		//smashing
		//voice lines
		if (level5_state.stage3_count % 150 == 0)
			audio_network_requests.push_back(shared::SOUNDS::AVILO_4);

		if(level5_state.stage3_count == 100)
			audio_network_requests.push_back(shared::SOUNDS::AVILO_CRY);

		if (level5_state.stage3_count == 250)
			audio_network_requests.push_back(shared::SOUNDS::AVILO_43);

		if (level5_state.stage3_count == 400)
			audio_network_requests.push_back(shared::SOUNDS::AVILO_CRY);

		if (level5_state.stage3_count == 550)
			audio_network_requests.push_back(shared::SOUNDS::AVILO_66);

		level5_state.stage3_count++;
		
		//simulate fist moving
		float sin_move = .5*sin(glm::radians(level5_state.fist_angle));
		level5_state.fist_angle += 20;
		for (int i = 0; i < level5_state.fists.size(); i++)
		{
			glm::vec2 avilo_pos = graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos();
			if(i == 0)
				avilo_pos += glm::vec2(2.5, -2);
			else
				avilo_pos += glm::vec2(-2.5, -2);
			graphics_manager.GetRenderRect(level5_state.fists[i].render_id).Translate(glm::vec3(avilo_pos, 0));
			graphics_manager.GetRenderRect(level5_state.fists[i].render_id).Move(glm::vec3(0, sin_move, 0));
			shared::gameobject_network_s obj_data;
			obj_data.game_id = level5_state.fists[i].gameobj_id;
			obj_data.operation = shared::OBJECTOPERATION::MOVE;
			obj_data.position = graphics_manager.GetRenderRect(level5_state.fists[i].render_id).getPos();
			gameobject_network_requests.push_back(obj_data);
		}

		//move him and the army
		float y_speed = 0.06;
		graphics_manager.GetRenderRect(level5_state.avilo.render_id).Move(glm::vec3(0, -y_speed, 0));
		shared::gameobject_network_s obj_data;
		obj_data.game_id = level5_state.avilo.gameobj_id;
		obj_data.operation = shared::OBJECTOPERATION::MOVE;
		obj_data.position = graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos();
		gameobject_network_requests.push_back(obj_data);
		
		for (int i = 0; i < level5_state.kongfus.size(); i++)
		{
			graphics_manager.GetRenderRect(level5_state.kongfus[i].render_id).Move(glm::vec3(0, -y_speed, 0));
			obj_data.game_id = level5_state.kongfus[i].gameobj_id;
			obj_data.operation = shared::OBJECTOPERATION::MOVE;
			obj_data.position = graphics_manager.GetRenderRect(level5_state.kongfus[i].render_id).getPos();
			gameobject_network_requests.push_back(obj_data);
		}
	}
	else if (level5_state.boss_running_stage_4)
	{
		if (level5_state.stage4_count == 0)
		{
			//destroy all kongfus
			for (int i = 0; i < level5_state.kongfus.size(); i++)
			{
				shared::gameobject_network_s obj_data;
				obj_data.game_id = level5_state.kongfus[i].gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::DESTROY;
				gameobject_network_requests_garuntee.push_back(obj_data);

				graphics_manager.RemoveRenderRect(level5_state.kongfus[i].render_id);
			}
			level5_state.kongfus.clear();

			//destroy 2 fists
			for (int i = 0; i < level5_state.fists.size(); i++)
			{
				shared::gameobject_network_s obj_data;
				obj_data.game_id = level5_state.fists[i].gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::DESTROY;
				gameobject_network_requests_garuntee.push_back(obj_data);

				graphics_manager.RemoveRenderRect(level5_state.fists[i].render_id);
			}
			level5_state.fists.clear();

			//switch avilo texture
			shared::gameobject_network_s button_data;
			button_data.game_id = level5_state.avilo.gameobj_id;
			button_data.operation = shared::OBJECTOPERATION::TEXTURE;
			button_data.texture_id = 58;

			gameobject_network_requests_garuntee.push_back(button_data);

			//play avilo manifesto
			audio_network_requests.push_back(shared::SOUNDS::AVILO_5);
		}
		level5_state.stage4_count++;

		if (level5_state.stage4_count >= 2200)
		{
			if (!level5_state.hammer_hit)
			{
				//ban hammer flies at avilo, when it gets there everything deletes, and he switches images
				glm::vec2 avilo_pos = graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos();
				glm::vec2 hammer = graphics_manager.GetRenderRect(level5_state.button_hammer.render_id).getPos();
				glm::vec2 dir = glm::normalize(avilo_pos - hammer) * glm::vec2(.05, .05) * deltaT;

				if (glm::distance(avilo_pos, hammer) <= 2.0)
				{
					//graphics_manager.GetRenderRect(level5_state.button_hammer.render_id).Translate(glm::vec3(avilo_pos, 0));

					level5_state.hammer_hit = true;
					audio_network_requests.push_back(shared::SOUNDS::AVILO_HAMMER);
					shared::gameobject_network_s button_data;
					button_data.game_id = level5_state.avilo.gameobj_id;
					button_data.operation = shared::OBJECTOPERATION::TEXTURE;
					button_data.texture_id = 59;

					gameobject_network_requests_garuntee.push_back(button_data);

					shared::gameobject_network_s obj_data;
					obj_data.game_id = level5_state.button_hammer.gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::MOVE;
					obj_data.position = graphics_manager.GetRenderRect(level5_state.button_hammer.render_id).getPos();
					gameobject_network_requests_garuntee.push_back(obj_data);

					audio_network_requests.push_back(shared::SOUNDS::AVILO_55);
				}
				else
				{
					graphics_manager.GetRenderRect(level5_state.button_hammer.render_id).Move(glm::vec3(dir, 0));
					shared::gameobject_network_s obj_data;
					obj_data.game_id = level5_state.button_hammer.gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::MOVE;
					obj_data.position = graphics_manager.GetRenderRect(level5_state.button_hammer.render_id).getPos();
					gameobject_network_requests.push_back(obj_data);
				}
			}
			else
			{
				level5_state.hammer_hit_count++;
				if (level5_state.hammer_hit_count > 3200)
				{
					level5_state.boss_running_stage_4 = false;
					level5_state.boss_running_stage_5 = true;

					//open gate
					level5_state.gate_2.activated = false;
					shared::gameobject_network_s gate_data;
					gate_data.game_id = 5;
					gate_data.operation = shared::OBJECTOPERATION::VISIBILITY;
					gate_data.visible = false;
					gameobject_network_requests_garuntee.push_back(gate_data);

					//play background music
					audio_network_requests.push_back(shared::SOUNDS::END_SONG);
					audio_network_requests.push_back(shared::SOUNDS::DOOR);
				}
			}
		}
	}
	else if (level5_state.boss_running_stage_5)
	{
		if (level5_state.cop_on)
		{
			graphics_manager.GetRenderRect(level5_state.cop_car.render_id).Move(glm::vec3(.1, 0, 0));
			shared::gameobject_network_s obj_data;
			obj_data.game_id = level5_state.cop_car.gameobj_id;
			obj_data.operation = shared::OBJECTOPERATION::MOVE;
			obj_data.position = graphics_manager.GetRenderRect(level5_state.cop_car.render_id).getPos();
			gameobject_network_requests.push_back(obj_data);
		}
	}
}

void ServerGameApp::resolve_gameobjectslvl5boss(player& main_player)
{
	if (!level5_state.boss_triggered)
	{
		//if any baneling is on ice we know we can trigger boss
		bool triggered = false;
		for (const auto& _player : current_players)
		{
			if (_player.second.current_ground == GROUNDTYPE::ICE)
			{
				triggered = true;
				break;
			}
		}
		if (triggered)
		{
			level5_state.boss_triggered = true;
			level5_state.boss_running_stage_1 = true;

			audio_network_requests.push_back(shared::SOUNDS::AVILO_1);
			//initialize stage 1
			//spawn tanks
			for (int i = 0; i < 4; i++)
			{
				level5_state.tanks.push_back(gameobj_idhandler.getAvailableId());

				shared::gameobject_network_s obj_data;
				obj_data.game_id = level5_state.tanks.back();
				obj_data.operation = shared::OBJECTOPERATION::CREATE;
				if (i == 0)
					obj_data.position = glm::vec2(0.0, 12);
				else if (i == 1)
					obj_data.position = glm::vec2(0.0, -12);
				else if (i == 2)
					obj_data.position = glm::vec2(20.5, 12);
				else if (i == 3)
					obj_data.position = glm::vec2(20.5, -12);
				obj_data.rotation_angle = 0;
				obj_data.texture_id = 0;
				obj_data.layer = 2;
				obj_data.visible = true;
				obj_data.scale = glm::vec2(5, 5);
				gameobject_network_requests_garuntee.push_back(obj_data);
			}

			//spawn avilo and play voice line of him calling you a maphacker, a stalker
			level5_state.avilo.gameobj_id = gameobj_idhandler.getAvailableId();
			level5_state.avilo_spot = glm::vec2(10.5, 0);
			level5_state.avilo_moving_done = false;
			level5_state.avilo.render_id = graphics_manager.CreateRenderRect(glm::vec3(10.5, 9, 0));
			graphics_manager.GetRenderRect(level5_state.avilo.render_id).Rotate(glm::vec3(0, 0, 1), 0);
			graphics_manager.GetRenderRect(level5_state.avilo.render_id).Scale(glm::vec3(2.0, 2.0, 0));
			graphics_manager.GetRenderRect(level5_state.avilo.render_id).SetTexture(42);
			graphics_manager.GetRenderRect(level5_state.avilo.render_id).SetLayer(2);

			shared::gameobject_network_s obj_data;
			obj_data.game_id = level5_state.avilo.gameobj_id;
			obj_data.operation = shared::OBJECTOPERATION::CREATE;
			obj_data.position = glm::vec3(10.5, 9, 0);
			obj_data.rotation_angle = 0;
			obj_data.texture_id = 42;
			obj_data.layer = 2;
			obj_data.visible = true;
			obj_data.scale = glm::vec2(2, 2);
			gameobject_network_requests_garuntee.push_back(obj_data);
		}
	}
	else if (level5_state.boss_running_stage_1)
	{
		//if player hits avilo they die
		bool collision = false;
		if (shared::rect_collision_test(graphics_manager, main_player.render_id, level5_state.avilo.render_id, .2))
		{
			audio_network_requests.push_back(shared::SOUNDS::AVILO_KONGFUSHORT);
			collision = true;
		}

		//if player hits bomb when its on they die
		for (const auto& bomb : level5_state.bombs)
		{
			if (bomb.activated && shared::rect_collision_test(graphics_manager, main_player.render_id, bomb.render_id, .15))
			{
				collision = true;
				break;
			}
		}

		//if player hits minion_stage 1 they die
		for (const auto& minionz : level5_state.minions_stage1)
		{
			if (shared::rect_collision_test(graphics_manager, main_player.render_id, minionz.render_id, .15))
			{
				audio_network_requests.push_back(shared::SOUNDS::AVILO_KONGFUSHORT);
				collision = true;
				break;
			}
		}

		if (collision)
		{
			playerDie(main_player);
		}
	}
	else if (level5_state.boss_running_stage_2)
	{
		bool collision = false;
		if (shared::rect_collision_test(graphics_manager, main_player.render_id, level5_state.avilo.render_id, .15))
		{
			collision = true;
		}

		//if player hits minions when its on they die
		if (level5_state.minion_delay > 100)
		{
			for (const auto& minion : level5_state.minions)
			{
				if (shared::rect_collision_test(graphics_manager, main_player.render_id, minion.render_id, .15))
				{
					collision = true;
					break;
				}
			}
		}

		if (collision)
		{
			audio_network_requests.push_back(shared::SOUNDS::AVILO_KONGFUSHORT);
			playerDie(main_player);
		}
	}
	else if (level5_state.boss_running_stage_3)
	{
		bool collision = false;
		if (shared::rect_collision_test(graphics_manager, main_player.render_id, level5_state.avilo.render_id, .15))
		{
			audio_network_requests.push_back(shared::SOUNDS::AVILO_KONGFUSHORT);
			collision = true;
		}

		//if player hits fist when its on they die
		for (const auto& fist : level5_state.fists)
		{
			if (shared::rect_collision_test(graphics_manager, main_player.render_id, fist.render_id, .15))
			{
				collision = true;
				break;
			}
		}

		for (const auto& kongfu : level5_state.kongfus)
		{
			if (shared::rect_collision_test(graphics_manager, main_player.render_id, kongfu.render_id, .75))
			{
				collision = true;
				break;
			}
		}

		if (collision)
		{
			//play a get shit on sound or something LOL
			//audio_network_requests.push_back(shared::SOUNDS::ZEALOT);
			playerDie(main_player);
		}

		//you hit the button for the ban hammer he dies and cries about getting banned
		if (shared::rect_collision_test(graphics_manager, main_player.render_id, level5_state.button_hammer.render_id, .15))
		{
			level5_state.boss_running_stage_3 = false;
			level5_state.boss_running_stage_4 = true;
		}
	}
	else if (level5_state.boss_running_stage_4)
	{
		//nothing
	}
	else if (level5_state.boss_running_stage_5)
	{
		if (!level5_state.cop_on)
		{
			for (const auto& _player : current_players)
			{
				if (graphics_manager.GetRenderRect(_player.second.render_id).getPos().x >= 22.5)
				{
					//spawn car
					game_object obj;
					obj.gameobj_id = gameobj_idhandler.getAvailableId();
					obj.render_id = graphics_manager.CreateRenderRect(glm::vec3(10.5, -61, 0));
					graphics_manager.GetRenderRect(obj.render_id).Scale(glm::vec3(3.0, 2.0, 0));
					graphics_manager.GetRenderRect(obj.render_id).SetTexture(55);
					graphics_manager.GetRenderRect(obj.render_id).SetLayer(2);
					level5_state.cop_car = obj;

					shared::gameobject_network_s obj_data;
					obj_data.game_id = level5_state.cop_car.gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::CREATE;
					obj_data.position = graphics_manager.GetRenderRect(obj.render_id).getPos();
					obj_data.rotation_angle = 0;
					obj_data.texture_id = graphics_manager.GetRenderRect(obj.render_id).gettextureid();
					obj_data.layer = 2;
					obj_data.visible = true;
					obj_data.scale = graphics_manager.GetRenderRect(obj.render_id).getScale();
					gameobject_network_requests_garuntee.push_back(obj_data);

					//play sound
					audio_network_requests.push_back(shared::SOUNDS::AVILO_COPS);
					audio_network_requests.push_back(shared::SOUNDS::AVILO_6);

					level5_state.cop_on = true;
					break;
				}
			}
		}
	}
}

void ServerGameApp::simulate_enemies(float deltaT)
{
	for (enemy_s& _enemy : current_enemies)
	{
		if (_enemy.simple_patrol)
		{
			glm::vec2 distance_step = _enemy.speed * deltaT;
			if (_enemy.dir)
			{
				graphics_manager.GetRenderRect(_enemy.render_id).Move(glm::vec3(distance_step, 0));
			}
			else
			{
				graphics_manager.GetRenderRect(_enemy.render_id).Move(glm::vec3(-distance_step, 0));
			}

			_enemy.distance_measure += glm::length(distance_step);
			if (_enemy.distance_measure >= _enemy.distance)
			{
				_enemy.dir = !_enemy.dir;
				_enemy.distance_measure = 0;
			}
		}
		else if (_enemy.strip_walking)
		{
			enemy_strip_s& current_strip = _enemy.walking_strips[_enemy.current_strip];
			
			glm::vec2 distance_step = (_enemy.dir) ? current_strip.speed * deltaT : -current_strip.speed * deltaT;
			graphics_manager.GetRenderRect(_enemy.render_id).Move(glm::vec3(distance_step, 0));

			current_strip.distance_measure += glm::length(distance_step);
			if (current_strip.distance_measure >= current_strip.distance)
			{
				float difference = current_strip.distance_measure - current_strip.distance;
				glm::vec2 correction_step = difference * glm::normalize(distance_step);
				graphics_manager.GetRenderRect(_enemy.render_id).Move(glm::vec3(correction_step, 0));
				
				current_strip.distance_measure = 0;

				if(!_enemy.backwards_loop)
					_enemy.current_strip = (_enemy.current_strip + 1) % _enemy.walking_strips.size();
				else
				{
					if ((_enemy.dir && _enemy.current_strip == _enemy.walking_strips.size() - 1) ||
						(!_enemy.dir && _enemy.current_strip == 0))
					{
						_enemy.dir = !_enemy.dir;
					}
					else
					{
						if (_enemy.dir)
							_enemy.current_strip++;
						else
							_enemy.current_strip--;
					}
				}
			}

		}
		else if(_enemy.elipse)
		{
			glm::vec2 ellipsepos = glm::vec2(_enemy.h + _enemy.a*cos(glm::radians(_enemy.distance_measure)), 
											 _enemy.k + _enemy.b *sin(glm::radians(_enemy.distance_measure)));
			graphics_manager.GetRenderRect(_enemy.render_id).Translate(glm::vec3(ellipsepos, 0));
			
			if(_enemy.dir)
				_enemy.distance_measure += glm::length(_enemy.speed * deltaT);
			else
				_enemy.distance_measure -= glm::length(_enemy.speed * deltaT);
		}
		else if (_enemy.randomness)
		{
			if (!_enemy.random_moving)
			{
				std::mt19937 mt(rd());
				std::uniform_real_distribution<float> dist_1(0.0, 1.0);
				if (dist_1(mt) <= _enemy.prob)
				{
					int exit_count = 0;
					glm::vec2 new_pos;
					do 
					{
						float rand_angle = 360.0 * dist_1(mt);
						glm::vec2 curr_pos = graphics_manager.GetRenderRect(_enemy.render_id).getPos();
	
						new_pos = glm::vec2(curr_pos.x + _enemy.distance * cos(glm::radians(rand_angle)),
							curr_pos.y + _enemy.distance * sin(glm::radians(rand_angle)));
						
						_enemy.random_moving = true;
						_enemy.distance_measure = 0;
						exit_count++;
					}
					while(!shared::point_in_rect_collision_test(graphics_manager, new_pos, _enemy.rect_boundary, .25) && exit_count < 5);
					if (exit_count >= 5) 
						_enemy.random_moving = false;
					else
					{
						glm::vec2 curr_pos = graphics_manager.GetRenderRect(_enemy.render_id).getPos();
						glm::vec2 dir = glm::normalize(new_pos - curr_pos);
						_enemy.new_dir = dir;
					}
				}
			}
			else
			{
				glm::vec2 distance_step = _enemy.new_dir * _enemy.speed * deltaT;

				graphics_manager.GetRenderRect(_enemy.render_id).Move(glm::vec3(distance_step, 0));
				
				_enemy.distance_measure += glm::length(distance_step);
				if (_enemy.distance_measure >= _enemy.distance)
				{
					_enemy.random_moving = false;
					_enemy.distance_measure = 0;
				}
			}
		}
	}
}

void ServerGameApp::simulate_player(float deltaT, player& main_player)
{
	//MOVEMENT
	if ((main_player.alive && main_player.ability_mr && main_player.current_ground != GROUNDTYPE::SLIDE))
	{
		glm::vec2 world_pos = glm::vec2(main_player.ability_mx, main_player.ability_my);

		main_player.click_pos = world_pos;
		main_player.moving_to_waypoint = true;

		if (main_player.current_ground == GROUNDTYPE::REVERSE)
		{
			//reflect click_pos around player
			glm::vec2 curr_pos = graphics_manager.GetRenderRect(main_player.render_id).getPos();
			glm::vec2 dir = main_player.click_pos - curr_pos;
			if (!isnan(dir.x) && !isnan(dir.y))
			{
				main_player.click_pos = curr_pos - dir;
			}
		}

		//if clickpos is too close to player move it out radially
		float min_radius = 0.001;
		glm::vec2 curr_pos = graphics_manager.GetRenderRect(main_player.render_id).getPos();
		glm::vec2 dir = main_player.click_pos - curr_pos;
		if (glm::length(dir) <= min_radius) {
			std::cout << "too close!\n";
			main_player.click_pos = curr_pos + glm::normalize(dir) * glm::vec2(min_radius, min_radius);
			if (glm::length(dir) == 0) {
				std::cout << "zero length!\n";
				main_player.click_pos = curr_pos + glm::normalize(glm::vec2(1, 0)) * glm::vec2(min_radius, min_radius);
			}
		}
	}
	if (main_player.alive && main_player.ability_s && !main_player.onIce()) {
		main_player.moving_to_waypoint = false;
	}

	if (main_player.moving_to_waypoint)
	{
		//std::cout << "moving to waypoing\n";
		glm::vec2 curr_pos = graphics_manager.GetRenderRect(main_player.render_id).getPos();
		glm::vec2 dir = main_player.click_pos - curr_pos;
		dir = glm::normalize(dir);
		if (!isnan(dir.x) && !isnan(dir.y))
		{
			float extra_speed = 1;
			if (main_player.onIce() && main_player.ongoo)
				extra_speed = GOO_EXTRASPEED;
			if (!main_player.onIce() && main_player.speed_active)
				extra_speed = SPEED_EXTRASPEED;

			glm::vec2 distance_step = dir * glm::vec2(main_player.speed, main_player.speed) * glm::vec2(extra_speed, extra_speed) * deltaT;
			graphics_manager.GetRenderRect(main_player.render_id).Move(glm::vec3(distance_step, 0));
			main_player.dir = dir;

			if (glm::distance(main_player.click_pos, curr_pos) <= (glm::length(distance_step) + .00001))
			{
				graphics_manager.GetRenderRect(main_player.render_id).Translate(glm::vec3(main_player.click_pos, 0));
				main_player.moving_to_waypoint = false;
			}
		}
	}
	else if (main_player.onIce())
	{
		float extra_speed = 1.0;
		if (main_player.onIce() && main_player.ongoo)
			extra_speed = GOO_EXTRASPEED;
		glm::vec2 distance_step = main_player.dir * glm::vec2(main_player.speed, main_player.speed) * glm::vec2(extra_speed, extra_speed) * deltaT;
		graphics_manager.GetRenderRect(main_player.render_id).Move(glm::vec3(distance_step, 0));
	}


	//ABILITIES
	//GOOPOWER
	
	if (main_player.alive && main_player.ability_f && main_player.goo_available)
	{
		main_player.goo_available = false;
		main_player.goo_active = true;
		main_player.goo_timer = 0;
	}
	if (!main_player.goo_available)
		main_player.goo_timer += deltaT;

	if (main_player.goo_active)
	{
		if (main_player.goo_timer >= GOOPOWER_ACTIVATION_TIME)
		{
			main_player.goo_active = false;
		}
		else
		{
			if ((int)main_player.goo_timer % GOOSPAWN_INTERVAL == 0)
			{
				goo new_goo;
				new_goo.render_id = graphics_manager.CreateRenderRect(
					glm::vec3(graphics_manager.GetRenderRect(main_player.render_id).getPos(), 0));
				graphics_manager.GetRenderRect(new_goo.render_id).Rotate(glm::vec3(0, 0, 1), glm::degrees(main_player.rot_angle));
				graphics_manager.GetRenderRect(new_goo.render_id).Scale(glm::vec3(.5, .5, 0));
				graphics_manager.GetRenderRect(new_goo.render_id).SetTexture(15);
				graphics_manager.GetRenderRect(new_goo.render_id).SetLayer(2);
				new_goo.timer = 0;
				new_goo.gameobj_id = gameobj_idhandler.getAvailableId();

				current_goo.push_back(new_goo);

				shared::gameobject_network_s obj_data;
				obj_data.game_id = new_goo.gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::CREATE;
				obj_data.position = graphics_manager.GetRenderRect(main_player.render_id).getPos();
				obj_data.rotation_angle = glm::degrees(main_player.rot_angle);
				obj_data.texture_id = 15;
				obj_data.layer = 2;
				obj_data.visible = true;
				obj_data.scale = glm::vec2(.5, .5);
				gameobject_network_requests_garuntee.push_back(obj_data);
			}
		}
	}
	if (!main_player.goo_active && !main_player.goo_available && main_player.goo_timer >= GOOPOWER_CD_TIME)
	{
		main_player.goo_available = true;
	}
	
	//DISKPOWER
	if (main_player.alive && main_player.ability_e && main_player.disk_avaiable)
	{
		if(!config::CLIENT_GOD)
			main_player.disk_avaiable = false;
		
		main_player.disk_timer = 0;

		disk new_disk;
		if (main_player.onIce())
			new_disk.dir = main_player.dir;
		else
			new_disk.dir = glm::vec2(0, 0);
		new_disk.render_id = graphics_manager.CreateRenderRect(
			glm::vec3(graphics_manager.GetRenderRect(main_player.render_id).getPos(), 0)); //players rotation?
		graphics_manager.GetRenderRect(new_disk.render_id).Scale(glm::vec3(.4, .4, 0));
		graphics_manager.GetRenderRect(new_disk.render_id).SetTexture(16);
		graphics_manager.GetRenderRect(new_disk.render_id).SetLayer(3);
		new_disk.timer = 0;
		new_disk.gameobj_id = gameobj_idhandler.getAvailableId();

		current_disks.push_back(new_disk);

		shared::gameobject_network_s obj_data;
		obj_data.game_id = new_disk.gameobj_id;
		obj_data.operation = shared::OBJECTOPERATION::CREATE;
		obj_data.position = graphics_manager.GetRenderRect(main_player.render_id).getPos();
		obj_data.texture_id = 16;
		obj_data.layer = 3;
		obj_data.visible = true;
		obj_data.scale = glm::vec2(.4,.4);
		gameobject_network_requests_garuntee.push_back(obj_data);
	}
	if (!main_player.disk_avaiable)
	{
		main_player.disk_timer += deltaT;
		if (main_player.disk_timer >= DISKPOWER_CD_TIME)
			main_player.disk_avaiable = true;
	}

	//SPEEDPOWER
	if (main_player.alive && main_player.ability_t && main_player.speed_available)
	{
		main_player.speed_available = false;
		main_player.speed_active = true;
		main_player.speed_timer = 0;
		graphics_manager.GetRenderRect(main_player.render_id).SetTexture(17);
	}
	if (!main_player.speed_available)
		main_player.speed_timer += deltaT;

	if (main_player.speed_active && main_player.speed_timer >= SPEEDPOWER_ACTIVATION_TIME)
	{
		main_player.speed_active = false;
		if (main_player.alive)
			graphics_manager.GetRenderRect(main_player.render_id).SetTexture(4);
	}
	if (!main_player.speed_available && !main_player.speed_active && main_player.speed_timer >= SPEEDPOWER_CD_TIME)
	{
		main_player.speed_available = true;
	}

	main_player.ability_e = false;
	main_player.ability_f = false;
	main_player.ability_mr = false;
	main_player.ability_s = false;
	main_player.ability_t = false;
}

void ServerGameApp::Resolve()
{
	for (auto itr = current_players.begin(); itr != current_players.end(); itr++)
	{
		if (itr->second.alive)
		{
			resolve_enemies(itr->second);
			resolve_ground(itr->second);
			resolve_players(itr->second);
		}
		resolve_gameobjects(itr->second);
	}
}

void ServerGameApp::resolve_players(player& main_player)
{
	//this player is checking if they revived any of the other players
	for (auto itr = current_players.begin(); itr != current_players.end(); itr++)
	{
		if (itr->second.player_id != main_player.player_id)
		{
			if (itr->second.alive == false && shared::rect_collision_test(graphics_manager, itr->second.render_id, main_player.render_id))
			{
				playerRevive(itr->second);
			}
		}
	}

	//physics? they bump into eachother?
}

void ServerGameApp::resolve_gameobjects(player& main_player)
{
	//disk
	for (int i = 0; i < current_disks.size(); i++)
	{
		if (main_player.alive == false && shared::rect_collision_test(graphics_manager, current_disks[i].render_id, main_player.render_id))
		{
			playerRevive(main_player);
			break;
		}
	}

	//goo
	main_player.ongoo = false;
	for (const auto& gooz : current_goo)
	{
		if (shared::rect_collision_test(graphics_manager, main_player.render_id, gooz.render_id))
		{
			main_player.ongoo = true;
		}
	}

	switch (current_level)
	{
		case 1: resolve_gameobjectslvl1(main_player); break;
		case 2: resolve_gameobjectslvl2(main_player); break;
		case 3: resolve_gameobjectslvl3(main_player); break;
		case 4: resolve_gameobjectslvl4(main_player); break;
		case 5: resolve_gameobjectslvl5(main_player); break;
		case 6: resolve_gameobjectslvl6(main_player); break;
	}
}

void ServerGameApp::resolve_gameobjectslvl1(player& main_player)
{
	if (main_player.alive)
	{
		//buttons
		for (int i = 0; i < level1_state.buttons.size(); i++)
		{
			if (!level1_state.buttons[i].activated &&
				shared::point_in_rect_collision_test(graphics_manager, graphics_manager.GetRenderRect(main_player.render_id).getPos(), level1_state.buttons[i].render_id))
			{
				level1_state.buttons[i].activated = true;

				shared::gameobject_network_s button_data;
				button_data.game_id = i;
				button_data.operation = shared::OBJECTOPERATION::TEXTURE;
				button_data.texture_id = 10;

				gameobject_network_requests_garuntee.push_back(button_data);
			}
		}
		bool all_activated = true;
		for (int i = 0; i < level1_state.buttons.size(); i++)
		{
			if (!level1_state.buttons[i].activated)
				all_activated = false;
		}
		if (level1_state.gate_1.activated && all_activated)
		{
			level1_state.gate_1.activated = false;
			shared::gameobject_network_s gate_data;
			gate_data.game_id = level1_state.buttons.size();
			gate_data.operation = shared::OBJECTOPERATION::VISIBILITY;
			gate_data.visible = false;

			gameobject_network_requests_garuntee.push_back(gate_data);

			audio_network_requests.push_back(shared::SOUNDS::DOOR);
		}

		//gate
		if (level1_state.gate_1.activated && shared::rect_collision_test(graphics_manager, main_player.render_id, level1_state.gate_1.render_id))
		{
			glm::vec2 pos = graphics_manager.GetRenderRect(main_player.render_id).getPos();
			pos.x = graphics_manager.GetRenderRect(level1_state.gate_1.render_id).getPos().x -
				(graphics_manager.GetRenderRect(level1_state.gate_1.render_id).getScale().x / 2.0f) -
				(graphics_manager.GetRenderRect(main_player.render_id).getScale().x / 2.0f);
			graphics_manager.GetRenderRect(main_player.render_id).Translate(glm::vec3(pos, 0));

			if (std::signbit(main_player.dir.y)) {
				//bottom of gate
				main_player.click_pos = graphics_manager.GetRenderRect(level1_state.gate_1.render_id).getPos() +
					glm::vec2(-graphics_manager.GetRenderRect(level1_state.gate_1.render_id).getScale().x / 2.0f,
						-graphics_manager.GetRenderRect(level1_state.gate_1.render_id).getScale().y);
			}
			else {
				//top of gate
				main_player.click_pos = graphics_manager.GetRenderRect(level1_state.gate_1.render_id).getPos() +
					glm::vec2(-graphics_manager.GetRenderRect(level1_state.gate_1.render_id).getScale().x / 2.0f,
						graphics_manager.GetRenderRect(level1_state.gate_1.render_id).getScale().y);
			}
			main_player.moving_to_waypoint = true;
		}
	}
}

void ServerGameApp::resolve_gameobjectslvl2(player& main_player)
{
	//none
}

void ServerGameApp::resolve_gameobjectslvl3(player& main_player)
{
	//none
}

void ServerGameApp::resolve_gameobjectslvl4(player& main_player)
{
	//none
}

void ServerGameApp::resolve_gameobjectslvl5(player& main_player)
{
	if (main_player.alive)
	{
		//buttons
		if (level5_state.avilo_moving_done)
		{
			for (int i = 0; i < level5_state.buttons.size(); i++)
			{
				if (!level5_state.buttons[i].activated &&
					shared::point_in_rect_collision_test(graphics_manager, graphics_manager.GetRenderRect(main_player.render_id).getPos(), level5_state.buttons[i].render_id))
				{
					level5_state.buttons[i].activated = true;

					shared::gameobject_network_s button_data;
					button_data.game_id = i;
					button_data.operation = shared::OBJECTOPERATION::TEXTURE;
					button_data.texture_id = 10;

					gameobject_network_requests_garuntee.push_back(button_data);
				}
			}
			bool all_activated = true;
			for (int i = 0; i < level5_state.buttons.size(); i++)
			{
				if (!level5_state.buttons[i].activated)
					all_activated = false;
			}

			if (all_activated && level5_state.boss_running_stage_1 == true && level5_state.avilo_chase == false)
			{
				level5_state.avilo_chase = true;
				level5_state.end_button.activated = true;
				audio_network_requests.push_back(shared::SOUNDS::AVILO_KONGFU);

				shared::gameobject_network_s gate_data2;
				gate_data2.game_id = level5_state.avilo.gameobj_id;
				gate_data2.operation = shared::OBJECTOPERATION::TEXTURE;
				gate_data2.texture_id = 57;
				gameobject_network_requests_garuntee.push_back(gate_data2);

				shared::gameobject_network_s gate_data;
				gate_data.game_id = 6;
				gate_data.operation = shared::OBJECTOPERATION::VISIBILITY;
				gate_data.visible = true;
				gameobject_network_requests_garuntee.push_back(gate_data);
			}
		}
		//end_button
		if (level5_state.end_button.activated &&
			shared::point_in_rect_collision_test(graphics_manager, graphics_manager.GetRenderRect(main_player.render_id).getPos(), level5_state.end_button.render_id))
		{
			//change color of button
			level5_state.end_button.activated = false;
			shared::gameobject_network_s button_data;
			button_data.game_id = 6;
			button_data.operation = shared::OBJECTOPERATION::TEXTURE;
			button_data.texture_id = 10;
			gameobject_network_requests_garuntee.push_back(button_data);

			//switch to phase 3 and clean up all bombs
			level5_state.boss_running_stage_1 = false;
			level5_state.boss_running_stage_2 = true;
			
			for (int i = 0; i < level5_state.bombs.size(); i++)
			{
				//delete
				shared::gameobject_network_s obj_data;
				obj_data.game_id = level5_state.bombs[i].gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::DESTROY;
				gameobject_network_requests_garuntee.push_back(obj_data);

				graphics_manager.RemoveRenderRect(level5_state.bombs[i].render_id);

				level5_state.bombs.erase(level5_state.bombs.begin() + i);
				i--;
			}
			level5_state.bombs.clear();

			//clean up minions_stage1
			for (int i = 0; i < level5_state.minions_stage1.size(); i++)
			{
				shared::gameobject_network_s obj_data;
				obj_data.game_id = level5_state.minions_stage1[i].gameobj_id;
				obj_data.operation = shared::OBJECTOPERATION::DESTROY;
				gameobject_network_requests_garuntee.push_back(obj_data);

				graphics_manager.RemoveRenderRect(level5_state.minions_stage1[i].render_id);
			}
			level5_state.minions_stage1.clear();

			shared::gameobject_network_s obj_data;
			obj_data.game_id = level5_state.avilo.gameobj_id;
			obj_data.operation = shared::OBJECTOPERATION::TEXTURE;
			obj_data.texture_id = 47;
			gameobject_network_requests_garuntee.push_back(obj_data);

			audio_network_requests.push_back(shared::SOUNDS::AVILO_3);
			//atira spawn
			level5_state.atira_created = true;
			level5_state.atira.gameobj_id = gameobj_idhandler.getAvailableId();
			level5_state.atira.render_id = graphics_manager.CreateRenderRect(glm::vec3(graphics_manager.GetRenderRect(level5_state.avilo.render_id).getPos() + glm::vec2(5,0),0));
			graphics_manager.GetRenderRect(level5_state.atira.render_id).Rotate(glm::vec3(0, 0, 1), 0);
			graphics_manager.GetRenderRect(level5_state.atira.render_id).Scale(glm::vec3(3.0, 3.0, 0));
			graphics_manager.GetRenderRect(level5_state.atira.render_id).SetTexture(48);
			graphics_manager.GetRenderRect(level5_state.atira.render_id).SetLayer(2);

			obj_data.game_id = level5_state.atira.gameobj_id;
			obj_data.operation = shared::OBJECTOPERATION::CREATE;
			obj_data.position = graphics_manager.GetRenderRect(level5_state.atira.render_id).getPos();
			obj_data.rotation_angle = 0;
			obj_data.texture_id = graphics_manager.GetRenderRect(level5_state.atira.render_id).gettextureid();
			obj_data.layer = 2;
			obj_data.visible = true;
			obj_data.scale = graphics_manager.GetRenderRect(level5_state.atira.render_id).getScale();
			gameobject_network_requests_garuntee.push_back(obj_data);

			//spawn all the minions in a grid
			level5_state.minion_delay = 0;
			int dimension1 = 10;
			//rect= pos(10.5,0), scale(18,18)
			for (int x = 0; x < dimension1; x++)
			{
				float x_pos = 1.0 + 2.0 + ((float)x / (float)dimension1) * 18;
				for (int y = 0; y < dimension1; y++)
				{
					float y_pos = .3 + -8.5 + ((float)y / (float)dimension1) * 18;
					if (x == 0 && y == 0)
					{	
						x_pos = 1.5;
						y_pos = 0;
					}
					minion_s obj;
					obj.gameobj_id = gameobj_idhandler.getAvailableId();
					obj.render_id = graphics_manager.CreateRenderRect();
					graphics_manager.GetRenderRect(obj.render_id).Scale(glm::vec3(.65, .65, 0));
					if (x == 0 && y == 0)
						graphics_manager.GetRenderRect(obj.render_id).Scale(glm::vec3(.5, 2, 0));
					graphics_manager.GetRenderRect(obj.render_id).Translate(glm::vec3(x_pos, y_pos, 0));
					graphics_manager.GetRenderRect(obj.render_id).SetTexture(50);
					if (x == 0 && y == 0)
						graphics_manager.GetRenderRect(obj.render_id).SetTexture(57);
					graphics_manager.GetRenderRect(obj.render_id).SetLayer(3);
					obj.moving = false;
					level5_state.minions.push_back(obj);
					
					obj_data.game_id = obj.gameobj_id;
					obj_data.operation = shared::OBJECTOPERATION::CREATE;
					obj_data.position = graphics_manager.GetRenderRect(obj.render_id).getPos();
					obj_data.rotation_angle = 0;
					obj_data.texture_id = graphics_manager.GetRenderRect(obj.render_id).gettextureid();
					obj_data.layer = 2;
					obj_data.visible = true;
					obj_data.scale = graphics_manager.GetRenderRect(obj.render_id).getScale();
					gameobject_network_requests_garuntee.push_back(obj_data);
				}
			}

		}

		//gate
		if (level5_state.gate_1.activated && shared::rect_collision_test(graphics_manager, main_player.render_id, level5_state.gate_1.render_id))
		{
			glm::vec2 pos = graphics_manager.GetRenderRect(main_player.render_id).getPos();
			pos.y = graphics_manager.GetRenderRect(level5_state.gate_1.render_id).getPos().y +
				(graphics_manager.GetRenderRect(level5_state.gate_1.render_id).getScale().y / 2.0f) +
				(graphics_manager.GetRenderRect(main_player.render_id).getScale().y / 2.0f);
			graphics_manager.GetRenderRect(main_player.render_id).Translate(glm::vec3(pos, 0));

			if (std::signbit(main_player.dir.x)) {
				//bottom of gate
				main_player.click_pos = graphics_manager.GetRenderRect(level5_state.gate_1.render_id).getPos() +
					glm::vec2(-20.0, graphics_manager.GetRenderRect(level5_state.gate_1.render_id).getScale().y);
			}
			else {
				//top of gate
				main_player.click_pos = graphics_manager.GetRenderRect(level5_state.gate_1.render_id).getPos() +
					glm::vec2(20.0, graphics_manager.GetRenderRect(level5_state.gate_1.render_id).getScale().y);
			}
			main_player.moving_to_waypoint = true;
		}
		if (level5_state.gate_2.activated && shared::rect_collision_test(graphics_manager, main_player.render_id, level5_state.gate_2.render_id))
		{
			glm::vec2 pos = graphics_manager.GetRenderRect(main_player.render_id).getPos();
			pos.x = graphics_manager.GetRenderRect(level5_state.gate_2.render_id).getPos().x -
				(graphics_manager.GetRenderRect(level5_state.gate_2.render_id).getScale().x / 2.0f) -
				(graphics_manager.GetRenderRect(main_player.render_id).getScale().x / 2.0f);
			graphics_manager.GetRenderRect(main_player.render_id).Translate(glm::vec3(pos, 0));

			if (std::signbit(main_player.dir.y)) {
				//bottom of gate
				main_player.click_pos = graphics_manager.GetRenderRect(level5_state.gate_2.render_id).getPos() +
					glm::vec2(-graphics_manager.GetRenderRect(level5_state.gate_2.render_id).getScale().x / 2.0f,
						-graphics_manager.GetRenderRect(level5_state.gate_2.render_id).getScale().y);
			}
			else {
				//top of gate
				main_player.click_pos = graphics_manager.GetRenderRect(level5_state.gate_2.render_id).getPos() +
					glm::vec2(-graphics_manager.GetRenderRect(level5_state.gate_2.render_id).getScale().x / 2.0f,
						graphics_manager.GetRenderRect(level5_state.gate_2.render_id).getScale().y);
			}
			main_player.moving_to_waypoint = true;
		}
	}

	resolve_gameobjectslvl5boss(main_player);
}

void ServerGameApp::resolve_gameobjectslvl6(player& main_player)
{
	if (!level6_state.decorations[3].activated &&
		shared::point_in_rect_collision_test(graphics_manager, graphics_manager.GetRenderRect(main_player.render_id).getPos(), 
			                                 level6_state.decorations[3].render_id))
	{
		level6_state.decorations[3].activated = true;
		level6_state.button_1_count = 0;
		audio_network_requests.push_back(shared::SOUNDS::COOKING);

		shared::gameobject_network_s button_data;
		button_data.game_id = 3;
		button_data.operation = shared::OBJECTOPERATION::TEXTURE;
		button_data.texture_id = 10;
		gameobject_network_requests_garuntee.push_back(button_data);
	}

	//if you hit sc2 button with cd play sound
	if (!level6_state.decorations[11].activated &&
		shared::point_in_rect_collision_test(graphics_manager, graphics_manager.GetRenderRect(main_player.render_id).getPos(),
			level6_state.decorations[11].render_id))
	{
		level6_state.decorations[11].activated = true;
		level6_state.button_2_count = 0;
		audio_network_requests.push_back(shared::SOUNDS::DEAD);

		shared::gameobject_network_s button_data;
		button_data.game_id = 11;
		button_data.operation = shared::OBJECTOPERATION::TEXTURE;
		button_data.texture_id = 10;
		gameobject_network_requests_garuntee.push_back(button_data);
	}
}

void ServerGameApp::resolve_enemies(player& main_player)
{
	bool collision = false;
	for (const auto& enemy : current_enemies)
	{
		if (shared::rect_collision_test(graphics_manager, main_player.render_id, enemy.render_id, .15))
		{
			audio_network_requests.push_back(shared::SOUNDS::ZEALOT);
			collision = true;
			break;
		}
	}
	if (collision)
		playerDie(main_player);
	
}

void ServerGameApp::resolve_ground(player& main_player)
{
	bool collision = false;
	GROUNDTYPE old_ground = main_player.current_ground;
	for (const auto& ground : current_ground)
	{
		if (shared::point_in_rect_collision_test(graphics_manager, graphics_manager.GetRenderRect(main_player.render_id).getPos(), 
			                                     ground.render_id))
		{
			collision = true;
			main_player.current_ground = ground.type;
			break;
		}
	}

	if (collision)
	{
		if (old_ground == GROUNDTYPE::REVERSE && !main_player.onIce())
			main_player.click_pos = graphics_manager.GetRenderRect(main_player.render_id).getPos();

		if (main_player.current_ground == GROUNDTYPE::GRASS) {
			main_player.speed = main_player.GRASS_SPEED;
		}
		else if (main_player.current_ground == GROUNDTYPE::ICE) {
			main_player.speed = main_player.ICE_SPEED;
		}
		else if (main_player.current_ground == GROUNDTYPE::FAST) {
			main_player.speed = main_player.FASTICE_SPEED;
		}
		else if (main_player.current_ground == GROUNDTYPE::REVERSE) {
			main_player.speed = main_player.ICE_SPEED;
		}
		else if (main_player.current_ground == GROUNDTYPE::SLIDE) {
			main_player.speed = main_player.ICE_SPEED;
		}
		else if (main_player.current_ground == GROUNDTYPE::EXIT) {
			//go next level
		}
	}
	else
	{
		if (config::CLIENT_GOD && main_player.current_ground == GROUNDTYPE::SLIDE)
		{
			main_player.current_ground = GROUNDTYPE::ICE;
		}
		if(config::CLIENT_GOD)
			main_player.speed = main_player.ICE_SPEED;

		playerDie(main_player);
	}
}

void ServerGameApp::playerDie(player& main_player)
{
	if (config::CLIENT_GOD)
		return;
	if (current_level == 6)
	{
		Spawnplayer(main_player);
		main_player.moving_to_waypoint = false;
		switch (death_sound_track)
		{
		case 0: audio_network_requests.push_back(shared::SOUNDS::DEATH1); break;
		case 1: audio_network_requests.push_back(shared::SOUNDS::DEATH2); break;
		case 2: audio_network_requests.push_back(shared::SOUNDS::DEATH3); break;
		}
		death_sound_track = (death_sound_track + 1) % 3;
		return;
	}
	if (main_player.alive)
	{
		main_player.alive = false;
		main_player.moving_to_waypoint = false;
		main_player.dir = glm::vec2(0, 0);
		main_player.rot_angle = 0.0;

		graphics_manager.GetRenderRect(main_player.render_id).Rotate(glm::vec3(0, 0, 1), glm::degrees(0.0));
		graphics_manager.GetRenderRect(main_player.render_id).Reflect(false);
		graphics_manager.GetRenderRect(main_player.render_id).SetTexture(11);
		
		switch (death_sound_track)
		{
		case 0: audio_network_requests.push_back(shared::SOUNDS::DEATH1); break;
		case 1: audio_network_requests.push_back(shared::SOUNDS::DEATH2); break;
		case 2: audio_network_requests.push_back(shared::SOUNDS::DEATH3); break;
		}
		death_sound_track = (death_sound_track + 1) % 3;
	}
}

void ServerGameApp::playerRevive(player& main_player)
{
	audio_network_requests.push_back(shared::SOUNDS::REVIVE);

	main_player.alive = true;
	graphics_manager.GetRenderRect(main_player.render_id).SetTexture(4);

	Spawnplayer(main_player);
}

void ServerGameApp::ParseIncomingStream()
{
	/*
		packet parser loop (for each client): 
			. handle disconnect
			. handle network delivery
			. go through each packet and process
			. handle errors
	
		for each new packet: 
			. what to do if sent packet never got received
			. what to do if you receive an old packet

		to send packet:
			. write down packet id
			. write down packetopcode
			. writer data
		
		to receive:
			. read packet id
			. read packetopcode
			. read data
	*/

	//kick any disconnected clients
	for (int i = 0; i < client_proxys.size(); i++)
	{
		if (!client_proxys[i].connected)
		{
			KickClient(i);
			break;
		}
	}

	for (int i = 0; i < client_proxys.size(); i++)
	{
		//check for timeout disconnection
		if (client_proxys[i].connection_timer.TimeElapsed(CONNECTION_TIMEOUT))
		{
			std::cout << "client timed out\n";
			client_proxys[i].connected = false;
		}

		//send out acknowledgements
		if (client_proxys[i].network_reciever.anyacknowledgements() && client_proxys[i].ack_timer.TimeElapsed(client_proxys[i].ACK_SEND_TIME))
		{
			client_proxys[i].ack_timer.Begin();
			bitwriter.reset();
			client_proxys[i].network_sender.labelpacketfake(bitwriter);
			client_proxys[i].network_reciever.writeacknowledgementpacket(bitwriter);
			server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));
		}

		//check and handle timed out inflight packets
		bool resent = false;
		client_proxys[i].network_sender.checkinflightTimeout(bitwriter, server_socket, resent);
	}

	//go through each client and see if we disconnected
	while (true)
	{
		bitreader.reset();
		sockaddr_in recfrom_addr;
		int bytesReceived = server_socket->RecvFromNoError(bitreader.getData(), bitreader.getCapacityBytes(), recfrom_addr);
		int client = findClientwithAddr(recfrom_addr);
		bool client_unkown = client == -1;
		if (bytesReceived == 0)
		{
			std::cout << "read 0 bytes, not sure why this happened for UDP\n";
			if (!client_unkown)
			{
				client_proxys[client].connected = false;
			}
		}
		else if (bytesReceived < 0)
		{
			int err = WSAGetLastError();
			if (err != WSAEWOULDBLOCK) 
			{	
				std::cout << "error: " << err << std::endl;
				if (!client_unkown)
				{
					client_proxys[client].connected = false;
				}
			}
			break;
		}
		else
		{
			if (!client_unkown)
			{
				client_proxys[client].connection_timer.Begin();
			}
			//std::cout << "\nbytes received: " << bytesReceived << std::endl;
			while (bitreader.getbithead() < bytesReceived*8)
			{
				bool packet_valid = true;
				uint16_t packet_id;
				bitreader.Read(&packet_id);
				GAMEPACKETCODE code;
				bitreader.Read(&code);
				if (client_unkown && code != GAMEPACKETCODE::HELLO)
				{
					break;
				}
				if (code != GAMEPACKETCODE::HELLO && code != GAMEPACKETCODE::ACKNOWLEDGEMENT
					&& code != GAMEPACKETCODE::INPUT && code != GAMEPACKETCODE::LEVEL_LOAD &&
					code != GAMEPACKETCODE::CHAT && code != GAMEPACKETCODE::PING &&
					code != GAMEPACKETCODE::WELCOME)
				{
					std::cout << "received unexpected code from client\n";
					//throw everything away
					break;
				}

				if (!client_unkown)
					client_proxys[client].network_reciever.unlabelpacket(bitreader, packet_id, packet_valid);
				
				//std::cout << "code: " << (int)code << std::endl;
				if (packet_valid)
				{
					if (code == GAMEPACKETCODE::HELLO)
					{
						handleHelloPacket(client, recfrom_addr);
					}
					else if (code == GAMEPACKETCODE::WELCOME)
					{
						handleWelcomePacket(client, recfrom_addr);
					}
					else if (code == GAMEPACKETCODE::ACKNOWLEDGEMENT)
					{
						handleAcknowledgePacket(client);
					}
					else if (code == GAMEPACKETCODE::INPUT)
					{
						handleInputPacket(client, recfrom_addr);
					}
					else if (code == GAMEPACKETCODE::LEVEL_LOAD)
					{
						handlelevelloadpacket(client);
					}
					else if (code == GAMEPACKETCODE::CHAT)
					{
						handlechatpacket(client);
					}
					else if (code == GAMEPACKETCODE::PING)
					{
						handlepingpacket(client);
					}
				}
				else
				{
					//std::cout << "packet old\n";
					//packet isnt valid, either still run it or parse through it. Could cause bug with packet_id acks, you read it when it
					//thinks your not
					if (code == GAMEPACKETCODE::ACKNOWLEDGEMENT)
					{
						//doesnt matter if this is out of order, handle it
						handleAcknowledgePacket(client);
					}
					//fake parse packet if we want to handle multiple packets in the same stream
					break;
				}
			}
		}
	}
}

void ServerGameApp::KickClient(int client_id)
{
	std::cout << "kicking client!\n";

	//destroy player
	if (current_players.count(client_proxys[client_id].player_id) != 0)
	{
		current_players.erase(client_proxys[client_id].player_id);

		//notify every player of the deletion of this player and his leaving
		std::vector<shared::player_network_data_event_s> player_data;
		shared::player_network_data_event_s data;
		data.player_id = client_proxys[client_id].player_id;
		data.operation = shared::OBJECTOPERATION::DESTROY;
		player_data.push_back(data);
		for (auto& client : client_proxys)
		{
			bitwriter.reset();
			client.network_sender.labelpacketGarunteeBegin(bitwriter);
			WriteGamePacket_playersevent(bitwriter, player_data);
			server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));
			client.network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));
		}
	}
	client_proxys.erase(client_proxys.begin() + client_id);
	
	player_count--;

	//if someone left while level switching maybe we can start
	if (level_switching)
	{
		if (players_switched == player_count)
		{
			std::cout << "player left while switching and we are ready to simulate next level " << current_level << std::endl;
			LevelSwitchDone();
		}
	}
	//if someone left while welcoming everyone
	std::cout << client_proxys.size() << " " << player_count << std::endl;
	if (loading_game && client_proxys.size() >= player_count)
	{
		std::cout << "someone left while launching!\n";
		//send each client the launch packet with all the player info and other stuff
		for (auto& client : client_proxys)
		{
			//if someone left while welcoming everyone
			if (client.welcoming == 0)
			{
				//client.welcoming = 1;

				//bitwriter.reset();
				//client.network_sender.labelpacketGarunteeBegin(bitwriter);
				//WriteGamePacket_launch(bitwriter);
				//server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));
				//client.network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));
			}
			//if someone left while launching everyone
			else if (client.welcoming == 2)
			{
				int clients_loaded = 0;
				for (auto& client : client_proxys)
				{
					if (client.welcoming == 2)
						clients_loaded++;
				}
				//if all players sent welcome we are ready to launchGame and simulate
				if (clients_loaded >= player_count)
					launchGame();
			}
		}
	}
}

void ServerGameApp::handlepingpacket(int client)
{
	float val;
	int timestamp;
	ReadGamePacket_ping(bitreader, timestamp, val);

	client_proxys[client].ping = Timer::getTimeSinceEpoch(client_proxys[client].ping_timer.getCurrentTime()) - timestamp;
}

void ServerGameApp::handlechatpacket(int client)
{
	std::cout << "sending chats\n";
	std::string msg;
	ReadGamePacket_chat(bitreader, msg);
	std::string new_msg = std::to_string(client_proxys[client].player_id) + ": ";
	new_msg += msg;
	if (chat_msgs.size() == 8)
		chat_msgs.pop();
	chat_msgs.push(new_msg);
	
	for (auto& client : client_proxys)
	{
		bitwriter.reset();
		client.network_sender.labelpacketGarunteeBegin(bitwriter);
		WriteGamePacket_serverchats(bitwriter, chat_msgs);
		server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));
		client.network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));
	}
}

void ServerGameApp::handleHelloPacket(int client, sockaddr_in addr)
{
	if (client == -1)
	{
		std::cout << "adding player \n";
		printSocketAddr(addr);

		clientproxy_s proxy;
		proxy.client_addr = addr;
		proxy.connected = true;
		proxy.connection_timer.Begin();
		proxy.ack_timer.Begin();
		proxy.welcoming = 0;
		client_proxys.push_back(proxy);
		std::cout << client_proxys.size() << std::endl;
		bitwriter.reset();
		client_proxys.back().network_sender.labelpacketGarunteeBegin(bitwriter);
		WriteGamePacket_welcome(bitwriter);
		server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&proxy.client_addr));
		client_proxys.back().network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&proxy.client_addr));
	
		//if all players sent welcome let clients launch their game, we wait until they have all launched before launching
		//ourselves and sending game state data
		if (client_proxys.size() >= player_count)
		{
			sendLaunchData();
		}	
	}
	else
	{
		//std::cout << "client already exists don't handle hello packet\n";
	}
}

void ServerGameApp::handleWelcomePacket(int client, sockaddr_in addr)
{
	if (client_proxys[client].welcoming == 1)
	{
		client_proxys[client].welcoming = 2;


		int clients_loaded = 0;
		for (auto& client : client_proxys)
		{
			if (client.welcoming == 2)
				clients_loaded++;
		}
		//if all players sent welcome we are ready to launchGame and simulate
		if (clients_loaded >= player_count)
			launchGame();
	}
	else
	{
		std::cout << "client already welcomed back\n";
	}
}

void ServerGameApp::handlelevelloadpacket(int client)
{
	int level;
	uint32_t uid;
	ReadGamePacket_levelload(bitreader, level, uid);

	if (level_switching)
	{
		if (client_proxys[client].switching_levels == true)
		{
			client_proxys[client].switching_levels = false;
			players_switched++;
			if (players_switched == player_count)
			{
				//std::cout << "all players switched levels starting simulation of level " << current_level << std::endl;
				LevelSwitchDone();
			}
		}
		else
		{
			std::cout << "client already switched levels ignoring...\n";
		}
	}
	else
	{
		std::cout << "got levelload packet from client when were not level switching??\n";
	}
}

void ServerGameApp::handleInputPacket(int client, sockaddr_in addr)
{
	my_assert(client != -1, "handleInputPacket from invalid client, aborting");

	shared::input_data_s old_state = client_proxys[client].input_data;
	ReadGamePacket_input(bitreader, client_proxys[client].input_data);

	if (current_players.count(client_proxys[client].player_id) != 0)
	{
		player& the_player = current_players[client_proxys[client].player_id];
		the_player.ability_e = client_proxys[client].input_data.key_e;
		the_player.ability_f = client_proxys[client].input_data.key_f;
		the_player.ability_t = client_proxys[client].input_data.key_t;
		the_player.ability_s = client_proxys[client].input_data.key_s;
		//the_player.ability_r = client_proxys[client].input_data.key_f;

		//if newer input add, else ignore if older input
		if (client_proxys[client].input_data.timestamp >= old_state.timestamp)
		{
			the_player.ability_mr = client_proxys[client].input_data.mouse_right;
			the_player.ability_mx = client_proxys[client].input_data.world_pos_click.x;
			the_player.ability_my = client_proxys[client].input_data.world_pos_click.y;
		}
		else
		{
			//std::cout << "older input packet received\n";
		}
	}
}

void ServerGameApp::handleAcknowledgePacket(int client)
{
	my_assert(client != -1, "acknowledgement packet from invalid client, aborting");
	bool resent = false;
	client_proxys[client].network_sender.handleacknowledgementsPacket(bitreader, bitwriter, server_socket, resent);
}

void ServerGameApp::sendLaunchData()
{
	//create the banelings, assign everyone thier ids, and give client their unique baneling id
	uint32_t current_id = 0;
	for (auto& client : client_proxys)
	{
		player new_player;
		new_player.Init();
		new_player.player_id = current_id;
		shared::LoadBaneling(graphics_manager, new_player.render_id);

		current_players.emplace(current_id, new_player);

		client.player_id = current_id;

		bitwriter.reset();
		client.network_sender.labelpacketGarunteeBegin(bitwriter);
		WriteGamePacket_playerunique(bitwriter, current_id);
		server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));
		client.network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));

		current_id++;
	}

	//send playerevent packet that banelings have been created so they can initialize
	std::vector<shared::player_network_data_event_s> player_data;
	for (auto itr = current_players.begin(); itr != current_players.end(); itr++)
	{
		shared::player_network_data_event_s data;
		data.operation = shared::OBJECTOPERATION::CREATE;
		data.player_id = itr->second.player_id;
		player_data.push_back(data);
	}

	for (auto& client : client_proxys)
	{
		bitwriter.reset();
		client.network_sender.labelpacketGarunteeBegin(bitwriter);
		WriteGamePacket_playersevent(bitwriter, player_data);
		server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));
		client.network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));
	}

	//send each client the launch packet with all the player info and other stuff
	for (auto& client : client_proxys)
	{
		client.welcoming = 1;

		bitwriter.reset();
		client.network_sender.labelpacketGarunteeBegin(bitwriter);
		WriteGamePacket_launch(bitwriter);
		server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));
		client.network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client.client_addr));
	}
}

void ServerGameApp::launchGame()
{
	std::cout << "launching game!\n";

	//load up game
	current_level = 1;
	LoadLevel1();
	for (auto itr = current_players.begin(); itr != current_players.end(); itr++)
	{
		Spawnplayer(itr->second);
	}

	launch_delay_timer.Begin();
}

void ServerGameApp::Spawnplayer(player& main_player)
{
	glm::vec2 spawn_location;
	switch (current_level)
	{
		case 1: spawn_location = level1_state.spawn_location; break;
		case 2: spawn_location = level2_state.spawn_location; break;
		case 3: spawn_location = level3_state.spawn_location; break;
		case 4: spawn_location = level4_state.spawn_location; break;
		case 5: spawn_location = level5_state.spawn_location; break;
		case 6: spawn_location = level6_state.spawn_location; break;
	}
	
	float radius = 0.5f;
	glm::vec2 circle_point = glm::vec2(cos(glm::radians(spawn_angle)), sin(glm::radians(spawn_angle))) * glm::vec2(radius, radius);
	graphics_manager.GetRenderRect(main_player.render_id).Translate(glm::vec3(spawn_location + circle_point, 0));
	spawn_angle += (360.0 / player_count);
}

void ServerGameApp::LoadLevel1GameObjects()
{
	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel1GameObjects(graphics_manager, gameobj_ids);

	//my_assert(gameobj_ids.size() == 2, "level1 objects mismatch");
	int gameobj_id = 0;
	level1_state.buttons.clear();

	game_object object;
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level1_state.buttons.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level1_state.buttons.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level1_state.buttons.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level1_state.buttons.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level1_state.buttons.push_back(object);

	level1_state.gate_1.activated = true;
	level1_state.gate_1.render_id = gameobj_ids[gameobj_id++];
}

void ServerGameApp::LoadLevel1Ground()
{
	std::vector<uint32_t> ground_ids;
	shared::LoadLevel1Ground(graphics_manager, ground_ids);

	//my_assert(ground_ids.size() == 8, "level 1 ground mismatch");
	int ground_id = 0;

	ground ground_start;
	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//ice
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
	level1_state.enemy_rect_boundary = ground_start.render_id;

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//reverse
	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//fast
	ground_start.type = GROUNDTYPE::FAST;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::FAST;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::FAST;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
	
	ground_start.type = GROUNDTYPE::FAST;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::FAST;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//exit
	ground_start.type = GROUNDTYPE::EXIT;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//slide ice
	ground_start.type = GROUNDTYPE::SLIDE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::SLIDE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
}

void ServerGameApp::LoadLevel1Enemies()
{
	std::vector<uint32_t> enemy_ids;
	shared::LoadLevel1Enemies(graphics_manager, enemy_ids);

	//my_assert(enemy_ids.size() == 4, "level 1 enemy mismatch");
	int enemy_id = 0;

	enemy_s enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(0, .01), 6.5);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_random(enemy_ids[enemy_id++], glm::vec2(.01, .01), 1.0, .10, level1_state.enemy_rect_boundary);
	current_enemies.push_back(enemy_tmp);

	//fast ice
	std::vector<enemy_strip_s> strips;
	for (int i = 0; i < 4; i++)
	{
		enemy_strip_s strip;
		if (i == 0 || i == 2)
			strip.distance = 2.5;
		else
			strip.distance = 1.0;
		strip.distance_measure = 0.0;
		float speed = .01;
		switch (i)
		{
		case 0: strip.speed = glm::vec2(-speed, 0); break;
		case 1: strip.speed = glm::vec2(0, speed*3); break;
		case 2: strip.speed = glm::vec2(speed*1, 0); break;
		case 3: strip.speed = glm::vec2(0, -speed*5); break;
		}
		strips.push_back(strip);
	}
	enemy_tmp = enemy_s::make_walk_strip(enemy_ids[enemy_id++], strips, false);
	current_enemies.push_back(enemy_tmp);

	glm::vec2 pos(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.5, .5), pos.x, pos.y, 3.0, 3.0);
	current_enemies.push_back(enemy_tmp);
	
	//reverse ice
	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.5, .5), pos.x, pos.y, 2.0, 2.0, 0.0f);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.5, .5), pos.x, pos.y, 2.0, 2.0, 90.0f);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.5, .5), pos.x, pos.y, 2.0, 2.0, 180.0f);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.5, .5), pos.x, pos.y, 2.0, 2.0, 270.0f);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.5, .5), pos.x, pos.y, 2.0, 2.0, 45.0f);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.5, .5), pos.x, pos.y, 2.0, 2.0, 135.0f);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(0, -.011), 4);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(0, .008), 4);
	current_enemies.push_back(enemy_tmp);

	//slide ice
	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(-.02, 0.0), 4);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(-.01, 0.0), 4);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(.01, 0.0), 4);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(-.025, 0.0), 3);
	current_enemies.push_back(enemy_tmp);
}

void ServerGameApp::LoadLevel1()
{
	level1_state.spawn_location = glm::vec2(-1, 0);
	LoadLevel1Ground();
	LoadLevel1Enemies();
	LoadLevel1GameObjects();
}

void ServerGameApp::UnLoadLevel1()
{
	//unload game objects
	for(int i = 0;i< level1_state.buttons.size();i++)
		graphics_manager.RemoveRenderRect(level1_state.buttons[i].render_id);
	graphics_manager.RemoveRenderRect(level1_state.gate_1.render_id);
}


void ServerGameApp::LoadLevel2()
{
	level2_state.spawn_location = glm::vec2(0, 0);
	LoadLevel2Ground();
	LoadLevel2Enemies();
	LoadLevel2GameObjects();
}

void ServerGameApp::UnLoadLevel2()
{
	//unload game objects
	//none
}

void ServerGameApp::LoadLevel2Enemies()
{
	std::vector<uint32_t> enemy_ids;
	shared::LoadLevel2Enemies(graphics_manager, enemy_ids);

	//my_assert(enemy_ids.size() == 1, "level 2 enemy mismatch");
	int enemy_id = 0;

	enemy_s enemy_tmp;
	glm::vec2 pos;
	

	//first line
	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, 0.0, 0.0, 45.0f);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, 0.75, 0.75, 0.0f);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, 1.5, 1.5, 0.0f);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, 0.75, 0.75, 180.0f);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, 1.5, 1.5, 180.0f);
	current_enemies.push_back(enemy_tmp);

	//second line
	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, 0, 0, 0.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, 1.7, 1.7, 0.0f, false);
	current_enemies.push_back(enemy_tmp);
	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, .75, .75, 0.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, 1.7, 1.7, 120.0f, false);
	current_enemies.push_back(enemy_tmp);
	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, .75, .75, 120.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, 1.7, 1.7, 240.0f, false);
	current_enemies.push_back(enemy_tmp);
	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.9, .9), pos.x, pos.y, .75, .75, 240.0f, false);
	current_enemies.push_back(enemy_tmp);

	//littel ring
	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.7, .7), pos.x, pos.y, 0, 0, 0.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.7, .7), pos.x, pos.y, .5, .5, 0.0f, false);
	current_enemies.push_back(enemy_tmp);
	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(.7, .7), pos.x, pos.y, 1, 1, 0.0f, false);
	current_enemies.push_back(enemy_tmp);

	//big ring
	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(1.2, 1.2), pos.x, pos.y, 1.25, 1.25, 0.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(1.2, 1.2), pos.x, pos.y, 1.25, 1.25, 40.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(1.2, 1.2), pos.x, pos.y, 1.25, 1.25, 80.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(1.2, 1.2), pos.x, pos.y, 1.25, 1.25, 120.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(1.2, 1.2), pos.x, pos.y, 1.25, 1.25, 160.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(1.2, 1.2), pos.x, pos.y, 1.25, 1.25, 200.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(1.2, 1.2), pos.x, pos.y, 1.25, 1.25, 240.0f, false);
	current_enemies.push_back(enemy_tmp);

	//reverse bugs
	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(-.01, .01), 6.5);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(.01, .0), 4);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(.01, .0), 6);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(.0075, .0), 3);
	current_enemies.push_back(enemy_tmp);

	//randoms
	enemy_tmp = enemy_s::make_random(enemy_ids[enemy_id++], glm::vec2(.005, .005), 1.5, .10, level2_state.enemy_rect_boundary_1);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_random(enemy_ids[enemy_id++], glm::vec2(.005, .005), 1.5, .10, level2_state.enemy_rect_boundary_2);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_random(enemy_ids[enemy_id++], glm::vec2(.005, .005), 1.5, .05, level2_state.enemy_rect_boundary_2);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_random(enemy_ids[enemy_id++], glm::vec2(.005, .005), 1.5, .05, level2_state.enemy_rect_boundary_2);
	current_enemies.push_back(enemy_tmp);
}
void ServerGameApp::LoadLevel2Ground()
{
	std::vector<uint32_t> ground_ids;
	shared::LoadLevel2Ground(graphics_manager, ground_ids);

	//my_assert(ground_ids.size() == 2, "level 2 ground mismatch");
	int ground_id = 0;

	//start
	ground ground_start;
	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//right lane
	ground_start;
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start;
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start;
	ground_start.type = GROUNDTYPE::SLIDE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start;
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//across lane
	ground_start;
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start;
	ground_start.type = GROUNDTYPE::SLIDE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//left lane
	ground_start;
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start;
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start;
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start;
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//ground end
	ground_start;
	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//Reverse Area
	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
	level2_state.enemy_rect_boundary_2 = ground_start.render_id;

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
	level2_state.enemy_rect_boundary_1 = ground_start.render_id;

	ground_start.type = GROUNDTYPE::EXIT;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
}
void ServerGameApp::LoadLevel2GameObjects()
{
	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel2GameObjects(graphics_manager, gameobj_ids);

	//my_assert(gameobj_ids.size() == 0, "level 2 objects mismatch");
	//no game objects
}

void ServerGameApp::LoadLevel3()
{
	level3_state.spawn_location = glm::vec2(-1, 0);
	LoadLevel3Ground();
	LoadLevel3Enemies();
	LoadLevel3GameObjects();
}

void ServerGameApp::UnLoadLevel3()
{
	//unload game objects
}


void ServerGameApp::LoadLevel3Enemies()
{
	std::vector<uint32_t> enemy_ids;
	shared::LoadLevel3Enemies(graphics_manager, enemy_ids);

	//my_assert(enemy_ids.size() == 1, "level 3 enemy mismatch");
	int enemy_id = 0;
	enemy_s enemy_tmp;
	glm::vec2 pos;

	//first random group
	int dimension1 = 6*6;
	for(int i = 0;i< dimension1;i++)	
	{
		enemy_tmp = enemy_s::make_random(enemy_ids[enemy_id++], glm::vec2(.005, .005), 1.5, .05, level3_state.enemy_rect_boundary_1);
		current_enemies.push_back(enemy_tmp);
	}
	
	//spinning

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(1.2, 1.2), pos.x, pos.y, 2.25, 2.25, 0.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(1.2, 1.2), pos.x, pos.y, 2.25, 2.25, 90.0f, false);
	current_enemies.push_back(enemy_tmp);

	pos = glm::vec2(graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().x, graphics_manager.GetRenderRect(enemy_ids[enemy_id]).getPos().y);
	enemy_tmp = enemy_s::make_ellipse(enemy_ids[enemy_id++], glm::vec2(1.2, 1.2), pos.x, pos.y, 2.25, 2.25, 240.0f, true);
	current_enemies.push_back(enemy_tmp);

	//big guy
	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(0.01, 0.0), 6);
	current_enemies.push_back(enemy_tmp);
	
	//second random group
	int dimension2 = 10 * 10;
	for (int i = 0; i < dimension2; i++)
	{
		enemy_tmp = enemy_s::make_random(enemy_ids[enemy_id++], glm::vec2(.005, .005), 1.5, .05, level3_state.enemy_rect_boundary_2);
		current_enemies.push_back(enemy_tmp);
	}

}
void ServerGameApp::LoadLevel3Ground()
{
	std::vector<uint32_t> ground_ids;
	shared::LoadLevel3Ground(graphics_manager, ground_ids);

	//my_assert(ground_ids.size() == 2, "level 3 ground mismatch");
	int ground_id = 0;

	//grass
	ground ground_start;
	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
	level3_state.enemy_rect_boundary_1 = ground_start.render_id;

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//ice
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::FAST;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
	level3_state.enemy_rect_boundary_2 = ground_start.render_id;
	//exit
	ground_start.type = GROUNDTYPE::EXIT;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//secret
	ground_start.type = GROUNDTYPE::FAST;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::FAST;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
}
void ServerGameApp::LoadLevel3GameObjects()
{
	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel3GameObjects(graphics_manager, gameobj_ids);

	//my_assert(gameobj_ids.size() == 0, "level 3 objects mismatch");
}

void ServerGameApp::LoadLevel4GameObjects()
{
	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel4GameObjects(graphics_manager, gameobj_ids);

	my_assert(gameobj_ids.size() == 0, "level 4 objects mismatch");
	//none
}

void ServerGameApp::LoadLevel5Enemies()
{
	std::vector<uint32_t> enemy_ids;
	shared::LoadLevel5Enemies(graphics_manager, enemy_ids);

	//my_assert(enemy_ids.size() == 1, "level 5 enemy mismatch");
	int enemy_id = 0;
	enemy_s enemy_tmp;
	glm::vec2 pos;

	//none all dynamic objects
}
void ServerGameApp::LoadLevel5Ground()
{
	std::vector<uint32_t> ground_ids;
	shared::LoadLevel5Ground(graphics_manager, ground_ids);

	//my_assert(ground_ids.size() == 2, "level 5 ground mismatch");
	int ground_id = 0;

	//grass
	ground ground_start;
	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//arena
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
	level5_state.minion_rect_boundary = ground_start.render_id;

	//escape area
	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::SLIDE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::FAST;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::SLIDE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
	//pathway and exit
	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::EXIT;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
}
void ServerGameApp::LoadLevel5GameObjects()
{
	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel5GameObjects(graphics_manager, gameobj_ids);

	//my_assert(gameobj_ids.size() == 0, "level 5 objects mismatch");
	int gameobj_id = 0;

	//4 boss buttons
	level5_state.buttons.clear();
	level5_state.decorations.clear();

	game_object object;
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.buttons.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.buttons.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.buttons.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.buttons.push_back(object);

	//gate
	object.activated = true;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.gate_1 = object;

	object.activated = true;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.gate_2 = object;

	//extra button
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.end_button = object;

	//decorations
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.decorations.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.decorations.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.decorations.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.decorations.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.decorations.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.decorations.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.decorations.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.decorations.push_back(object);

	//desk
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.decorations.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.decorations.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level5_state.decorations.push_back(object);
}

void ServerGameApp::LoadLevel6Enemies()
{
	std::vector<uint32_t> enemy_ids;
	shared::LoadLevel6Enemies(graphics_manager, enemy_ids);

	//my_assert(enemy_ids.size() == 1, "level 6 enemy mismatch");
	int enemy_id = 0;
	enemy_s enemy_tmp;
	glm::vec2 pos;
}

void ServerGameApp::LoadLevel6Ground()
{
	std::vector<uint32_t> ground_ids;
	shared::LoadLevel6Ground(graphics_manager, ground_ids);

	//my_assert(ground_ids.size() == 2, "level 6 ground mismatch");
	int ground_id = 0;

	//grass
	ground ground_start;
	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//fast
	ground_start.type = GROUNDTYPE::FAST;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	//reverse
	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
}
void ServerGameApp::LoadLevel6GameObjects()
{
	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel6GameObjects(graphics_manager, gameobj_ids);

	//my_assert(gameobj_ids.size() == 0, "level 6 objects mismatch");
	int gameobj_id = 0;
	game_object object;

	//dance floor
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);
	
	//jaccuzzi
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);

	//avilo
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);

	//button - index 3
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);

	//trees
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);

	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);
	
	//notebook
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);
	
	//ice baneling statue
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);
	
	//server
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);
	
	//sc2
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);

	//sc2 button - 11
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);

	//you escaped
	object.activated = false;
	object.render_id = gameobj_ids[gameobj_id++];
	level6_state.decorations.push_back(object);
}

void ServerGameApp::LoadLevel4Ground()
{
	std::vector<uint32_t> ground_ids;
	shared::LoadLevel4Ground(graphics_manager, ground_ids);

	//my_assert(ground_ids.size() == 2, "level 4 ground mismatch");
	int ground_id = 0;

	ground ground_start;
	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
	
	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::SLIDE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::SLIDE;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
	
	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::EXIT;
	ground_start.render_id = ground_ids[ground_id++];
	current_ground.push_back(ground_start);
}

void ServerGameApp::LoadLevel4Enemies()
{
	std::vector<uint32_t> enemy_ids;
	shared::LoadLevel4Enemies(graphics_manager, enemy_ids);

	//my_assert(enemy_ids.size() == 1, "level 4 enemy mismatch");
	int enemy_id = 0;

	std::vector<enemy_strip_s> strips;
	for (int i = 0; i < 4; i++)
	{
		enemy_strip_s strip;
		strip.distance = 4.0;
		strip.distance_measure = 0.0;
		float speed = .01;
		switch (i)
		{
		case 0: strip.speed = glm::vec2(0, speed); break;
		case 1: strip.speed = glm::vec2(-speed, 0); break;
		case 2: strip.speed = glm::vec2(0, -speed); break;
		case 3: strip.speed = glm::vec2(speed, 0); break;
		}
		strips.push_back(strip);
	}
	enemy_s enemy_tmp = enemy_s::make_walk_strip(enemy_ids[enemy_id++], strips, false);
	current_enemies.push_back(enemy_tmp);

	//reverse ice
	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(-.005, .01), 4);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(-.005, .01), 4);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(-.01, 0.0), 4);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(-.01, -0.01), 4);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(.01, 0.01), 4);
	current_enemies.push_back(enemy_tmp);

	//bears
	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(0, -0.01), 5);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(-0.01, 0), 3);
	current_enemies.push_back(enemy_tmp);

	//dogs
	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(0, 0.01), 1.5);
	current_enemies.push_back(enemy_tmp);

	strips.clear();
	for (int i = 0; i < 4; i++)
	{
		enemy_strip_s strip;
		if (i == 0 || i == 2)
			strip.distance = 2.0;
		else
			strip.distance = .5;
		strip.distance_measure = 0.0;
		float speed = .01;
		switch (i)
		{
		case 0: strip.speed = glm::vec2(0, speed); break;
		case 1: strip.speed = glm::vec2(-speed, 0); break;
		case 2: strip.speed = glm::vec2(0, -speed); break;
		case 3: strip.speed = glm::vec2(speed, 0); break;
		}
		strips.push_back(strip);
	}
	enemy_tmp = enemy_s::make_walk_strip(enemy_ids[enemy_id++], strips, false);
	current_enemies.push_back(enemy_tmp);

	strips.clear();
	for (int i = 0; i < 4; i++)
	{
		enemy_strip_s strip;
		if (i == 0 || i == 2)
			strip.distance = 2.0;
		else
			strip.distance = .5;
		strip.distance_measure = 0.0;
		float speed = .01;
		switch (i)
		{
		case 0: strip.speed = glm::vec2(0, speed); break;
		case 1: strip.speed = glm::vec2(speed, 0); break;
		case 2: strip.speed = glm::vec2(0, -speed); break;
		case 3: strip.speed = glm::vec2(-speed, 0); break;
		}
		strips.push_back(strip);
	}
	enemy_tmp = enemy_s::make_walk_strip(enemy_ids[enemy_id++], strips, false);
	current_enemies.push_back(enemy_tmp);

	strips.clear();
	for (int i = 0; i < 5; i++)
	{
		enemy_strip_s strip;
		if (i == 0)
			strip.distance = 1.0;
		else if (i == 2)
			strip.distance = 2.0;
		else if (i == 4)
			strip.distance = 1.0;
		else
			strip.distance = .5;
		strip.distance_measure = 0.0;
		float speed = .01;
		switch (i)
		{
		case 0: strip.speed = glm::vec2(0, speed); break;
		case 1: strip.speed = glm::vec2(-speed, 0); break;
		case 2: strip.speed = glm::vec2(0, -speed); break;
		case 3: strip.speed = glm::vec2(speed, 0); break;
		case 4: strip.speed = glm::vec2(0, speed); break;
		}
		strips.push_back(strip);
	}
	enemy_tmp = enemy_s::make_walk_strip(enemy_ids[enemy_id++], strips, false);
	current_enemies.push_back(enemy_tmp);

	strips.clear();
	for (int i = 0; i < 5; i++)
	{
		enemy_strip_s strip;
		if (i == 0)
			strip.distance = 1.0;
		else if (i == 2)
			strip.distance = 2.0;
		else if (i == 4)
			strip.distance = 1.0;
		else
			strip.distance = .5;
		strip.distance_measure = 0.0;
		float speed = .01;
		switch (i)
		{
		case 0: strip.speed = glm::vec2(0, speed); break;
		case 1: strip.speed = glm::vec2(speed, 0); break;
		case 2: strip.speed = glm::vec2(0, -speed); break;
		case 3: strip.speed = glm::vec2(-speed, 0); break;
		case 4: strip.speed = glm::vec2(0, speed); break;
		}
		strips.push_back(strip);
	}
	enemy_tmp = enemy_s::make_walk_strip(enemy_ids[enemy_id++], strips, false);
	current_enemies.push_back(enemy_tmp);

	strips.clear();
	for (int i = 0; i < 4; i++)
	{
		enemy_strip_s strip;
		if (i == 1 || i == 3)
			strip.distance = 2.0;
		else
			strip.distance = .5;
		strip.distance_measure = 0.0;
		float speed = .01;
		switch (i)
		{
		case 0: strip.speed = glm::vec2(-speed, 0); break;
		case 1: strip.speed = glm::vec2(0, -speed); break;
		case 2: strip.speed = glm::vec2(speed, 0); break;
		case 3: strip.speed = glm::vec2(0, speed); break;
		}
		strips.push_back(strip);
	}
	enemy_tmp = enemy_s::make_walk_strip(enemy_ids[enemy_id++], strips, false);
	current_enemies.push_back(enemy_tmp);

	strips.clear();
	for (int i = 0; i < 4; i++)
	{
		enemy_strip_s strip;
		if (i == 1 || i == 3)
			strip.distance = 2.0;
		else
			strip.distance = .5;
		strip.distance_measure = 0.0;
		float speed = .01;
		switch (i)
		{
		case 0: strip.speed = glm::vec2(speed, 0); break;
		case 1: strip.speed = glm::vec2(0, -speed); break;
		case 2: strip.speed = glm::vec2(-speed, 0); break;
		case 3: strip.speed = glm::vec2(0, speed); break;
		}
		strips.push_back(strip);
	}
	enemy_tmp = enemy_s::make_walk_strip(enemy_ids[enemy_id++], strips, false);
	current_enemies.push_back(enemy_tmp);
	//big guys
	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(0, -0.01), 2.8);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(0, 0.01), 2.8);
	current_enemies.push_back(enemy_tmp);

	//dogs
	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(-.01, 0.005), 2.5);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(.01, -.01), 2);
	current_enemies.push_back(enemy_tmp);

	//rest
	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(.01, 0.0), 3.5);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(.01, 0.0), 3);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(.01, 0.0), 3.75);
	current_enemies.push_back(enemy_tmp);

	//last dogs
	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(0.0, -0.02), 3);
	current_enemies.push_back(enemy_tmp);

	enemy_tmp = enemy_s::make_simple_patrol(enemy_ids[enemy_id++], glm::vec2(0.0, -0.02), 2.2);
	current_enemies.push_back(enemy_tmp);
}

void ServerGameApp::LoadLevel4()
{
	level4_state.spawn_location = glm::vec2(-1, 0);
	LoadLevel4Ground();
	LoadLevel4Enemies();
	LoadLevel4GameObjects();
}

void ServerGameApp::UnLoadLevel4()
{
	//unload game objects
	//none
}

void ServerGameApp::LoadLevel5()
{
	level5_state.spawn_location = glm::vec2(-5, 0);
	level5_state.init();
	LoadLevel5Ground();
	LoadLevel5Enemies();
	LoadLevel5GameObjects();
}

void ServerGameApp::UnLoadLevel5()
{
	//unload game objects
	for (int i = 0; i < level5_state.buttons.size(); i++)
	{
		graphics_manager.RemoveRenderRect(level5_state.buttons[i].render_id);
	}
	level5_state.buttons.clear();

	graphics_manager.RemoveRenderRect(level5_state.gate_1.render_id);
	graphics_manager.RemoveRenderRect(level5_state.gate_2.render_id);
	graphics_manager.RemoveRenderRect(level5_state.end_button.render_id);

	for (int i = 0; i < level5_state.decorations.size(); i++)
	{
		graphics_manager.RemoveRenderRect(level5_state.decorations[i].render_id);
	}
	level5_state.decorations.clear();

	for (int i = 0; i < level5_state.fists.size(); i++)
	{
		graphics_manager.RemoveRenderRect(level5_state.fists[i].render_id);
	}
	level5_state.fists.clear();

	for (int i = 0; i < level5_state.kongfus.size(); i++)
	{
		graphics_manager.RemoveRenderRect(level5_state.kongfus[i].render_id);
	}
	level5_state.kongfus.clear();

	for (int i = 0; i < level5_state.bombs.size(); i++)
	{
		graphics_manager.RemoveRenderRect(level5_state.bombs[i].render_id);
	}
	level5_state.bombs.clear();
	
	for (int i = 0; i < level5_state.minions.size(); i++)
	{
		graphics_manager.RemoveRenderRect(level5_state.minions[i].render_id);
	}
	level5_state.minions.clear();

	for (int i = 0; i < level5_state.minions_stage1.size(); i++)
	{
		graphics_manager.RemoveRenderRect(level5_state.minions_stage1[i].render_id);
	}
	level5_state.minions_stage1.clear();

	if (level5_state.boss_triggered)
		graphics_manager.RemoveRenderRect(level5_state.avilo.render_id);

	if(level5_state.cop_on)
		graphics_manager.RemoveRenderRect(level5_state.cop_car.render_id);

	if(level5_state.hammer_created)
		graphics_manager.RemoveRenderRect(level5_state.button_hammer.render_id);
	
	if(level5_state.atira_created)
		graphics_manager.RemoveRenderRect(level5_state.atira.render_id);
}

void ServerGameApp::LoadLevel6()
{
	level6_state.spawn_location = glm::vec2(0, 0);
	LoadLevel6Ground();
	LoadLevel6Enemies();
	LoadLevel6GameObjects();
}

void ServerGameApp::UnLoadLevel6()
{
	for (int i = 0; i < level6_state.decorations.size(); i++)
	{
		graphics_manager.RemoveRenderRect(level6_state.decorations[i].render_id);
	}
	level6_state.decorations.clear();
}

void ServerGameApp::UnLoadGeneral()
{
	for (auto& x : current_ground) 
	{
		graphics_manager.RemoveRenderRect(x.render_id);
	}
	current_ground.clear();
	
	for (auto& x : current_enemies) 
	{
		graphics_manager.RemoveRenderRect(x.render_id);
	}
	current_enemies.clear();
	
	for (auto& x : current_goo) 
	{
		graphics_manager.RemoveRenderRect(x.render_id);
	}
	current_goo.clear();
	
	for (auto& x : current_disks) 
	{
		graphics_manager.RemoveRenderRect(x.render_id);
	}
	current_disks.clear();

	//gameobj_idhandler.init();
}

void ServerGameApp::SwitchLevels(int new_level)
{
	if (level_switching) {
		//std::cout << "already switching level!\n";
		return;
	}
	//unload and load new level state
	UnLoadGeneral();
	switch (current_level)
	{
		case 1: UnLoadLevel1(); break;
		case 2: UnLoadLevel2(); break;
		case 3: UnLoadLevel3(); break;
		case 4: UnLoadLevel4(); break;
		case 5: UnLoadLevel5(); break;
		case 6: UnLoadLevel6(); break;
		default: std::cout << "tryint to unload unknown level " << current_level << std::endl;
	}

	//std::cout << "switching levels from " << current_level << " to " << new_level << std::endl;
	current_level = new_level;

	switch (new_level)
	{
	case 1: LoadLevel1(); break;
	case 2: LoadLevel2(); break;
	case 3: LoadLevel3(); break;
	case 4: LoadLevel4(); break;
	case 5: LoadLevel5(); break;
	case 6: LoadLevel6(); break;
	default: std::cout << "tryint to load unknown level " << new_level << std::endl;
	}
	for (auto itr = current_players.begin(); itr != current_players.end(); itr++)
	{
		Spawnplayer(itr->second);
	}

	//send level state switch to all players, and stop garuntee messages
	for (int i = 0; i < client_proxys.size(); i++)
	{
		if (client_proxys[i].connected)
		{
			client_proxys[i].network_sender.clearGarunteeMessages();
			client_proxys[i].switching_levels = true;
			bitwriter.reset();
			client_proxys[i].network_sender.labelpacketGarunteeBegin(bitwriter);
			WriteGamePacket_levelload(bitwriter, new_level, level_switch_uid);
			server_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));
			client_proxys[i].network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&client_proxys[i].client_addr));
		}
	}
	level_switch_uid++;

	//pause network sending: we can't send anymore packets to client until we recieve a levelswitch packet from them
	gameobject_network_requests.clear();
	gameobject_network_requests_garuntee.clear();
	audio_network_requests.clear();

	level_switching = true;
	players_switched = 0;
}

void ServerGameApp::LevelSwitchDone()
{
	players_switched = 0;
	level_switching = false;

	audio_network_requests.push_back(shared::SOUNDS::NEXTLEVEL);
	if (current_level == 5)
	{
		std::mt19937 mt(rd());
		std::uniform_real_distribution<float> dist_1(0.0, 1.0);
		float prob = dist_1(mt);
		if(prob <= .33)
			audio_network_requests.push_back(shared::SOUNDS::AVILO_0);
		else if(prob <=.66)
			audio_network_requests.push_back(shared::SOUNDS::AVILO_00);
		else
			audio_network_requests.push_back(shared::SOUNDS::AVILO_000);
	}
	force_abilityui_packet = true;

	for (auto itr = current_players.begin(); itr != current_players.end(); itr++)
	{
		if (!itr->second.alive)
			playerRevive(itr->second);
		itr->second.Init();
		graphics_manager.GetRenderRect(itr->second.render_id).SetTexture(4);
	}
}

int ServerGameApp::findClientwithAddr(sockaddr_in addr)
{
	for (int i = 0; i < client_proxys.size(); i++)
	{
		if (client_proxys[i].client_addr.sin_port == addr.sin_port &&
			client_proxys[i].client_addr.sin_addr.S_un.S_addr == addr.sin_addr.S_un.S_addr)
		{
			return i;
		}
	}
	return -1;
}

void ServerGameApp::cleanup() 
{
	server_socket.reset();

	if(config::STANDALONE_GAME)
		CloseWSA();
}