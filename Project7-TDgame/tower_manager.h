#pragma once
#include"manager.h"
#include"config_manager.h"
#include"resources_manager.h"
#include"tower.h"
#include"archer_tower.h"
#include"axeman_tower.h"
#include"gunner_tower.h"
#include"barracks_tower.h"

#include<algorithm>
#include<unordered_map>

class TowerManager : public Manager<TowerManager>
{
	friend class Manager<TowerManager>;
public:
	typedef std::vector<Tower*> TowerList;
protected:
	TowerManager() = default;
	~TowerManager()
	{
		clear();
	}
public:
	void on_update(double delta)
	{
		for (Tower* tower : tower_list)
			tower->on_update(delta);
		process_silencer_enemies(delta);
	}

	void on_render(SDL_Renderer* renderer)
	{
		for (Tower* tower : tower_list)
			tower->on_render(renderer);
	}

	TowerList& get_tower_list()
	{
		return tower_list;
	}

public:
	double get_place_tower_cost(TowerType type)
	{

		static ConfigManager* config_ins = ConfigManager::instance();
		switch (type)
		{
		case Archer:
			return config_ins->archer_template.cost[0];
			break;
		case Axeman:
			return config_ins->axeman_template.cost[0];
			break;
		case Gunner:
			return config_ins->gunner_template.cost[0];
			break;
		case Barracks:
			return config_ins->barracks_template.cost[0];
			break;
		}

		return 0;
	}

	double get_upgrade_tower_cost(Tower* tower)
	{
		if (!tower)
			return -1;
		return get_upgrade_tower_cost(tower->get_tower_type(), tower->get_level());
	}

	double get_upgrade_tower_cost(const SDL_Point& idx_tile)
	{
		return get_upgrade_tower_cost(find_tower(idx_tile));
	}

	double get_upgrade_tower_cost(TowerType type, int level)
	{
		static ConfigManager* config_ins = ConfigManager::instance();
		if (level >= 9)
			return -1;
		switch (type)
		{
		case Archer:
			return config_ins->archer_template.upgrade_cost[level];
			break;
		case Axeman:
			return config_ins->axeman_template.upgrade_cost[level];
			break;
		case Gunner:
			return config_ins->gunner_template.upgrade_cost[level];
			break;
		case Barracks:
			return config_ins->barracks_template.upgrade_cost[level];
			break;
		}

		return 0;
	}

	double get_tower_invested_cost(TowerType type, int level)
	{
		double total = get_place_tower_cost(type);
		for (int i = 0; i < level; i++)
		{
			double upgrade_cost = get_upgrade_tower_cost(type, i);
			if (upgrade_cost > 0)
				total += upgrade_cost;
		}
		return total;
	}

	double get_remove_tower_refund(TowerType type, int level)
	{
		return get_tower_invested_cost(type, level) * 0.5;
	}

	double get_remove_tower_refund(Tower* tower)
	{
		if (!tower)
			return 0;
		return get_remove_tower_refund(tower->get_tower_type(), tower->get_level());
	}

	double get_remove_tower_refund(const SDL_Point& idx_tile)
	{
		return get_remove_tower_refund(find_tower(idx_tile));
	}

	double get_tower_damage_range(TowerType type)
	{
		static ConfigManager* config_ins = ConfigManager::instance();
		switch (type)
		{
		case Archer:
			return config_ins->archer_template.view_range[0];
			break;
		case Axeman:
			return config_ins->axeman_template.view_range[0];
			break;
		case Gunner:
			return config_ins->gunner_template.view_range[0];
			break;
		case Barracks:
			return config_ins->barracks_template.view_range[0];
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
		case Barracks:
			tower = new BarracksTower();
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
		tower->set_idx_tile(idx_pos);
		tower_list.push_back(tower);
		ConfigManager::instance()->map.place_tower(idx_pos);

		const static ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_PlaceTower)->second, 0);
		return;
    }

	bool upgrade_tower(const SDL_Point& idx_tile)
	{
		Tower* tower = find_tower(idx_tile);
		if (!tower || !tower->upgrade())
			return false;
		play_upgrade_sound();
		return true;
	}

	bool choose_specialization(const SDL_Point& idx_tile, TowerSpecialization spec)
	{
		Tower* tower = find_tower(idx_tile);
		if (!tower || !tower->choose_specialization(spec))
			return false;
		play_upgrade_sound();
		return true;
	}

	bool remove_tower(const SDL_Point& idx_tile)
	{
		for (auto iter = tower_list.begin(); iter != tower_list.end(); ++iter)
		{
			Tower* tower = *iter;
			const SDL_Point& tower_tile = tower->get_idx_tile();
			if (tower_tile.x != idx_tile.x || tower_tile.y != idx_tile.y)
				continue;

			ConfigManager::instance()->map.remove_tower(idx_tile);
			delete tower;
			tower_list.erase(iter);
			play_upgrade_sound();
			return true;
		}
		return false;
	}

	Tower* find_tower(const SDL_Point& idx_tile)
	{
		for (Tower* tower : tower_list)
		{
			const SDL_Point& tower_tile = tower->get_idx_tile();
			if (tower_tile.x == idx_tile.x && tower_tile.y == idx_tile.y)
				return tower;
		}
		return nullptr;
	}

	Tower* find_nearest_tower(const Vector2& position, double radius)
	{
		Tower* nearest = nullptr;
		double nearest_distance = radius;
		for (Tower* tower : tower_list)
		{
			double distance = (tower->get_position() - position).length();
			if (distance <= nearest_distance)
			{
				nearest = tower;
				nearest_distance = distance;
			}
		}
		return nearest;
	}

	void play_upgrade_sound()
	{
		const static ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_TowerLevelUp)->second, 0);
	}

	void clear()
	{
		for (Tower* tower : tower_list)
			delete tower;
		tower_list.clear();
		silencer_cooldowns.clear();
	}
private:
	void process_silencer_enemies(double delta)
	{
		EnemyManager::EnemyList& enemy_list = EnemyManager::instance()->get_enemy_list();
		for (Enemy* enemy : enemy_list)
		{
			if (enemy->get_enemy_type() != EnemyType::Silencer || enemy->can_remove())
				continue;

			double& cooldown = silencer_cooldowns[enemy];
			cooldown -= delta;
			if (cooldown > 0)
				continue;

			Tower* target = find_nearest_tower(enemy->get_position(), enemy->get_recover_radius());
			if (target)
				target->apply_silence(enemy->get_recover_intensity());
			cooldown = (std::max)(1.0, ConfigManager::instance()->silencer_template.recover_interval);
		}

		for (auto iter = silencer_cooldowns.begin(); iter != silencer_cooldowns.end();)
		{
			bool still_alive = false;
			for (Enemy* enemy : enemy_list)
				if (enemy == iter->first && !enemy->can_remove())
					still_alive = true;
			if (still_alive)
				++iter;
			else
				iter = silencer_cooldowns.erase(iter);
		}
	}

	TowerList tower_list;
	std::unordered_map<Enemy*, double> silencer_cooldowns;
};

