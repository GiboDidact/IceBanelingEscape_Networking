#include "pchclient.h"
#include "ClientGameAppold.h"
#include "SDL.h"
#include "SDL_mixer.h"

//old standalone game, clientGameApp is main one that works with networking

struct audio_s
{
	Mix_Chunk* sound_death = 0;
	Mix_Chunk* sound_death2 = 0;
	Mix_Chunk* sound_death3 = 0;
	Mix_Chunk* sound_door = 0;
	Mix_Chunk* sound_revive = 0;
	Mix_Chunk* sound_zealot = 0;
	Mix_Chunk* sound_endlevel = 0;

	Mix_Music* music_wishlist = 0;
	Mix_Music* music_greatbigsled = 0;

	int death_sound = 0;
};
audio_s audio_data;

ClientGameAppold::ClientGameAppold()
{
	launchlobby = true;
	cam_main = nullptr;
}

int ClientGameAppold::Run()
{
	if (!initializeGame())
	{
		cleanUpGame();
		return -1;
	}
	mainLoopGame();
	cleanUpGame();

	return launchlobby;
}

int ClientGameAppold::initializeGame()
{
	if (graphics_manager.Initialize("Ice Baneling Escape", false) == -1)
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

	initializeData();

	return 1;
}

void ClientGameAppold::initializeData()
{
	//AUDIO
	audio_data.sound_death = Mix_LoadWAV("C:\\Users\\gibom\\Documents\\NetCode\\socket_2dshooter\\GameData\\Sounds\\effects\\deathsound.wav");
	audio_data.sound_death2 = Mix_LoadWAV("C:\\Users\\gibom\\Documents\\NetCode\\socket_2dshooter\\GameData\\Sounds\\effects\\deathsound2.wav");
	audio_data.sound_death3 = Mix_LoadWAV("C:\\Users\\gibom\\Documents\\NetCode\\socket_2dshooter\\GameData\\Sounds\\effects\\deathsound3.wav");
	audio_data.sound_door = Mix_LoadWAV("C:\\Users\\gibom\\Documents\\NetCode\\socket_2dshooter\\GameData\\Sounds\\effects\\gate.wav");
	audio_data.sound_revive = Mix_LoadWAV("C:\\Users\\gibom\\Documents\\NetCode\\socket_2dshooter\\GameData\\Sounds\\effects\\revive.wav");
	audio_data.sound_zealot = Mix_LoadWAV("C:\\Users\\gibom\\Documents\\NetCode\\socket_2dshooter\\GameData\\Sounds\\effects\\protoss3.wav");
	audio_data.sound_endlevel = Mix_LoadWAV("C:\\Users\\gibom\\Documents\\NetCode\\socket_2dshooter\\GameData\\Sounds\\effects\\endlevel.wav");

	Mix_VolumeChunk(audio_data.sound_death, 25);
	Mix_VolumeChunk(audio_data.sound_death2, MIX_MAX_VOLUME);
	Mix_VolumeChunk(audio_data.sound_death3, MIX_MAX_VOLUME);
	Mix_VolumeChunk(audio_data.sound_revive, 50);
	Mix_VolumeChunk(audio_data.sound_zealot, 50);
	Mix_VolumeChunk(audio_data.sound_door, 50);

	Mix_VolumeMusic(30);
	audio_data.music_wishlist = Mix_LoadMUS("C:\\Users\\gibom\\Documents\\NetCode\\socket_2dshooter\\GameData\\Sounds\\music\\wishlist.wav");
	audio_data.music_greatbigsled = Mix_LoadMUS("C:\\Users\\gibom\\Documents\\NetCode\\socket_2dshooter\\GameData\\Sounds\\music\\greatbigsled.wav");

	//global
	global_state.Init();
	global_state.background_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(global_state.background_rectid).Move(glm::vec3(0, 0, 0));
	graphics_manager.GetRenderRect(global_state.background_rectid).SetTexture(18);
	graphics_manager.GetRenderRect(global_state.background_rectid).SetLayer(0);

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

	//msgui
	ui_msg.toggle_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_msg.toggle_rectid).SetTexture(20);
	graphics_manager.GetRenderRect(ui_msg.toggle_rectid).SetLayer(5);

	ui_msg.background_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_msg.background_rectid).SetTexture(30);
	graphics_manager.GetRenderRect(ui_msg.background_rectid).SetLayer(4);
	graphics_manager.GetRenderRect(ui_msg.background_rectid).SetVisibility(false);

	//playerui
	ui_player.toggle_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_player.toggle_rectid).SetTexture(24);
	graphics_manager.GetRenderRect(ui_player.toggle_rectid).SetLayer(5);

	ui_player.background_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ui_player.background_rectid).SetTexture(30);
	graphics_manager.GetRenderRect(ui_player.background_rectid).SetLayer(4);
	graphics_manager.GetRenderRect(ui_player.background_rectid).SetVisibility(false);

	//PLAYER
	main_player.Init();
	main_player.render_id = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(main_player.render_id).Scale(glm::vec3(.5, .5, 0));
	graphics_manager.GetRenderRect(main_player.render_id).Move(glm::vec3(0, 0, 0));
	graphics_manager.GetRenderRect(main_player.render_id).SetTexture(4);
	graphics_manager.GetRenderRect(main_player.render_id).SetLayer(3);

	main_player.respawn_rectid = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(main_player.respawn_rectid).Scale(glm::vec3(.5, 1.5, 0));
	graphics_manager.GetRenderRect(main_player.respawn_rectid).Move(glm::vec3(0, 0, 0));
	graphics_manager.GetRenderRect(main_player.respawn_rectid).SetTexture(31);
	graphics_manager.GetRenderRect(main_player.respawn_rectid).SetVisibility(false);
	graphics_manager.GetRenderRect(main_player.respawn_rectid).SetLayer(2);

	//CAMERA
	cam_main = new Camera(glm::vec3(0, 0, 15), glm::vec3(0, 1, 0), 270.0f, 90.0f);	
	if (cam_main)
	{
		cam_main->scale.x = 10;
		cam_main->scale.y = 10;
	}
}

