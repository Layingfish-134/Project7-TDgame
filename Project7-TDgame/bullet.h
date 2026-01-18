#pragma once
#include"Vector2.h"
#include"animation.h"
#include"config_manager.h"
#include"enemy.h"

class Bullet
{
public:
	Bullet() = default;
	~Bullet() = default;

public:
	void set_velocity(Vector2 val)
	{
		velocity = val;
		if (can_rotated)
		{
			double randian = std::atan2(velocity.y, velocity.x);
			angle_animation = randian * 180.0 / 3.14159265;

		}
	}

	void set_position(Vector2 pos)
	{
		this->position = pos;
	}


	void set_damage(double val)
	{
		damage = val;
	}

	void set_invaild()
	{
		is_vaild = false;
		is_collisional = false;
	}

	void disable_collided()
	{
		is_collisional = false;
	}

public:
	const Vector2& get_size()
	{
		return size;
	}

	const Vector2& get_position()
	{
		return position;
	}

	double get_damage() const
	{
		return damage;
	}

	double get_damage_range() const
	{
		return damage_range;
	}

	bool can_collided() const
	{
		return is_collisional;
	}

	bool can_remove() const
	{
		return !is_vaild;
	}

public:
	virtual void on_update(double delta)
	{
		animation.on_update(delta);

		position += velocity * delta;

		//判定子弹失效
		static const SDL_Rect& rect_map = ConfigManager::instance()->rect_tile_map;
		if (position.x - size.x / 2 >= rect_map.x + rect_map.w || position.x + size.x / 2 <= rect_map.x
			|| position.y - size.y / 2 >= rect_map.y + rect_map.h || position.y + size.y / 2 <= rect_map.y)
		{
			this->set_invaild();
		}

	}

	virtual void on_render(SDL_Renderer* renderer)
	{
		static SDL_Point point_anim;

		point_anim.x = (int)(position.x - size.x / 2);
		point_anim.y = (int)(position.y - size.y / 2);

		animation.on_render(renderer,point_anim,angle_animation);
	}


	virtual void on_collide(Enemy* enemy)
	{
		is_collisional = false;
		is_vaild = false; // ?
	}
protected:
	Vector2 velocity;
	Vector2 position;
	Vector2 size;

	Animation animation;
	bool can_rotated = false;


	double damage = 0;
	double damage_range = -1;
private:
	bool is_collisional = true;
	bool is_vaild = true;
	double angle_animation = 0;
};

