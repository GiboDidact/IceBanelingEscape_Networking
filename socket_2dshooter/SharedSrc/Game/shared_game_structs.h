#pragma once

namespace shared 
{
	enum class OBJECTOPERATION : uint8_t { CREATE, UPDATE, DESTROY, MOVE, TEXTURE, VISIBILITY, DEATH, LAYER, MAX_ENUM };
	enum class SOUNDS : uint8_t { DEATH1, DEATH2, DEATH3, DOOR, REVIVE, ZEALOT, NEXTLEVEL, AVILO_0, AVILO_00, AVILO_000, 
		                          AVILO_KONGFU, AVILO_KONGFUSHORT, AVILO_1, AVILO_11, TANK_1, TANK_2, TANK_3,
								  AVILO_3, AVILO_31, AVILO_32, AVILO_4, AVILO_42, AVILO_43, AVILO_CRY,
								  AVILO_5, AVILO_55, AVILO_HAMMER, AVILO_6, AVILO_66, AVILO_COPS, END_SONG, CHEERS,
									COOKING, DEAD, MAX_ENUM };
	
	//0-19 are for static
	static uint32_t dynamic_game_object_start_id = 20;

	static uint32_t getBitsRequired(uint32_t number)
	{
		uint32_t bits_required = 0;
		while (number != 0)
		{
			number = number >> 1;
			bits_required++;
		}
		return bits_required;
	}

	struct uiability_network_s {
		uiability_network_s() { Init(); }
		void Init() { goo_percent = 0.0f; disk_percent = 0.0f; speedup_percent = 0.0f; }
		bool anyActive() { return goo_percent != 0.0f || disk_percent != 0.0f || speedup_percent != 0.0f; }

		float goo_percent;
		float disk_percent;
		float speedup_percent;
	};

	struct gameobject_network_s
	{
		uint32_t game_id;
		OBJECTOPERATION operation;
		glm::vec2 position;
		float rotation_angle;
		uint32_t texture_id;
		glm::vec2 scale;
		bool visible;
		uint8_t layer;
	};

	struct player_network_data_event_s
	{
		uint8_t player_id;
		OBJECTOPERATION operation;
		uint32_t texture_id;
		//cd activation
		//death
		//
	};

	struct player_network_data_dynamic_s
	{
		uint8_t player_id;
		glm::vec2 pos;
		float angle;
		bool reflect;
		uint32_t texture_id;
	};

	struct input_data_s
	{
		input_data_s() { Init(); }
		void Init() 
		{
			key_s = false; key_f = false; key_e = false; key_t = false; key_r = false; mouse_right = false; world_pos_click = glm::vec2(0, 0); 
			timestamp = 0;
		}
		bool anyinput()
		{
			return key_s || key_f || key_e || key_t || key_r || mouse_right;
		}


		bool key_s;
		bool key_f;
		bool key_e;
		bool key_t;
		bool key_r;

		bool mouse_right;
		glm::vec2 world_pos_click;

		int timestamp;
	};
}