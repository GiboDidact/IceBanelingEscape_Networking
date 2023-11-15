#include "pchclient.h"
#include "ClientGameApp.h"

#include "SDL.h"
#include "SDL_mixer.h"

struct audio_ss
{
	Mix_Chunk* sound_death = 0;
	Mix_Chunk* sound_death2 = 0;
	Mix_Chunk* sound_death3 = 0;
	Mix_Chunk* sound_door = 0;
	Mix_Chunk* sound_revive = 0;
	Mix_Chunk* sound_zealot = 0;
	Mix_Chunk* sound_endlevel = 0;
	Mix_Chunk* sound_cheers = 0;
	Mix_Chunk* sound_cooking = 0;
	Mix_Chunk* sound_dead = 0;

	//avilo FREE
	Mix_Chunk* avilo_0 = 0;
	Mix_Chunk* avilo_00 = 0;
	Mix_Chunk* avilo_000 = 0;
	Mix_Chunk* avilo_kongfu = 0;
	Mix_Chunk* avilo_kongfushort = 0;
	Mix_Chunk* avilo__1 = 0;
	Mix_Chunk* avilo__11 = 0;
	Mix_Chunk* avilo__3 = 0;
	Mix_Chunk* avilo__31 = 0;
	Mix_Chunk* avilo__32 = 0;

	Mix_Chunk* avilo__4 = 0;
	Mix_Chunk* avilo__42 = 0;
	Mix_Chunk* avilo__43 = 0;
	Mix_Chunk* avilo__cry = 0;

	Mix_Chunk* avilo__5 = 0;
	Mix_Chunk* avilo__55 = 0;
	Mix_Chunk* avilo__hammer = 0;

	Mix_Chunk* avilo__6 = 0;
	Mix_Chunk* avilo__66 = 0;
	Mix_Chunk* avilo__cops = 0;

	Mix_Chunk* tank_1 = 0;
	Mix_Chunk* tank_2 = 0;
	Mix_Chunk* tank_3 = 0;

	//songs
	Mix_Music* music_greatbigsled = 0;
	Mix_Music* music_avilobeaten = 0;
	Mix_Music* music_lostviking = 0;
	Mix_Music* music_bossfight = 0;

	int death_sound = 0;
};
audio_ss audio_data;

ClientGameApp::ClientGameApp(int _port_id)
{
	config::LoadConfigValues();

	cam_main = nullptr;
	connected = true;
	CONNECTION_TIMEOUT = 10000;
	load_state = LOADSTATE::SAYINGHI;
	owned_player_id = 255;

	nextexpected_levelswitch_uid = 0;

    server_addr;
	portid = _port_id;
	server_addr = createSocketAddr_Mixed(config::ServerIP.c_str(), portid++);
	std::cout << "SERVER UDP ADDRESS: ";
	printSocketAddr(*reinterpret_cast<sockaddr_in*>(&server_addr));
}

int ClientGameApp::Run() 
{
	std::cout << "running game\n";
	if (!initializeGame())
	{
		std::cout << "failed to initialize!\n";
		cleanUpGame();
		return -1;
	}
	mainLoopGame();
	cleanUpGame();

	return 1;
}

int ClientGameApp::initializeGame()
{
	if (StartWSA() == -1)
		return -1;

	if (graphics_manager.Initialize("Ice Baneling Escape", config::FULL_SCREEN) == -1)
		return -1;

	glfwSetFramebufferSizeCallback(graphics_manager.getWindow(), framebuffer_size_callback);
	glfwSetKeyCallback(graphics_manager.getWindow(), key_callback);
	glfwSetCursorPosCallback(graphics_manager.getWindow(), cursor_position_callback);
	glfwSetMouseButtonCallback(graphics_manager.getWindow(), mouse_button_callback);
	glfwSetScrollCallback(graphics_manager.getWindow(), scroll_callback);

	glfwSetWindowUserPointer(graphics_manager.getWindow(), reinterpret_cast<void*>(this));

	
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		std::cout << "error initing SDL audio\n";
		return -1;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
		std::cout << "error initing mix open audio\n";
		return -1;
	}

	bitreader.Allocate(1000);

	clientsocket = std::make_shared<UDP_Socket>();
	sockaddr clientaddr = createSocketAddr_IPv4andPort(nullptr, "0", true);
	clientsocket->Bind(clientaddr);
	clientsocket->SetNonBlocking(true);

	//printSocketAddr(*reinterpret_cast<sockaddr_in*>(&clientaddr));
	printSockName(clientsocket->getSocket());

	//CAMERA
	cam_main = new Camera(glm::vec3(0, 0, 15), glm::vec3(0, 1, 0), 270.0f, 90.0f);
	if (cam_main)
	{
		cam_main->scale.x = 10;
		cam_main->scale.y = 10;
	}
	graphics_manager.setcameraref(cam_main);

	initializeData();

	return 1;
}

