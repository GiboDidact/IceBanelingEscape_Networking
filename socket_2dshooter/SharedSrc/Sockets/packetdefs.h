#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "BitStream.h"
#include "shared_structs.h"

enum class PACKETCODE : uint8_t { USERNAME, WELCOMETOSERVER, REFRESH, MATCHMAKING, ENTERLOBBY, WELCOMETOLOBBY, SERVERLOBBY, 
	                              LOBBYFAIL, LOBBYLEAVING, LOBBY_DISBANDED, LOBBYREADY, LOBBYTYPE, LAUNCHGAME, CONNECTION_CHECK};


static void WritePacket_username(BitStreamWriter& bitwriter, const std::string& username)
{
	bitwriter.Write(PACKETCODE::USERNAME);
	bitwriter.Write(static_cast<uint8_t>(username.length()));
	bitwriter.WriteBytes(username.data(), username.length());
}

static void ReadPacket_username(BitStreamReader& bitreader, std::string& username)
{
	//verify we have enough memory
	uint8_t name_len;
	bitreader.Read(&name_len);

	username.resize(name_len);
	bitreader.ReadBytes(username.data(), name_len);
}

static void WritePacket_welcometoserver(BitStreamWriter& bitwriter)
{
	bitwriter.Write(PACKETCODE::WELCOMETOSERVER);
}
static void ReadPacket_welcometoserver(BitStreamReader& bitreader)
{
}

static void WritePacket_lobbydisbanded(BitStreamWriter& bitwriter)
{
	bitwriter.Write(PACKETCODE::LOBBY_DISBANDED);
}
static void ReadPacket_lobbydisbanded(BitStreamReader& bitreader)
{
}

static void WritePacket_welcometolobby(BitStreamWriter& bitwriter)
{
	bitwriter.Write(PACKETCODE::WELCOMETOLOBBY);
}
static void ReadPacket_welcometolobby(BitStreamReader& bitreader)
{
}

static void WritePacket_refresh(BitStreamWriter& bitwriter)
{
	bitwriter.Write(PACKETCODE::REFRESH);
}
static void ReadPacket_refresh(BitStreamReader& bitreader)
{
}

static void WritePacket_connectioncheck(BitStreamWriter& bitwriter)
{
	bitwriter.Write(PACKETCODE::CONNECTION_CHECK);
}
static void ReadPacket_connectioncheck(BitStreamReader& bitreader)
{
}

static void WritePacket_matchmaking(BitStreamWriter& bitwriter, const std::vector<lobbydata>& lobbies)
{
	bitwriter.Write(PACKETCODE::MATCHMAKING);
	bitwriter.Write(static_cast<uint8_t>(lobbies.size()));
	for (const auto& lobby : lobbies)
	{
		bitwriter.Write(static_cast<uint8_t>(lobby.info.info.lobby_name.length()));
		bitwriter.WriteBytes(lobby.info.info.lobby_name.data(), lobby.info.info.lobby_name.length());

		bitwriter.Write(static_cast<uint8_t>(lobby.info.info.lobby_host.length()));
		bitwriter.WriteBytes(lobby.info.info.lobby_host.data(), lobby.info.info.lobby_host.length());

		bitwriter.Write(lobby.info.info.player_count);
	}
}
static void ReadPacket_matchmaking(BitStreamReader& bitreader, std::vector<lobby_info>& lobbies)
{
	lobbies.clear();
	
	uint8_t lobby_count;
	bitreader.Read(&lobby_count);
	lobbies.resize(lobby_count);
	for (int i = 0; i < lobby_count; i++)
	{
		uint8_t name_length;

		bitreader.Read(&name_length);
		lobbies[i].lobby_name.resize(name_length);
		bitreader.ReadBytes(lobbies[i].lobby_name.data(), name_length);

		bitreader.Read(&name_length);
		lobbies[i].lobby_host.resize(name_length);
		bitreader.ReadBytes(lobbies[i].lobby_host.data(), name_length);
		
		bitreader.Read(&lobbies[i].player_count);
	}
}

static void WritePacket_enterlobby(BitStreamWriter& bitwriter, const create_info& info)
{
	bitwriter.Write(PACKETCODE::ENTERLOBBY);
	bitwriter.Write(info.creator);
	bitwriter.Write(info.open_access);

	bitwriter.Write(static_cast<uint8_t>(info.lobby_name.length()));
	bitwriter.WriteBytes(info.lobby_name.data(), info.lobby_name.length());
}
static void ReadPacket_enterlobby(BitStreamReader& bitreader, create_info& info)
{
	bitreader.Read(&info.creator);
	bitreader.Read(&info.open_access);

	uint8_t namelength;
	bitreader.Read(&namelength);
	info.lobby_name.resize(namelength);
	bitreader.ReadBytes(info.lobby_name.data(), namelength);
}

