#pragma once
#include"Vector2.h"
#include"animation.h"
#include"tower_type.h"
#include"tower_specialization.h"
#include"bullet_manager.h"
#include"enemy_manager.h"
#include"config_manager.h"
#include"timer.h"
#include"facing.h"

#include <algorithm>

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

		timer_silence.set_one_shot(true);
		timer_silence.set_on_timeout([&]()
			{
				silenced = false;
			});

		anim_idle_up.set_loop(true);
		anim_idle_up.set_interval(0.2);
		anim_idle_down.set_loop(true);
		anim_idle_down.set_interval(0.2);
		anim_idle_left.set_loop(true);
		anim_idle_left.set_interval(0.2);
		anim_idle_right.set_loop(true);
		anim_idle_right.set_interval(0.2);

		anim_fire_up.set_loop(false);
		anim_fire_up.set_interval(0.2);
		anim_fire_up.set_on_finished([&]()
			{
				update_anim_idle();
			});

		anim_fire_down.set_loop(false);
		anim_fire_down.set_interval(0.2);
		anim_fire_down.set_on_finished([&]()
			{
				update_anim_idle();
			});

		anim_fire_left.set_loop(false);
		anim_fire_left.set_interval(0.2);
		anim_fire_left.set_on_finished([&]()
			{
				update_anim_idle();
			});

		anim_fire_right.set_loop(false);
		anim_fire_right.set_interval(0.2);
		anim_fire_right.set_on_finished([&]()
			{
				update_anim_idle();
			});

	}
	virtual ~Tower() = default;
public:
	void set_position(Vector2 pos)
	{
		position = pos;
		return;
	}

	const Vector2& get_size() const
	{
		return size;
	}

	const Vector2& get_position() const
	{
		return position;
	}

	TowerType get_tower_type() const
	{
		return tower_type;
	}

	int get_level() const
	{
		return level;
	}

	TowerSpecialization get_specialization() const
	{
		return specialization;
	}

	bool has_specialization() const
	{
		return specialization != TowerSpecialization::None;
	}

	bool is_silenced() const
	{
		return silenced;
	}

	const SDL_Point& get_idx_tile() const
	{
		return idx_tile;
	}

	Facing get_facing() const
	{
		return dir_face;
	}

	void set_idx_tile(const SDL_Point& idx)
	{
		idx_tile = idx;
	}

	void apply_silence(double duration)
	{
		silenced = true;
		timer_silence.set_wait_time(duration);
		timer_silence.restart();
	}

	bool upgrade()
	{
		if (level >= max_level)
			return false;
		level++;
		return true;
	}

	bool choose_specialization(TowerSpecialization spec)
	{
		if (specialization != TowerSpecialization::None || level < specialization_level)
			return false;
		specialization = spec;
		return true;
	}

	bool can_choose_specialization() const
	{
		return level >= specialization_level && specialization == TowerSpecialization::None;
	}
public:
	virtual void on_fire()
	{
		if (silenced)
			return;

		Enemy* target_enemy = find_target_enemy();
		if (target_enemy == nullptr)
			return;

		can_fire_now = false;

		static ConfigManager* config_ins = ConfigManager::instance();
		const static ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();

		double interval = get_attack_interval();
		double damage = get_attack_damage();
		bool armor_break = can_break_armor();
		double slow_factor = get_bullet_slow_factor();
		double slow_duration = get_bullet_slow_duration();
		double damage_range = get_damage_range_override();
		switch (tower_type)
		{
		case Archer:
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
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_AxeFire)->second, 0);
			break;
		case Gunner:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ShellFire)->second, 0);
			break;
		case Barracks:
			break;
		}

		timer_fire.set_wait_time(interval);
		timer_fire.restart();

		Vector2 direction_fire = target_enemy->get_position() - position;
		BulletManager::instance()->fire_bullet(bullet_type, position, direction_fire.normalize() * fire_speed * SIZE_TILE,
			damage, armor_break, slow_factor, slow_duration, damage_range);

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
			view_range = config_ins->archer_template.view_range[level];
			break;
		case Axeman:
			view_range = config_ins->axeman_template.view_range[level];
			break;
		case Gunner:
			view_range = config_ins->gunner_template.view_range[level];
			break;
		case Barracks:
			view_range = config_ins->barracks_template.view_range[level];
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
	virtual void on_update(double delta)
	{
		timer_silence.on_update(delta);
		timer_fire.on_update(delta);
		anim_cur->on_update(delta);

		if (can_fire_now && !silenced)
		{
			on_fire();
		}
	}
	
	virtual void on_render(SDL_Renderer* renderer)
	{
		static SDL_Point point;
		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);

		anim_cur->on_render(renderer, point);
		render_status(renderer);

	}
protected:
	double get_attack_interval() const
	{
		const ConfigManager* config = ConfigManager::instance();
		double interval = 1;
		switch (tower_type)
		{
		case Archer: interval = config->archer_template.interval[level]; break;
		case Axeman: interval = config->axeman_template.interval[level]; break;
		case Gunner: interval = config->gunner_template.interval[level]; break;
		case Barracks: interval = config->barracks_template.interval[level]; break;
		}
		if (specialization == TowerSpecialization::ArcherRapidFire || specialization == TowerSpecialization::GunnerShotgun)
			interval *= 0.6;
		return (std::max)(0.15, interval);
	}

	double get_attack_damage() const
	{
		const ConfigManager* config = ConfigManager::instance();
		double damage = 1;
		switch (tower_type)
		{
		case Archer: damage = config->archer_template.damage[level]; break;
		case Axeman: damage = config->axeman_template.damage[level]; break;
		case Gunner: damage = config->gunner_template.damage[level]; break;
		case Barracks: damage = config->barracks_template.damage[level]; break;
		}
		if (specialization == TowerSpecialization::ArcherPiercingArmor)
			damage += 4;
		else if (specialization == TowerSpecialization::AxemanCleave)
			damage *= 1.15;
		else if (specialization == TowerSpecialization::GunnerShotgun)
			damage *= 0.75;
		return damage;
	}

	bool can_break_armor() const
	{
		return specialization == TowerSpecialization::ArcherPiercingArmor ||
			specialization == TowerSpecialization::AxemanCleave ||
			specialization == TowerSpecialization::GunnerBarrage;
	}

	double get_bullet_slow_factor() const
	{
		return specialization == TowerSpecialization::AxemanStrongSlow ? 0.25 : 0.5;
	}

	double get_bullet_slow_duration() const
	{
		return specialization == TowerSpecialization::AxemanStrongSlow ? 2.5 : 1.0;
	}

	double get_damage_range_override() const
	{
		if (specialization == TowerSpecialization::AxemanCleave)
			return 70;
		if (specialization == TowerSpecialization::GunnerBarrage)
			return 130;
		if (specialization == TowerSpecialization::GunnerShotgun)
			return 64;
		return -2;
	}
private:
	Timer timer_fire;
	Timer timer_silence;
	Vector2 position;
	bool can_fire_now = true;
	bool silenced = false;
	int level = 0;
	static const int max_level = 9;
	static const int specialization_level = 2;
	SDL_Point idx_tile = { -1, -1 };
	TowerSpecialization specialization = TowerSpecialization::None;

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

	void render_status(SDL_Renderer* renderer)
	{
		if (!silenced)
			return;

		SDL_Texture* texture = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_UIStatusSilence)->second;
		SDL_Rect dst =
		{
			(int)(position.x - 12),
			(int)(position.y - size.y / 2 - 18),
			24,
			24
		};
		SDL_RenderCopy(renderer, texture, nullptr, &dst);
	}
};
