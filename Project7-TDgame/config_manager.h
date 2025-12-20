#pragma once
#include"manager.h"
#include"Map.h"
#include"Wave.h"
#include<cJSON.h>
#include<SDL.h>


#include<string>
#include<iostream>
#include<vector>
#include<fstream>
class ConfigManager : Manager<ConfigManager> 
{
	friend class Manager<ConfigManager>;
protected:
	ConfigManager() = default;
	~ConfigManager() = default;
public:
	struct BasicTemplate
	{
		std::string window_title = u8"村庄保卫战";
		int window_width = 1280;
		int window_height = 720;
	};

	struct PlayerTemplate
	{
		double speed = 1;
		double normal_attack_damage = 0;
		double normal_attack_interval = 0.5;
		double skill_interval = 10;
		double skill_damage = 1;
	};

	struct TowerTemplate
	{
		double interval[10] = { 1 };
		double damage[10] = { 2 };
		double view_range[10] = { 5 };
		double cost[10] = { 10 };
		double upgrade_cost[9] = { 75 };
	};

	struct EnemyTemplate
	{
		double hp = 10;
		double speed = 1;
		double damage = 1;
		double reward_ratio = 0.8;
		double recover_interval = 100;
		double recover_range = -1;
		double recover_intensity = 10;
	};
public:
	Map map;

	std::vector<Wave> wave_list;

	int level_axeman = 0;
	int level_archer = 0;
	int level_gunner = 0;

	bool is_game_win = false;
	bool is_game_over = false;

	SDL_Rect rect_tile_map = { 0 };

	BasicTemplate basic_template;
	PlayerTemplate player_template;

	TowerTemplate axeman_template;
	TowerTemplate archer_template;
	TowerTemplate gunner_template;

	EnemyTemplate slim_template;
	EnemyTemplate king_slim_template;
	EnemyTemplate skeleton_template;
	EnemyTemplate goblin_template;
	EnemyTemplate goblin_priest_template;

	const double num_init_hp = 10;
	const double num_init_coin = 100;
	const double num_coin_per_prop = 10;
public:
	bool load_level_config(const std::string& path)
	{
		std::ifstream file(path);
		if (!file.good())
			return false;

		std::stringstream str_stream;
		str_stream << file.rdbuf();
		file.close();

		cJSON* json_root = cJSON_Parse(str_stream.str().c_str());
		if (json_root == nullptr)
			return false;

		if (json_root->type != cJSON_Array)
		{
			cJSON_Delete(json_root);
			return false;
		}

		cJSON* json_wave;
		cJSON_ArrayForEach(json_wave, json_root)
		{
			if (json_wave == nullptr || json_wave->type != cJSON_Object)
				continue;

			wave_list.emplace_back();
			Wave& cur_wave = wave_list.back();

			cJSON* json_wave_interval = cJSON_GetObjectItem(json_wave, "interval");
			if (json_wave_interval && json_wave_interval->type == cJSON_Number)
				cur_wave.interval = json_wave_interval->valuedouble;
			cJSON* json_wave_rewards = cJSON_GetObjectItem(json_wave, "rewards");
			if (json_wave_rewards && json_wave_rewards->type == cJSON_Number)
				cur_wave.rewards = json_wave_rewards->valuedouble;

			cJSON* json_wave_spawn_list = cJSON_GetObjectItem(json_wave, "spawn_list");
			if (json_wave_spawn_list && json_wave_spawn_list->type == cJSON_Array)
			{
				cJSON* json_wave_spawn_event;
				cJSON_ArrayForEach(json_wave_spawn_event, json_wave_spawn_list)
				{
					if (json_wave_spawn_event == nullptr || json_wave_spawn_event->type != cJSON_Object)
						continue;

					cur_wave.spawn_event_list.emplace_back();
					Wave::SpawnEvent& cur_spawn_event = cur_wave.spawn_event_list.back();

					cJSON* json_wave_spawn_event_interval = cJSON_GetObjectItem(json_wave_spawn_event, "interval");
					if (json_wave_spawn_event_interval && json_wave_spawn_event_interval->type == cJSON_Number)
						cur_spawn_event.interval = json_wave_spawn_event_interval->valuedouble;

					cJSON* json_wave_spawn_event_point = cJSON_GetObjectItem(json_wave_spawn_event, "point");
					if (json_wave_spawn_event_point && json_wave_spawn_event_point->type == cJSON_Number)
						cur_spawn_event.point = json_wave_spawn_event_point->valueint;

					cJSON* json_wave_spawn_event_enemy = cJSON_GetObjectItem(json_wave_spawn_event, "enemy");
					if (json_wave_spawn_event_enemy && json_wave_spawn_event_enemy->type == cJSON_String)
					{
						std::string spawn_event_enemy = json_wave_spawn_event_enemy->valuestring;
						if (spawn_event_enemy == "Silm")
							cur_spawn_event.enemy_type = EnemyType::Silm;
						else if (spawn_event_enemy == "KingSilm")
							cur_spawn_event.enemy_type = EnemyType::KingSilm;
						else if (spawn_event_enemy == "Skeleton")
							cur_spawn_event.enemy_type = EnemyType::Skeleton;
						else if (spawn_event_enemy == "Goblin")
							cur_spawn_event.enemy_type = EnemyType::Goblin;
						else if (spawn_event_enemy == "GoblinPriest")
							cur_spawn_event.enemy_type = EnemyType::GoblinPriest;
					}
				}
				if (cur_wave.spawn_event_list.empty())
					wave_list.pop_back();

			}

		}

		cJSON_Delete(json_root);

		if (wave_list.empty())
			return false;

		return true;
	}

