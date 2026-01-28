#include"enemy.h"
#include"config_manager.h"
#include"resources_manager.h"

class GoblinPriest : public Enemy
{
public:
	GoblinPriest()
	{
		ConfigManager::EnemyTemplate& goblin_priest_template = ConfigManager::instance()->goblin_priest_template;
		SDL_Texture* tex_goblin_priest = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_GoblinPriest)->second;
		SDL_Texture* tex_goblin_priest_sketch = ResourcesManager::instance()->get_texture_pool().
			find(ResID::Tex_GoblinPriestSketch)->second;

		//加载动画
		std::vector<int> idx_list_up = { 5, 6, 7, 8, 9 };
		std::vector<int> idx_list_down = { 0, 1, 2, 3, 4 };
		std::vector<int> idx_list_right = { 10, 11, 12, 13, 14 };
		std::vector<int> idx_list_left = { 15, 16, 17, 18, 19 };

		animation_up.set_interval(0.15f);
		animation_up.set_loop(true);
		animation_up.set_frame_data(tex_goblin_priest, 5, 4, idx_list_up);

		animation_down.set_interval(0.15f);
		animation_down.set_loop(true);
		animation_down.set_frame_data(tex_goblin_priest, 5, 4, idx_list_down);

		animation_right.set_interval(0.15f);
		animation_right.set_loop(true);
		animation_right.set_frame_data(tex_goblin_priest, 5, 4, idx_list_right);

		animation_left.set_interval(0.15f);
		animation_left.set_loop(true);
		animation_left.set_frame_data(tex_goblin_priest, 5, 4, idx_list_left);


		animation_sketch_up.set_interval(0.15f);
		animation_sketch_up.set_loop(true);
		animation_sketch_up.set_frame_data(tex_goblin_priest_sketch, 5, 4, idx_list_up);

		animation_sketch_down.set_interval(0.15f);
		animation_sketch_down.set_loop(true);
		animation_sketch_down.set_frame_data(tex_goblin_priest_sketch, 5, 4, idx_list_down);

		animation_sketch_right.set_interval(0.15f);
		animation_sketch_right.set_loop(true);
		animation_sketch_right.set_frame_data(tex_goblin_priest_sketch, 5, 4, idx_list_right);

		animation_sketch_left.set_interval(0.15f);
		animation_sketch_left.set_loop(true);
		animation_sketch_left.set_frame_data(tex_goblin_priest_sketch, 5, 4, idx_list_left);

		//加载数值
		max_hp = goblin_priest_template.hp;
		max_speed = goblin_priest_template.speed;
		damage = goblin_priest_template.damage;
		recover_intensity = goblin_priest_template.recover_intensity;
		recover_interval = goblin_priest_template.recover_interval;
		recover_range = goblin_priest_template.recover_range;
		reward_ratio = goblin_priest_template.reward_ratio;

		hp = max_hp; speed = max_speed;
		size_anim.x = 48;
		size_anim.y = 48;

		timer_skill.set_wait_time(recover_interval);
	}
	~GoblinPriest() = default;

private:

};