static void WritePacket_serverlobby(BitStreamWriter& bitwriter, const lobby_fullinfo_client& data)
{
	bitwriter.Write(PACKETCODE::SERVERLOBBY);

	//lobby info
	bitwriter.Write(static_cast<uint8_t>(data.info.info.lobby_name.length()));
	bitwriter.WriteBytes(data.info.info.lobby_name.data(), data.info.info.lobby_name.length());

	bitwriter.Write(static_cast<uint8_t>(data.info.info.lobby_host.length()));
	bitwriter.WriteBytes(data.info.info.lobby_host.data(), data.info.info.lobby_host.length());

	bitwriter.Write(data.info.info.player_count);
	bitwriter.Write(data.info.openaccess);

	//names
	bitwriter.Write(static_cast<uint8_t>(data.names.size()));
	for (int i = 0; i < data.names.size(); i++)
	{
		bitwriter.Write(static_cast<uint8_t>(data.names[i].length()));
		bitwriter.WriteBytes(data.names[i].data(), data.names[i].length());
	}

	//readys
	bitwriter.Write(static_cast<uint8_t>(data.info.ready.size()));
	for (int i = 0; i < data.info.ready.size(); i++)
	{
		bitwriter.Write(data.info.ready[i]);
	}
	
	//messages
	bitwriter.Write(static_cast<uint8_t>(data.info.messages.size()));
	for (int i = 0; i < data.info.messages.size(); i++)
	{
		bitwriter.Write(static_cast<uint8_t>(data.info.messages[i].length()));
		bitwriter.WriteBytes(data.info.messages[i].data(), data.info.messages[i].length());
	}
}
static void ReadPacket_serverlobby(BitStreamReader& bitreader, lobby_fullinfo_client& data)
{
	uint8_t name_len;

	bitreader.Read(&name_len);
	data.info.info.lobby_name.resize(name_len);
	bitreader.ReadBytes(data.info.info.lobby_name.data(), name_len);

	bitreader.Read(&name_len);
	data.info.info.lobby_host.resize(name_len);
	bitreader.ReadBytes(data.info.info.lobby_host.data(), name_len);
	
	bitreader.Read(&data.info.info.player_count);
	bitreader.Read(&data.info.openaccess);

	//names
	bitreader.Read(&name_len);
	data.names.resize(name_len);
	for (int i = 0; i < data.names.size(); i++)
	{
		bitreader.Read(&name_len);
		data.names[i].resize(name_len);
		bitreader.ReadBytes(data.names[i].data(), name_len);
	}

	//readys
	bitreader.Read(&name_len);
	data.info.ready.resize(name_len);
	for (int i = 0; i < data.info.ready.size(); i++)
	{
		bitreader.Read(&data.info.ready[i]);
	}

	//messages
	bitreader.Read(&name_len);
	data.info.messages.resize(name_len);
	for (int i = 0; i < data.info.messages.size(); i++)
	{
		bitreader.Read(&name_len);
		data.info.messages[i].resize(name_len);
		bitreader.ReadBytes(data.info.messages[i].data(), name_len);
	}
	
}

static void WritePacket_lobbyleaving(BitStreamWriter& bitwriter)
{
	bitwriter.Write(PACKETCODE::LOBBYLEAVING);
}
static void ReadPacket_lobbyleaving(BitStreamReader& bitreader)
{
}

static void WritePacket_lobbyready(BitStreamWriter& bitwriter, const bool& val)
{
	bitwriter.Write(PACKETCODE::LOBBYREADY);
	bitwriter.Write(val);
}
static void ReadPacket_lobbyready(BitStreamReader& bitreader, bool& val)
{
	bitreader.Read(&val);
}

static void WritePacket_lobbytype(BitStreamWriter& bitwriter, std::string_view msg)
{
	bitwriter.Write(PACKETCODE::LOBBYTYPE);
	bitwriter.Write(static_cast<uint8_t>(msg.length()));
	bitwriter.WriteBytes(msg.data(), msg.length());
}
static void ReadPacket_lobbytype(BitStreamReader& bitreader, std::string& msg)
{
	uint8_t str_size;
	bitreader.Read(&str_size);
	msg.resize(str_size);
	bitreader.ReadBytes(msg.data(), str_size);
}

static void WritePacket_lobbyfail(BitStreamWriter& bitwriter, const uint8_t& data)
{
	bitwriter.Write(PACKETCODE::LOBBYFAIL);
	bitwriter.Write(data);
}
static void ReadPacket_lobbyfail(BitStreamReader& bitreader, uint8_t& data)
{
	bitreader.Read(&data);
}

static void WritePacket_launchgame(BitStreamWriter& bitwriter, int port_id)
{
	bitwriter.Write(PACKETCODE::LAUNCHGAME);
	bitwriter.Write(port_id);
}
static void ReadPacket_launchgame(BitStreamReader& bitreader, int& port_id)
{
	bitreader.Read(&port_id);	
}
