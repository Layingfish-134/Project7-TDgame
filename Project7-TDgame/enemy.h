#pragma once
#include "timer.h"
#include "route.h"
#include "Vector2.h"
#include "animation.h"
#include "config_manager.h"
#include "enemy_type.h"
#include "resources_manager.h"

#include <algorithm>
#include <functional>

class Enemy
{
public:
	typedef std::function<void(Enemy* enemy)> SkillCallback;

public:
	Enemy()
	{
		this->set_on_skill_released([&](Enemy* enemy_src)
			{
				return;
			});

		timer_skill.set_one_shot(false);
		timer_skill.set_on_timeout([&]() { on_skill_released(this); });

		timer_sketch.set_one_shot(true);
		timer_sketch.set_wait_time(0.075);
		timer_sketch.set_on_timeout([&]() { is_show_sketch = false; });

		timer_restore_speed.set_one_shot(true);
		timer_restore_speed.set_on_timeout([&]() { speed = max_speed; });

		timer_intercept.set_one_shot(true);
		timer_intercept.set_on_timeout([&]() { intercepted = false; });
	}

	virtual ~Enemy() = default;

	virtual void on_update(double delta)
	{
		timer_skill.on_update(delta);
		timer_sketch.on_update(delta);
		timer_restore_speed.on_update(delta);
		timer_intercept.on_update(delta);

		Vector2 move_distance = velocity * delta;
		Vector2 target_distance = position_target - position;
		position += move_distance < target_distance ? move_distance : target_distance;

		if (target_distance.approx_zero())
		{
			idx_target++;
			refresh_position_target();

			direction = (position_target - position).normalize();
		}

		velocity.x = direction.x * speed * SIZE_TILE;
		velocity.y = direction.y * speed * SIZE_TILE;

		bool is_show_x_amin = abs(velocity.x) >= abs(velocity.y);

		if (is_show_sketch)
		{
			if (is_show_x_amin)
				anim_current = velocity.x > 0 ? &animation_sketch_right : &animation_sketch_left;
			else
				anim_current = velocity.y > 0 ? &animation_sketch_down : &animation_sketch_up;
		}
		else
		{
			if (is_show_x_amin)
				anim_current = velocity.x > 0 ? &animation_right : &animation_left;
			else
				anim_current = velocity.y > 0 ? &animation_down : &animation_up;
		}

		anim_current->on_update(delta);
	}

	virtual void on_render(SDL_Renderer* renderer)
	{
		static SDL_Rect rect;
		static SDL_Point point;
		static const int offset_y = 2;
		static const Vector2 size_hp_bar = { 40, 8 };
		static const SDL_Color color_border = { 116, 185, 124, 255 };
		static const SDL_Color color_content = { 226, 255, 194, 255 };

		point.x = (int)(position.x - size_anim.x / 2);
		point.y = (int)(position.y - size_anim.y / 2);

		anim_current->on_render(renderer, point);

		if (hp < max_hp)
		{
			rect.x = (int)(position.x - size_hp_bar.x / 2);
			rect.y = (int)(position.y - size_anim.y / 2 - size_hp_bar.y - offset_y);
			rect.w = (int)(size_hp_bar.x * (hp / max_hp));
			rect.h = (int)size_hp_bar.y;
			SDL_SetRenderDrawColor(renderer, color_content.r, color_content.g, color_content.b, color_content.a);
			SDL_RenderFillRect(renderer, &rect);

			rect.w = (int)size_hp_bar.x;
			SDL_SetRenderDrawColor(renderer, color_border.r, color_border.g, color_border.b, color_border.a);
			SDL_RenderDrawRect(renderer, &rect);
		}

		render_status_icons(renderer);
	}

	void set_on_skill_released(SkillCallback on_skill_released)
	{
		this->on_skill_released = on_skill_released;
	}

	void increase_hp(double val)
	{
		hp += val;

		if (hp > max_hp)
			hp = max_hp;
	}

	void decrease_hp(double val, bool armor_break = false)
	{
		double final_damage = val;
		if (armor > 0 && !armor_break)
			final_damage *= (1.0 - armor);

		hp -= final_damage;

		if (hp <= 0)
		{
			hp = 0;
			is_valid = false;
		}

		is_show_sketch = true;
		timer_sketch.restart();
	}

