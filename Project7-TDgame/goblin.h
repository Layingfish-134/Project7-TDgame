#pragma once
#include"enemy.h"
#include"config_manager.h"
#include"resources_manager.h"

class Goblin : public Enemy
{
public:
	Goblin() 
	{
		ConfigManager::EnemyTemplate& goblin_template = ConfigManager::instance()->goblin_template;
		SDL_Texture* tex_goblin = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Goblin)->second;
		SDL_Texture* tex_goblin_sketch = ResourcesManager::instance()->get_texture_pool().
			find(ResID::Tex_GoblinSketch)->second;

		//加载动画
		std::vector<int> idx_list_up = { 0,1,2,3,4,5 };
		std::vector<int> idx_list_down = { 6,7,8,9,10,11 };
		std::vector<int> idx_list_right = { 12,13,14,15,16,17 };
		std::vector<int> idx_list_left = { 18,19,20,21,22,23 };

		animation_up.set_interval(0.1f);
		animation_up.set_loop(true);
		animation_up.set_frame_data(tex_goblin, 6, 4, idx_list_up);

		animation_down.set_interval(0.1f);
		animation_down.set_loop(true);
		animation_down.set_frame_data(tex_goblin, 6, 4, idx_list_down);

		animation_right.set_interval(0.1f);
		animation_right.set_loop(true);
		animation_right.set_frame_data(tex_goblin, 6, 4, idx_list_right);

		animation_left.set_interval(0.1f);
		animation_left.set_loop(true);
		animation_left.set_frame_data(tex_goblin, 6, 4, idx_list_left);


		animation_sketch_up.set_interval(0.1f);
		animation_sketch_up.set_loop(true);
		animation_sketch_up.set_frame_data(tex_goblin_sketch, 6, 4, idx_list_up);

		animation_sketch_down.set_interval(0.1f);
		animation_sketch_down.set_loop(true);
		animation_sketch_down.set_frame_data(tex_goblin_sketch, 6, 4, idx_list_down);

		animation_sketch_right.set_interval(0.1f);
		animation_sketch_right.set_loop(true);
		animation_sketch_right.set_frame_data(tex_goblin_sketch, 6, 4, idx_list_right);

		animation_sketch_left.set_interval(0.1f);
		animation_sketch_left.set_loop(true);
		animation_sketch_left.set_frame_data(tex_goblin_sketch, 6, 4, idx_list_left);

		//加载数值
		max_hp = goblin_template.hp;
		max_speed = goblin_template.speed;
		damage = goblin_template.damage;
		recover_intensity = goblin_template.recover_intensity;
		recover_interval = goblin_template.recover_interval;
		recover_range = goblin_template.recover_range;
		reward_ratio = goblin_template.reward_ratio;

		hp = max_hp; speed = max_speed;
		size_anim.x = 48;
		size_anim.y = 48;
	}
	~Goblin() = default;

private:

};

