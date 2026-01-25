#pragma once
#include"Vector2.h"
#include"animation.h"
#include"tower_type.h"
#include"bullet_manager.h"
#include"enemy_manager.h"
#include"config_manager.h"
#include"timer.h"
#include"facing.h"


class Tower
{
public:
	Tower()
	{
		timer_fire.set_one_shot(true);
		timer_fire.set_on_timeout([&]()
			{
				can_fire_now = true;

			});

		anim_idle_up.set_loop(true);
		anim_idle_up.set_interval(0.2);
		anim_idle_down.set_loop(true);
		anim_idle_down.set_interval(0.2);
		anim_idle_left.set_loop(true);
		anim_idle_left.set_interval(0.2);
		anim_idle_right.set_loop(true);
		anim_idle_right.set_interval(0.2);

		anim_fire_up.set_loop(true);
		anim_fire_up.set_interval(0.2);
		anim_fire_up.set_on_finished([&]()
			{
				update_anim_idle();
			});

		anim_fire_down.set_loop(true);
		anim_fire_down.set_interval(0.2);
		anim_fire_down.set_on_finished([&]()
			{
				update_anim_idle();
			});

		anim_fire_left.set_loop(true);
		anim_fire_left.set_interval(0.2);
		anim_fire_left.set_on_finished([&]()
			{
				update_anim_idle();
			});

		anim_fire_right.set_loop(true);
		anim_fire_right.set_interval(0.2);
		anim_fire_right.set_on_finished([&]()
			{
				update_anim_idle();
			});

	}
	~Tower() = default;
public:
	void set_position(Vector2 pos)
	{
		position = pos;
		return;
	}

	const Vector2& get_size()
	{
		return size;
	}

	const Vector2& get_position()
	{
		return position;
	}
public:
	void on_fire()
	{
		Enemy* target_enemy = find_target_enemy();
		if (target_enemy == nullptr)
			return;

		can_fire_now = false;

		static ConfigManager* config_ins = ConfigManager::instance();
		const static ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();

		double interval = 0, damage = 0;
		switch (tower_type)
		{
		case Archer:
			interval = config_ins->archer_template.interval[config_ins->level_archer];
			damage = config_ins->archer_template.damage[config_ins->level_archer];
			switch (rand()%2)
			{
			case 0:
				Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowFire_1)->second, 0);
				break;
			case 1:
				Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowFire_2)->second, 0);
				break;
			}
			break;
		case Axeman:
			interval = config_ins->axeman_template.interval[config_ins->level_axeman];
			damage = config_ins->axeman_template.damage[config_ins->level_axeman];

			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_AxeFire)->second, 0);
			break;
		case Gunner:
			interval = config_ins->gunner_template.interval[config_ins->level_gunner];
			damage = config_ins->gunner_template.damage[config_ins->level_gunner];

			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ShellFire)->second, 0);
			break;
		}

		timer_fire.set_wait_time(interval);
		timer_fire.restart();

		Vector2 direction_fire = target_enemy->get_position() - position;
		BulletManager::instance()->fire_bullet(bullet_type, position, direction_fire.normalize() * fire_speed * SIZE_TILE, damage);

		bool is_show_x_anim = abs(direction_fire.x) >= abs(direction_fire.y);
		if (is_show_x_anim)
		{
			dir_face = direction_fire.x > 0 ? Facing::Right : Facing::Left;
		}
		else
		{
			dir_face = direction_fire.y > 0 ? Facing::Down : Facing::Up;
		}

		update_animation_fire();
		anim_cur->reset();
	}

	Enemy* find_target_enemy()
	{
		double process_enemy = -1;
		double view_range = -1;
		static ConfigManager* config_ins = ConfigManager::instance();
		Enemy* target_enemy = nullptr;
		switch (tower_type)
		{
		case Archer:
			view_range = config_ins->archer_template.view_range[config_ins->level_archer];
			break;
		case Axeman:
			view_range = config_ins->axeman_template.view_range[config_ins->level_axeman];
			break;
		case Gunner:
			view_range = config_ins->gunner_template.view_range[config_ins->level_gunner];
			break;
		}

		EnemyManager::EnemyList& enemy_list = EnemyManager::instance()->get_enemy_list();
		for (Enemy* enemy : enemy_list)
		{
			double distance = (enemy->get_position() - position).length();
			if (distance < view_range * SIZE_TILE)
			{
				double process_now = enemy->get_route_process();
				if (process_now > process_enemy)
				{
					target_enemy = enemy;
					process_enemy = process_now;
				}
			}
		}

		return target_enemy;
	}
public:
	void on_update(double delta)
	{
		timer_fire.on_update(delta);
		anim_cur->on_update(delta);

		if (can_fire_now)
		{
			on_fire();
		}
	}
	
	void on_render(SDL_Renderer* renderer)
	{
		static SDL_Point point;
		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);

		anim_cur->on_render(renderer, point);

	}
private:
	Timer timer_fire;
	Vector2 position;
	bool can_fire_now = true;

	Facing dir_face = Facing::Right;
	Animation* anim_cur = &anim_idle_right;
protected:
	Vector2 size;

	Animation anim_idle_up;
	Animation anim_idle_down;
	Animation anim_idle_left;
	Animation anim_idle_right;

	Animation anim_fire_up;
	Animation anim_fire_down;
	Animation anim_fire_left;
	Animation anim_fire_right;

	TowerType tower_type = TowerType::Archer;

	double fire_speed = 0.5;
	BulletType bullet_type = BulletType::Arrow;

private:
	void update_anim_idle()
	{
		switch (dir_face)
		{
		case Up:
			anim_cur = &anim_idle_up;
			break;
		case Down:
			anim_cur = &anim_idle_down;
			break;
		case Left:
			anim_cur = &anim_idle_left;
			break;
		case Right:
			anim_cur = &anim_idle_right;
			break;
		default:
			anim_cur = &anim_idle_up;
			break;
		}
	}

	void update_animation_fire()
	{
		switch (dir_face)
		{
		case Left:
			anim_cur = &anim_fire_left;
			break;
		case Right:
			anim_cur = &anim_fire_right;
			break;
		case Up:
			anim_cur = &anim_fire_up;
			break;
		case Down:
			anim_cur = &anim_fire_down;
			break;
		}
	}
};