	bool load_game_config(const std::string& path)
	{
		std::ifstream file(path);
		if (!file.good())
			return false;

		std::stringstream str_stream;
		str_stream << file.rdbuf();
		file.close();

		cJSON* json_root = cJSON_Parse(str_stream.str().c_str());
		if (!json_root || json_root->type != cJSON_Object)
		{
			return false;
		}
		cJSON* json_basic = cJSON_GetObjectItem(json_root, "basic");
		cJSON* json_player = cJSON_GetObjectItem(json_root, "player");
		cJSON* json_tower = cJSON_GetObjectItem(json_root, "tower");
		cJSON* json_enemy = cJSON_GetObjectItem(json_root, "enemy");

		if (!json_basic || !json_player || !json_tower || !json_enemy
			|| json_basic->type != cJSON_Object
			|| json_player->type != cJSON_Object
			|| json_tower->type != cJSON_Object
			|| json_enemy->type != cJSON_Object)
		{
			cJSON_Delete(json_root);
			return false;
		}

		parse_basic_template(basic_template, json_basic);

		parse_player_template(player_template, json_player);

		parse_tower_template(archer_template, cJSON_GetObjectItem(json_tower, "archer"));
		parse_tower_template(axeman_template, cJSON_GetObjectItem(json_tower, "axeman"));
		parse_tower_template(gunner_template, cJSON_GetObjectItem(json_tower, "gunner"));


		parse_enemy_template(slim_template, cJSON_GetObjectItem(json_enemy, "slim"));
		parse_enemy_template(king_slim_template, cJSON_GetObjectItem(json_enemy, "king_slim"));
		parse_enemy_template(skeleton_template, cJSON_GetObjectItem(json_enemy, "skeleton"));
		parse_enemy_template(goblin_template, cJSON_GetObjectItem(json_enemy, "goblin"));
		parse_enemy_template(goblin_priest_template, cJSON_GetObjectItem(json_enemy, "goblin_priest"));

		cJSON_Delete(json_root);

		return true;
	}

private: 
	//config解析辅助函数
	void parse_basic_template(BasicTemplate& tpl, cJSON* json_root)
	{
		if (!json_root || json_root->type != cJSON_Object)
			return;

		cJSON* json_win_title = cJSON_GetObjectItem(json_root, "window_title");
		cJSON* json_win_width = cJSON_GetObjectItem(json_root, "window_width");
		cJSON* json_win_height = cJSON_GetObjectItem(json_root, "window_height");

		if (json_win_title && json_win_title->type == cJSON_String)
			tpl.window_title = json_win_title->valuestring;
		if (json_win_width && json_win_width->type == cJSON_Number)
			tpl.window_width = json_win_width->valueint;
		if (json_win_height && json_win_height->type == cJSON_Number)
			tpl.window_height = json_win_height->valueint;

		return;
	}

