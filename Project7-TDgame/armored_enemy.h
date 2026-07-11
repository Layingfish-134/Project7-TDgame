#pragma once
#include"enemy.h"
#include"config_manager.h"
#include"resources_manager.h"

class ArmoredEnemy : public Enemy
{
public:
	ArmoredEnemy()
	{
		ConfigManager::EnemyTemplate& tpl = ConfigManager::instance()->armored_template;
		SDL_Texture* tex_walk = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_ArmoredWalk)->second;
		SDL_Texture* tex_hit = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_ArmoredHit)->second;

		std::vector<int> idx_list_up = { 6, 7, 8, 9, 10, 11 };
		std::vector<int> idx_list_down = { 0, 1, 2, 3, 4, 5 };
		std::vector<int> idx_list_right = { 12, 13, 14, 15, 16, 17 };
		std::vector<int> idx_list_left = { 18, 19, 20, 21, 22, 23 };

		animation_up.set_interval(0.14f);
		animation_up.set_loop(true);
		animation_up.set_frame_data(tex_walk, 6, 4, idx_list_up);
		animation_down.set_interval(0.14f);
		animation_down.set_loop(true);
		animation_down.set_frame_data(tex_walk, 6, 4, idx_list_down);
		animation_right.set_interval(0.14f);
		animation_right.set_loop(true);
		animation_right.set_frame_data(tex_walk, 6, 4, idx_list_right);
		animation_left.set_interval(0.14f);
		animation_left.set_loop(true);
		animation_left.set_frame_data(tex_walk, 6, 4, idx_list_left);

		animation_sketch_up.set_interval(0.14f);
		animation_sketch_up.set_loop(true);
		animation_sketch_up.set_frame_data(tex_hit, 6, 4, idx_list_up);
		animation_sketch_down.set_interval(0.14f);
		animation_sketch_down.set_loop(true);
		animation_sketch_down.set_frame_data(tex_hit, 6, 4, idx_list_down);
		animation_sketch_right.set_interval(0.14f);
		animation_sketch_right.set_loop(true);
		animation_sketch_right.set_frame_data(tex_hit, 6, 4, idx_list_right);
		animation_sketch_left.set_interval(0.14f);
		animation_sketch_left.set_loop(true);
		animation_sketch_left.set_frame_data(tex_hit, 6, 4, idx_list_left);

		max_hp = tpl.hp;
		max_speed = tpl.speed;
		damage = tpl.damage;
		attack_damage = tpl.attack_damage;
		attack_range = tpl.attack_range;
		recover_intensity = tpl.recover_intensity;
		recover_interval = tpl.recover_interval;
		recover_range = tpl.recover_range;
		reward_ratio = tpl.reward_ratio;
		hp = max_hp;
		speed = max_speed;
		size_anim.x = 48;
		size_anim.y = 48;
		set_armor(0.45);
	}
};
