#pragma once
#include "enemy.h"
#include "config_manager.h"
#include "resources_manager.h"

#include <functional>

class BossEnemy : public Enemy
{
public:
	typedef std::function<void(Enemy* enemy)> SummonCallback;

	BossEnemy()
	{
		ConfigManager::EnemyTemplate& tpl = ConfigManager::instance()->king_slim_template;
		SDL_Texture* tex_boss = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Boss)->second;
		SDL_Texture* tex_boss_sketch = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_BossSketch)->second;

		std::vector<int> idx_list_up = { 6, 7, 8, 9, 10, 11 };
		std::vector<int> idx_list_down = { 0, 1, 2, 3, 4, 5 };
		std::vector<int> idx_list_right = { 12, 13, 14, 15, 16, 17 };
		std::vector<int> idx_list_left = { 18, 19, 20, 21, 22, 23 };

		animation_up.set_interval(0.12f);
		animation_up.set_loop(true);
		animation_up.set_frame_data(tex_boss, 6, 4, idx_list_up);
		animation_down.set_interval(0.12f);
		animation_down.set_loop(true);
		animation_down.set_frame_data(tex_boss, 6, 4, idx_list_down);
		animation_right.set_interval(0.12f);
		animation_right.set_loop(true);
		animation_right.set_frame_data(tex_boss, 6, 4, idx_list_right);
		animation_left.set_interval(0.12f);
		animation_left.set_loop(true);
		animation_left.set_frame_data(tex_boss, 6, 4, idx_list_left);

		animation_sketch_up.set_interval(0.12f);
		animation_sketch_up.set_loop(true);
		animation_sketch_up.set_frame_data(tex_boss_sketch, 6, 4, idx_list_up);
		animation_sketch_down.set_interval(0.12f);
		animation_sketch_down.set_loop(true);
		animation_sketch_down.set_frame_data(tex_boss_sketch, 6, 4, idx_list_down);
		animation_sketch_right.set_interval(0.12f);
		animation_sketch_right.set_loop(true);
		animation_sketch_right.set_frame_data(tex_boss_sketch, 6, 4, idx_list_right);
		animation_sketch_left.set_interval(0.12f);
		animation_sketch_left.set_loop(true);
		animation_sketch_left.set_frame_data(tex_boss_sketch, 6, 4, idx_list_left);

		max_hp = tpl.hp * 2.4;
		max_speed = tpl.speed * 0.75;
		damage = tpl.damage * 2.0;
		attack_damage = tpl.attack_damage * 2.0;
		attack_range = tpl.attack_range * 1.2;
		recover_intensity = tpl.recover_intensity;
		recover_interval = tpl.recover_interval;
		recover_range = tpl.recover_range;
		reward_ratio = 1.0;

		hp = max_hp;
		speed = max_speed;
		size_anim.x = 72;
		size_anim.y = 72;
	}
	~BossEnemy() = default;

	void on_update(double delta) override
	{
		Enemy::on_update(delta);
		if (can_remove())
			return;

		if (!stage_heal_used && get_hp() <= get_max_hp() * 0.5)
		{
			increase_hp(get_max_hp() * 0.18);
			stage_heal_used = true;
		}

		summon_elapsed += delta;
		if (summon_elapsed >= summon_interval)
		{
			summon_elapsed = 0;
			if (on_summon_requested)
				on_summon_requested(this);
		}
	}

	void set_on_summon_requested(SummonCallback callback)
	{
		on_summon_requested = callback;
	}

private:
	bool stage_heal_used = false;
	double summon_elapsed = 0;
	double summon_interval = 8.0;
	SummonCallback on_summon_requested;
};
