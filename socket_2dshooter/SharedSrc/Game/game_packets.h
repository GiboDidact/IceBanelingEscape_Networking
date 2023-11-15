#pragma once
#include <iostream>
#include <queue>
#include "../Sockets/BitStream.h"
#include "shared_game_structs.h"
#include "NetworkDelivery.h"

enum class GAMEPACKETCODE : uint8_t { HELLO, WELCOME, LAUNCH, ACKNOWLEDGEMENT, ENEMY, PLAYER_UNIQUE, PLAYERS_EVENT, PLAYER_DYNAMIC,
									  INPUT, GAMEOBJECT, UIABILITY, AUDIO, LEVEL_LOAD, CHAT, SERVER_CHATS, PING, MAX_ENUM };

//make sure each message ends on a byte
static void NextByte(BitStreamWriter& bitwriter)
{
	//each packet makes sure it ends on a byte, round to byte
	if (bitwriter.getbithead() % 8 != 0)
	{
		int remainder = bitwriter.getbithead() % 8;
		bitwriter.Write(0, (8 - remainder));
	}
	my_assert(bitwriter.getbithead() % 8 == 0, "bit writer should be at first position nextbyte");
}
static void NextByteRead(BitStreamReader& bitreader)
{
	//each packet makes sure it ends on a byte, round to byte
	if (bitreader.getbithead() % 8 != 0)
	{
		int remainder = bitreader.getbithead() % 8;
		bitreader.setbithead(bitreader.getbithead() + (8 - remainder));
	}
	my_assert(bitreader.getbithead() % 8 == 0, "bit reader should be at first position NextByteRead");
}

static void WriteGamePacket_hello(BitStreamWriter& bitwriter)
{
	bitwriter.Write(GAMEPACKETCODE::HELLO);
	NextByte(bitwriter);
}

static void ReadGamePacket_hello(BitStreamReader& bitreader)
{
	NextByteRead(bitreader);
}

static void WriteGamePacket_welcome(BitStreamWriter& bitwriter)
{
	bitwriter.Write(GAMEPACKETCODE::WELCOME);
	NextByte(bitwriter);
}

static void ReadGamePacket_welcome(BitStreamReader& bitreader)
{
	NextByteRead(bitreader);
}

static void WriteGamePacket_launch(BitStreamWriter& bitwriter)
{
	bitwriter.Write(GAMEPACKETCODE::LAUNCH);
	NextByte(bitwriter);
}

static void ReadGamePacket_launch(BitStreamReader& bitreader)
{
	//bitwriter.Write(PACKETCODE::MATCHMAKING);
	//bitwriter.Write(static_cast<uint8_t>(lobbies.size()));
	NextByteRead(bitreader);
}

static void WriteGamePacket_acknowledgement(BitStreamWriter& bitwriter, std::vector<NetworkDelivery::acknowledgerange_s>& acks)
{
	bitwriter.Write(GAMEPACKETCODE::ACKNOWLEDGEMENT);

	bitwriter.Write(static_cast<uint8_t>(acks.size()));
	for (int i = 0; i < acks.size(); i++)
	{
		bitwriter.Write(acks[i].start_id);
		bitwriter.Write(acks[i].count);
	}
	NextByte(bitwriter);
}

static void ReadGamePacket_acknowledgement(BitStreamReader& bitreader, std::vector<NetworkDelivery::acknowledgerange_s>& acks)
{
	uint8_t ack_size;
	bitreader.Read(&ack_size);
	
	acks.clear();
	for (int i = 0; i < ack_size; i++)
	{
		NetworkDelivery::acknowledgerange_s tmp;
		bitreader.Read(&tmp.start_id);
		bitreader.Read(&tmp.count);
	
		acks.push_back(tmp);
	}
	NextByteRead(bitreader);
}


static void WriteGamePacket_enemy(BitStreamWriter& bitwriter, std::vector<glm::vec3>& enemy_pos, uint32_t starting_index)
{
	bitwriter.Write(GAMEPACKETCODE::ENEMY);
	bitwriter.Write(static_cast<uint8_t>(enemy_pos.size()));
	bitwriter.Write(starting_index);
	for (int i = 0; i < enemy_pos.size(); i++)
	{
		bitwriter.Write(enemy_pos[i].x);
		bitwriter.Write(enemy_pos[i].y);
		bitwriter.Write(enemy_pos[i].z);
	}
	NextByte(bitwriter);
}

static void ReadGamePacket_enemy(BitStreamReader& bitreader, std::vector<glm::vec3>& enemy_pos, uint32_t& starting_index)
{
	//lowp_fvec1
	uint8_t enemy_count = 0;
	bitreader.Read(&enemy_count);
	enemy_pos.reserve(enemy_count);
	bitreader.Read(&starting_index);
	for (int i = 0; i < enemy_count; i++)
	{
		glm::vec3 pos;
		bitreader.Read(&pos.x);
		bitreader.Read(&pos.y);
		bitreader.Read(&pos.z);
		enemy_pos.push_back(pos);
	}
	NextByteRead(bitreader);
}

