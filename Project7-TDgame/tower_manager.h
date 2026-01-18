#pragma once
#include"manager.h"
#include"config_manager.h"
#include"resources_manager.h"
#include"tower.h"
#include"archer_tower.h"
#include"axeman_tower.h"
#include"gunner_tower.h"


class TowerManager : public Manager<TowerManager>
{
	friend class Manager<TowerManager>;
public:
	typedef std::vector<Tower*> TowerList;
protected:
	TowerManager() = default;
	~TowerManager() = default;
public:
	void on_update(double delta)
	{
		for (Tower* tower : tower_list)
			tower->on_update(delta);
	}

	void on_render(SDL_Renderer* renderer)
	{
		for (Tower* tower : tower_list)
			tower->on_render(renderer);
	}

public:
	double get_place_tower_cost(TowerType type)
	{

		static ConfigManager* config_ins = ConfigManager::instance();
		switch (type)
		{
		case Archer:
			return config_ins->archer_template.cost[config_ins->level_archer];
			break;
		case Axeman:
			return config_ins->axeman_template.cost[config_ins->level_axeman];
			break;
		case Gunner:
			return config_ins->gunner_template.cost[config_ins->level_gunner];
			break;
		}

		return 0;
	}

	double get_upgrade_tower_cost(TowerType type)
	{
		static ConfigManager* config_ins = ConfigManager::instance();
		switch (type)
		{
		case Archer:
			if (config_ins->level_archer == 9)
				return -1;
			return config_ins->archer_template.upgrade_cost[config_ins->level_archer];
			break;
		case Axeman:
			if (config_ins->level_axeman == 9)
				return -1;
			return config_ins->axeman_template.upgrade_cost[config_ins->level_axeman];
			break;
		case Gunner:
			if (config_ins->level_gunner == 9)
				return -1;
			return config_ins->gunner_template.upgrade_cost[config_ins->level_gunner];
			break;
		}

		return 0;
	}

	double get_tower_damage_range(TowerType type)
	{
		static ConfigManager* config_ins = ConfigManager::instance();
		switch (type)
		{
		case Archer:
			return config_ins->archer_template.view_range[config_ins->level_archer];
			break;
		case Axeman:
			return config_ins->axeman_template.view_range[config_ins->level_axeman];
			break;
		case Gunner:
			return config_ins->gunner_template.view_range[config_ins->level_gunner];
			break;
		}

		return 0;
	}
public:
	void place_tower(TowerType type, SDL_Point idx_pos)
	{
		Tower* tower = nullptr;
		switch (type)
		{
		case Archer:
			tower = new ArcherTower();
			break;
		case Axeman:
			tower = new AxemanTower();
			break;
		case Gunner:
			tower = new GunnerTower();
			break;
		default:
			tower = new ArcherTower();
			break;
		}

		static Vector2 place_positon_world;
		static SDL_Rect& rect_map = ConfigManager::instance()->rect_tile_map;

		place_positon_world.x = rect_map.x + idx_pos.x * SIZE_TILE + SIZE_TILE / 2;
		place_positon_world.y = rect_map.y + idx_pos.y * SIZE_TILE + SIZE_TILE / 2;
		tower->set_position(place_positon_world);
		tower_list.push_back(tower);

		const static ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_PlaceTower)->second, 0);
		return;
    }

	void upgrade_tower(TowerType type)
	{
		static ConfigManager* config_ins = ConfigManager::instance();
		switch (type)
		{
		case Archer:
			config_ins->level_archer = config_ins->level_archer >= 9 ? 9 : config_ins->level_archer + 1;
			break;
		case Axeman:
			config_ins->level_axeman = config_ins->level_axeman >= 9 ? 9 : config_ins->level_axeman + 1;
			break;
		case Gunner:
			config_ins->level_gunner = config_ins->level_gunner >= 9 ? 9 : config_ins->level_gunner + 1;
			break;
		}
		const static ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_TowerLevelUp)->second, 0);

		return;
	}
private:
	TowerList tower_list;
};

