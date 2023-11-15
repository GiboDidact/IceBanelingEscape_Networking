#pragma once
#include <vector>

class Input_Manager {
public:
	Input_Manager();
	~Input_Manager() = default;

	//no copying/moving should be allowed from this class
	// disallow copy and assignment
	Input_Manager(Input_Manager const&) = delete;
	Input_Manager(Input_Manager&&) = delete;
	Input_Manager& operator=(Input_Manager const&) = delete;
	Input_Manager& operator=(Input_Manager&&) = delete;

	//callbacks
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

	void PrintInfo();

	void GetMousePos(double& x, double& y) const;
	void GetMouseVelocity(double& x, double& y) const;
	void GetMouseButtons(bool& l, bool& r) const;
	void GetMouseButtonsOnce(bool& l, bool& r);
	bool GetKeyPress(int i) const;
	bool GetKeyPressOnce(int i);
	int GetScrollOnce();
	double Getmousex() const { return mousex; }
	double Getmousey() const { return mousey; }
	bool* GetKeys() { return &Keys[0]; }

	void Setmousexy(double x, double y) { mousex = x; mousey = y; }
	void Setmousexyv(double x, double y) { mousevx = x; mousevy = y; }
	void Setmouseleft(bool l) { leftmouse = l; leftmouse_once = l;  }
	void Setmouseright(bool r) { rightmouse = r; rightmouse_once = r; }
	void SetKeys(int i, bool val) { Keys[i] = val; }
	void SetKeysOnce(int i, bool val) { KeysOnce[i] = val; }
	void SetKeysOnceTmp(int i, bool val) 
	{
		if (recordTmp)
		{
			if (val == true && KeysOnceTmp[i] == false)
			{
				KeysOnceList.push_back(i);
			}
			KeysOnceTmp[i] = val;
		}
	}
	void Setscroll(int i) { scroll = i; }

	void startrecordingTmp() { recordTmp = true; }
	std::vector<int> peekrecordingTmp()
	{
		return KeysOnceList;
	}
	std::vector<int> stopandreceiverecordingTmp() 
	{ 
		recordTmp = false; 
		std::vector<int> data = KeysOnceList;

		KeysOnceList.clear();
		memset(KeysOnceTmp, 0, 500);
		return data;
	}
private:
	double mousex;
	double mousey;
	double mousevx, mousevy; //the difference between the mouse movement in the last 2 mouse position callbacks. MouseCallback1.pos - MouseCallBack1.pos
	int scroll;
	bool Keys[500];
	bool KeysOnce[500];
	bool recordTmp = false;
	std::vector<int> KeysOnceList;
	bool KeysOnceTmp[500]; //if you want to capture keysonce in a specific time frame
	bool leftmouse;
	bool rightmouse;
	
	bool leftmouse_once;
	bool rightmouse_once;
};
