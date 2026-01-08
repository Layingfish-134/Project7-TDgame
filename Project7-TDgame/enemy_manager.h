
#pragma once
#include"manager.h"
#include"enemy.h"
#include"config_manager.h"
#include"home_manager.h"

#include"slim_enemy.h"
#include"king_slim_enemy.h"
#include"skeleton.h"
#include"goblin.h"
#include"goblin_priest.h"


#include<vector>
#include<SDL.h>
#include<iostream>
class EnemyManager : public Manager<EnemyManager>
{
	friend class Manager<EnemyManager>;
public:
	typedef std::vector<Enemy*> EnemyList;
public:
	EnemyManager() = default;
	~EnemyManager()
	{
		for (Enemy* enemy : enemy_list)
			delete enemy;

	}

public:
	void on_update(double delta)
	{
		for (Enemy* enemy : enemy_list)
			enemy->on_update(delta);

		process_collision_home();
		process_collision_bulllet();

		remove_invaild_enemy();
	}

	void on_render(SDL_Renderer* renderer)
	{
		for (Enemy* enemy : enemy_list)
		{
			enemy->on_render(renderer);
			//std::cout << "绘制敌人" << std::endl;
		}
	}

public:
	void spawn_enemy(EnemyType enemy_type,int idx_spawn_point)
	{
		static Vector2 position;
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		static const Map::SpawnerRoutePool& spawner_route_pool = ConfigManager::instance()->map.get_idx_spawner_pool();
		//计算敌人的初始生成位置，需要得到地图的世界坐标rect_tile_map、怪物逻辑路径route

		const auto& iter = spawner_route_pool.find(idx_spawn_point);
		if (iter == spawner_route_pool.end())
			return;
		//防止无效的生成点位

		Enemy* enemy = nullptr;
		switch (enemy_type)
		{
		case EnemyType::Silm:
			enemy = new SlimEnemy();
			//std::cout << "敌人生成slim" << std::endl;
			break;
		case EnemyType::KingSilm:
			enemy = new KingSlimEnemy();
			//std::cout << "敌人生成kingslim" << std::endl;
			break;
		case EnemyType::Skeleton:
			enemy = new Skeleton();
			break;
		case EnemyType::Goblin:
			enemy = new Goblin();
			break;
		case EnemyType::GoblinPriest:
			enemy = new GoblinPriest();
			break;
		default:
			enemy = new SlimEnemy();
			break;
		}

		enemy->set_on_skill_released([&](Enemy* enemy_src)
			{
				double recover_radius = enemy_src->get_recover_radius();
				if (recover_radius < 0)
					return;

				const Vector2 position_src = enemy_src->get_position();
				for (Enemy* enemy_dst : enemy_list)
				{
					const Vector2 position_dst = enemy_dst->get_position();
					double distance = (position_dst - position_src).length();
					if (distance <= recover_radius)
						enemy_dst->increase_hp(enemy_src->get_recover_intensity());
				}
			});

		const Route::IdxList& route_idx_list = iter->second.get_idx_list();
		position.x = rect_tile_map.x + route_idx_list[0].x * SIZE_TILE + SIZE_TILE/2;
		position.y = rect_tile_map.y + route_idx_list[0].y * SIZE_TILE + SIZE_TILE/2;

		enemy->set_position(position);
		enemy->set_route(&iter->second);

		enemy_list.push_back(enemy);

	}

	bool check_enemy_all_killed()
	{
		return enemy_list.empty();
	}
private:
	void process_collision_home()
	{
		static const SDL_Point& point_home_idx = ConfigManager::instance()->map.get_home_idx();
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		static const Vector2 position_home =
		{
			(double)rect_tile_map.x + point_home_idx.x * SIZE_TILE,
			(double)rect_tile_map.y + point_home_idx.y * SIZE_TILE
		};

		for (Enemy* enemy : enemy_list)
		{
			if (enemy->can_remove()) continue;
			const Vector2& position_enemy = enemy->get_position();

			if (position_enemy.x >= position_home.x && position_enemy.x <= position_home.x + SIZE_TILE
				&& position_enemy.y >= position_home.y && position_enemy.y <= position_home.y + SIZE_TILE)
			{
				enemy->set_invaild();

				HomeManager::instance()->decrease_hp(enemy->get_damage());
			}
		}
		//计算房屋的世界坐标
	}

	void process_collision_bulllet()
	{

	}

	void remove_invaild_enemy()
	{
		enemy_list.erase(std::remove_if(enemy_list.begin(), enemy_list.end(), [](const Enemy* enemy)
			{
				bool can_delete = enemy->can_remove();
				if (can_delete)
				{
					delete enemy;
				}
				return can_delete;
			}
		), enemy_list.end());
		//erase本身从列表当中去除，remove if用于释放内存
	}
private:
	EnemyList enemy_list;
};









