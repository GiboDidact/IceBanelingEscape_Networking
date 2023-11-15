#pragma once
#include "pch.h"

#include <fstream>
#include <map>
#include <queue>
#include "ThirdParty/stb_image.h"

#include <ft2build.h>
#include FT_FREETYPE_H


class GraphicsManager
{
public:
	GraphicsManager()
	{
		is_full_screen = false;
		SCREEN_WIDTH = 1000;
		SCREEN_HEIGHT = 1000;
		cam_save = nullptr;

		std::fill(free_id.begin(), free_id.end(), false);
		last_id = 0;

		FOV = 60.0f;
		N = 0.1f;
		F = 100.0f;
	}

	void setFOV(float fov)
	{
		FOV = fov;
	}

	int Initialize(std::string window_name, bool fullscreen)
	{	
		is_full_screen = fullscreen;
		//initialize GLFW
		if (!glfwInit())
		{
			Logger::Log(Logger::_ERROR, "glfw could not initialize!");
			return -1;
		}

		//create window
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); 
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_SAMPLES, 8);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

		window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, window_name.c_str(), NULL, NULL);
		if (!window)
		{
			Logger::Log(Logger::_ERROR, "Could not create glfw Window!");
			return -1;
		}
	
		glfwMakeContextCurrent(window);

		stbi_set_flip_vertically_on_load(false);
		//set window icon
		GLFWimage images[1];
		int bitDepth = -1;
		std::string path_name = config::start_path + "GameData/Images/baneling2.jpg";
		
		images[0].pixels = stbi_load(path_name.c_str(), &images[0].width, &images[0].height, &bitDepth, STBI_rgb_alpha);
		if (images[0].pixels == nullptr) {
			std::cout << "can't load " << path_name<<std::endl;
		}
		else
		{
			glfwSetWindowIcon(window, 1, images);
			if (images[0].pixels)
				stbi_image_free(images[0].pixels);
		}
		
		//cursor
		bitDepth = -1;
		path_name = config::start_path + "GameData/Images/scmouse.png";
		images[0].pixels = stbi_load(path_name.c_str(), &images[0].width, &images[0].height, &bitDepth, STBI_rgb_alpha);

		if (images[0].pixels == nullptr) {
			std::cout << "can't load " << path_name << std::endl;
		}
		else
		{
			cursor = glfwCreateCursor(&images[0], 1, 1);
			if (cursor) {
				glfwSetCursor(window, cursor);
			}
			else
			{
				std::cout << "cursor couldn't be created!\n";
			}
			if (images[0].pixels)
				stbi_image_free(images[0].pixels);
		}
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		
		//initialize GLEW
		if (glewInit() != GLEW_OK)
		{
			Logger::Log(Logger::_ERROR, "GLEW could not initialize!");
			return -1;
		}
		fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
		std::cout << "GLFW VERSION: " << glfwGetVersionString() << std::endl;

		if (is_full_screen)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
			std::cout << mode->width << " " << mode->height << std::endl;
			WindowSizeChanged(mode->width, mode->height);
		}


		Logger::Log(Logger::_INFO, "GL_VERSION: ", glGetString(GL_VERSION));

		//Initialize OpenGL
		glfwSwapInterval(1);

		GLint texture_units;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
		Logger::Log(Logger::_INFO, "max texture slots: ", texture_units);

		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_FALSE);
		glClearDepth(1.0);
		glClearColor(0.7, 0.7, 0.7, 1.0);

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		CreateGeometry();
		SetupShaders();
		CreateTextures();

		setupFonts();

		checkError("end of initialize");
		return 1;
	}

	void Render(Camera* cam_main)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//SQUARE Rendering
		glUseProgram(program_main);
		glBindVertexArray(square_vao);

		glm::mat4 v_mat = cam_main->CreateLookAtFunction();
		float factor = 0.01;
		glm::mat4 p_mat = cam_main->CreateOrthoMat(N, F);
		glm::mat4 pv_mat = p_mat * v_mat;
		for (int layer_id = 0; layer_id <= RenderRect::layer_count; layer_id++)
		{
			for (int i = 0; i < Render_Rects.size(); i++)
			{
				if (free_id[i])
				{
					RenderRect& rect = Render_Rects[i];
					if (!rect.invisible && rect.layer == layer_id)
					{
						glm::mat4 pvm_mat = pv_mat * rect.GetMatrix();
						glUniformMatrix4fv(program_main_mvp, 1, GL_FALSE, glm::value_ptr(pvm_mat));

						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, texture_ids[rect.GetTextureID()]);

						glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					}
				}
			}
		}

		//TEXT
		while (!rendertextqueue.empty())
		{
			RenderText& text = rendertextqueue.front();
			RenderTextFont(cam_main, text.text, text.x, text.y, text.scale, text.color);
			rendertextqueue.pop();
		}
		checkError("end of render loop: text");
	
		checkError("end of render loop: static");	
	}

	void cleanUp()
	{
		glDeleteBuffers(1, &square_vbo);
		glDeleteBuffers(1, &square_ibo);
		glDeleteBuffers(1, &square_vbo);
		glDeleteBuffers(1, &square_vbo);
		glDeleteBuffers(1, &VBO_font);
		glDeleteVertexArrays(1, &square_vao);
		glDeleteVertexArrays(1, &VAO_font);
		
		glDeleteProgram(program_main);
		glDeleteProgram(program_main_mvp);
		glDeleteProgram(program_font);
		glDeleteProgram(program_font_proj);
		glDeleteProgram(program_font_color);

		glDeleteTextures(texture_ids.size(), texture_ids.data());

		for (auto itr = Characters.begin(); itr != Characters.end(); itr++)
		{
			glDeleteTextures(1, &itr->second.TextureID);
		}

		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		glfwDestroyCursor(cursor);

		if(window)
			glfwDestroyWindow(window);
		glfwTerminate();
	}

	void WindowSizeChanged(int w, int h)
	{
		if (w <= 0) w = 1;
		if (h <= 0) h = 1;

		SCREEN_WIDTH = w;
		SCREEN_HEIGHT = h;
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		if (cam_save)
		{
			//w/h  = aspectratio    10/y = aspectationm  10/aspectratio    (10*h)/w = y
			cam_save->scale.x = 10;
			cam_save->scale.y = 10.0 * ((float)SCREEN_HEIGHT / (float)SCREEN_WIDTH);

			last_save_scale = cam_save->scale;
		}
	}
	glm::vec2 getlast_savescale()
	{
		return last_save_scale;
	}
	GLFWwindow* getWindow() { return window; }
	GLFWcursor* getCursor() { return cursor; }
	
	struct RenderRect;
	uint32_t CreateRenderRect(glm::vec3 position = glm::vec3(0,0,0), glm::vec3 rot_axis = glm::vec3(1,0,0), float angle = 0.0f, glm::vec3 scale = glm::vec3(1,1,1))
	{
		uint32_t id = GetFreeID();
		Render_Rects[id].init();

		Render_Rects[id].Translate(position);
		Render_Rects[id].Rotate(rot_axis, angle);
		Render_Rects[id].Scale(scale);
		Render_Rects[id].SetTexture(0);

		//active_rects.push_back(id);
		return id;
	}

	void RemoveRenderRect(uint32_t index)
	{
		my_assert(index >= 0 && index < MAX_RECTS, "error removerenderrect");
		if (free_id[index] == false) {
			std::cout << "freeing a render rect thats already freed!\n";
		}

		free_id[index] = false;
		/*for (int i = 0; i < active_rects.size(); i++)
		{
			if (active_rects[i] == index)
			{
				active_rects.erase(active_rects.begin() + i);
				break;
			}
		}*/
		//std::cout << index << " freed active rect id that wanst in active?\n";
	}

	RenderRect& GetRenderRect(uint32_t index)
	{
		my_assert(index >= 0 && index < MAX_RECTS, "max rects hits!");
		if (free_id[index] == false)
		{
			std::cout << index << "trying to get render get index that is not used?\n";
		}
		return Render_Rects[index];
	}

	void setcameraref(Camera* cam_main)
	{
		cam_save = cam_main;
		last_save_scale = cam_save->scale;

		if (cam_save)
		{
			//w/h  = aspectratio    10/y = aspectationm  10/aspectratio    (10*h)/w = y
			cam_save->scale.x = 10;
			cam_save->scale.y = 10.0 * ((float)SCREEN_HEIGHT / (float)SCREEN_WIDTH);

			last_save_scale = cam_save->scale;
		}
	}
	int GetRenderRectCount() 
	{ 
		int count = 0;
		for (int i = 0; i < free_id.size(); i++)
		{
			if (free_id[i])
				count++;
		}
		return count;
	}

	void AddText(std::string text, float x, float y, float scale, glm::vec3 color)
	{
		rendertextqueue.emplace(text, x, y, scale, color);
	}

	uint32_t getscreenwidth() { return SCREEN_WIDTH; }
	uint32_t getscreenheight() { return SCREEN_HEIGHT; }
