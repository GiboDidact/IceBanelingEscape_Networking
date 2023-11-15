#pragma once

#define PLATFORM_WINDOWS

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <array>
#include <assert.h>
#include <chrono>

#ifdef PLATFORM_WINDOWS
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#endif

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GL/glew.h"
#include <GLFW/glfw3.h>

#include "Util/Logger.h"
#include "Util/Timer.h"
#include "Util/Input_Manager.h"
#include "Util/Global_Functions.h"
#include "Util/Camera.h"

#include "ConfigInfo.h"

static void my_assert(bool condition, std::string msg)
{
	if (condition == false) {
		std::cout <<"ABORTING: " << msg << std::endl;
		std::abort();
	}
}