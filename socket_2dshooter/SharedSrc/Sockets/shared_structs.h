#pragma once
#include <iostream>
#include <array>

struct lobby_info
{
	std::string lobby_name;
	std::string lobby_host;
	uint8_t player_count;

	lobby_info()
	{
		player_count = 0;
	}
};

struct lobby_fullinfo
{
	lobby_info info;
	bool openaccess;
	std::vector<std::string> messages;
	std::vector<uint8_t> ready;

	lobby_fullinfo()
	{
		openaccess = false;
		for (int i = 0; i < ready.size(); i++)
		{
			ready[i] = 0;
		}
	}
};

struct lobby_fullinfo_client
{
	lobby_fullinfo info;
	std::vector<std::string> names;

	lobby_fullinfo_client()
	{
	}
};

struct create_info
{
	std::string lobby_name;
	bool creator;
	bool open_access;

	create_info()
	{
		creator = false;
		open_access = false;
	}
};

struct lobbydata
{
	lobby_fullinfo info;
	//ping?
	//gamemode?

	int creator_index;
	std::vector<int> joiner_indices;

	lobbydata()
	{
		creator_index = -1;
		std::fill(joiner_indices.begin(), joiner_indices.end(), -1);
	}
};