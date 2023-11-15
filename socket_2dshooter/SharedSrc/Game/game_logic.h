#pragma once
#include "shared_game_structs.h"
#include "../GraphicsManager.h"

namespace shared {

	static bool rect_collision_test(GraphicsManager& graphics_manager, uint32_t render_id_a, uint32_t render_id_b, float padding = 0.0f)
	{
		glm::vec2 pos_a = graphics_manager.GetRenderRect(render_id_a).getPos();
		glm::vec2 halfs_a = graphics_manager.GetRenderRect(render_id_a).getScale() * glm::vec2(.5, .5);

		glm::vec2 pos_b = graphics_manager.GetRenderRect(render_id_b).getPos();
		glm::vec2 halfs_b = graphics_manager.GetRenderRect(render_id_b).getScale() * glm::vec2(.5, .5);

		if (pos_a.x + halfs_a.x < pos_b.x - halfs_b.x + padding)
			return false;
		if (pos_a.x - halfs_a.x > pos_b.x + halfs_b.x - padding)
			return false;
		if (pos_a.y + halfs_a.y < pos_b.y - halfs_b.y + padding)
			return false;
		if (pos_a.y - halfs_a.y > pos_b.y + halfs_b.y - padding)
			return false;
		return true;
	}

	static bool point_in_rect_collision_test(GraphicsManager& graphics_manager, glm::vec2 pos, uint32_t render_id, float padding = 0.0f)
	{
		glm::vec2 pos_a = graphics_manager.GetRenderRect(render_id).getPos();
		glm::vec2 halfs = graphics_manager.GetRenderRect(render_id).getScale() * glm::vec2(.5, .5);

		if (pos.x < pos_a.x - halfs.x + padding)
			return false;
		if (pos.x > pos_a.x + halfs.x - padding)
			return false;
		if (pos.y < pos_a.y - halfs.y + padding)
			return false;
		if (pos.y > pos_a.y + halfs.y - padding)
			return false;
		return true;
	}