void ClientGameApp::initializeData()
{
	//audio
	audio_data.sound_death = Mix_LoadWAV((config::start_path + "GameData/Sounds/effects/deathsound.wav").c_str());
	audio_data.sound_death2 = Mix_LoadWAV((config::start_path + "GameData/Sounds/effects/deathsound2.wav").c_str());
	audio_data.sound_death3 = Mix_LoadWAV((config::start_path + "GameData/Sounds/effects/deathsound3.wav").c_str());
	audio_data.sound_door = Mix_LoadWAV((config::start_path + "GameData/Sounds/effects/gate.wav").c_str());
	audio_data.sound_revive = Mix_LoadWAV((config::start_path + "GameData/Sounds/effects/revive.wav").c_str());
	audio_data.sound_zealot = Mix_LoadWAV((config::start_path + "GameData/Sounds/effects/protoss3.wav").c_str());
	audio_data.sound_endlevel = Mix_LoadWAV((config::start_path + "GameData/Sounds/effects/endlevel.wav").c_str());
	audio_data.sound_cheers = Mix_LoadWAV((config::start_path + "GameData/Sounds/effects/cheers.wav").c_str());
	audio_data.sound_cooking = Mix_LoadWAV((config::start_path + "GameData/Sounds/cook.mp3").c_str());
	audio_data.sound_dead = Mix_LoadWAV((config::start_path + "GameData/Sounds/dead.mp3").c_str());

	Mix_VolumeChunk(audio_data.sound_death, 75);
	Mix_VolumeChunk(audio_data.sound_death2, 100);
	Mix_VolumeChunk(audio_data.sound_death3, MIX_MAX_VOLUME);
	//Mix_VolumeChunk(audio_data.sound_revive, 50);
	//Mix_VolumeChunk(audio_data.sound_zealot, 50);
	Mix_VolumeChunk(audio_data.sound_door, 50);

	//avilo
	audio_data.avilo_0 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_1_.mp3").c_str());
	audio_data.avilo_00 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_11_.mp3").c_str());
	audio_data.avilo_000 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_111_.mp3").c_str());
	audio_data.avilo_kongfu = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_d1.mp3").c_str());
	audio_data.avilo_kongfushort = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_d2.mp3").c_str());
	audio_data.avilo__1 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_22_.mp3").c_str());
	audio_data.avilo__11 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_22.mp3").c_str());
	
	audio_data.avilo__3 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_3.mp3").c_str());
	audio_data.avilo__31 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_31.mp3").c_str());
	audio_data.avilo__32 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_32.mp3").c_str());

	audio_data.avilo__4 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_4.mp3").c_str());
	audio_data.avilo__42 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_42.mp3").c_str());
	audio_data.avilo__43 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_43.mp3").c_str());
	audio_data.avilo__cry = Mix_LoadWAV((config::start_path + "GameData/Sounds/cry.mp3").c_str());
	
	Mix_VolumeChunk(audio_data.avilo__4, 25);
	Mix_VolumeChunk(audio_data.avilo__42, MIX_MAX_VOLUME);
	Mix_VolumeChunk(audio_data.avilo__43, MIX_MAX_VOLUME);
	Mix_VolumeChunk(audio_data.avilo__cry, MIX_MAX_VOLUME);

	audio_data.avilo__5 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_5.mp3").c_str());
	audio_data.avilo__55 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_55.mp3").c_str());
	audio_data.avilo__hammer = Mix_LoadWAV((config::start_path + "GameData/Sounds/hammer.mp3").c_str());

	audio_data.avilo__6 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_6.mp3").c_str());
	audio_data.avilo__66 = Mix_LoadWAV((config::start_path + "GameData/Sounds/sound_66.mp3").c_str());
	audio_data.avilo__cops = Mix_LoadWAV((config::start_path + "GameData/Sounds/cop.mp3").c_str());

	audio_data.tank_1 = Mix_LoadWAV((config::start_path + "GameData/Sounds/tank_1.mp3").c_str());
	audio_data.tank_2 = Mix_LoadWAV((config::start_path + "GameData/Sounds/tank_2.mp3").c_str());
	audio_data.tank_3 = Mix_LoadWAV((config::start_path + "GameData/Sounds/tank_3.mp3").c_str());

	Mix_VolumeMusic(MIX_MAX_VOLUME);
	audio_data.music_lostviking = Mix_LoadMUS((config::start_path + "GameData/Sounds/music/lost_viking.mp3").c_str());
	audio_data.music_greatbigsled = Mix_LoadMUS((config::start_path + "GameData/Sounds/music/greatbigsled.mp3").c_str());
	audio_data.music_avilobeaten = Mix_LoadMUS((config::start_path + "GameData/Sounds/music/endsong.mp3").c_str());
	audio_data.music_bossfight = Mix_LoadMUS((config::start_path + "GameData/Sounds/music/avsong.mp3").c_str());

	//global
	global_state.camera_lock = false;
	global_state.msg_on = false;
	global_state.player_info_on = false;

	load_screen_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(load_screen_rectid).Scale(glm::vec3(cam_main->scale.x, cam_main->scale.y, 1));
	graphics_manager.GetRenderRect(load_screen_rectid).SetTexture(32);
	graphics_manager.GetRenderRect(load_screen_rectid).SetLayer(6);

	//UI
	ui_bottom.background_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.background_rectid).SetTexture(3);
	graphics_manager.GetRenderRect(ui_bottom.background_rectid).SetLayer(4);

	ui_bottom.speed_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.speed_rectid).SetTexture(26);
	graphics_manager.GetRenderRect(ui_bottom.speed_rectid).SetLayer(4);

	ui_bottom.speed_cd_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.speed_cd_rectid).SetTexture(27);
	graphics_manager.GetRenderRect(ui_bottom.speed_cd_rectid).SetLayer(5);

	ui_bottom.goo_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.goo_rectid).SetTexture(21);
	graphics_manager.GetRenderRect(ui_bottom.goo_rectid).SetLayer(4);

	ui_bottom.goo_cd_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.goo_cd_rectid).SetTexture(27);
	graphics_manager.GetRenderRect(ui_bottom.goo_cd_rectid).SetLayer(5);

	ui_bottom.disk_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.disk_rectid).SetTexture(25);
	graphics_manager.GetRenderRect(ui_bottom.disk_rectid).SetLayer(4);

	ui_bottom.disk_cd_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.disk_cd_rectid).SetTexture(27);
	graphics_manager.GetRenderRect(ui_bottom.disk_cd_rectid).SetLayer(5);

	ui_bottom.halt_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.halt_rectid).SetTexture(22);
	graphics_manager.GetRenderRect(ui_bottom.halt_rectid).SetLayer(4);

	ui_bottom.lock_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.lock_rectid).SetTexture(23);
	graphics_manager.GetRenderRect(ui_bottom.lock_rectid).SetLayer(4);

	ui_bottom.icon_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.icon_rectid).SetTexture(19);
	graphics_manager.GetRenderRect(ui_bottom.icon_rectid).SetLayer(4);

	ui_bottom.timer_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.timer_rectid).SetTexture(30);
	graphics_manager.GetRenderRect(ui_bottom.timer_rectid).SetLayer(4);

	ui_bottom.topbar_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.topbar_rectid).SetTexture(3);
	graphics_manager.GetRenderRect(ui_bottom.topbar_rectid).SetLayer(4);

	ui_bottom.chatback_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.chatback_rectid).SetTexture(30);
	graphics_manager.GetRenderRect(ui_bottom.chatback_rectid).SetLayer(4);

	ui_bottom.highlatency_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.highlatency_rectid).SetTexture(33);
	graphics_manager.GetRenderRect(ui_bottom.highlatency_rectid).SetLayer(4);

	ui_bottom.latencyvariation_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.latencyvariation_rectid).SetTexture(35);
	graphics_manager.GetRenderRect(ui_bottom.latencyvariation_rectid).SetLayer(4);

	ui_bottom.packetloss_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_bottom.packetloss_rectid).SetTexture(34);
	graphics_manager.GetRenderRect(ui_bottom.packetloss_rectid).SetLayer(4);

	//msgui
	ui_msg.toggle_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_msg.toggle_rectid).SetTexture(20);
	graphics_manager.GetRenderRect(ui_msg.toggle_rectid).SetLayer(5);

	ui_msg.background_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_msg.background_rectid).SetTexture(30);
	graphics_manager.GetRenderRect(ui_msg.background_rectid).SetLayer(4);
	graphics_manager.GetRenderRect(ui_msg.background_rectid).SetVisibility(false);

	ui_msg.exit_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_msg.exit_rectid).SetTexture(70);
	graphics_manager.GetRenderRect(ui_msg.exit_rectid).SetLayer(5);
	graphics_manager.GetRenderRect(ui_msg.exit_rectid).SetVisibility(false);

	//playerui
	ui_player.toggle_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_player.toggle_rectid).SetTexture(24);
	graphics_manager.GetRenderRect(ui_player.toggle_rectid).SetLayer(5);

	ui_player.background_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_player.background_rectid).SetTexture(30);
	graphics_manager.GetRenderRect(ui_player.background_rectid).SetLayer(4);
	graphics_manager.GetRenderRect(ui_player.background_rectid).SetVisibility(false);
}

void ClientGameApp::mainLoopGame()
{
	global_state.game_time.Begin();


	global_state.timer.Begin();
	global_state.input_timer.Begin();
	acktimer.Begin();
	connection_timer.Begin();

	auto last_time = std::chrono::high_resolution_clock::now();
	bool leave_game = false;
	while (!glfwWindowShouldClose(graphics_manager.getWindow()) && !leave_game)
	{
#ifdef PING_EMULATOR
		PingEmulatorUDP::client = true;
		PingEmulatorUDP::update();
#endif
		if (!connected)
		{
			//just leave the game
			break;
		}

		if (!Mix_PlayingMusic() && global_state.current_level < 5)
			Mix_PlayMusic(audio_data.music_lostviking, 0);

		auto current_time = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(last_time).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(current_time).time_since_epoch().count();
		auto elapsed = end - start;
		last_time = current_time;

		//INPUT
		glfwPollEvents();

		ParseIncomingStream();

		if (load_state != LOADSTATE::LAUNCHED)
		{
			if (load_state == LOADSTATE::SAYINGHI /*global_state.timer.TimeElapsed(16)*/)
			{
				sendHelloPacket();
				global_state.timer.Begin();
			}

			if (load_state == LOADSTATE::SAYINGHI)
				graphics_manager.AddText("Connecting...", -2, -4.5, 0.005, glm::vec3(1, 1, 1));
			else if (load_state == LOADSTATE::WAITINGLAUNCH)
				graphics_manager.AddText("Loading...", -2, -4.5, 0.005, glm::vec3(1, 1, 1));
		}
		else
		{
			if(!global_state.level_switching)
				handleInput();

			//p switch levels
			//r player revive
			if (InputManager.GetKeyPressOnce(GLFW_KEY_C) && !global_state.typing) {
				global_state.camera_lock = !global_state.camera_lock;
			}
			if ((InputManager.GetKeyPressOnce(GLFW_KEY_SPACE) || global_state.camera_lock) && !global_state.typing) {
				if(current_players.count(owned_player_id) != 0)
					cam_main->position = glm::vec3(graphics_manager.GetRenderRect(current_players[owned_player_id]).getPos(), 15);
			}

			InputManager.GetMouseButtonsOnce(global_state.mouse_left, global_state.mouse_right);

			double mx, my;
			InputManager.GetMousePos(mx, my);
			glm::vec2 mouse_pos = mousetoworldpos(glm::vec2(mx, my));

			if (global_state.mouse_left && shared::point_in_rect_collision_test(graphics_manager, mouse_pos, ui_player.toggle_rectid)) {
				global_state.player_info_on = !global_state.player_info_on;
				global_state.msg_on = false;
			}

			if (global_state.mouse_left && shared::point_in_rect_collision_test(graphics_manager, mouse_pos, ui_msg.toggle_rectid)) {
				global_state.msg_on = !global_state.msg_on;
				global_state.player_info_on = false;
			}
			//exit game
			if (global_state.mouse_left && shared::point_in_rect_collision_test(graphics_manager, mouse_pos, ui_msg.exit_rectid)) {
				leave_game = true;
			}


			if (!global_state.level_switching)
				handlecamera(elapsed);

			handleUI();

			if (global_state.current_level == 5 && current_level_gameobjects.size() >= 8)
			{
				if (current_players.count(owned_player_id) != 0)
				{
					if (shared::rect_collision_test(graphics_manager, current_players[owned_player_id], current_level_gameobjects[7]))
					{
						graphics_manager.GetRenderRect(current_level_gameobjects[7]).SetLayer(1);
					}
					else
					{
						graphics_manager.GetRenderRect(current_level_gameobjects[7]).SetLayer(3);
					}
				}
			}
		}

		graphics_manager.Render(cam_main);
		glfwSwapBuffers(graphics_manager.getWindow());
	}
}