private:
	uint32_t SCREEN_WIDTH;
	uint32_t SCREEN_HEIGHT;

	float FOV;
	float N;
	float F;

	GLFWwindow* window;
	GLFWcursor* cursor;

	GLuint square_vao, square_vbo, square_ibo;
	GLuint program_main;
	GLuint program_main_mvp;
	GLuint program_font;
	GLuint program_font_proj;
	GLuint program_font_color;
	std::vector<GLuint> texture_ids;
	
	static const int MAX_RECTS = 1500;

	class RenderRect {
	public:
		void Translate(glm::vec3 val)
		{
			pos = val;
			trans_m = glm::translate(glm::mat4(1.0f), pos);
			dirty = true;
		}
		void Move(glm::vec3 val)
		{
			pos += val;
			trans_m = glm::translate(glm::mat4(1.0f), pos);
			dirty = true;
		}
		void Rotate(glm::vec3 axis, float angle)
		{
			rotate_m = glm::rotate(glm::mat4(1.0), glm::radians(angle), axis);
			current_angle = angle;
			dirty = true;
		}
		void Scale(glm::vec3 val)
		{
			scale_m = glm::scale(glm::mat4(1.0), val);
			dirty = true;
		}
		void SetTexture(uint32_t id)
		{
			textureid = id;
		}
		void SetVisibility(bool visible)
		{
			invisible = !visible;
		}
		void SetLayer(int _layer)
		{
			layer = _layer;

			layer_count = std::max(layer_count, _layer);
		}
		void Reflect(bool on) {
			reflected = on;
			dirty = true;
		}
		glm::vec2 getPos() { return pos; }
		glm::vec2 getScale() { return glm::vec2(scale_m[0][0], scale_m[1][1]); }
		float getangle() { return current_angle; }
		bool getreflected() { return reflected;  }
		uint32_t gettextureid() { return textureid; }
	private:
		friend class GraphicsManager;

		void init()
		{
			trans_m = glm::mat4(1.0f);
			scale_m = glm::mat4(1.0f);
			rotate_m = glm::mat4(1.0f);
			m_mat = glm::mat4(1.0f);
			invisible = false;
			layer = 0;
			textureid = 0;
			reflected = false;
			current_angle = 0.0;
		}

		glm::mat4 trans_m, rotate_m, scale_m, m_mat;
		glm::vec3 pos;
		uint32_t textureid;
		bool invisible = false;
		int layer = 0; //draw layer 0 first, then 1,2,...
		bool dirty = false;
		bool reflected = false;
		float current_angle = 0.0;

		static int layer_count;

		glm::mat4 GetMatrix()
		{
			if (dirty)
			{
				glm::mat4 scale_reflect = scale_m;
				if(reflected)
					scale_reflect  = glm::scale(glm::mat4(1.0), glm::vec3(-abs(scale_m[0][0]), scale_m[1][1], scale_m[2][2]));
				m_mat = trans_m * scale_reflect * rotate_m;
				dirty = false;
			}
			return m_mat;
		}

		uint32_t GetTextureID()
		{
			return textureid;
		}
	};

	std::array<RenderRect, MAX_RECTS> Render_Rects;
	std::array<bool, MAX_RECTS> free_id;
	uint32_t last_id;

	struct Character {
		unsigned int TextureID;  // ID handle of the glyph texture
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
		unsigned int Advance;    // Offset to advance to next glyph
	};
	struct RenderText {
		std::string text;
		float x;
		float y;
		float scale;
		glm::vec3 color;

		RenderText(std::string _text, float _x, float _y, float _scale, glm::vec3 _color) : 
			text(_text), x(_x), y(_y), scale(_scale), color(_color){}
	};

	unsigned int VAO_font, VBO_font;
	std::map<char, Character> Characters;
	FT_Library ft;
	FT_Face face;
	std::queue<RenderText> rendertextqueue;

	Camera* cam_save;
	glm::vec2 last_save_scale;
	bool is_full_screen;