static void WriteGamePacket_playerunique(BitStreamWriter& bitwriter, uint8_t player_id)
{
	bitwriter.Write(GAMEPACKETCODE::PLAYER_UNIQUE);
	bitwriter.Write(player_id);

	NextByte(bitwriter);
}

static void ReadGamePacket_playerunique(BitStreamReader& bitreader, uint8_t& player_id)
{
	bitreader.Read(&player_id);
	NextByteRead(bitreader);
}

static void WriteGamePacket_playersevent(BitStreamWriter& bitwriter, std::vector<shared::player_network_data_event_s>& player_info)
{
	bitwriter.Write(GAMEPACKETCODE::PLAYERS_EVENT);
	bitwriter.Write(static_cast<uint8_t>(player_info.size()));

	for (int i = 0; i < player_info.size();i++)
	{
		bitwriter.Write(player_info[i].player_id);
		bitwriter.Write(player_info[i].operation, shared::getBitsRequired((uint8_t)shared::OBJECTOPERATION::MAX_ENUM));
		if (player_info[i].operation == shared::OBJECTOPERATION::TEXTURE)
		{
			bitwriter.Write(player_info[i].texture_id);
		}
	}
	NextByte(bitwriter);
}

static void ReadGamePacket_playersevent(BitStreamReader& bitreader, std::vector<shared::player_network_data_event_s>& player_info)
{
	uint8_t player_count = 0;
	bitreader.Read(&player_count);

	player_info.reserve(player_count);
	for (int i = 0; i < player_count; i++)
	{
		shared::player_network_data_event_s player_data;
		bitreader.Read(&player_data.player_id);
		bitreader.Read(&player_data.operation, shared::getBitsRequired((uint8_t)shared::OBJECTOPERATION::MAX_ENUM));
		if (player_data.operation == shared::OBJECTOPERATION::TEXTURE)
		{
			bitreader.Read(&player_data.texture_id);
		}
		
		player_info.push_back(player_data);
	}
	NextByteRead(bitreader);
}

static void WriteGamePacket_playersdynamic(BitStreamWriter& bitwriter, std::vector<shared::player_network_data_dynamic_s>& player_info)
{
	bitwriter.Write(GAMEPACKETCODE::PLAYER_DYNAMIC);
	bitwriter.Write(static_cast<uint8_t>(player_info.size()));
	for (int i = 0; i < player_info.size(); i++)
	{
		bitwriter.Write(player_info[i].player_id);
		bitwriter.Write(player_info[i].pos.x);
		bitwriter.Write(player_info[i].pos.y);
		bitwriter.Write(player_info[i].angle);
		bitwriter.Write(player_info[i].reflect);
		bitwriter.Write(player_info[i].texture_id);
	}
	NextByte(bitwriter);
}

static void ReadGamePacket_playersdynamic(BitStreamReader& bitreader, std::vector<shared::player_network_data_dynamic_s>& player_info)
{
	uint8_t player_count = 0;
	bitreader.Read(&player_count);
	player_info.reserve(player_count);
	for (int i = 0; i < player_count; i++)
	{
		shared::player_network_data_dynamic_s player_data;
		bitreader.Read(&player_data.player_id);
		bitreader.Read(&player_data.pos.x);
		bitreader.Read(&player_data.pos.y);
		bitreader.Read(&player_data.angle);
		bitreader.Read(&player_data.reflect);
		bitreader.Read(&player_data.texture_id);

		player_info.push_back(player_data);
	}
	NextByteRead(bitreader);
}

static void WriteGamePacket_input(BitStreamWriter& bitwriter, shared::input_data_s player_input)
{
	bitwriter.Write(GAMEPACKETCODE::INPUT);
	
	bitwriter.Write(player_input.key_s);
	bitwriter.Write(player_input.key_f);
	bitwriter.Write(player_input.key_e);
	bitwriter.Write(player_input.key_t);
	bitwriter.Write(player_input.key_r);

	bitwriter.Write(player_input.mouse_right);
	if (player_input.mouse_right)
	{
		bitwriter.Write(player_input.world_pos_click.x);
		bitwriter.Write(player_input.world_pos_click.y);
	}

	bitwriter.Write(player_input.timestamp);

	NextByte(bitwriter);
}