	static void LoadBaneling(GraphicsManager& graphics_manager, uint32_t& rect_id)
	{
		rect_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(rect_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(rect_id).Move(glm::vec3(0, 0, 0));
		graphics_manager.GetRenderRect(rect_id).SetTexture(4);
		graphics_manager.GetRenderRect(rect_id).SetLayer(3);
	}

	//reverseice - 12, grass - 6, nonmoveice - 2, ice - 7, fastice - 14
	static void LoadLevel1Ground(GraphicsManager& graphics_manager, std::vector<uint32_t>& ground_ids)
	{	
		//start
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(2, 3.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(5, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//ice
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(4, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(2, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 4, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(3, -3, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(8, .5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(8, -4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 4, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(13, -4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//reverse ice
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1, -3.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(5, 5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1, -8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(7, 3.5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(5, -8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10, -8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//fast ice
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(4, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-4, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(14);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(8, 4, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-10, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(14);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 8, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-15.5, 4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(14);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(8, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-12, 9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(14);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 8, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-8.5, 14, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(14);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-8.5, 19, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//exit
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(7, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(13);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//slide ice
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1, 3.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(2);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1, 6.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1, 9.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(2);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1, 13, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);
	}

	static void LoadLevel1Enemies(GraphicsManager& graphics_manager, std::vector<uint32_t>& enemy_ids)
	{
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(2, -1.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(3, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);
		
		//fast ice
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-3, -.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-12, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//reverse ice
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-1, -8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-1, -8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-1, -8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-1, -8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-1, -8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-1, -8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(8, -6.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.6, .6, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(6, -10.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//slide ice
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.8, .8, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(1, 5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(37);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.8, .8, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(2, 8, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(37);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 1, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4, 11, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(40);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, .4, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(1, 9.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(39);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);
	}

	static void LoadLevel1GameObjects(GraphicsManager& graphics_manager, std::vector<uint32_t>& gameobj_ids)
	{
		//button-grass
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(2, 5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);


		//button-fast
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-8.5, 19, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//button-slide
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1, 13, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//button-reverse
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10, -8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//button-ice
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(13, -4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//gate-1
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(4.5, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(9);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);
	}

	//reverseice - 12, grass - 6, nonmoveice - 2, ice - 7, fastice - 14
	static void LoadLevel4Ground(GraphicsManager& graphics_manager, std::vector<uint32_t>& ground_ids)
	{
		//starting zone
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(4, 4, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//reverse ice start
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(2.5, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(4.5, 1, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3.5, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(6.75, 2, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2.5, 4, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(6.25, 5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(5.75, 8, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//grass patch
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(3.75, 8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//ice loop
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(2.25, 8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(1.75, 10.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3.5, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(4, 11.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//grass patch
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1.5, .8, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(5, 10.6, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//reverse ice loop
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.8, 1.6, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(5.35, 12.8, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.8, 1.6, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(6.75, 12.8, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(6, 13.1, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//grass area
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(7.85, 11.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 3.5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(8.85, 13.75, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//no-move ice
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(4.1, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(6.3, 15.0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(2);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);
		
		//grass s
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1.75, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(2.725, 13.15, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(3.4, 13.65, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1.35, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(3.575, 15.0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//next ice s
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2.0, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(.850, 13.15, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1.0, 4.65, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(.350, 10.325, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-.350, 8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);
		
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2.5, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-2.25, 8.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-3.0, 10, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1.9, 1.5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1.55, 10.25, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//nonmove ice 2
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, 2.5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-.8, 8.25, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(2);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//final grass
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(4, .4, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-2.6, 6.8, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-4.1, 5.1, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-3.1, 4.1, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//exit
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 2.5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1.1, 4.85, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(13);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

	}

	static void LoadLevel4Enemies(GraphicsManager& graphics_manager, std::vector<uint32_t>& enemy_ids)
	{
		//29 enemies
		//bear
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 1, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(1, -2, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(40);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//reverse ice
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(4.5, -2.3, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(7, -1, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(9.6, 5.6, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(9, 9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(3.5, 6.0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);
		
		//2 bears
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 1, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(1.75, 13, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(40);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(7.5, 13, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(37);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);


		//all the dogs
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, .6, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(7.5, 10.3, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, .4, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(8.65, 12.26, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(39);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, .4, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(9.1, 12.26, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(39);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, .4, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(8.65, 13.26, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(39);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, .4, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(9.1, 13.26, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(39);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, .4, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(8.65, 14.26, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(39);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, .4, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(9.1, 14.26, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(39);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//big guys
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.8, .8, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(7.7, 16.6, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(40);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.8, .8, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(5, 14, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(37);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//dogs
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.3, .3, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(4.6, 13.9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(39);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.3, .3, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(2.4, 14.2, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(39);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//rest
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.7, .7, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-2.5, 13.2, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(37);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, .4, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-3, 7.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.7, .7, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4.5, 10.0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(37);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//last dogs
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, .4, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4.4, 6.8, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(39);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.4, .4, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-3.8, 6.4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(39);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);
	}

	static void LoadLevel4GameObjects(GraphicsManager& graphics_manager, std::vector<uint32_t>& gameobj_ids)
	{
		//no game objects
	}

	//reverseice - 12, grass - 6, nonmoveice - 2, ice - 7, fastice - 14
	static void LoadLevel2Ground(GraphicsManager& graphics_manager, std::vector<uint32_t>& ground_ids)
	{
		//start
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//right lane
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1.5, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, -2, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, -4.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1.5, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, -6.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(2);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(4, 4, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//across lane
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, .75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-2.5, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2.9, .7, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-2.2, -6.425, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(2);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);
		
		//left lane
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-4, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 7, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-4.5, -4.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.35, 2.75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-3.825, -6, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-4.25, -4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//grass safe
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-4.5, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);


		//REVERSE Area
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(10, 10, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-7, 6, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(4, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, 10, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 6, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(.5, 6, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, .8, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(9.5, 3, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(6, 6, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(5, 3, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(12.5, 3, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(13);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);
	}		

	static void LoadLevel2Enemies(GraphicsManager& graphics_manager, std::vector<uint32_t>& enemy_ids)
	{
		//first line
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -4.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -4.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -4.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -4.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -4.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//second line
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(0, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//little ring
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.3, .3, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.3, .3, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.3, .3, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//big ring
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4.25, -4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4.25, -4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4.25, -4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4.25, -4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4.25, -4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4.25, -4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-4.25, -4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(41);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//reverse men
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-3.25, 2, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-7.25, 6, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-3.25, 8, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-1.25, 5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);
		
		//randoms
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(5, 3, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-10.25, 4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-10.25, 9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-3.25, 9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(38);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);
	}

	static void LoadLevel2GameObjects(GraphicsManager& graphics_manager, std::vector<uint32_t>& gameobj_ids)
	{
		//no game objects
	}

	//reverseice - 12, grass - 6, nonmoveice - 2, ice - 7, fastice - 14
	static void LoadLevel3Ground(GraphicsManager& graphics_manager, std::vector<uint32_t>& ground_ids)
	{
		//grass
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(10, 6, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(6.5, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(13, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//ice
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(6, 20, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(17.5, -7, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(10, 6, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(9.5, -14.0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(14);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(15, 12, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-3.0, -14.0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);
	
		//exit
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-12, -14.0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(13);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//secret
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(15, .3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-9.0, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(14);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.3, 6, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-16.35, -3, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(14);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(5, .3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-14.0, -6.15, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.3, 6.25, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-11.65, -9.4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);
	}

	static void LoadLevel3Enemies(GraphicsManager& graphics_manager, std::vector<uint32_t>& enemy_ids)
	{
		uint32_t render_id;

		//first random group
		int dimension1 = 6;
		//rect1 =   pos(6.5,0), scale(10,6)
		for (int x = 0; x < dimension1; x++)
		{
			float x_pos = 1.0 + 1.5 + ((float)x / (float)dimension1) * 10;
			for (int y = 0; y < dimension1; y++)
			{
				float y_pos = .3 + -3 + ((float)y / (float)dimension1) * 6;

				render_id = graphics_manager.CreateRenderRect();
				graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
				graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(x_pos, y_pos, 0));
				graphics_manager.GetRenderRect(render_id).SetTexture(5);
				graphics_manager.GetRenderRect(render_id).SetLayer(3);
				enemy_ids.push_back(render_id);
			}
		}

		//spinning

		//		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(6, 20, 0));
		//      graphics_manager.GetRenderRect(render_id).Move(glm::vec3(17.5, -7, 0));
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(17.5, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(17.5, -7, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(17.5, -14, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(5);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//big guys
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1.5, 1.5, 0));
		graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(-18, -3, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(40);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		enemy_ids.push_back(render_id);

		//second random group
		int dimension2 = 10;
		//rect1 =   pos(-3,-14), scale(15, 12)
		for (int x = 0; x < dimension2; x++)
		{
			float x_pos = 1.0 - 10.5 + ((float)x / (float)dimension2) * 15;
			for (int y = 0; y < dimension2; y++)
			{
				float y_pos = .3 + -20 + ((float)y / (float)dimension2) * 12;

				render_id = graphics_manager.CreateRenderRect();
				graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, .5, 0));
				graphics_manager.GetRenderRect(render_id).Translate(glm::vec3(x_pos, y_pos, 0));
				graphics_manager.GetRenderRect(render_id).SetTexture(5);
				graphics_manager.GetRenderRect(render_id).SetLayer(3);
				enemy_ids.push_back(render_id);
			}
		}
	}

	static void LoadLevel3GameObjects(GraphicsManager& graphics_manager, std::vector<uint32_t>& gameobj_ids)
	{
		//none
	}

	//reverseice - 12, grass - 6, nonmoveice - 2, ice - 7, fastice - 14
	static void LoadLevel5Ground(GraphicsManager& graphics_manager, std::vector<uint32_t>& ground_ids)
	{
		//grass
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(8, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-2.5, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//main area
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(18, 18, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10.5, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//escape area
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(6, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10.5, -10, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(10, 10, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10.5, -16, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(2);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(10, 10, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10.5, -26, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(14);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(10, 10, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10.5, -36, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(2);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(10, 10, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10.5, -46, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(10, 10, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10.5, -56, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(7);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(10, 10, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10.5, -66, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);
		//pathway and exit
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(40, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(35.5, -66, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(43);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		ground_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(4, 4, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(57.5, -66, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(13);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		ground_ids.push_back(render_id);
	}

	static void LoadLevel5Enemies(GraphicsManager& graphics_manager, std::vector<uint32_t>& enemy_ids)
	{
		//none, all dynamic game objects
	}

	static void LoadLevel5GameObjects(GraphicsManager& graphics_manager, std::vector<uint32_t>& gameobj_ids)
	{
		//		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(18, 18, 0));
		//		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10.5, 0, 0));

		//4 boss buttons
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(2.5, 8, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(18.5, 8, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(2.5, -8, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(18.5, -8, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//1 gate
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(6, .5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10.5, -9, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(9);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//2 gate
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.5, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(15.75, -66, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(9);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//1 extra button
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(10.5, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		graphics_manager.GetRenderRect(render_id).SetVisibility(false);
		gameobj_ids.push_back(render_id);
		
		//decorations
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(44, 12, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(38.0, -66, 0));
		graphics_manager.GetRenderRect(render_id).Rotate(glm::vec3(0, 0, 1), 0);
		graphics_manager.GetRenderRect(render_id).SetTexture(60);
		graphics_manager.GetRenderRect(render_id).SetLayer(3);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(41.5, -63.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(1);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(45.5, -63.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(1);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(49.5, -63.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(1);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(53.5, -63.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(1);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(41.5, -68.5, 0));
		graphics_manager.GetRenderRect(render_id).Rotate(glm::vec3(0, 0, 1), 180.0);
		graphics_manager.GetRenderRect(render_id).SetTexture(1);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(45.5, -68.5, 0));
		graphics_manager.GetRenderRect(render_id).Rotate(glm::vec3(0,0,1), 180.0);
		graphics_manager.GetRenderRect(render_id).SetTexture(1);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(49.5, -68.5, 0));
		graphics_manager.GetRenderRect(render_id).Rotate(glm::vec3(0, 0, 1), 180.0);
		graphics_manager.GetRenderRect(render_id).SetTexture(1);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(53.5, -68.5, 0));
		graphics_manager.GetRenderRect(render_id).Rotate(glm::vec3(0, 0, 1), 180.0);
		graphics_manager.GetRenderRect(render_id).SetTexture(1);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//desk
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-2.5, 1, 0));
		graphics_manager.GetRenderRect(render_id).Rotate(glm::vec3(0, 0, 1), 0);
		graphics_manager.GetRenderRect(render_id).SetTexture(54);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1.5, 1.5, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-1.0, 1, 0));
		graphics_manager.GetRenderRect(render_id).Rotate(glm::vec3(0, 0, 1), 0);
		graphics_manager.GetRenderRect(render_id).SetTexture(56);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);
	}

	static void LoadLevel6Ground(GraphicsManager& graphics_manager, std::vector<uint32_t>& ground_ids)
	{
		//grass
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(8, 8, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(6);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//fast
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(8, 4, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, -6, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(14);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);

		//reverse
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(4, 8, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(6, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(12);
		graphics_manager.GetRenderRect(render_id).SetLayer(1);
		ground_ids.push_back(render_id);
	}

	static void LoadLevel6Enemies(GraphicsManager& graphics_manager, std::vector<uint32_t>& enemy_ids)
	{

	}

	static void LoadLevel6GameObjects(GraphicsManager& graphics_manager, std::vector<uint32_t>& gameobj_ids)
	{
		//none
		//dance floor
		uint32_t render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(61);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);
		
		//jaccuzzi
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(62);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//avilo
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, 5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(65);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//button
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75,.75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, 3.25, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//trees
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-2.5, 2.5, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(63);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(2.5, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(63);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-3, -2, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(63);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//notebook
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.25, .25, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-2.5, 1, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(64);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//ice baneling statue
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(1, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-3.4, -3.4, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(1);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//server
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 3, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(4, -2, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(67);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//sc2
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(2, 2, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-5, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(68);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//sc2 button
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(.75, .75, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(-3.25, 0, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(8);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);

		//you escaped
		render_id = graphics_manager.CreateRenderRect();
		graphics_manager.GetRenderRect(render_id).Scale(glm::vec3(3, 1, 0));
		graphics_manager.GetRenderRect(render_id).Move(glm::vec3(0, -2, 0));
		graphics_manager.GetRenderRect(render_id).SetTexture(69);
		graphics_manager.GetRenderRect(render_id).SetLayer(2);
		gameobj_ids.push_back(render_id);
	}
}