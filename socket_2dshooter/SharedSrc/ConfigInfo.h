#pragma once
#include <string>
#include <fstream>

namespace config
{
#define DEVELOPER
	
#ifdef DEVELOPER
	static const std::string start_path = "../";
	static const bool override_config_file = true;
#else
	static const std::string start_path = "";
	static const bool override_config_file = false;
#endif
	static const std::string config_file_name = start_path + "Config.txt";
	
	//CONFIG VALUES
	static bool STANDALONE_GAME = false;
	static int STANDALONE_PLAYER_COUNT = 1;
	static bool INFINITE_LIVES_MODE = false;
	static bool SERVER_LEVEL_SWITCH = true;
	static bool FULL_SCREEN = false;
	static bool CLIENT_GOD = false;

	//hide this part, it should only be read from config file
	static std::string ServerIP = "0.0.0.0";//
	static uint16_t ServerPort = 55000; //dynamic ports: 49152 to 65535

	static bool stoi_exceptions(std::string str, int& out_val)
	{
		try {
			out_val = std::stoi(str);
		}
		catch (const std::invalid_argument& ia) {
			std::cout << "Invalid argument: " << ia.what() << std::endl;
			return false;
		}
		catch (const std::out_of_range& oor) {
			std::cout << "Out of Range error: " << oor.what() << std::endl;
			return false;
		}
		return true;
	}

	static std::string default_config_text =
		"ServerIPV4= 0.0.0.0		//IPV4 address of server (0.0.0.0)\n"
		"ServerPort= 55000			//Server port number (49152-65535)\n"
		"STANDALONE= 0			//run without lobby system (0 or 1)\n"
		"STANDALONE_PLAYER_COUNT= 1 //how many clients can connect\n"
		"SERVER_LEVEL_SWITCH= 0  //server can swap levels hitting keys: 1-6(0 or 1)\n"
		"INFINITE_LIVES= 0   //server resets player to current level on death(0 or 1)\n"
		"FULL_SCREEN= 0         //runs in fullscreen(0 or 1)\n"
		"GOD_MODE= 0         //server lets everyone have god mode(0 or 1)\n"
		"\n"
		"***Only change values in this file.\n"
		"If anything else changes default values\n"
		"will be used and file will be recreated.\n"
		"If you want to reset just destroy file ***\n";

	static void CreateDefaultConfigFile()
	{
		std::fstream config_file;
		config_file.open(config_file_name, std::fstream::out);
		if (config_file.is_open())
		{
			config_file.write(default_config_text.data(), default_config_text.size());
			if (config_file.bad() || config_file.fail())
			{
				std::cout << "failed to create default config file\n";
			}
		}
	}

	static void SeeIfConfigFileExists()
	{
		std::fstream config_file(config_file_name);
		if (!config_file.good())
		{
			CreateDefaultConfigFile();
		}
	}

	static std::array<int, 8> ReadConfigFile(std::string& ipv4)
	{
		SeeIfConfigFileExists();

		std::array<int, 8> config_values;
		config_values.fill(-1);

		std::fstream config_file;
		config_file.open(config_file_name, std::fstream::in);
		if (config_file.is_open())
		{
			int counter = 0;
			std::string out_string;
			while (!config_file.eof() && counter < config_values.size())
			{
				//get string up to =
				std::getline(config_file, out_string, '=');
				if (config_file.bad() || config_file.fail())
				{
					std::cout << "file opened but getline failed\n";
					return config_values;
				}

				//validate it with counter
				switch (counter)
				{
				case 0: if (out_string != "ServerIPV4") {
					std::cout << "ServerIPV4 error while parsing\n";      return config_values;
				}    break;
				case 1: if (out_string != "ServerPort") {
					std::cout << "ServerPort error while parsing\n";     return config_values;
				}    break;
				case 2: if (out_string != "STANDALONE") {
					std::cout << "STANDALONE error while parsing\n";    return config_values;
				}    break;
				case 3: if (out_string != "STANDALONE_PLAYER_COUNT") {
					std::cout << "STANDALONE_PLAYER_COUNT error while parsing\n";     return config_values;
				}    break;
				case 4: if (out_string != "SERVER_LEVEL_SWITCH") {
					std::cout << "SERVER_LEVEL_SWITCH error while parsing\n";             return config_values;
				}    break;
				case 5: if (out_string != "INFINITE_LIVES") {
					std::cout << "INFINITE_LIVES error while parsing\n";             return config_values;
				} break;
				case 6: if (out_string != "FULL_SCREEN") {
					std::cout << "FULL_SCREEN error while parsing\n";             return config_values;
				} break;
				case 7: if (out_string != "GOD_MODE") {
					std::cout << "GOD_MODE error while parsing\n";             return config_values;
				}   break;
				default: break;
				};

				//get string up to /
				std::getline(config_file, out_string, '/');
				if (config_file.bad() || config_file.fail())
				{
					std::cout << "file opened but getline failed\n";
						return config_values;
				}

				//convert that to a number
				if (counter != 0)
				{
					if (!stoi_exceptions(out_string, config_values[counter]))
					{
						return config_values;
					}
				}
				else
				{
					//get string
					ipv4 = out_string;
				}

				//skip everything and go nextline
				std::getline(config_file, out_string);
				if (config_file.bad() || config_file.fail())
				{
					std::cout << "file opened but getline failed\n";
						return config_values;
				}

				counter++;
			}
		}
		else
		{
			std::cout << "failed to open " << config_file_name << std::endl;
				return config_values;
		}

		return config_values;
	}
	

	static void LoadConfigValues()
	{
		if (override_config_file)
			return;

		SeeIfConfigFileExists();
		std::string ipv;
		std::array<int, 8> values = ReadConfigFile(ipv);
		std::cout << ipv << " " << values[1] << " " << values[2] << " " << values[3] << " " << values[4] << " " 
			             << values[5] << " " << values[6] << " " << values[7] << std::endl;
		if (values[7] != -1)
		{
			ServerIP = ipv;
			ServerIP.erase(remove_if(ServerIP.begin(), ServerIP.end(), isspace), ServerIP.end());
			ServerPort = (uint16_t)values[1];
			STANDALONE_GAME = (bool)values[2];
			STANDALONE_PLAYER_COUNT = values[3];
			SERVER_LEVEL_SWITCH = (bool)values[4];
			INFINITE_LIVES_MODE = (bool)values[5];
			FULL_SCREEN = (bool)values[6];
			CLIENT_GOD = (bool)values[7];
		}
		else
		{
			std::cout << "Failed to load config file correctly\n";
			CreateDefaultConfigFile();
		}
	}
}