void ClientGameApp::ParseIncomingStream()
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

	//check for disconnect
	if (connection_timer.TimeElapsed(CONNECTION_TIMEOUT))
	{
		std::cout << "connection to server timed out!\n";
		connected = false;
		return;
	}

	//send out any acknowledgements
	if (network_reciever.anyacknowledgements() && acktimer.TimeElapsed(ACK_SEND_TIME))
	{
		acktimer.Begin();
		bitwriter.reset();
		network_sender.labelpacketfake(bitwriter);
		network_reciever.writeacknowledgementpacket(bitwriter);
		clientsocket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), server_addr);
	}

	//check and handle timed out inflight packets
	bool resent = false;
	network_sender.checkinflightTimeout(bitwriter, clientsocket, resent);
	if (resent)
	{
		ui_bottom.packetloss_on = true;
		ui_bottom.packetloss_timer.Begin();
	}
	
	//read incoming stream from server *can validate that it is from server but for now assume*
	while (true)
	{
		bitreader.reset();
		sockaddr_in recfrom_addr;
		int bytesReceived = clientsocket->RecvFromNoError(bitreader.getData(), bitreader.getCapacityBytes(), recfrom_addr);
		if (bytesReceived == 0)
		{
			std::cout << "read 0 bytes, not sure why this happened for UDP\n";
			connected = false;
		}
		else if (bytesReceived < 0)
		{
			int err = WSAGetLastError();
			if (err != WSAEWOULDBLOCK) {
				std::cout << "reading from server error: " << err << std::endl;
				connected = false;
			}
			//we have error so probably disconnect from server
			break;
		}
		else
		{
			connection_timer.Begin();
			//std::cout << "\nbytes received: " << bytesReceived << std::endl;
			while (bitreader.getbithead() < bytesReceived * 8)
			{
				bool packet_valid = true;
				uint16_t packet_id;
				bitreader.Read(&packet_id);
				GAMEPACKETCODE code;
				bitreader.Read(&code);

				if (global_state.level_switching && global_state.level_switching_timer.TimeElapsed(1000) && code != GAMEPACKETCODE::PING &&
					code != GAMEPACKETCODE::WELCOME && code != GAMEPACKETCODE::LAUNCH && code != GAMEPACKETCODE::ACKNOWLEDGEMENT)
				{
					global_state.level_switching = false;
					graphics_manager.GetRenderRect(load_screen_rectid).SetVisibility(false);
				}
				//std::cout << "code: " << (int)code << std::endl;
				//std::cout << "packet_id: "<<(int)packet_id << std::endl;
				if (code != GAMEPACKETCODE::WELCOME && code != GAMEPACKETCODE::LAUNCH &&
					code != GAMEPACKETCODE::ACKNOWLEDGEMENT && code != GAMEPACKETCODE::ENEMY &&
					code != GAMEPACKETCODE::PLAYER_UNIQUE && code != GAMEPACKETCODE::PLAYERS_EVENT &&
					code != GAMEPACKETCODE::PLAYER_DYNAMIC && code != GAMEPACKETCODE::GAMEOBJECT &&
					code != GAMEPACKETCODE::UIABILITY && code != GAMEPACKETCODE::AUDIO && 
					code != GAMEPACKETCODE::LEVEL_LOAD && code != GAMEPACKETCODE::SERVER_CHATS &&
					code != GAMEPACKETCODE::PING)
				{
					std::cout << "received unexpected code from server\n";
					//throw everything away
					break;
				}

				network_reciever.unlabelpacket(bitreader, packet_id, packet_valid);

				if (packet_valid)
				{
					if (code == GAMEPACKETCODE::WELCOME)
					{
						handleWelcomepacket(recfrom_addr);
					}
					else if (code == GAMEPACKETCODE::LAUNCH)
					{
						handleLaunchpacket(recfrom_addr);
					}
					else if (code == GAMEPACKETCODE::ACKNOWLEDGEMENT)
					{
						handleacknowledgepacket();
					}
					else if (code == GAMEPACKETCODE::ENEMY)
					{
						handleenemypacket();
					}
					else if (code == GAMEPACKETCODE::PLAYER_UNIQUE)
					{
						handleplayer_uniquepacket();
					}
					else if (code == GAMEPACKETCODE::PLAYERS_EVENT)
					{
						handleplayer_eventpacket();
					}
					else if (code == GAMEPACKETCODE::PLAYER_DYNAMIC)
					{
						handleplayer_dyanmicpacket();
					}
					else if (code == GAMEPACKETCODE::GAMEOBJECT)
					{
						handlegameobjectpacket();
					}
					else if (code == GAMEPACKETCODE::UIABILITY)
					{
						handleuiabilitypacket();
					}
					else if (code == GAMEPACKETCODE::AUDIO)
					{
						handleaudiopacket();
					}
					else if (code == GAMEPACKETCODE::LEVEL_LOAD)
					{
						handlelevelloadpacket();
					}
					else if (code == GAMEPACKETCODE::SERVER_CHATS)
					{
						handleserverchatpacket();
					}
					else if (code == GAMEPACKETCODE::PING)
					{
						handlepingpacket();
					}
				}
				else
				{
					ui_bottom.packetloss_on = true;
					ui_bottom.packetloss_timer.Begin();
					//std::cout << "packet old\n";
					//packet isnt valid, either still run it or parse through it. Could cause bug with packet_id acks, you read it when it
					//thinks your not
					if (code == GAMEPACKETCODE::ACKNOWLEDGEMENT)
					{
						handleacknowledgepacket();
					}
					break;
					//fake parse packet if we want to handle multiple packets in the same stream
				}
			}
		}
	}
}

void ClientGameApp::handlepingpacket()
{
	int timestamp;
	ReadGamePacket_ping(bitreader, timestamp, current_ping);

	if (current_ping > 150)
	{
		ui_bottom.highlatency_on = true;
		ui_bottom.latency_timer.Begin();
	}

	bitwriter.reset();
	network_sender.labelpacketfake(bitwriter);
	WriteGamePacket_ping(bitwriter, timestamp, 0);
	clientsocket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&server_addr));
}

void ClientGameApp::handleserverchatpacket()
{
	std::cout << "handleserverchatpacket\n";
	chat_msgs.clear();
	ReadGamePacket_serverchats(bitreader, chat_msgs);
}

void ClientGameApp::handlelevelloadpacket()
{
	int new_level;
	uint32_t level_uid;
	ReadGamePacket_levelload(bitreader, new_level, level_uid);
	if (level_uid >= nextexpected_levelswitch_uid)
	{
		nextexpected_levelswitch_uid++;

		//std::cout << "switching to new level " << new_level << std::endl;
		SwitchLevels(new_level);

		network_sender.clearGarunteeMessages();

		bitwriter.reset();
		network_sender.labelpacketGarunteeBegin(bitwriter);
		uint32_t uid;
		WriteGamePacket_levelload(bitwriter, new_level, uid);
		clientsocket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&server_addr));
		network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&server_addr));

		//disable input and turn screen black for a second?
		global_state.level_switching_timer.Begin();
		global_state.level_switching = true;
		graphics_manager.GetRenderRect(load_screen_rectid).SetTexture(36);
		graphics_manager.GetRenderRect(load_screen_rectid).SetVisibility(true);

		//cam_main->position = glm::vec3(graphics_manager.GetRenderRect(current_players[owned_player_id]).getPos(), 15);
	}
}

