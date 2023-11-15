#include "../pch.h"
#include "Camera.h"

#include <GLFW/glfw3.h>


	Camera::Camera()
	{
		position = glm::vec3(0, 0, 0);
		lookdirection = glm::vec3(0, 0, 0);
		scale = glm::vec3(1, 1, 0);
	}

	Camera::Camera(glm::vec3 pos, glm::vec3 worldup, float yawz, float pitchz)
	{
		position = pos;
		worldupdirection = worldup;

		yaw = yawz;
		pitch = pitchz;
		xrotation = 0;

		lookdirection = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);

		update();
	}

	glm::mat4 Camera::CreateLookAtFunction()
	{
		//glm::lookAt(glm::vec3(0.0f, -0.2f, 5.0f), glm::vec3(0.0f, -0.2f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 mat = glm::lookAt(position, position + glm::vec3(lookdirection.x, lookdirection.y, lookdirection.z), updirection);
		return glm::lookAt(position, position + glm::vec3(lookdirection.x, lookdirection.y, lookdirection.z), updirection);
	}

	glm::mat4 Camera::CreateOrthoMat(float N, float F)
	{
		glm::vec2 half_scale = scale / 2.0f;
		return glm::ortho(-half_scale.x, half_scale.x, -half_scale.y, half_scale.y, N, F);
	}

	void Camera::handlekeyinput(bool* keys, float fps)
	{
		movespeed = 7.0f / fps * 2;

		if (keys[GLFW_KEY_W])
		{
			position += updirection * movespeed;
		}
		if (keys[GLFW_KEY_S])
		{
			position -= updirection * movespeed;
		}
		if (keys[GLFW_KEY_A])
		{
			position -= rightdirection * movespeed;
		}
		if (keys[GLFW_KEY_D])
		{
			position += rightdirection * movespeed;
		}
		if (keys[GLFW_KEY_LEFT_CONTROL])
		{
			position -= lookdirection * movespeed;
		}
		if (keys[GLFW_KEY_SPACE])
		{
			position += lookdirection * movespeed;
		}
		if (keys[GLFW_KEY_M])
		{
			position = glm::vec3(0, 80000, 0);
		}
		if (keys[GLFW_KEY_N])
		{
			position = glm::vec3(0, 0, 0);
		}

		update();
	}

	void Camera::handlemouseinput(bool* keys, float fps)
	{
		turnspeed = 75.0f / fps;

		if (keys[GLFW_KEY_UP])
		{
			pitch -= turnspeed;
		}
		if (keys[GLFW_KEY_DOWN])
		{
			pitch += turnspeed;
		}
		if (keys[GLFW_KEY_LEFT])
		{
			yaw -= turnspeed;
			yaw = fmod(yaw, 360);
		}
		if (keys[GLFW_KEY_RIGHT])
		{
			yaw += turnspeed;
			yaw = fmod(yaw, 360);
		}

		if (pitch > 179.0f)
		{
			pitch = 179.0f;
		}

		if (pitch < 1.0f)
		{
			pitch = 1.0f;
		}


		update();
	}

	void Camera::update()
	{
		lookdirection.x = sin(glm::radians(pitch)) * cos(glm::radians(yaw));
		lookdirection.y = cos(glm::radians(pitch));
		lookdirection.z = sin(glm::radians(pitch)) * sin(glm::radians(yaw));
		lookdirection = glm::normalize(lookdirection);

		rightdirection = glm::normalize(glm::cross(lookdirection, worldupdirection));
		updirection = glm::normalize(glm::cross(rightdirection, lookdirection));

		//std::cout << lookdirection.x << " " << lookdirection.y << " " << lookdirection.z << std::endl;
	}

	Camera::~Camera()
	{

	}
