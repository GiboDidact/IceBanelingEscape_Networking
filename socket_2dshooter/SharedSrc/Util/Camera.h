#pragma once

	class Camera
	{
	public:
		Camera();
		Camera(glm::vec3 pos, glm::vec3 worldup, float yawz, float pitchz);

		glm::mat4 CreateLookAtFunction();
		glm::mat4 CreateOrthoMat(float N, float F);

		void handlekeyinput(bool* keys, float fps);
		void handlemouseinput(bool* keys, float fps);
		glm::vec3 GetPosition() { return position; }
		glm::vec3 GetLookDirection() { return lookdirection; }

		void update();

		~Camera();

		glm::vec3 scale;
		glm::vec3 position;
		glm::vec3 lookdirection;
		glm::vec3 rightdirection;
		glm::vec3 updirection;
		glm::vec3 worldupdirection;
		float yaw;
		float pitch;
	private:


		float movespeed;
		float turnspeed;
		float xrotation;
	};