void ClientGameApp::handleaudiopacket()
{
	std::vector<shared::SOUNDS> sounds;
	ReadGamePacket_audio(bitreader, sounds);
	
	for (int i = 0; i < sounds.size(); i++)
	{
		switch (sounds[i])
		{
		case shared::SOUNDS::DEATH1: if (!Mix_Playing(0))   { Mix_PlayChannel(0, audio_data.sound_death, 0);    }		break;
		case shared::SOUNDS::DEATH2: if (!Mix_Playing(0))   { Mix_PlayChannel(0, audio_data.sound_death2, 0);   }		break;
		case shared::SOUNDS::DEATH3: if (!Mix_Playing(0))   { Mix_PlayChannel(0, audio_data.sound_death3, 0);	}		break;
		case shared::SOUNDS::DOOR:	if (!Mix_Playing(1))    { Mix_PlayChannel(1, audio_data.sound_door, 0);     }		break;
		case shared::SOUNDS::REVIVE:if (!Mix_Playing(2))    { Mix_PlayChannel(2, audio_data.sound_revive, 0);   }		break;
		case shared::SOUNDS::ZEALOT:if (!Mix_Playing(3))    { Mix_PlayChannel(3, audio_data.sound_zealot, 0);   }		break;
		case shared::SOUNDS::NEXTLEVEL:if (!Mix_Playing(1)) { Mix_PlayChannel(1, audio_data.sound_endlevel, 0); }		break;
		case shared::SOUNDS::CHEERS:	Mix_PlayChannel(0, audio_data.sound_cheers, 0); 		break;
		case shared::SOUNDS::COOKING:	Mix_PlayChannel(1, audio_data.sound_cooking, 0); 		break;
		case shared::SOUNDS::DEAD:		Mix_PlayChannel(2, audio_data.sound_dead, 0); 		break;

		case shared::SOUNDS::AVILO_0:   { Mix_PlayChannel(1, audio_data.avilo_0, 0); }				break;
		case shared::SOUNDS::AVILO_00:  { Mix_PlayChannel(1, audio_data.avilo_00, 0); }				break;
		case shared::SOUNDS::AVILO_000: { Mix_PlayChannel(1, audio_data.avilo_000, 0); }				break;

		case shared::SOUNDS::AVILO_KONGFU: { Mix_PlayChannel(1, audio_data.avilo_kongfu, 0); }				break;
		case shared::SOUNDS::AVILO_KONGFUSHORT: { Mix_PlayChannel(0, audio_data.avilo_kongfushort, 0); }	break;
		
		case shared::SOUNDS::AVILO_1:		{ Mix_PlayChannel(1, audio_data.avilo__1, 0); }				break;
		case shared::SOUNDS::AVILO_11:		{ Mix_PlayChannel(1, audio_data.avilo__11, 0); }	break;
		
		case shared::SOUNDS::AVILO_3: { Mix_PlayChannel(1, audio_data.avilo__3, 0); }	break;
		case shared::SOUNDS::AVILO_31: { Mix_PlayChannel(1, audio_data.avilo__31, 0); }	break;
		case shared::SOUNDS::AVILO_32: { Mix_PlayChannel(1, audio_data.avilo__32, 0); }	break;

		case shared::SOUNDS::AVILO_4: { Mix_PlayChannel(0, audio_data.avilo__4, 0); }	break;
		case shared::SOUNDS::AVILO_42: { Mix_PlayChannel(1, audio_data.avilo__42, 0); }	break;
		case shared::SOUNDS::AVILO_43: { Mix_PlayChannel(1, audio_data.avilo__43, 0); }	break;
		case shared::SOUNDS::AVILO_CRY: { Mix_PlayChannel(1, audio_data.avilo__cry, 0); }	break;
		case shared::SOUNDS::AVILO_66: { Mix_PlayChannel(1, audio_data.avilo__66, 0); }	break;

		case shared::SOUNDS::AVILO_5: { Mix_PlayChannel(0, audio_data.avilo__5, 0); }	break;
		case shared::SOUNDS::AVILO_55: { Mix_PlayChannel(0, audio_data.avilo__55, 0); }	break;
		case shared::SOUNDS::AVILO_HAMMER: { Mix_PlayChannel(0, audio_data.avilo__hammer, 0); }	break;

		case shared::SOUNDS::AVILO_6: { Mix_PlayChannel(0, audio_data.avilo__6, 0); }	break;
		case shared::SOUNDS::AVILO_COPS: { Mix_PlayChannel(1, audio_data.avilo__cops, 0); }	break;

		case shared::SOUNDS::END_SONG: { Mix_PlayMusic(audio_data.music_avilobeaten, 0); }	break;

		case shared::SOUNDS::TANK_1: { Mix_PlayChannel(2, audio_data.tank_1, 0); }	break;
		case shared::SOUNDS::TANK_2: { Mix_PlayChannel(2, audio_data.tank_2, 0); }	break;
		case shared::SOUNDS::TANK_3: { Mix_PlayChannel(2, audio_data.tank_3, 0); }	break;
		}
	}
}

void ClientGameApp::handleuiabilitypacket()
{
	shared::uiability_network_s uiability_data;
	ReadGamePacket_uiability(bitreader, uiability_data);
	
	uiability_network_data.disk_percent = uiability_data.disk_percent;
	uiability_network_data.goo_percent = uiability_data.goo_percent;
	uiability_network_data.speedup_percent = uiability_data.speedup_percent;
}

void ClientGameApp::handleenemypacket()
{
	std::vector<glm::vec3> enemy_pos;
	uint32_t starting_index = 0;
	ReadGamePacket_enemy(bitreader, enemy_pos, starting_index);

	my_assert(starting_index + enemy_pos.size() <= current_enemies.size(), "enemy sizes aren't same");

	int count = 0;
	for (int i = starting_index; i < enemy_pos.size() + starting_index; i++)
	{
		graphics_manager.GetRenderRect(current_enemies[i]).Translate(glm::vec3(enemy_pos[count].x, enemy_pos[count].y, 0));
		graphics_manager.GetRenderRect(current_enemies[i]).Reflect(enemy_pos[count].z);
		count++;
	}
}

void ClientGameApp::handleWelcomepacket(sockaddr_in addr)
{
	if (load_state == LOADSTATE::SAYINGHI)
	{
		load_state = LOADSTATE::WAITINGLAUNCH;
		std::cout << "welcomed by server! ";
		printSocketAddr(addr);
	}
}

void ClientGameApp::handleplayer_uniquepacket()
{
	uint8_t id;
	ReadGamePacket_playerunique(bitreader, id);
	if (owned_player_id == 255) {
		owned_player_id = id;
		std::cout << "owned_player_id: " << (int)owned_player_id << std::endl;
	}
}

void ClientGameApp::handleplayer_eventpacket()
{
	std::vector<shared::player_network_data_event_s> player_event_data;
	ReadGamePacket_playersevent(bitreader, player_event_data);

	//create a baneling rect for each player
	for (int i = 0; i < player_event_data.size(); i++)
	{
		//std::cout << (int)player_event_data[i].operation << " " << (int)player_event_data[i].player_id << std::endl;
		if (player_event_data[i].operation == shared::OBJECTOPERATION::CREATE && current_players.count(player_event_data[i].player_id) == 0)
		{
			uint32_t rect_id;
			shared::LoadBaneling(graphics_manager, rect_id);
			current_players[player_event_data[i].player_id] = rect_id;
		}
		else if (player_event_data[i].operation == shared::OBJECTOPERATION::TEXTURE && current_players.count(player_event_data[i].player_id) != 0)
		{
			graphics_manager.GetRenderRect(current_players[player_event_data[i].player_id]).SetTexture(player_event_data[i].texture_id);
		}
		else if(player_event_data[i].operation == shared::OBJECTOPERATION::DESTROY && current_players.count(player_event_data[i].player_id) != 0)
		{
			graphics_manager.RemoveRenderRect(current_players[player_event_data[i].player_id]);
			current_players.erase(player_event_data[i].player_id);
		}
	}
}

void ClientGameApp::handleplayer_dyanmicpacket()
{
	std::vector<shared::player_network_data_dynamic_s> player_dynamic_info;
	ReadGamePacket_playersdynamic(bitreader, player_dynamic_info);

	for (int i = 0; i < player_dynamic_info.size(); i++)
	{
		uint8_t playerid = player_dynamic_info[i].player_id;
		if (current_players.count(playerid) != 0)
		{
			//move and rotate
			graphics_manager.GetRenderRect(current_players[playerid]).Translate(glm::vec3(player_dynamic_info[i].pos,0));
			graphics_manager.GetRenderRect(current_players[playerid]).Rotate(glm::vec3(0,0,1),player_dynamic_info[i].angle);
			graphics_manager.GetRenderRect(current_players[playerid]).Reflect(player_dynamic_info[i].reflect);
			graphics_manager.GetRenderRect(current_players[playerid]).SetTexture(player_dynamic_info[i].texture_id);
		}
		else
		{
			std::cout << "dynamic packet got invalid player id???\n";
		}
	}
}