static void ReadGamePacket_input(BitStreamReader& bitreader, shared::input_data_s& player_input)
{
	bitreader.Read(&player_input.key_s);
	bitreader.Read(&player_input.key_f);
	bitreader.Read(&player_input.key_e);
	bitreader.Read(&player_input.key_t);
	bitreader.Read(&player_input.key_r);

	bitreader.Read(&player_input.mouse_right);
	if (player_input.mouse_right)
	{
		bitreader.Read(&player_input.world_pos_click.x);
		bitreader.Read(&player_input.world_pos_click.y);
	}

	bitreader.Read(&player_input.timestamp);

	NextByteRead(bitreader);
}

static void WriteGamePacket_gameobject(BitStreamWriter& bitwriter, std::vector<shared::gameobject_network_s>& gameobj)
{
	bitwriter.Write(GAMEPACKETCODE::GAMEOBJECT);
	bitwriter.Write(static_cast<uint8_t>(gameobj.size()));
	for (int i = 0; i < gameobj.size(); i++)
	{
		bitwriter.Write(gameobj[i].game_id);
		bitwriter.Write(gameobj[i].operation, shared::getBitsRequired((uint8_t)shared::OBJECTOPERATION::MAX_ENUM));
		if (gameobj[i].operation == shared::OBJECTOPERATION::CREATE) {
			bitwriter.Write(gameobj[i].position.x);
			bitwriter.Write(gameobj[i].position.y);
			bitwriter.Write(gameobj[i].texture_id);
			bitwriter.Write(gameobj[i].visible);
			bitwriter.Write(gameobj[i].rotation_angle);
			bitwriter.Write(gameobj[i].layer);
			bitwriter.Write(gameobj[i].scale.x);
			bitwriter.Write(gameobj[i].scale.y);
		}
		else if (gameobj[i].operation == shared::OBJECTOPERATION::UPDATE) {
			bitwriter.Write(gameobj[i].position.x);
			bitwriter.Write(gameobj[i].position.y);
			bitwriter.Write(gameobj[i].texture_id);
			bitwriter.Write(gameobj[i].visible);
		}
		else if (gameobj[i].operation == shared::OBJECTOPERATION::MOVE) {
			bitwriter.Write(gameobj[i].position.x);
			bitwriter.Write(gameobj[i].position.y);
		}
		else if (gameobj[i].operation == shared::OBJECTOPERATION::DESTROY) {

		}
		else if (gameobj[i].operation == shared::OBJECTOPERATION::LAYER) {
			bitwriter.Write(gameobj[i].layer);
		}
		else if (gameobj[i].operation == shared::OBJECTOPERATION::TEXTURE) {
			bitwriter.Write(gameobj[i].texture_id);
		}
		else if (gameobj[i].operation == shared::OBJECTOPERATION::VISIBILITY) {
			bitwriter.Write(gameobj[i].visible);
		}
		else
		{
			std::cout << "Error invalid game object operation\n";
		}
	}
	NextByte(bitwriter);
}

static void ReadGamePacket_gameobject(BitStreamReader& bitreader, std::vector<shared::gameobject_network_s>& gameobjs)
{
	uint8_t gameobj_count;
	bitreader.Read(&gameobj_count);
	gameobjs.reserve(gameobj_count);
	for (int i = 0; i < gameobj_count; i++)
	{
		shared::gameobject_network_s gameobj;
		bitreader.Read(&gameobj.game_id);
		bitreader.Read(&gameobj.operation, shared::getBitsRequired((uint8_t)shared::OBJECTOPERATION::MAX_ENUM));
		if (gameobj.operation == shared::OBJECTOPERATION::CREATE) {
			bitreader.Read(&gameobj.position.x);
			bitreader.Read(&gameobj.position.y);
			bitreader.Read(&gameobj.texture_id);
			bitreader.Read(&gameobj.visible);
			bitreader.Read(&gameobj.rotation_angle);
			bitreader.Read(&gameobj.layer);
			bitreader.Read(&gameobj.scale.x);
			bitreader.Read(&gameobj.scale.y);
		}
		else if (gameobj.operation == shared::OBJECTOPERATION::UPDATE) {
			bitreader.Read(&gameobj.position.x);
			bitreader.Read(&gameobj.position.y);
			bitreader.Read(&gameobj.texture_id);
			bitreader.Read(&gameobj.visible);
		}
		else if (gameobj.operation == shared::OBJECTOPERATION::MOVE) {
			bitreader.Read(&gameobj.position.x);
			bitreader.Read(&gameobj.position.y);
		}
		else if (gameobj.operation == shared::OBJECTOPERATION::DESTROY) {

		}
		else if (gameobj.operation == shared::OBJECTOPERATION::LAYER) {
			bitreader.Read(&gameobj.layer);
		}
		else if (gameobj.operation == shared::OBJECTOPERATION::TEXTURE) {
			bitreader.Read(&gameobj.texture_id);
		}
		else if (gameobj.operation == shared::OBJECTOPERATION::VISIBILITY) {
			bitreader.Read(&gameobj.visible);
		}
		gameobjs.push_back(gameobj);
	}
	NextByteRead(bitreader);
}