void ClientGameAppold::LoadLevel1()
{
	level1_state.spawn_location = glm::vec2(-1, 0);
	graphics_manager.GetRenderRect(main_player.render_id).Translate(glm::vec3(level1_state.spawn_location, 0));
	graphics_manager.GetRenderRect(main_player.respawn_rectid).Translate(glm::vec3(level1_state.spawn_location, 0));
	//GROUND-lvl1
	ground ground_start;
	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ground_start.render_id).Scale(glm::vec3(2, 2, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).Move(glm::vec3(-1, 0, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).SetTexture(6);
	graphics_manager.GetRenderRect(ground_start.render_id).SetLayer(1);
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ground_start.render_id).Scale(glm::vec3(2, 2, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).Move(glm::vec3(2, 2, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).SetTexture(6);
	graphics_manager.GetRenderRect(ground_start.render_id).SetLayer(1);
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::GRASS;
	ground_start.render_id = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ground_start.render_id).Scale(glm::vec3(2, 2, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).Move(glm::vec3(5, 0, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).SetTexture(6);
	graphics_manager.GetRenderRect(ground_start.render_id).SetLayer(1);
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::ICE;
	ground_start.render_id = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ground_start.render_id).Scale(glm::vec3(4, 2, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).Move(glm::vec3(2, 0, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).SetTexture(7);
	graphics_manager.GetRenderRect(ground_start.render_id).SetLayer(1);
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::REVERSE;
	ground_start.render_id = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ground_start.render_id).Scale(glm::vec3(2, 5, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).Move(glm::vec3(-1, -3.5, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).SetTexture(12);
	graphics_manager.GetRenderRect(ground_start.render_id).SetLayer(1);
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::FAST;
	ground_start.render_id = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ground_start.render_id).Scale(glm::vec3(4, 2, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).Move(glm::vec3(-4, 0, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).SetTexture(14);
	graphics_manager.GetRenderRect(ground_start.render_id).SetLayer(1);
	current_ground.push_back(ground_start);

	ground_start.type = GROUNDTYPE::EXIT;
	ground_start.render_id = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(ground_start.render_id).Scale(glm::vec3(2, 2, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).Move(glm::vec3(7, 0, 0));
	graphics_manager.GetRenderRect(ground_start.render_id).SetTexture(13);
	graphics_manager.GetRenderRect(ground_start.render_id).SetLayer(1);
	current_ground.push_back(ground_start);

	//ENEMIES-lvl1
	enemy enemy_tmp;
	enemy_tmp.dir = true;
	enemy_tmp.speed = glm::vec2(0, .01);
	enemy_tmp.distance = 3.5;
	enemy_tmp.distance_measure = 0;
	enemy_tmp.render_id = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(enemy_tmp.render_id).Scale(glm::vec3(.75, .75, 0));
	graphics_manager.GetRenderRect(enemy_tmp.render_id).Move(glm::vec3(2, -1.5, 0));
	graphics_manager.GetRenderRect(enemy_tmp.render_id).SetTexture(5);
	graphics_manager.GetRenderRect(enemy_tmp.render_id).SetLayer(3);
	current_enemies.push_back(enemy_tmp);

	//GAME OBJECTS
	level1_state.button_1.activated = false;
	level1_state.button_1.render_id = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(level1_state.button_1.render_id).Scale(glm::vec3(.75, .75, 0));
	graphics_manager.GetRenderRect(level1_state.button_1.render_id).Move(glm::vec3(2, 2, 0));
	graphics_manager.GetRenderRect(level1_state.button_1.render_id).SetTexture(8);
	graphics_manager.GetRenderRect(level1_state.button_1.render_id).SetLayer(2);
	global_state.mouse_left = false;
	global_state.mouse_right = false;

	level1_state.gate_1.activated = true;
	level1_state.gate_1.render_id = graphics_manager.CreateRenderRect();
	graphics_manager.GetRenderRect(level1_state.gate_1.render_id).Scale(glm::vec3(1, 2, 0));
	graphics_manager.GetRenderRect(level1_state.gate_1.render_id).Move(glm::vec3(4.5, 0, 0));
	graphics_manager.GetRenderRect(level1_state.gate_1.render_id).SetTexture(9);
	graphics_manager.GetRenderRect(level1_state.gate_1.render_id).SetLayer(2);
}

void ClientGameAppold::unLoadLevel1()
{
	unLoadEnemyGround();
	graphics_manager.RemoveRenderRect(level1_state.button_1.render_id);
	graphics_manager.RemoveRenderRect(level1_state.gate_1.render_id);
}

void ClientGameAppold::unLoadEnemyGround()
{
	for (int i = 0; i < current_enemies.size(); i++)
	{
		graphics_manager.RemoveRenderRect(current_enemies[i].render_id);
		current_enemies.erase(current_enemies.begin() + i);
		i--;
	}
	for (int i = 0; i < current_ground.size(); i++)
	{
		graphics_manager.RemoveRenderRect(current_ground[i].render_id);
		current_ground.erase(current_ground.begin() + i);
		i--;
	}
}

void ClientGameAppold::SwitchLevels(int new_level, bool first)
{
	//unload goo and disks
	for (int i = 0; i < goo_rects.size(); i++)
	{
		graphics_manager.RemoveRenderRect(goo_rects[i].render_id);
		goo_rects.erase(goo_rects.begin() + i);
		i--;
	}
	for (int i = 0; i < disk_rects.size(); i++)
	{
		graphics_manager.RemoveRenderRect(disk_rects[i].render_id);
		disk_rects.erase(disk_rects.begin() + i);
		i--;
	}

	//unload current level
	if (!first)
	{
		switch (global_state.current_level)
		{
			case 1: unLoadLevel1(); break;
		}
	}

	//load new level
	switch (global_state.current_level)
	{
		case 1: LoadLevel1(); break;
	}

	Mix_PlayChannel(4, audio_data.sound_endlevel, 0);

	if (!main_player.alive)
		playerRevive();
	main_player.Init();
	graphics_manager.GetRenderRect(main_player.respawn_rectid).SetVisibility(true);
	main_player.respawn_animation_timer.Begin();
	main_player.respawn_on = true;
}

void ClientGameAppold::mainLoopGame()
{
	SwitchLevels(global_state.current_level, true);
	global_state.game_time.Begin();

	if (!Mix_PlayingMusic())
		Mix_PlayMusic(audio_data.music_greatbigsled, 0);
	
	//update loop
	//This is the fixed timestep of our simulation loop in milliseconds. We simulate in fixed steps, the renderer will interpolate the remainders.
	//.01666 is 1/60th of a frame. so a little less than that. Too fast and slow computers might not be able to catch up.
	const double MS_PER_UPDATE = 10.0; //10.0
	//This is the max time in milliseconds that we will simulate per frame. If computer is super slow and reaches this, it will go out of sync.
	const double MS_MAX_TIME = 1000.0; //1000.0
	//accumulated time passed that we need to simulate
	double lag = 0.0f;
	auto current_time = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(graphics_manager.getWindow()))
	{
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

		//INPUT
		glfwPollEvents();

		if (InputManager.GetKeyPressOnce(GLFW_KEY_P)) {
			//std::cout << graphics_manager.GetRenderRectCount() << " ";
			SwitchLevels(1);
			//std::cout << graphics_manager.GetRenderRectCount() << "\n";
		}

		InputManager.GetMouseButtonsOnce(global_state.mouse_left, global_state.mouse_right);

		if (InputManager.GetKeyPressOnce(GLFW_KEY_5)) {
			global_state.camera_lock = !global_state.camera_lock;
		}
		if (InputManager.GetKeyPressOnce(GLFW_KEY_SPACE) || global_state.camera_lock) {
			cam_main->position = glm::vec3(graphics_manager.GetRenderRect(main_player.render_id).getPos(), 15);
		}

		double mx, my;
		InputManager.GetMousePos(mx, my);
		glm::vec2 mouse_pos = mousetoworldpos(glm::vec2(mx, my));
		if (global_state.mouse_left && point_in_rect_collision_test(mouse_pos, ui_player.toggle_rectid)) {
			global_state.player_info_on = !global_state.player_info_on;
			global_state.msg_on = false;
		}

		if (global_state.mouse_left && point_in_rect_collision_test(mouse_pos, ui_msg.toggle_rectid)) {
			global_state.msg_on = !global_state.msg_on;
			global_state.player_info_on = false;
		}
		
		if (InputManager.GetKeyPressOnce(GLFW_KEY_R) && !main_player.alive)
			playerRevive();

		//handle player input
		playerInput();

		//simulate with a delta time fixed time step, and based on fps just spam do it or less
		float deltaT = 1.0f; //milliseconds
		//std::cout << lag << std::endl;
		while (lag >= MS_PER_UPDATE)
		{
			handlecamera(deltaT);
			Simulate(deltaT);
			Resolve();

			lag -= MS_PER_UPDATE;
		}

		//render
		playerAnimation();	
		handleUI();
		graphics_manager.Render(cam_main);
		//if(graphics_manager.getCursor())
			//glfwSetCursor(graphics_manager.getWindow(), graphics_manager.getCursor());
		glfwSwapBuffers(graphics_manager.getWindow());
	}
}

void ClientGameAppold::handlecamera(float deltaT)
{
	double x, y;
	InputManager.GetMousePos(x, y);
	//[0,1]
	glm::vec2 pos_percent = glm::vec2(x, y) / glm::vec2(graphics_manager.getscreenwidth(), graphics_manager.getscreenheight());
	pos_percent.y = (1-pos_percent.y);

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

	//this is each input, so speed doesn't matter on computer
	int scroll_y_offset = InputManager.GetScrollOnce();
	if (scroll_y_offset > 0)
	{
		//zoom in
		cam_main->scale =  glm::max(glm::vec3(SCROLL_MIN_SCALE), cam_main->scale - glm::vec3(SCROLL_SPEED, SCROLL_SPEED, 0));
	}
	else if (scroll_y_offset < 0)
	{
		//zoom out
		cam_main->scale = glm::min(glm::vec3(SCROLL_MAX_SCALE), cam_main->scale + glm::vec3(SCROLL_SPEED, SCROLL_SPEED, 0));
	}
}

void ClientGameAppold::playerDie()
{
	if (main_player.alive)
	{
		switch (audio_data.death_sound)
		{
		case 0: Mix_PlayChannel(0, audio_data.sound_death, 0); break;
		case 1: Mix_PlayChannel(0, audio_data.sound_death2, 0); break;
		case 2: Mix_PlayChannel(0, audio_data.sound_death3, 0); break;
		}
		audio_data.death_sound = (audio_data.death_sound + 1) % 3;

		main_player.alive = false;
		main_player.moving_to_waypoint = false;
		main_player.dir = glm::vec2(0, 0);

		graphics_manager.GetRenderRect(main_player.render_id).Rotate(glm::vec3(0, 0, 1), glm::degrees(0.0));
		main_player.rot_angle = 0.0;
		graphics_manager.GetRenderRect(main_player.render_id).Reflect(false);

		graphics_manager.GetRenderRect(main_player.render_id).SetTexture(11);
	}
}

void ClientGameAppold::playerRevive()
{
	Mix_PlayChannel(0, audio_data.sound_revive, 0);

	main_player.alive = true;
	graphics_manager.GetRenderRect(main_player.render_id).SetTexture(4);
	graphics_manager.GetRenderRect(main_player.respawn_rectid).SetVisibility(true);

	main_player.respawn_animation_timer.Begin();
	main_player.respawn_on = true;
	if (global_state.current_level == 1) {
		graphics_manager.GetRenderRect(main_player.render_id).Translate(glm::vec3(level1_state.spawn_location, 0));
		graphics_manager.GetRenderRect(main_player.respawn_rectid).Translate(glm::vec3(level1_state.spawn_location, 0));
	}
}

void ClientGameAppold::playerAnimation()
{
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

	//players clientside respawn animation
	if (main_player.respawn_on && main_player.respawn_animation_timer.TimeElapsed(1000))
	{
		main_player.respawn_on = false;
		graphics_manager.GetRenderRect(main_player.respawn_rectid).SetVisibility(false);
	}
}

void ClientGameAppold::playerInput()
{
	if(!main_player.ability_e)
		main_player.ability_e = InputManager.GetKeyPressOnce(GLFW_KEY_E);
	if (!main_player.ability_f)
		main_player.ability_f = InputManager.GetKeyPressOnce(GLFW_KEY_F);
	if (!main_player.ability_mr) {
		main_player.ability_mr = global_state.mouse_right;
	}
	if (global_state.mouse_right) {
		double x, y;
		InputManager.GetMousePos(x, y);
		main_player.ability_mx = x;
		main_player.ability_my = y;
	}
	if (!main_player.ability_s)
		main_player.ability_s = InputManager.GetKeyPressOnce(GLFW_KEY_S);
	if (!main_player.ability_t)
		main_player.ability_t = InputManager.GetKeyPressOnce(GLFW_KEY_T);
}

void ClientGameAppold::Simulate(float deltaT)
{
	simulate_enemies(deltaT);
	simulate_objects(deltaT);
	simulate_player(deltaT);
}

void ClientGameAppold::simulate_enemies(float deltaT)
{
	for (enemy& _enemy : current_enemies)
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
}

void ClientGameAppold::simulate_objects(float deltaT)
{
	//simulate goo
	for (int i = 0; i < goo_rects.size(); i++)
	{
		goo_rects[i].timer += deltaT;
		if (goo_rects[i].timer >= GOO_LIFETIME) {
			graphics_manager.RemoveRenderRect(goo_rects[i].render_id);
			goo_rects.erase(goo_rects.begin() + i);
			i--;
		}
	}

	//simulate disk
	for (int i = 0; i < disk_rects.size(); i++)
	{
		disk_rects[i].timer += deltaT;
		if (disk_rects[i].timer >= DISK_LIFETIME) {
			graphics_manager.RemoveRenderRect(disk_rects[i].render_id);
			disk_rects.erase(disk_rects.begin() + i);
			i--;
		}
		else
		{
			graphics_manager.GetRenderRect(disk_rects[i].render_id).Move(glm::vec3(disk_rects[i].dir * DISK_SPEED * deltaT, 0));
		}
	}
}

void ClientGameAppold::simulate_player(float deltaT)
{
	//MOVEMENT
	if (main_player.alive && main_player.ability_mr)
	{
		//double x, y;
		//InputManager.GetMousePos(x, y);
		glm::vec2 world_pos = mousetoworldpos(glm::vec2(main_player.ability_mx, main_player.ability_my));

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
				main_player.click_pos = curr_pos + glm::normalize(glm::vec2(1,0)) * glm::vec2(min_radius, min_radius);
			}
		}
	}
	if (main_player.alive && main_player.ability_s && !main_player.onIce()) {
		main_player.moving_to_waypoint = false;
	}

	if (main_player.moving_to_waypoint)
	{
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
	if(!main_player.goo_available)
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
					glm::vec3(graphics_manager.GetRenderRect(main_player.render_id).getPos(),0));
				graphics_manager.GetRenderRect(new_goo.render_id).Rotate(glm::vec3(0, 0, 1), glm::degrees(main_player.rot_angle));
				graphics_manager.GetRenderRect(new_goo.render_id).Scale(glm::vec3(.5, .5, 0));
				graphics_manager.GetRenderRect(new_goo.render_id).SetTexture(15);
				graphics_manager.GetRenderRect(new_goo.render_id).SetLayer(2);
				new_goo.timer = 0;

				goo_rects.push_back(new_goo);
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

		disk_rects.push_back(new_disk);
	}
	if (!main_player.disk_avaiable)
	{
		main_player.disk_timer += deltaT;
		if(main_player.disk_timer >= DISKPOWER_CD_TIME)
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
		if(main_player.alive)
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

void ClientGameAppold::Resolve()
{
	if (main_player.alive)
	{
		resolve_enemies();
		resolve_ground();
		resolve_objects();
	}
}

void ClientGameAppold::resolve_enemies()
{
	bool collision = false;
	for (const auto& enemy : current_enemies)
	{
		if (rect_collision_test(main_player.render_id, enemy.render_id)) 
		{
			Mix_PlayChannel(1, audio_data.sound_zealot, 0);
			collision = true;
			break;
		}
	}
	if (collision)
		playerDie();
}

void ClientGameAppold::resolve_ground()
{
	bool collision = false;
	GROUNDTYPE old_ground = main_player.current_ground;
	for (const auto& ground : current_ground)
	{
		if (point_in_rect_collision_test(graphics_manager.GetRenderRect(main_player.render_id).getPos(), ground.render_id)) 
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
		else if (main_player.current_ground == GROUNDTYPE::EXIT) {
			//go next level
		}
	}
	else
	{
		playerDie();
	}
}

void ClientGameAppold::resolve_objects()
{
	//disk
	for (int i = 0; i < disk_rects.size(); i++)
	{
		if (main_player.alive == false && rect_collision_test(disk_rects[i].render_id, main_player.render_id))
		{
			playerRevive();
			break;
		}
	}

	//goo
	main_player.ongoo = collidewithgoo();

	//button
	if (!level1_state.button_1.activated &&
		 point_in_rect_collision_test(graphics_manager.GetRenderRect(main_player.render_id).getPos(), level1_state.button_1.render_id))
	{
		level1_state.button_1.activated = true;
		graphics_manager.GetRenderRect(level1_state.button_1.render_id).SetTexture(10);


		level1_state.gate_1.activated = false;
		graphics_manager.GetRenderRect(level1_state.gate_1.render_id).SetVisibility(0);

		Mix_PlayChannel(2, audio_data.sound_door, 0);
	}

	//gate
	if (level1_state.gate_1.activated && rect_collision_test(main_player.render_id, level1_state.gate_1.render_id))
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
	}
}

bool ClientGameAppold::collidewithgoo()
{
	for (const auto& gooz : goo_rects)
	{
		if (rect_collision_test(main_player.render_id, gooz.render_id))
		{
			return true;
		}
	}
	return false;
}

bool ClientGameAppold::rect_collision_test(uint32_t render_id_a, uint32_t render_id_b)
{
	glm::vec2 pos_a =	graphics_manager.GetRenderRect(render_id_a).getPos();
	glm::vec2 halfs_a = graphics_manager.GetRenderRect(render_id_a).getScale() * glm::vec2(.5,.5);
	
	glm::vec2 pos_b = graphics_manager.GetRenderRect(render_id_b).getPos();
	glm::vec2 halfs_b = graphics_manager.GetRenderRect(render_id_b).getScale() * glm::vec2(.5, .5);

	if (pos_a.x + halfs_a.x < pos_b.x - halfs_b.x)
		return false;
	if (pos_a.x - halfs_a.x > pos_b.x + halfs_b.x)
		return false;
	if (pos_a.y + halfs_a.y < pos_b.y - halfs_b.y)
		return false;
	if (pos_a.y - halfs_a.y > pos_b.y + halfs_b.y)
		return false;
	return true;
}

bool ClientGameAppold::point_in_rect_collision_test(glm::vec2 pos, uint32_t render_id)
{
	glm::vec2 pos_a = graphics_manager.GetRenderRect(render_id).getPos();
	glm::vec2 halfs = graphics_manager.GetRenderRect(render_id).getScale() * glm::vec2(.5, .5);

	if (pos.x < pos_a.x - halfs.x)
		return false;
	if (pos.x > pos_a.x + halfs.x)
		return false;
	if (pos.y < pos_a.y - halfs.y)
		return false;
	if (pos.y > pos_a.y + halfs.y)
		return false;
	return true;
}

glm::vec2 ClientGameAppold::mousetoworldpos(glm::vec2 mousepos)
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

void ClientGameAppold::cleanUpGame()
{
	if(cam_main)
		delete cam_main;
	graphics_manager.cleanUp();

	Mix_FreeChunk(audio_data.sound_death);
	Mix_FreeChunk(audio_data.sound_death2);
	Mix_FreeChunk(audio_data.sound_death3);
	Mix_FreeChunk(audio_data.sound_endlevel);
	Mix_FreeChunk(audio_data.sound_door);
	Mix_FreeChunk(audio_data.sound_revive);
	Mix_FreeChunk(audio_data.sound_zealot);

	Mix_FreeMusic(audio_data.music_wishlist);
	Mix_FreeMusic(audio_data.music_greatbigsled);
	Mix_CloseAudio();
	SDL_Quit();
}

void ClientGameAppold::handleUI()
{
	//BACKGROUND
	graphics_manager.GetRenderRect(global_state.background_rectid).Scale(glm::vec3(cam_main->scale.x, cam_main->scale.y, 0));
	graphics_manager.GetRenderRect(global_state.background_rectid).Translate(glm::vec3(cam_main->position.x, cam_main->position.y, 0));

	//BOTTOM UI
	graphics_manager.GetRenderRect(ui_bottom.background_rectid).Scale(glm::vec3(cam_main->scale.x * .45, cam_main->scale.y / 6.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.background_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .3,
		cam_main->position.y - cam_main->scale.y / 2.4, 0));

	graphics_manager.GetRenderRect(ui_bottom.speed_rectid).Scale(glm::vec3(cam_main->scale.x / 20, cam_main->scale.y / 20.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.speed_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .275,
		cam_main->position.y - cam_main->scale.y * .375, 0));

	if (!main_player.speed_available)
		graphics_manager.GetRenderRect(ui_bottom.speed_cd_rectid).SetVisibility(true);
	else
		graphics_manager.GetRenderRect(ui_bottom.speed_cd_rectid).SetVisibility(false);

	float cd_scale_percent = 1.0 - (main_player.speed_timer / SPEEDPOWER_CD_TIME);
	float diff = (cam_main->scale.y * .05) - ((cam_main->scale.y * .05) * cd_scale_percent);
	graphics_manager.GetRenderRect(ui_bottom.speed_cd_rectid).Scale(glm::vec3(cam_main->scale.x / 20, (cam_main->scale.y * .05) * cd_scale_percent, 0));
	graphics_manager.GetRenderRect(ui_bottom.speed_cd_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .275,
		(cam_main->position.y - cam_main->scale.y * .375) - diff * .5, 0));

	graphics_manager.GetRenderRect(ui_bottom.disk_rectid).Scale(glm::vec3(cam_main->scale.x / 20, cam_main->scale.y / 20.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.disk_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .35,
		cam_main->position.y - cam_main->scale.y * .375, 0));

	if (!main_player.disk_avaiable)
		graphics_manager.GetRenderRect(ui_bottom.disk_cd_rectid).SetVisibility(true);
	else
		graphics_manager.GetRenderRect(ui_bottom.disk_cd_rectid).SetVisibility(false);

	cd_scale_percent = 1.0 - (main_player.disk_timer / DISKPOWER_CD_TIME);
	diff = (cam_main->scale.y * .05) - ((cam_main->scale.y * .05) * cd_scale_percent);
	graphics_manager.GetRenderRect(ui_bottom.disk_cd_rectid).Scale(glm::vec3(cam_main->scale.x / 20, (cam_main->scale.y * .05) * cd_scale_percent, 0));
	graphics_manager.GetRenderRect(ui_bottom.disk_cd_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .35,
		(cam_main->position.y - cam_main->scale.y * .375) - diff * .5, 0));


	graphics_manager.GetRenderRect(ui_bottom.goo_rectid).Scale(glm::vec3(cam_main->scale.x / 20, cam_main->scale.y / 20.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.goo_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .2,
		cam_main->position.y - cam_main->scale.y * .375, 0));

	if (!main_player.goo_available)
		graphics_manager.GetRenderRect(ui_bottom.goo_cd_rectid).SetVisibility(true);
	else
		graphics_manager.GetRenderRect(ui_bottom.goo_cd_rectid).SetVisibility(false);

	cd_scale_percent = 1.0 - (main_player.goo_timer / GOOPOWER_CD_TIME);
	diff = (cam_main->scale.y * .05) - ((cam_main->scale.y * .05) * cd_scale_percent);
	graphics_manager.GetRenderRect(ui_bottom.goo_cd_rectid).Scale(glm::vec3(cam_main->scale.x / 20, (cam_main->scale.y * .05) * cd_scale_percent, 0));
	graphics_manager.GetRenderRect(ui_bottom.goo_cd_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .2,
		(cam_main->position.y - cam_main->scale.y * .375) - diff * .5, 0));

	if (global_state.camera_lock || InputManager.GetKeyPress(GLFW_KEY_SPACE))
		graphics_manager.GetRenderRect(ui_bottom.lock_rectid).SetTexture(29);
	else
		graphics_manager.GetRenderRect(ui_bottom.lock_rectid).SetTexture(23);

	graphics_manager.GetRenderRect(ui_bottom.lock_rectid).Scale(glm::vec3(cam_main->scale.x / 20, cam_main->scale.y / 20.0, 0));
	graphics_manager.GetRenderRect(ui_bottom.lock_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .235,
		cam_main->position.y - cam_main->scale.y * .45, 0));

	if (InputManager.GetKeyPress(GLFW_KEY_S))
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

	std::string time_str = std::to_string(global_state.game_time.getTime() / 1000.0 / 60.0);
	time_str = time_str.substr(0, 4);
	time_str[1] = ':';
	graphics_manager.AddText(time_str.c_str(), 4.25, -3.85, 10 * .0005, glm::vec3(0, 0, 0));

	//MSG UI
	if (global_state.msg_on)
		graphics_manager.GetRenderRect(ui_msg.background_rectid).SetVisibility(true);
	else
		graphics_manager.GetRenderRect(ui_msg.background_rectid).SetVisibility(false);

	graphics_manager.GetRenderRect(ui_msg.toggle_rectid).Scale(glm::vec3(cam_main->scale.x / 30, cam_main->scale.y / 30.0, 0));
	graphics_manager.GetRenderRect(ui_msg.toggle_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .475,
		cam_main->position.y + cam_main->scale.y * .475, 0));

	graphics_manager.GetRenderRect(ui_msg.background_rectid).Scale(glm::vec3(cam_main->scale.x / 2.5, cam_main->scale.y / 3.5, 0));
	graphics_manager.GetRenderRect(ui_msg.background_rectid).Translate(glm::vec3(cam_main->position.x + cam_main->scale.x * .3,
		cam_main->position.y + cam_main->scale.y * .4, 0));

	if (global_state.msg_on)
	{
		graphics_manager.AddText("jack: sup", 1, 4.75, 10 * .0005, glm::vec3(0, 0, 0));
		graphics_manager.AddText("tom: im speedrunning this", 1, 4.75 - .25, 10 * .0005, glm::vec3(0, 0, 0));
		graphics_manager.AddText("blow: the design in this game sucks", 1, 4.75 - .25 * 2, 10 * .0005, glm::vec3(0, 0, 0));
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
		graphics_manager.AddText("player 1:       ping: 52", 1, 4.75, 10 * .0005, glm::vec3(0, 0, 0));
		graphics_manager.AddText("player 2:       ping: 71", 1, 4.75 - .5, 10 * .0005, glm::vec3(0, 0, 0));
		graphics_manager.AddText("player 3:       ping: 15", 1, 4.75 - .5 * 2, 10 * .0005, glm::vec3(0, 0, 0));
	}
}