void ClientGameApp::handlegameobjectpacket()
{
	std::vector<shared::gameobject_network_s> gameobjects_data;
	ReadGamePacket_gameobject(bitreader, gameobjects_data);

	for (int i = 0; i < gameobjects_data.size(); i++)
	{
		//any new game_id put onto destroy with false
		if (current_level_gameobjects_destroyed.count(gameobjects_data[i].game_id) == 0)
			current_level_gameobjects_destroyed[gameobjects_data[i].game_id] = false;
		//std::cout << "game object id: " << (int)gameobjects_data[i].game_id << " code: " << (int)gameobjects_data[i].operation << std::endl;
		
		//if its already been destroyed never execute a command for that game id
		if (current_level_gameobjects_destroyed.count(gameobjects_data[i].game_id) != 0 && current_level_gameobjects_destroyed[gameobjects_data[i].game_id])
		{
			//std::cout << "game object id has already been destroyed, ignore\n";
			continue;
		}

		//if we get a destroy message without a create then just destroy and never create
		if(current_level_gameobjects.count(gameobjects_data[i].game_id) == 0 && gameobjects_data[i].operation == shared::OBJECTOPERATION::DESTROY)
			current_level_gameobjects_destroyed[gameobjects_data[i].game_id] = true;

		if (gameobjects_data[i].operation != shared::OBJECTOPERATION::CREATE && 
			current_level_gameobjects.count(gameobjects_data[i].game_id) == 0)
		{
			//if(gameobjects_data[i].operation != shared::OBJECTOPERATION::DESTROY)
			//	std::cout << "invalid gameobject id: " << (int)gameobjects_data[i].game_id << std::endl;
			continue;
		}

		shared::gameobject_network_s obj_data = gameobjects_data[i];
		uint32_t rect_id;
		if(gameobjects_data[i].operation != shared::OBJECTOPERATION::CREATE)
			 rect_id = current_level_gameobjects.at(gameobjects_data[i].game_id);

		if (gameobjects_data[i].operation == shared::OBJECTOPERATION::CREATE && current_level_gameobjects.count(obj_data.game_id) == 0)
		{
			uint32_t rect = graphics_manager.CreateRenderRect();
			graphics_manager.GetRenderRect(rect).Translate(glm::vec3(obj_data.position.x, obj_data.position.y, 0));
			graphics_manager.GetRenderRect(rect).SetTexture(obj_data.texture_id);
			graphics_manager.GetRenderRect(rect).SetVisibility(obj_data.visible);
			graphics_manager.GetRenderRect(rect).Rotate(glm::vec3(0,0,1), obj_data.rotation_angle);
			graphics_manager.GetRenderRect(rect).SetLayer(obj_data.layer);
			graphics_manager.GetRenderRect(rect).Scale(glm::vec3(obj_data.scale.x, obj_data.scale.y, 0));
			current_level_gameobjects[obj_data.game_id] = rect;
		}
		else if (gameobjects_data[i].operation == shared::OBJECTOPERATION::UPDATE) 
		{
			graphics_manager.GetRenderRect(rect_id).Translate(glm::vec3(obj_data.position.x, obj_data.position.y, 0));
			graphics_manager.GetRenderRect(rect_id).SetTexture(obj_data.texture_id);
			graphics_manager.GetRenderRect(rect_id).SetVisibility(obj_data.visible);
		}
		else if (gameobjects_data[i].operation == shared::OBJECTOPERATION::MOVE) 
		{
			graphics_manager.GetRenderRect(rect_id).Translate(glm::vec3(obj_data.position.x, obj_data.position.y, 0));
		}
		else if (gameobjects_data[i].operation == shared::OBJECTOPERATION::DESTROY)
		{
			graphics_manager.RemoveRenderRect(rect_id);
			current_level_gameobjects.erase(obj_data.game_id);
			my_assert(current_level_gameobjects_destroyed.count(obj_data.game_id) != 0, "destroying id that doesnt exist?");
			current_level_gameobjects_destroyed[obj_data.game_id] = true;
		}
		else if (gameobjects_data[i].operation == shared::OBJECTOPERATION::LAYER)
		{
			graphics_manager.GetRenderRect(rect_id).SetLayer(obj_data.layer);
		}
		else if (gameobjects_data[i].operation == shared::OBJECTOPERATION::TEXTURE) 
		{
			graphics_manager.GetRenderRect(rect_id).SetTexture(obj_data.texture_id);
		}
		else if (gameobjects_data[i].operation == shared::OBJECTOPERATION::VISIBILITY) 
		{
			graphics_manager.GetRenderRect(rect_id).SetVisibility(obj_data.visible);
		}
		
	}
}

void ClientGameApp::handleLaunchpacket(sockaddr_in addr)
{
	handleWelcomepacket(addr);
	if (load_state == LOADSTATE::WAITINGLAUNCH)
	{
		std::cout << "loading up level now!\n";
		load_state = LOADSTATE::LAUNCHED;

		background_rectid = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(background_rectid).Move(glm::vec3(0, 0, 0));
		graphics_manager.GetRenderRect(background_rectid).SetTexture(18);
		graphics_manager.GetRenderRect(background_rectid).SetLayer(0);
		graphics_manager.GetRenderRect(background_rectid).Scale(glm::vec3(cam_main->scale.x, cam_main->scale.y, 0));
		graphics_manager.GetRenderRect(background_rectid).Translate(glm::vec3(cam_main->position.x, cam_main->position.y, 0));

		LoadLevel1();
		
		//tell server you loaded
		bitwriter.reset();
		network_sender.labelpacketGarunteeBegin(bitwriter);
		WriteGamePacket_welcome(bitwriter);
		clientsocket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&server_addr));
		network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), *reinterpret_cast<sockaddr*>(&server_addr));

		global_state.level_switching_timer.Begin();
		global_state.level_switching = true;
		global_state.current_level = 1;
		if(current_players.count(owned_player_id) != 0)
			cam_main->position = glm::vec3(graphics_manager.GetRenderRect(current_players[owned_player_id]).getPos(), 15);
	}
}

void ClientGameApp::handleacknowledgepacket()
{
	bool resent_any = false;
	network_sender.handleacknowledgementsPacket(bitreader, bitwriter, clientsocket, resent_any);
	if (resent_any)
	{
		ui_bottom.latencyvariation_on = true;
		ui_bottom.variation_timer.Begin();
	}
}

void ClientGameApp::handleInput()
{
	if (!global_state.typing && InputManager.GetKeyPressOnce(GLFW_KEY_ENTER))
	{
		InputManager.startrecordingTmp();
		global_state.typing = true;
	}
	if (global_state.typing && InputManager.GetKeyPressOnce(GLFW_KEY_BACKSPACE))
	{
		InputManager.stopandreceiverecordingTmp();
		global_state.typing = false;
	}

	if (global_state.typing && InputManager.GetKeyPressOnce(GLFW_KEY_ENTER))
	{
		std::vector<int> keys_pressed = InputManager.stopandreceiverecordingTmp();
		global_state.typing = false;

		std::string msg;
		for (int i = 0; i < keys_pressed.size(); i++)
		{
			if (keys_pressed[i] != GLFW_KEY_ENTER)
				msg += keys_pressed[i];
		}

		if (msg.length() > 0)
		{
			bitwriter.reset();
			network_sender.labelpacketGarunteeBegin(bitwriter);
			WriteGamePacket_chat(bitwriter, msg);
			clientsocket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), server_addr);
			network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), server_addr);
		}
	}
	if (global_state.typing)
		return;


	//accumulate input every frame
	global_state.input_data.key_e |= InputManager.GetKeyPressOnce(GLFW_KEY_E);
	global_state.input_data.key_s |= InputManager.GetKeyPressOnce(GLFW_KEY_S);
	global_state.input_data.key_f |= InputManager.GetKeyPressOnce(GLFW_KEY_F);
	global_state.input_data.key_r |= InputManager.GetKeyPressOnce(GLFW_KEY_R);
	global_state.input_data.key_t |= InputManager.GetKeyPressOnce(GLFW_KEY_T);

	global_state.input_data.mouse_right |= global_state.mouse_right;
	if (global_state.mouse_right)
	{
		double mx, my;
		InputManager.GetMousePos(mx, my);
		glm::vec2 world_pos = mousetoworldpos(glm::vec2(mx, my));
		global_state.input_data.world_pos_click = world_pos;
	}

	global_state.input_data.timestamp = Timer::getTimeSinceEpoch(global_state.timer.getCurrentTime());

	//when time has elapsed send input packet, reset input
	if (global_state.input_timer.TimeElapsed(INPUT_SEND_TIME) && global_state.input_data.anyinput())
	{
		global_state.input_timer.Begin();

		bitwriter.reset();
		network_sender.labelpacketGarunteeBegin(bitwriter);
		WriteGamePacket_input(bitwriter, global_state.input_data);
		clientsocket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), server_addr);
		network_sender.labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), server_addr);

		global_state.input_data.Init();
	}
}