	void slow_down(double factor = 0.5, double duration = 1)
	{
		speed = (std::max)(0.05, max_speed * factor);
		timer_restore_speed.set_wait_time(duration);
		timer_restore_speed.restart();
	}

	void apply_intercept(double duration)
	{
		intercepted = true;
		slow_down(0.12, duration);
		timer_intercept.set_wait_time(duration);
		timer_intercept.restart();
	}

	void set_armor(double val)
	{
		armor = (std::max)(0.0, (std::min)(val, 0.9));
	}

	void set_position(const Vector2& position)
	{
		this->position = position;
	}

	void set_route(const Route* route)
	{
		this->route = route;

		refresh_position_target();
	}

	void set_invaild()
	{
		is_valid = false;
	}

	void set_enemy_type(EnemyType type)
	{
		enemy_type = type;
	}

	double get_hp() const
	{
		return hp;
	}

	double get_max_hp() const
	{
		return max_hp;
	}

	bool is_showing_sketch() const
	{
		return is_show_sketch;
	}

	bool is_armored() const
	{
		return armor > 0;
	}

	bool is_intercepted() const
	{
		return intercepted;
	}

	EnemyType get_enemy_type() const
	{
		return enemy_type;
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

	double get_attack_damage() const
	{
		return attack_damage;
	}

	double get_attack_range() const
	{
		return SIZE_TILE * attack_range;
	}

	double get_reward_ratio() const
	{
		return reward_ratio;
	}

	double get_recover_radius() const
	{
		return SIZE_TILE * recover_range;
	}

	double get_recover_intensity() const
	{
		return recover_intensity;
	}

	bool can_remove() const
	{
		return !is_valid;
	}

	double get_route_process() const
	{
		if (route->get_idx_list().size() == 1)
			return 1;

		return (double)idx_target / (route->get_idx_list().size() - 1);
	}

protected:
	Vector2 size_anim;

	Timer timer_skill;

	Animation animation_up;
	Animation animation_down;
	Animation animation_left;
	Animation animation_right;
	Animation animation_sketch_down;
	Animation animation_sketch_up;
	Animation animation_sketch_left;
	Animation animation_sketch_right;

	double hp = 0;
	double max_hp = 0;
	double speed = 0;
	double max_speed = 0;
	double damage = 0;
	double attack_damage = 0;
	double attack_range = 0;
	double reward_ratio = 0;
	double recover_interval = 0;
	double recover_range = 0;
	double recover_intensity = 0;
	EnemyType enemy_type = EnemyType::Silm;

private:
	Vector2 position;
	Vector2 velocity;
	Vector2 direction;

	bool is_valid = true;

	Timer timer_sketch;
	bool is_show_sketch = false;

	Animation* anim_current = nullptr;

	SkillCallback on_skill_released;

	Timer timer_restore_speed;
	Timer timer_intercept;
	bool intercepted = false;
	double armor = 0;

	const Route* route = nullptr;
	int idx_target = 0;
	Vector2 position_target;

private:
	void refresh_position_target()
	{
		const Route::IdxList& idx_list = route->get_idx_list();

		if (idx_target < idx_list.size())
		{
			const SDL_Point& point = idx_list[idx_target];
			static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

			position_target.x = rect_tile_map.x + point.x * SIZE_TILE + SIZE_TILE / 2;
			position_target.y = rect_tile_map.y + point.y * SIZE_TILE + SIZE_TILE / 2;
		}
	}

	void render_status_icons(SDL_Renderer* renderer)
	{
		const ResourcesManager::TexturePool& tex_pool = ResourcesManager::instance()->get_texture_pool();
		int icon_x = (int)(position.x - 20);
		int icon_y = (int)(position.y - size_anim.y / 2 - 12);

		if (armor > 0)
			render_status_icon(renderer, tex_pool.find(ResID::Tex_UIStatusArmor)->second, icon_x, icon_y);
		if (intercepted)
			render_status_icon(renderer, tex_pool.find(ResID::Tex_UIStatusIntercept)->second, icon_x + 18, icon_y);
	}

	void render_status_icon(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y)
	{
		SDL_Rect dst = { x, y, 16, 16 };
		SDL_RenderCopy(renderer, texture, nullptr, &dst);
	}

};