private:

	uint32_t GetFreeID()
	{
		for (uint32_t i = 0; i < free_id.size(); i++)
		{
			uint32_t index = (last_id + i) % MAX_RECTS;
			if (free_id[index] == false)
			{
				free_id[index] = true;
				last_id = index;
				return index;
			}
		}

		std::cout<<"max squares reached!\n";
		return 100000;
	}

	void CreateGeometry()
	{
		//SQUARE
		std::vector<float> square =
		{
			-.5,.5, 0.0,   0.0,1.0,    //tl
			-.5,-.5,0.0,   0.0,0.0,  //bl
			.5,-.5, 0.0,   1.0,0.0,   //br
			.5,.5, 0.0,    1.0,1.0   //tr
		};

		std::vector<uint32_t> square_index;
		square_index.push_back(3);
		square_index.push_back(0);
		square_index.push_back(1);
		square_index.push_back(3);
		square_index.push_back(1);
		square_index.push_back(2);


		glGenVertexArrays(1, &square_vao);
		glBindVertexArray(square_vao);
		glGenBuffers(1, &square_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, square_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * square.size(), square.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0); //position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3)); //uv
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &square_ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, square_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t)*square_index.size(), square_index.data(), GL_STATIC_DRAW);

		checkError("create geometry");
	}

	void SetupShaders()
	{
		//main shader
		CompileGLShader(config::start_path + "GameData/Shaders/main.vert", config::start_path + "GameData/Shaders/main.frag", program_main);
		glUseProgram(program_main);
		program_main_mvp = glGetUniformLocation(program_main, "mvp");
		glUseProgram(0);
		std::cout << "main shader compiled\n";
	
		//font shader
		CompileGLShader(config::start_path + "GameData/Shaders/font.vert", config::start_path + "GameData/Shaders/font.frag", program_font);
		glUseProgram(program_font);
		program_font_proj = glGetUniformLocation(program_font, "projection");
		program_font_color = glGetUniformLocation(program_font, "textColor");
		glUseProgram(0);
		std::cout << "font shader compiled\n";

		checkError("After setup main shader");
	}

	void CreateTextures()
	{
		texture_ids.resize(75);
		CreateTexture(texture_ids[0],  std::string(config::start_path + "GameData/Images/tank.jpg").c_str(), true);
		CreateTexture(texture_ids[1],  std::string(config::start_path + "GameData/Images/banelingend.jpg").c_str(), true);
		CreateTexture(texture_ids[2],  std::string(config::start_path +"GameData/Images/nonmoveice.png").c_str(), true);
		CreateTexture(texture_ids[3],  std::string(config::start_path +"GameData/Images/dark.jpg").c_str(), true);
		CreateTexture(texture_ids[4],  std::string(config::start_path +"GameData/Images/baneling.png").c_str(), true);
		CreateTexture(texture_ids[5],  std::string(config::start_path +"GameData/Images/zealot.jpg").c_str(), true);
		CreateTexture(texture_ids[6],  std::string(config::start_path +"GameData/Images/grass.jpg").c_str(), true);
		CreateTexture(texture_ids[7],  std::string(config::start_path +"GameData/Images/ice.jpg").c_str(), true);
		CreateTexture(texture_ids[8],  std::string(config::start_path +"GameData/Images/button.png").c_str(), true);
		CreateTexture(texture_ids[9],  std::string(config::start_path +"GameData/Images/gate.png").c_str(), true);
		CreateTexture(texture_ids[10], std::string(config::start_path +"GameData/Images/buttonred.png").c_str(), true);
		CreateTexture(texture_ids[11], std::string(config::start_path +"GameData/Images/ring.jpg").c_str(), true);
		CreateTexture(texture_ids[12], std::string(config::start_path +"GameData/Images/reverseice.jpg").c_str(), true);
		CreateTexture(texture_ids[13], std::string(config::start_path +"GameData/Images/exit.jpg").c_str(), true);
		CreateTexture(texture_ids[14], std::string(config::start_path +"GameData/Images/fastice.jpg").c_str(), true);
		CreateTexture(texture_ids[15], std::string(config::start_path +"GameData/Images/goo.jpg").c_str(), true);
		CreateTexture(texture_ids[16], std::string(config::start_path +"GameData/Images/disk.png").c_str(), true);
		CreateTexture(texture_ids[17], std::string(config::start_path +"GameData/Images/banelingspeed.png").c_str(), true);
		CreateTexture(texture_ids[18], std::string(config::start_path +"GameData/Images/snow.jpg").c_str(), true);
												
		CreateTexture(texture_ids[19], std::string(config::start_path +"GameData/Images/icons/baneling_icon.png").c_str(), true);
		CreateTexture(texture_ids[20], std::string(config::start_path +"GameData/Images/icons/chat_icon.png").c_str(), true);
		CreateTexture(texture_ids[21], std::string(config::start_path +"GameData/Images/icons/goodrop.png").c_str(), true);
		CreateTexture(texture_ids[22], std::string(config::start_path +"GameData/Images/icons/halt_icon.png").c_str(), true);
		CreateTexture(texture_ids[23], std::string(config::start_path +"GameData/Images/icons/lock_icon.png").c_str(), true);
		CreateTexture(texture_ids[24], std::string(config::start_path +"GameData/Images/icons/player_icon.png").c_str(), true);
		CreateTexture(texture_ids[25], std::string(config::start_path +"GameData/Images/icons/ring_icon.png").c_str(), true);
		CreateTexture(texture_ids[26], std::string(config::start_path +"GameData/Images/icons/speed_icon.png").c_str(), true);
												
		CreateTexture(texture_ids[27], std::string(config::start_path +"GameData/Images/red.png").c_str(), true);
		CreateTexture(texture_ids[28], std::string(config::start_path +"GameData/Images/icons/halt_icon_held.png").c_str(), true);
		CreateTexture(texture_ids[29], std::string(config::start_path +"GameData/Images/icons/lock_icon_held.png").c_str(), true);
		CreateTexture(texture_ids[30], std::string(config::start_path +"GameData/Images/tan.png").c_str(), true);
		CreateTexture(texture_ids[31], std::string(config::start_path +"GameData/Images/teleport.png").c_str(), true);
		CreateTexture(texture_ids[32], std::string(config::start_path +"GameData/Images/load_screen.png").c_str(), true);
		CreateTexture(texture_ids[33], std::string(config::start_path +"GameData/Images/highlatency.png").c_str(), true);
		CreateTexture(texture_ids[34], std::string(config::start_path +"GameData/Images/packet_loss.png").c_str(), true);
		CreateTexture(texture_ids[35], std::string(config::start_path +"GameData/Images/latency_variation.png").c_str(), true);
		CreateTexture(texture_ids[36], std::string(config::start_path + "GameData/Images/black.jpg").c_str(), true);

		CreateTexture(texture_ids[37], std::string(config::start_path + "GameData/Images/bear.png").c_str(), true);
		CreateTexture(texture_ids[38], std::string(config::start_path + "GameData/Images/bug.png").c_str(), true);
		CreateTexture(texture_ids[39], std::string(config::start_path + "GameData/Images/phoenix.png").c_str(), true);
		CreateTexture(texture_ids[40], std::string(config::start_path + "GameData/Images/bigbear.png").c_str(), true);
		CreateTexture(texture_ids[41], std::string(config::start_path + "GameData/Images/blue_crystal.jpg").c_str(), true);
		CreateTexture(texture_ids[42], std::string(config::start_path + "GameData/Images/test/testmore/aviloface.png").c_str(), true);
		CreateTexture(texture_ids[43], std::string(config::start_path + "GameData/Images/tile.jpg").c_str(), true);

		CreateTexture(texture_ids[44], std::string(config::start_path + "GameData/Images/explosion.jpg").c_str(), true);
		CreateTexture(texture_ids[45], std::string(config::start_path + "GameData/Images/target.png").c_str(), true);
		CreateTexture(texture_ids[46], std::string(config::start_path + "GameData/Images/targetred.png").c_str(), true);
		CreateTexture(texture_ids[47], std::string(config::start_path + "GameData/Images/test/testmore/avilo2.png").c_str(), true);
		CreateTexture(texture_ids[48], std::string(config::start_path + "GameData/Images/test/testmore/atira.png").c_str(), true);
		CreateTexture(texture_ids[49], std::string(config::start_path + "GameData/Images/fist.jpg").c_str(), true);
		CreateTexture(texture_ids[50], std::string(config::start_path + "GameData/Images/kongfu.png").c_str(), true);
		CreateTexture(texture_ids[51], std::string(config::start_path + "GameData/Images/fist2.jpg").c_str(), true);
		CreateTexture(texture_ids[52], std::string(config::start_path + "GameData/Images/ban.png").c_str(), true);
		CreateTexture(texture_ids[53], std::string(config::start_path + "GameData/Images/test/testmore/redb.jpg").c_str(), true);

		CreateTexture(texture_ids[54], std::string(config::start_path + "GameData/Images/bunkbed.png").c_str(), true);
		CreateTexture(texture_ids[55], std::string(config::start_path + "GameData/Images/cop.png").c_str(), true);
		CreateTexture(texture_ids[56], std::string(config::start_path + "GameData/Images/desk.png").c_str(), true);
		CreateTexture(texture_ids[57], std::string(config::start_path + "GameData/Images/test/testmore/avilo_kong.png").c_str(), true);
		CreateTexture(texture_ids[58], std::string(config::start_path + "GameData/Images/test/testmore/avilo_4.png").c_str(), true);
		CreateTexture(texture_ids[59], std::string(config::start_path + "GameData/Images/test/testmore/avilo5.png").c_str(), true);
		CreateTexture(texture_ids[60], std::string(config::start_path + "GameData/Images/walls.png").c_str(), true);

		CreateTexture(texture_ids[61], std::string(config::start_path + "GameData/Images/disco.jpg").c_str(), true);
		CreateTexture(texture_ids[62], std::string(config::start_path + "GameData/Images/jacuzzi.png").c_str(), true);
		CreateTexture(texture_ids[63], std::string(config::start_path + "GameData/Images/tree.jpg").c_str(), true);
		CreateTexture(texture_ids[64], std::string(config::start_path + "GameData/Images/notes.png").c_str(), true);
		CreateTexture(texture_ids[65], std::string(config::start_path + "GameData/Images/test/testmore/avi_5.jpg").c_str(), true);

		CreateTexture(texture_ids[67], std::string(config::start_path + "GameData/Images/server.png").c_str(), true);
		CreateTexture(texture_ids[68], std::string(config::start_path + "GameData/Images/sc2.png").c_str(), true);
		CreateTexture(texture_ids[69], std::string(config::start_path + "GameData/Images/escape.png").c_str(), true);
		CreateTexture(texture_ids[70], std::string(config::start_path + "GameData/Images/exitgame.png").c_str(), true);
	}

	void CreateTexture(GLuint& textureid, const char* fileLocation, bool alpha, bool flip = true)
	{
		stbi_set_flip_vertically_on_load(flip);
		int width, height, bitdepth;
		unsigned char *texData = stbi_load(fileLocation, &width, &height, &bitdepth, (alpha) ? STBI_rgb_alpha : 0);
		if (!texData)
		{
			std::cout<<"failed to load texture: " << fileLocation;
			return;
		}

		glGenTextures(1, &textureid);
		glBindTexture(GL_TEXTURE_2D, textureid);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE_EXT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE_EXT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 1);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 1);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 1);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(texData);

		checkError("created texture");
	}

	void setupFonts()
	{
		//FREETYPE
		if (FT_Init_FreeType(&ft))
		{
			std::cout << "Could not init FreeType error\n";
			return;
		}
		FT_Int maj, min, patch;
		FT_Library_Version(ft, &maj, &min, &patch);
		std::cout << "FreeType Version: " << maj << " " << min << " " << patch << std::endl;

		if (FT_New_Face(ft, std::string(config::start_path + "GameData/Fonts/OpenSans-Regular.ttf").c_str(), 0, &face))
		{
			std::cout << "failed to load font arial.ttf\n";
			return;
		}
		FT_Set_Pixel_Sizes(face, 0, 48);
		if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
		{
			std::cout << "failed to load glyph\n";
			return;
		}
	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

		for (unsigned char c = 0; c < 128; c++)
		{
			// load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			// generate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);

			// set texture options 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE_EXT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE_EXT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// now store character for later use
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};
			Characters.insert(std::pair<char, Character>(c, character));
		}
		checkError("created font textures");

		glGenVertexArrays(1, &VAO_font);
		glGenBuffers(1, &VBO_font);
		glBindVertexArray(VAO_font);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_font);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		checkError("created font vao");
	}

	void RenderTextFont(Camera* cam_main, std::string text, float x, float y, float scale, glm::vec3 color)
	{
		glUseProgram(program_font);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VAO_font);

		glm::mat4 p_mat = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, N, F);// cam_main->CreateOrthoMat(N, F);
		glUniformMatrix4fv(program_font_proj, 1, GL_FALSE, glm::value_ptr(p_mat));

		glUniform3f(program_font_color, color.x, color.y, color.z);

		// iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = Characters[*c];

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;
			// update VBO for each character
			float vertices[6][4] = {
				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
			};
			// render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			// update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, VBO_font);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_BLEND);
	}

	void checkError(std::string msg)
	{
		GLenum error = glGetError();
		if (error)
		{
			Logger::Log(Logger::_ERROR, "GLERROR (", msg, "): ", error);
		}
	}

	std::string ReadFile(const std::string& fileLocation)
	{
		std::string content;
		std::ifstream fileStream(fileLocation.c_str(), std::ios::in);

		if (!fileStream.is_open()) {
			Logger::Log(Logger::_WARNING, "Failed to read" + fileLocation + "! File doesn't exist.\n");
			return "";
		}

		std::string line = "";
		while (!fileStream.eof())
		{
			std::getline(fileStream, line);
			content.append(line + "\n");
		}

		fileStream.close();
		return content;
	}

	void printShaderlog(unsigned int shader)
	{
		int len;
		char* log;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		if (len > 0)
		{
			log = (char*)malloc(len);
			glGetShaderInfoLog(shader, len, 0, log);
			Logger::Log(Logger::_WARNING, "Shader info log: ", std::string(log), "\n");
			free(log);
		}
	}

	void printProgramlog(unsigned int program)
	{
		int len;
		char* log;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		if (len > 0)
		{
			log = (char*)malloc(len);
			glGetProgramInfoLog(program, len, 0, log);
			Logger::Log(Logger::_WARNING, "Program info log: ", std::string(log), "\n");
			free(log);
		}
	}

	void CompileGLShader(const std::string& vertex, const std::string& frag, GLuint& programid)
	{
		std::string shadersource = ReadFile(vertex);
		std::string fragsource = ReadFile(frag);
		const char *vshadersource = shadersource.c_str();
		const char *fshadersource = fragsource.c_str();

		unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(vShader, 1, &vshadersource, NULL);
		glShaderSource(fShader, 1, &fshadersource, NULL);
		glCompileShader(vShader);
		glCompileShader(fShader);

		GLint compiled;
		glGetShaderiv(vShader, GL_COMPILE_STATUS, &compiled);
		if (compiled != 1)
		{
			Logger::Log(Logger::_WARNING, "vertex shader failed to compile!\n");
			printShaderlog(vShader);
		}
		glGetShaderiv(fShader, GL_COMPILE_STATUS, &compiled);
		if (compiled != 1)
		{
			Logger::Log(Logger::_WARNING, "fragment shader failed to compile!\n");
			printShaderlog(fShader);
		}

		programid = glCreateProgram();
		glAttachShader(programid, vShader);
		glAttachShader(programid, fShader);
		glLinkProgram(programid);

		glGetProgramiv(programid, GL_LINK_STATUS, &compiled);
		if (compiled != 1)
		{
			Logger::Log(Logger::_WARNING, "Program linking failed!\n");
			printProgramlog(programid);
		}
	}
};