void ClientGameApp::handlecamera(float deltaT)
{
	double x, y;
	InputManager.GetMousePos(x, y);
	glm::vec2 world_pos = mousetoworldpos(glm::vec2(x, y));
	bool ontop_gui = false;
	if (shared::point_in_rect_collision_test(graphics_manager, world_pos, ui_bottom.topbar_rectid)) {
		ontop_gui = true;
	}

	if (!ontop_gui)
	{
		MAP_PAN_SIZE = MAP_PAN_SIZE_DEFAULT;
		if (global_state.current_level == 5)
		{
			MAP_PAN_SIZE = MAP_PAN_SIZE_lvl5;
		}
		//[0,1]
		glm::vec2 pos_percent = glm::vec2(x, y) / glm::vec2(graphics_manager.getscreenwidth(), graphics_manager.getscreenheight());
		pos_percent.y = (1 - pos_percent.y);

		if (pos_percent.x <= CAM_PADDING) //left
		{
			cam_main->position.x = glm::max(-MAP_PAN_SIZE, cam_main->position.x - CAM_SPEED * deltaT);
		}
		if (pos_percent.x >= 1 - CAM_PADDING) //right
		{
			cam_main->position.x = glm::min(MAP_PAN_SIZE, cam_main->position.x + CAM_SPEED * deltaT);
		}
		if (pos_percent.y >= 1 - CAM_PADDING) //up
		{
			cam_main->position.y = glm::min(MAP_PAN_SIZE, cam_main->position.y + CAM_SPEED * deltaT);
		}
		if (pos_percent.y <= CAM_PADDING) //down
		{
			cam_main->position.y = glm::max(-MAP_PAN_SIZE, cam_main->position.y - CAM_SPEED * deltaT);
		}
	}

	//this is each input, so speed doesn't matter on computer
	int scroll_y_offset = InputManager.GetScrollOnce();
	if (scroll_y_offset > 0)
	{
		//zoom in
		current_zoom = glm::max(SCROLL_MIN_PERCENT, current_zoom - SCROLL_SPEED);
		cam_main->scale = glm::vec3(graphics_manager.getlast_savescale(), 0) * current_zoom;
	}
	else if (scroll_y_offset < 0)
	{
		//zoom out
		current_zoom = glm::min(SCROLL_MAX_PERCENT, current_zoom + SCROLL_SPEED);
		cam_main->scale = glm::vec3(graphics_manager.getlast_savescale(),0) * current_zoom;
	}



	//w/h = (w-x)/(h-x) =  w(h-x) = (w-x)h   wh- wx = wh - xh = xh = wx h = w
	//w/h = (w*x)/(h*x) = whx = whx
}

void ClientGameApp::sendHelloPacket()
{
	bitwriter.reset();
	network_sender.labelpacket(bitwriter);
	WriteGamePacket_hello(bitwriter);
	clientsocket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), server_addr);
}

void ClientGameApp::cleanUpGame()
{
	if (cam_main)
		delete cam_main;
	graphics_manager.cleanUp();

	Mix_FreeChunk(audio_data.sound_death);
	Mix_FreeChunk(audio_data.sound_death2);
	Mix_FreeChunk(audio_data.sound_death3);
	Mix_FreeChunk(audio_data.sound_endlevel);
	Mix_FreeChunk(audio_data.sound_door);
	Mix_FreeChunk(audio_data.sound_revive);
	Mix_FreeChunk(audio_data.sound_zealot);
	Mix_FreeChunk(audio_data.sound_cheers);
	Mix_FreeChunk(audio_data.sound_cooking);
	Mix_FreeChunk(audio_data.sound_dead);

	Mix_FreeChunk(audio_data.avilo_0);
	Mix_FreeChunk(audio_data.avilo_00);
	Mix_FreeChunk(audio_data.avilo_000);
	Mix_FreeChunk(audio_data.avilo_kongfu);
	Mix_FreeChunk(audio_data.avilo_kongfushort);
	Mix_FreeChunk(audio_data.avilo__1);
	Mix_FreeChunk(audio_data.avilo__11);

	Mix_FreeChunk(audio_data.avilo__3);
	Mix_FreeChunk(audio_data.avilo__31);
	Mix_FreeChunk(audio_data.avilo__32);
	Mix_FreeChunk(audio_data.avilo__4);
	Mix_FreeChunk(audio_data.avilo__42);
	Mix_FreeChunk(audio_data.avilo__43);
	Mix_FreeChunk(audio_data.avilo__cry);


	Mix_FreeChunk(audio_data.avilo__5);
	Mix_FreeChunk(audio_data.avilo__55);
	Mix_FreeChunk(audio_data.avilo__hammer);
	Mix_FreeChunk(audio_data.avilo__6);
	Mix_FreeChunk(audio_data.avilo__66);
	Mix_FreeChunk(audio_data.avilo__cops);
	Mix_FreeChunk(audio_data.tank_1);
	Mix_FreeChunk(audio_data.tank_2);
	Mix_FreeChunk(audio_data.tank_3);

	Mix_FreeMusic(audio_data.music_lostviking);
	Mix_FreeMusic(audio_data.music_greatbigsled);
	Mix_FreeMusic(audio_data.music_avilobeaten);
	Mix_CloseAudio();
	SDL_Quit();

	clientsocket.reset();
	CloseWSA();
}

glm::vec2 ClientGameApp::mousetoworldpos(glm::vec2 mousepos)
{
	//scale window dimensions to world dimensions
	//shift 0,0 to center
	//flip y so negative is bottom and positive is top
	glm::vec2 world_pos_scale = mousepos * glm::vec2(cam_main->scale.x / graphics_manager.getscreenwidth(),
		cam_main->scale.y / graphics_manager.getscreenheight());
	glm::vec2 world_pos = world_pos_scale - glm::vec2(cam_main->scale.x * .5, cam_main->scale.y * .5);
	world_pos.y *= -1;

	//shift to camera
	glm::vec2 campos = world_pos + glm::vec2(cam_main->position.x, cam_main->position.y);

	return campos;
}