	void parse_player_template(PlayerTemplate& tpl, cJSON* json_root)
	{
		if (!json_root || json_root->type != cJSON_Object)
			return;

		cJSON* json_speed = cJSON_GetObjectItem(json_root, "speed");
		cJSON* json_normal_attack_interval = cJSON_GetObjectItem(json_root, "normal_attack_interval");
		cJSON* json_normal_attack_damage = cJSON_GetObjectItem(json_root, "normal_attack_damage");
		cJSON* json_skill_interval = cJSON_GetObjectItem(json_root, "skill_interval");
		cJSON* json_skill_damage = cJSON_GetObjectItem(json_root, "skill_damage");

		if (json_speed && json_speed->type == cJSON_Number)
			tpl.speed = json_speed->valuedouble;
		if (json_normal_attack_damage && json_normal_attack_damage->type == cJSON_Number)
			tpl.normal_attack_damage = json_normal_attack_damage->valuedouble;
		if (json_normal_attack_interval && json_normal_attack_interval->type == cJSON_Number)
			tpl.normal_attack_interval= json_normal_attack_interval->valuedouble;
		if (json_skill_damage && json_skill_damage->type == cJSON_Number)
			tpl.skill_damage = json_skill_damage->valuedouble;
		if (json_skill_interval && json_skill_interval->type == cJSON_Number)
			tpl.skill_interval = json_skill_interval->valuedouble;

		return;
	}

	void parse_array_helper(double* arr, int max_len, cJSON* json_root)
	{
		if (!json_root || json_root->type != cJSON_Array)
			return;

		int idx_num = -1;
		cJSON* json_number = nullptr;
		cJSON_ArrayForEach(json_number, json_root)
		{
			idx_num++;
			if (!json_number || json_number->type != cJSON_Number || idx_num >= max_len)
				continue;

			arr[idx_num] = json_number->valuedouble;
		}

		return;
	}
	void parse_tower_template(TowerTemplate& tpl, cJSON* json_root)
	{
		if (!json_root || json_root->type != cJSON_Object)
			return;

		cJSON* json_interval = cJSON_GetObjectItem(json_root, "interval");
		cJSON* json_damage = cJSON_GetObjectItem(json_root, "damage");
		cJSON* json_view_range = cJSON_GetObjectItem(json_root, "view_range");
		cJSON* json_cost = cJSON_GetObjectItem(json_root, "cost");
		cJSON* json_upgrade_cost = cJSON_GetObjectItem(json_root, "upgrade_cost");

		parse_array_helper(tpl.interval, 10, json_interval);
		parse_array_helper(tpl.damage, 10, json_damage);
		parse_array_helper(tpl.view_range, 10, json_view_range);
		parse_array_helper(tpl.cost, 10, json_cost);
		parse_array_helper(tpl.upgrade_cost, 9, json_upgrade_cost);

		return;
	}
	
	void parse_enemy_template(EnemyTemplate& tpl, cJSON* json_root)
	{
		if (!json_root || json_root->type != cJSON_Object)
			return;

		cJSON* json_hp = cJSON_GetObjectItem(json_root, "hp");
		cJSON* json_speed = cJSON_GetObjectItem(json_root, "speed");
		cJSON* json_damage = cJSON_GetObjectItem(json_root, "damage");
		cJSON* json_reward_ratio = cJSON_GetObjectItem(json_root, "reward_ratio");
		cJSON* json_recover_interval = cJSON_GetObjectItem(json_root, "recover_interval");
		cJSON* json_recover_range = cJSON_GetObjectItem(json_root, "recover_range");
		cJSON* json_recover_intensity = cJSON_GetObjectItem(json_root, "recover_intensity");

		if (json_hp && json_hp->type == cJSON_Number)
			tpl.hp = json_hp->valuedouble;
		if (json_speed && json_speed->type == cJSON_Number)
			tpl.speed = json_speed->valuedouble;
		if (json_damage && json_damage->type == cJSON_Number)
			tpl.damage = json_damage->valuedouble;
		if (json_reward_ratio && json_reward_ratio->type == cJSON_Number)
			tpl.reward_ratio = json_reward_ratio->valuedouble;
		if (json_recover_interval && json_recover_interval->type == cJSON_Number)
			tpl.recover_interval = json_recover_interval->valuedouble;
		if (json_recover_range && json_recover_range->type == cJSON_Number)
			tpl.recover_range = json_recover_range->valuedouble;
		if (json_recover_intensity && json_recover_intensity->type == cJSON_Number)
			tpl.recover_intensity = json_recover_intensity->valuedouble;

		return;
	}
};
