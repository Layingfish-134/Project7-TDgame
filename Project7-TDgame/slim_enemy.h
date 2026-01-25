#pragma once
#include"enemy.h"
#include"config_manager.h"
#include"resources_manager.h"

class SlimEnemy : public Enemy
{
public:
	SlimEnemy()
	{
		//得到配置所需要的资源
		ConfigManager::EnemyTemplate& slim_template = ConfigManager::instance()->slim_template;
		SDL_Texture* tex_slim = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Slime)->second;
		SDL_Texture* tex_slim_sketch = ResourcesManager::instance()->get_texture_pool().
			find(ResID::Tex_SlimeSketch)->second;

		//加载动画
		std::vector<int> idx_list_up = { 6, 7, 8, 9, 10, 11 };
		std::vector<int> idx_list_down = { 0, 1, 2, 3, 4, 5 };
		std::vector<int> idx_list_right = { 12, 13, 14, 15, 16, 17 };
		std::vector<int> idx_list_left = { 18, 19, 20, 21, 22, 23 };

		animation_up.set_interval(0.1f);
		animation_up.set_loop(true);
		animation_up.set_frame_data(tex_slim, 6, 4, idx_list_up);

		animation_down.set_interval(0.1f);
		animation_down.set_loop(true);
		animation_down.set_frame_data(tex_slim, 6, 4, idx_list_down);

		animation_right.set_interval(0.1f);
		animation_right.set_loop(true);
		animation_right.set_frame_data(tex_slim, 6, 4, idx_list_right);

		animation_left.set_interval(0.1f);
		animation_left.set_loop(true);
		animation_left.set_frame_data(tex_slim, 6, 4, idx_list_left);


		animation_sketch_up.set_interval(0.1f);
		animation_sketch_up.set_loop(true);
		animation_sketch_up.set_frame_data(tex_slim_sketch, 6, 4, idx_list_up);

		animation_sketch_down.set_interval(0.1f);
		animation_sketch_down.set_loop(true);
		animation_sketch_down.set_frame_data(tex_slim_sketch, 6, 4, idx_list_down);
		
		animation_sketch_right.set_interval(0.1f);
		animation_sketch_right.set_loop(true);
		animation_sketch_right.set_frame_data(tex_slim_sketch, 6, 4, idx_list_right);
		
		animation_sketch_left.set_interval(0.1f);
		animation_sketch_left.set_loop(true);
		animation_sketch_left.set_frame_data(tex_slim_sketch, 6, 4, idx_list_left);

		//加载数值
		max_hp = slim_template.hp;
		max_speed = slim_template.speed;
		damage = slim_template.damage;
		recover_intensity = slim_template.recover_intensity;
		recover_interval = slim_template.recover_interval;
		recover_range = slim_template.recover_range;
		reward_ratio = slim_template.reward_ratio;
		//reward_ratio = 1;

		hp = max_hp; speed = max_speed;
		size_anim.x = 48;
		size_anim.y = 48;

	}
	~SlimEnemy() = default;
};