void ClientGameApp::handleUI()
{
	graphics_manager.GetRenderRect(load_screen_rectid).Scale(glm::vec3(cam_main->scale.x, cam_main->scale.y, 0));
	graphics_manager.GetRenderRect(load_screen_rectid).Translate(glm::vec3(cam_main->position.x, cam_main->position.y, 0));
	//BACKGROUND
	graphics_manager.GetRenderRect(background_rectid).Scale(glm::vec3(cam_main->scale.x, cam_main->scale.y, 0));
	graphics_manager.GetRenderRect(background_rectid).Translate(glm::vec3(cam_main->position.x, cam_main->position.y, 0));

	//BOTTOM UI
	graphics_manager.GetRenderRect(ui_bottom.background_rectid).Scale(glm::vec3(cam_main->scale.x * .45, cam_main->scale.y / 6.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.background_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .3,
		cam_main->position.y - cam_main->scale.y / 2.4, 0));

	graphics_manager.GetRenderRect(ui_bottom.speed_rectid).Scale(glm::vec3(cam_main->scale.x / 20, cam_main->scale.y / 20.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.speed_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .275,
		cam_main->position.y - cam_main->scale.y * .375, 0));
		
	if (uiability_network_data.speedup_percent != 0.0f)
		graphics_manager.GetRenderRect(ui_bottom.speed_cd_rectid).SetVisibility(true);
	else
		graphics_manager.GetRenderRect(ui_bottom.speed_cd_rectid).SetVisibility(false);

	float cd_scale_percent = uiability_network_data.speedup_percent;
	float diff = (cam_main->scale.y * .05) - ((cam_main->scale.y * .05) * cd_scale_percent);
	graphics_manager.GetRenderRect(ui_bottom.speed_cd_rectid).Scale(glm::vec3(cam_main->scale.x / 20, (cam_main->scale.y * .05) * cd_scale_percent, 0));
	graphics_manager.GetRenderRect(ui_bottom.speed_cd_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .275,
		(cam_main->position.y - cam_main->scale.y * .375) - diff * .5, 0));
	
	graphics_manager.GetRenderRect(ui_bottom.disk_rectid).Scale(glm::vec3(cam_main->scale.x / 20, cam_main->scale.y / 20.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.disk_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .35,
		cam_main->position.y - cam_main->scale.y * .375, 0));


	if (uiability_network_data.disk_percent != 0.0f)
		graphics_manager.GetRenderRect(ui_bottom.disk_cd_rectid).SetVisibility(true);
	else
		graphics_manager.GetRenderRect(ui_bottom.disk_cd_rectid).SetVisibility(false);

	cd_scale_percent = uiability_network_data.disk_percent;
	diff = (cam_main->scale.y * .05) - ((cam_main->scale.y * .05) * cd_scale_percent);
	graphics_manager.GetRenderRect(ui_bottom.disk_cd_rectid).Scale(glm::vec3(cam_main->scale.x / 20, (cam_main->scale.y * .05) * cd_scale_percent, 0));
	graphics_manager.GetRenderRect(ui_bottom.disk_cd_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .35,
		(cam_main->position.y - cam_main->scale.y * .375) - diff * .5, 0));
		

	graphics_manager.GetRenderRect(ui_bottom.goo_rectid).Scale(glm::vec3(cam_main->scale.x / 20, cam_main->scale.y / 20.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.goo_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .2,
		cam_main->position.y - cam_main->scale.y * .375, 0));
	

	if (uiability_network_data.goo_percent != 0.0f)
		graphics_manager.GetRenderRect(ui_bottom.goo_cd_rectid).SetVisibility(true);
	else
		graphics_manager.GetRenderRect(ui_bottom.goo_cd_rectid).SetVisibility(false);

	cd_scale_percent = uiability_network_data.goo_percent;
	diff = (cam_main->scale.y * .05) - ((cam_main->scale.y * .05) * cd_scale_percent);
	graphics_manager.GetRenderRect(ui_bottom.goo_cd_rectid).Scale(glm::vec3(cam_main->scale.x / 20, (cam_main->scale.y * .05) * cd_scale_percent, 0));
	graphics_manager.GetRenderRect(ui_bottom.goo_cd_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .2,
		(cam_main->position.y - cam_main->scale.y * .375) - diff * .5, 0));
	
	if ((global_state.camera_lock || InputManager.GetKeyPress(GLFW_KEY_SPACE)) && !global_state.typing)
		graphics_manager.GetRenderRect(ui_bottom.lock_rectid).SetTexture(29);
	else
		graphics_manager.GetRenderRect(ui_bottom.lock_rectid).SetTexture(23);

	graphics_manager.GetRenderRect(ui_bottom.lock_rectid).Scale(glm::vec3(cam_main->scale.x / 20, cam_main->scale.y / 20.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.lock_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .235,
		cam_main->position.y - cam_main->scale.y * .45, 0));

	if (InputManager.GetKeyPress(GLFW_KEY_S) && !global_state.typing)
		graphics_manager.GetRenderRect(ui_bottom.halt_rectid).SetTexture(28);
	else
		graphics_manager.GetRenderRect(ui_bottom.halt_rectid).SetTexture(22);

	graphics_manager.GetRenderRect(ui_bottom.halt_rectid).Scale(glm::vec3(cam_main->scale.x / 20, cam_main->scale.y / 20.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.halt_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .315,
		cam_main->position.y - cam_main->scale.y * .45, 0));

	graphics_manager.GetRenderRect(ui_bottom.icon_rectid).Scale(glm::vec3(cam_main->scale.x / 10, cam_main->scale.y / 5.8, 0));
	graphics_manager.GetRenderRect(ui_bottom.icon_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .12,
		cam_main->position.y - cam_main->scale.y * .42, 0));

	graphics_manager.GetRenderRect(ui_bottom.timer_rectid).Scale(glm::vec3(cam_main->scale.x / 12, cam_main->scale.y / 20.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.timer_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .45,
		cam_main->position.y - cam_main->scale.y * .375, 0));

	graphics_manager.GetRenderRect(ui_bottom.topbar_rectid).Scale(glm::vec3(cam_main->scale.x * .1, cam_main->scale.y / 20.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.topbar_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x*.450,
		cam_main->position.y + cam_main->scale.y * .4825, 0));

	graphics_manager.GetRenderRect(ui_bottom.chatback_rectid).Scale(glm::vec3(cam_main->scale.x * .25, cam_main->scale.y * .25, 0));
	graphics_manager.GetRenderRect(ui_bottom.chatback_rectid).Translate(glm::vec3(cam_main->position.x - cam_main->scale.x * .375,
		cam_main->position.y - cam_main->scale.y * .375, 0));


	if (ui_bottom.latency_timer.TimeElapsed(1000)) 
		ui_bottom.highlatency_on = false;
	if (ui_bottom.variation_timer.TimeElapsed(1000))
		ui_bottom.latencyvariation_on = false;
	if (ui_bottom.packetloss_timer.TimeElapsed(1000))
		ui_bottom.packetloss_on = false;
	//latency
	graphics_manager.GetRenderRect(ui_bottom.highlatency_rectid).SetVisibility(ui_bottom.highlatency_on);
	graphics_manager.GetRenderRect(ui_bottom.highlatency_rectid).Scale(glm::vec3(cam_main->scale.x * .035, cam_main->scale.y * .035, 0));
	graphics_manager.GetRenderRect(ui_bottom.highlatency_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .475,
		cam_main->position.y - cam_main->scale.y * .455, 0));
	//variation
	graphics_manager.GetRenderRect(ui_bottom.latencyvariation_rectid).SetVisibility(ui_bottom.latencyvariation_on);
	graphics_manager.GetRenderRect(ui_bottom.latencyvariation_rectid).Scale(glm::vec3(cam_main->scale.x * .035, cam_main->scale.y * .035, 0));
	graphics_manager.GetRenderRect(ui_bottom.latencyvariation_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .425,
		cam_main->position.y - cam_main->scale.y * .455, 0));
	//packetloss
	graphics_manager.GetRenderRect(ui_bottom.packetloss_rectid).SetVisibility(ui_bottom.packetloss_on);
	graphics_manager.GetRenderRect(ui_bottom.packetloss_rectid).Scale(glm::vec3(cam_main->scale.x * .035, cam_main->scale.y * .035, 0));
	graphics_manager.GetRenderRect(ui_bottom.packetloss_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .375,
		cam_main->position.y - cam_main->scale.y * .455, 0));

	//current chat type
	std::string typing_msg = "enter: ";
	if (global_state.typing)
	{
		std::vector<int> keys_pressed = InputManager.peekrecordingTmp();
		for (int i = 0; i < keys_pressed.size(); i++)
		{
			if (keys_pressed[i] != GLFW_KEY_ENTER)
				typing_msg += keys_pressed[i];
		}
		
	}
	graphics_manager.AddText(typing_msg.c_str(), -5, -4.85, 10 * .00035, (global_state.typing) ? glm::vec3(1, 1, 1) : glm::vec3(0,0,0));
	//chat messages
	for (int i = 0; i < chat_msgs.size(); i++)
	{
		graphics_manager.AddText(chat_msgs[i].c_str(), -5, -2.75 - .25*i, 10 * .00035, glm::vec3(0, 0, 0));
	}

	//game time
	std::string time_str = std::to_string((int)(global_state.game_time.getTime() / 1000.0));
	time_str = time_str.substr(0, 5);
	graphics_manager.AddText(time_str.c_str(), 4.25, -3.85, 10 * .0005, glm::vec3(0, 0, 0));

	//ping
	std::string ping_str = "ping: " + std::to_string(current_ping);
	ping_str = ping_str.substr(0, 11);
	graphics_manager.AddText(ping_str.c_str(), 4.0, -4.95, 10 * .00035, glm::vec3(1, 1, 1));

	//MSG UI
	if (global_state.msg_on) {
		graphics_manager.GetRenderRect(ui_msg.background_rectid).SetVisibility(true);
		graphics_manager.GetRenderRect(ui_msg.exit_rectid).SetVisibility(true);
	}
	else {
		graphics_manager.GetRenderRect(ui_msg.background_rectid).SetVisibility(false);
		graphics_manager.GetRenderRect(ui_msg.exit_rectid).SetVisibility(false);
	}

	graphics_manager.GetRenderRect(ui_msg.toggle_rectid).Scale(glm::vec3(cam_main->scale.x / 30, cam_main->scale.y / 30.0, 0));
	graphics_manager.GetRenderRect(ui_msg.toggle_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .475,
		cam_main->position.y + cam_main->scale.y * .475, 0));

	graphics_manager.GetRenderRect(ui_msg.background_rectid).Scale(glm::vec3(cam_main->scale.x / 2.5, cam_main->scale.y / 3.5, 0));
	graphics_manager.GetRenderRect(ui_msg.background_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .3,
		cam_main->position.y + cam_main->scale.y * .4, 0));

	graphics_manager.GetRenderRect(ui_msg.exit_rectid).Scale(glm::vec3(cam_main->scale.x / 30, cam_main->scale.y / 30.0, 0));
	graphics_manager.GetRenderRect(ui_msg.exit_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .475,
		cam_main->position.y + cam_main->scale.y * .375, 0));
	if (global_state.msg_on)
	{
		//graphics_manager.AddText("Exit Game", 1, 4.75, 10 * .0005, glm::vec3(0, 0, 0));
		//graphics_manager.AddText("tom: im speedrunning this", 1, 4.75 - .25, 10 * .0005, glm::vec3(0, 0, 0));
		//graphics_manager.AddText("blow: the design in this game sucks", 1, 4.75 - .25 * 2, 10 * .0005, glm::vec3(0, 0, 0));
	}

	//PLAYER UI
	if (global_state.player_info_on)
		graphics_manager.GetRenderRect(ui_player.background_rectid).SetVisibility(true);
	else
		graphics_manager.GetRenderRect(ui_player.background_rectid).SetVisibility(false);

	graphics_manager.GetRenderRect(ui_player.toggle_rectid).Scale(glm::vec3(cam_main->scale.x / 30, cam_main->scale.y / 30.0, 0));
	graphics_manager.GetRenderRect(ui_player.toggle_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .425,
		cam_main->position.y + cam_main->scale.y * .475, 0));

	graphics_manager.GetRenderRect(ui_player.background_rectid).Scale(glm::vec3(cam_main->scale.x / 2.5, cam_main->scale.y / 3.5, 0));
	graphics_manager.GetRenderRect(ui_player.background_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .3,
		cam_main->position.y + cam_main->scale.y * .4, 0));

	if (global_state.player_info_on)
	{
		graphics_manager.AddText("ice speedup: f", 1, 4.75, 10 * .00035, glm::vec3(0, 0, 0));
		graphics_manager.AddText("ground speedup: t", 1, 4.75 - .4, 10 * .00035, glm::vec3(0, 0, 0));
		graphics_manager.AddText("revive disk: e", 1, 4.75 - .4 * 2, 10 * .00035, glm::vec3(0, 0, 0));
		graphics_manager.AddText("stop moving: s", 1, 4.75 - .4 * 3, 10 * .00035, glm::vec3(0, 0, 0));
		graphics_manager.AddText("camera lock: c", 1, 4.75 - .4 * 4, 10 * .00035, glm::vec3(0, 0, 0));
		graphics_manager.AddText("camera center: spacebar", 1, 4.75 - .4 * 5, 10 * .00035, glm::vec3(0, 0, 0));
		graphics_manager.AddText("zoom: mouse wheel", 3, 4.75 - .4 * 3, 10 * .00035, glm::vec3(0, 0, 0));
	}
}


