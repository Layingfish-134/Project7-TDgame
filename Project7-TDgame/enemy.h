#pragma once
#include"Vector2.h"
#include"timer.h"
#include"animation.h"
#include"Route.h"
#include"config_manager.h"


#include<functional>

class Enemy
{
public:
	typedef std::function<void(Enemy*)> SkillCallback;
public:
	Enemy()
	{
		timer_skill.set_one_shot(false);
		timer_skill.set_on_timeout(
			[&]()
			{
				on_skiil_released(this);
			}
		);

		timer_sketch.set_one_shot(true);
		timer_sketch.set_wait_time(0.075f);
		timer_sketch.set_on_timeout(
			[&]()
			{
				is_show_sketch = false;
			}
		);

		timer_restore_speed.set_one_shot(true);
		timer_restore_speed.set_on_timeout(
			[&]()
			{
				speed = max_speed;
			}
		);

	}
	~Enemy() = default;

public:
	void on_update(double delta)
	{
		timer_skill.on_update(delta);
		timer_sketch.on_update(delta);
		timer_restore_speed.on_update(delta);

		//计时器更新

		Vector2 target_distance = position_target - position;
		Vector2 move_distance = velocity * delta;
		position += std::min(target_distance, move_distance);
		if (target_distance.approx_zero())
		{
			idx_target++;
			refresh_position_target();

			direction = (position_target - position).normalize();
		}

		velocity.x = direction.x * speed * SIZE_TILE;
		velocity.y = direction.y * speed * SIZE_TILE;
		//speed本身的单位是单元格，此处需要转化为坐标 
		
		//位置更新

		bool is_show_x_anim = abs(velocity.x) >= abs(velocity.y) ? true : false;
		if (is_show_sketch)
		{
			if (is_show_x_anim)
				animation_current = velocity.x > 0 ? &animation_sketch_right : &animation_sketch_left;
			else
				animation_current = velocity.y > 0 ? &animation_sketch_down : &animation_sketch_up;
		}
		else
		{
			if (is_show_x_anim)
				animation_current = velocity.x > 0 ? &animation_right : &animation_left;
			else
				animation_current = velocity.y > 0 ? &animation_down : &animation_up;
		}

		animation_current->on_update(delta);
		//动画更新
	}

	void on_renderer(SDL_Renderer* renderer)
	{
		
		static SDL_Point point_anim;
		point_anim.x = position.x - size_anim.x / 2;
		point_anim.y = position.y - size_anim.y / 2;

		animation_current->on_render(renderer, point_anim);

		//绘制当前动画
		//除了当前动画之外还需要绘制血条

		static SDL_Rect rect_hp_content;
		static SDL_Rect rect_hp_border;

		static const int offset_hp_bar_y = 2;
		static const Vector2 size_hp_bar = { 40,8 };

		static const SDL_Color color_hp_border = {116,185,124,255};
		static const SDL_Color color_hp_content = {226,255,194,255};

		if (hp < max_hp)
		{
			rect_hp_content.x = (int)position.x - size_hp_bar.x / 2;
			rect_hp_content.y = (int)position.y - size_anim.y / 2 - offset_hp_bar_y;
			rect_hp_content.w = (int)size_hp_bar.x * (hp / max_hp);
			rect_hp_content.h = (int)size_hp_bar.y;

			rect_hp_border = rect_hp_content;
			rect_hp_border.w = size_hp_bar.x;

			SDL_SetRenderDrawColor(renderer, color_hp_content.r, color_hp_content.g, color_hp_content.b, color_hp_content.a);
			SDL_RenderFillRect(renderer, &rect_hp_content);

			SDL_SetRenderDrawColor(renderer, color_hp_border.r, color_hp_border.g, color_hp_border.b, color_hp_border.a);
			SDL_RenderDrawRect(renderer, &rect_hp_border);
		}

	}
public:
	void set_on_skill_released(SkillCallback on_skill_released)
	{
		this->on_skiil_released = on_skill_released;
	}

	void increase_hp(int val)
	{
		hp = std::min(hp + val, max_hp);
	}

	void decrease_hp(int val)
	{
		hp -= val;
		if (hp <= 0)
		{
			hp = 0;
			is_vaild = false;
		}

		is_show_sketch = true;
		timer_sketch.restart();
	}

	void slow_down()
	{
		speed = max_speed - 0.5;
		timer_restore_speed.set_wait_time(1.0);
		timer_restore_speed.restart();
	}

	void set_position(const Vector2 positon)
	{
		this->position = positon;
	}

	void set_route(const Route* route)
	{
		this->route = route;

		refresh_position_target();
	}

	void set_invaild()
	{
		is_vaild = false;
	}

public:
	int get_hp() const
	{
		return hp;
	}

	const Vector2& get_size() const
	{
		return size_anim;
	}

	const Vector2& get_position() const
	{
		return position;
	}

	const Vector2& get_velocity() const
	{
		return velocity;
	}

	double get_damage() const
	{
		return damage;
	}

	double get_reward_ratio() const
	{
		return reward_ratio;
	}

	double get_recover_radius() const
	{
		return recover_range * SIZE_TILE;
	}

	double get_recover_intensity() const
	{
		return recover_intensity;
	}

	bool can_move() const
	{
		return !is_vaild;
	}

	double get_route_process() const
	{
		if (route->get_route().size() == 1)
			return 1;

		return (double)(idx_target) / (route->get_route().size() - 1);
	}
protected:
	Vector2 size_anim;

	Timer timer_skill;

	Animation animation_up;
	Animation animation_down;
	Animation animation_left;
	Animation animation_right;
	Animation animation_sketch_up;
	Animation animation_sketch_down;
	Animation animation_sketch_left;
	Animation animation_sketch_right;

	int hp = 0;
	int max_hp = 0;
	double speed = 0;
	double max_speed = 0;
	double damage = 0;
	double reward_ratio = 0;
	double recover_interval = 0;
	double recover_intensity = 0;
	double recover_range = 0;

private:
	Vector2 position;
	Vector2 velocity;
	Vector2 direction;

	bool is_vaild = true;

	Animation* animation_current = nullptr;

	bool is_show_sketch = false;
	Timer timer_sketch;

	SkillCallback on_skiil_released;

	Timer timer_restore_speed;

	const Route* route = nullptr;//同类型的怪物共用同一个route，选择指针（或者引用）
	int idx_target = 0;
	Vector2 position_target;

private:
	void refresh_position_target()
	{
		const Route::IdxList& route_idx_list = route->get_route();

		if (idx_target < route_idx_list.size())
		{
			const SDL_Point& target_point = route_idx_list[idx_target];
			//目标位置在瓦片地图数组上的下标
			static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

			position_target.x += rect_tile_map.x + target_point.x * SIZE_TILE + SIZE_TILE / 2;
			position_target.y += rect_tile_map.y + target_point.y * SIZE_TILE + SIZE_TILE / 2;
		}
	}
};