static void WriteGamePacket_uiability(BitStreamWriter& bitwriter, shared::uiability_network_s& uiability_info)
{
	bitwriter.Write(GAMEPACKETCODE::UIABILITY);
	bitwriter.Write(uiability_info.disk_percent);
	bitwriter.Write(uiability_info.goo_percent);
	bitwriter.Write(uiability_info.speedup_percent);

	NextByte(bitwriter);
}

static void ReadGamePacket_uiability(BitStreamReader& bitreader, shared::uiability_network_s& uiability_info)
{
	bitreader.Read(&uiability_info.disk_percent);
	bitreader.Read(&uiability_info.goo_percent);
	bitreader.Read(&uiability_info.speedup_percent);

	NextByteRead(bitreader);
}

static void WriteGamePacket_audio(BitStreamWriter& bitwriter, std::vector<shared::SOUNDS>& sounds)
{
	bitwriter.Write(GAMEPACKETCODE::AUDIO);
	bitwriter.Write(static_cast<uint8_t>(sounds.size()));
	for(int i = 0;i<sounds.size();i++)
		bitwriter.Write(sounds[i], shared::getBitsRequired((uint8_t)shared::SOUNDS::MAX_ENUM));

	NextByte(bitwriter);
}

static void ReadGamePacket_audio(BitStreamReader& bitreader, std::vector<shared::SOUNDS>& sounds)
{
	uint8_t sound_count;
	bitreader.Read(&sound_count);
	sounds.resize(sound_count);
	for (int i = 0; i < sounds.size(); i++)
		bitreader.Read(&sounds[i], shared::getBitsRequired((uint8_t)shared::SOUNDS::MAX_ENUM));

	NextByteRead(bitreader);
}

static void WriteGamePacket_levelload(BitStreamWriter& bitwriter, int level, uint32_t& uid)
{
	bitwriter.Write(GAMEPACKETCODE::LEVEL_LOAD);
	bitwriter.Write(static_cast<uint8_t>(level));
	bitwriter.Write(uid);
	NextByte(bitwriter);
}

static void ReadGamePacket_levelload(BitStreamReader& bitreader, int& level, uint32_t& uid)
{
	uint8_t lvl;
	bitreader.Read(&lvl);
	level = lvl;
	bitreader.Read(&uid);
	NextByteRead(bitreader);
}

static void WriteGamePacket_chat(BitStreamWriter& bitwriter, std::string& msg)
{
	bitwriter.Write(GAMEPACKETCODE::CHAT);
	bitwriter.Write(static_cast<uint8_t>(msg.length()));
	bitwriter.WriteBytes(msg.data(), msg.length());
	
	NextByte(bitwriter);
}

static void ReadGamePacket_chat(BitStreamReader& bitreader, std::string& msg)
{
	uint8_t name_length;
	bitreader.Read(&name_length);

	msg.resize(name_length);
	bitreader.ReadBytes(msg.data(), name_length);

	NextByteRead(bitreader);
}

static void WriteGamePacket_serverchats(BitStreamWriter& bitwriter, std::queue<std::string> msgs)
{
	bitwriter.Write(GAMEPACKETCODE::SERVER_CHATS);
	bitwriter.Write(static_cast<uint8_t>(msgs.size()));
	while (!msgs.empty())
	{
		bitwriter.Write(static_cast<uint8_t>(msgs.front().length()));
		bitwriter.WriteBytes(msgs.front().data(), msgs.front().length());
		msgs.pop();
	}

	NextByte(bitwriter);
}

static void ReadGamePacket_serverchats(BitStreamReader& bitreader, std::vector<std::string>& msgs)
{
	uint8_t msg_count = 0;
	bitreader.Read(&msg_count);
	msgs.reserve(msg_count);

	for (int i = 0; i < msg_count; i++)
	{
		std::string msg;
		uint8_t name_length;
		bitreader.Read(&name_length);
		msg.resize(name_length);
		bitreader.ReadBytes(msg.data(), name_length);
		
		msgs.push_back(msg);
	}


	NextByteRead(bitreader);
}

static void WriteGamePacket_ping(BitStreamWriter& bitwriter, int timestamp, float ping)
{
	bitwriter.Write(GAMEPACKETCODE::PING);
	bitwriter.Write(timestamp);
	bitwriter.Write(ping);

	NextByte(bitwriter);
}

static void ReadGamePacket_ping(BitStreamReader& bitreader, int& timestamp, float& ping)
{
	bitreader.Read(&timestamp);
	bitreader.Read(&ping);

	NextByteRead(bitreader);
}