void ClientGameApp::LoadLevel1()
{
	shared::LoadLevel1Ground(graphics_manager, current_ground);

	shared::LoadLevel1Enemies(graphics_manager, current_enemies);

	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel1GameObjects(graphics_manager, gameobj_ids);
	my_assert(gameobj_ids.size() <= 20, "game objects loading cant be over 20");
	//set static gameobj ids (0-19 are reserved)
	for (int i = 0; i < gameobj_ids.size(); i++)
	{
		current_level_gameobjects[i] = gameobj_ids[i];
	}
}

void ClientGameApp::LoadLevel2()
{
	shared::LoadLevel2Ground(graphics_manager, current_ground);
	shared::LoadLevel2Enemies(graphics_manager, current_enemies);

	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel2GameObjects(graphics_manager, gameobj_ids);
	//set static gameobj ids (0-19 are reserved)
	for (int i = 0; i < gameobj_ids.size(); i++)
	{
		current_level_gameobjects[i] = gameobj_ids[i];
	}
}

void ClientGameApp::LoadLevel3()
{
	shared::LoadLevel3Ground(graphics_manager, current_ground);
	shared::LoadLevel3Enemies(graphics_manager, current_enemies);

	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel3GameObjects(graphics_manager, gameobj_ids);
	//set static gameobj ids (0-19 are reserved)
	for (int i = 0; i < gameobj_ids.size(); i++)
	{
		current_level_gameobjects[i] = gameobj_ids[i];
	}
}

void ClientGameApp::LoadLevel4()
{
	shared::LoadLevel4Ground(graphics_manager, current_ground);
	shared::LoadLevel4Enemies(graphics_manager, current_enemies);

	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel4GameObjects(graphics_manager, gameobj_ids);
	//set static gameobj ids (0-19 are reserved)
	for (int i = 0; i < gameobj_ids.size(); i++)
	{
		current_level_gameobjects[i] = gameobj_ids[i];
	}
}

void ClientGameApp::LoadLevel5()
{
	shared::LoadLevel5Ground(graphics_manager, current_ground);
	shared::LoadLevel5Enemies(graphics_manager, current_enemies);

	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel5GameObjects(graphics_manager, gameobj_ids);
	//set static gameobj ids (0-19 are reserved)
	for (int i = 0; i < gameobj_ids.size(); i++)
	{
		current_level_gameobjects[i] = gameobj_ids[i];
	}
	Mix_PlayMusic(audio_data.music_bossfight, 0);
}

void ClientGameApp::LoadLevel6()
{
	shared::LoadLevel6Ground(graphics_manager, current_ground);
	shared::LoadLevel6Enemies(graphics_manager, current_enemies);

	std::vector<uint32_t> gameobj_ids;
	shared::LoadLevel6GameObjects(graphics_manager, gameobj_ids);
	//set static gameobj ids (0-19 are reserved)
	for (int i = 0; i < gameobj_ids.size(); i++)
	{
		current_level_gameobjects[i] = gameobj_ids[i];
	}
	Mix_PlayMusic(audio_data.music_greatbigsled, 0);
	Mix_PlayChannel(2, audio_data.sound_cheers, 0);
}

void ClientGameApp::UnLoadGeneral()
{
	//enemies
	for (auto& enemy : current_enemies)
	{
		graphics_manager.RemoveRenderRect(enemy);
	}
	current_enemies.clear();

	//ground
	for (auto& ground : current_ground)
	{
		graphics_manager.RemoveRenderRect(ground);
	}
	current_ground.clear();

	//game objects
	for (auto& object : current_level_gameobjects)
	{
		graphics_manager.RemoveRenderRect(object.second);
	}
	current_level_gameobjects.clear();
}

void ClientGameApp::SwitchLevels(int new_level)
{
	//stop sounds
	if(global_state.current_level >= 5)
		Mix_HaltMusic();
	Mix_HaltChannel(0);
	Mix_HaltChannel(1);
	Mix_HaltChannel(2);
	Mix_HaltChannel(3);

	UnLoadGeneral();
	
	switch (new_level)
	{
	case 1: LoadLevel1(); break;
	case 2: LoadLevel2(); break;
	case 3: LoadLevel3(); break;
	case 4: LoadLevel4(); break;
	case 5: LoadLevel5(); break;
	case 6: LoadLevel6(); break;
	default: std::cout << "switching to invalid level: " << new_level << std::endl; break;
	}

	Mix_PlayChannel(4, audio_data.sound_endlevel, 0);

	if (cam_main)
	{
		cam_main->position = glm::vec3(0, 0, 15);
	}

	if(new_level == 1)
		global_state.game_time.Begin();

	global_state.current_level = new_level;
}

