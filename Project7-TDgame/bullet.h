#pragma once
#include"Vector2.h"
#include"animation.h"
#include"config_manager.h"
#include"enemy.h"
#include"bullet_type.h"

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

	void set_armor_break(bool val)
	{
		armor_break = val;
	}

	void set_slow_factor(double val)
	{
		slow_factor = val;
	}

	void set_slow_duration(double val)
	{
		slow_duration = val;
	}

	void set_damage_range(double val)
	{
		damage_range = val;
	}

	void set_invaild()
	{
		is_vaild = false;
		is_collisional = false;
	}

	void set_bullet_type(BulletType type)
	{
		bullet_type = type;
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

	const Vector2& get_velocity() const
	{
		return velocity;
	}

	BulletType get_bullet_type() const
	{
		return bullet_type;
	}

	double get_damage() const
	{
		return damage;
	}

	double get_damage_range() const
	{
		return damage_range;
	}

	bool can_break_armor() const
	{
		return armor_break;
	}

	double get_slow_factor() const
	{
		return slow_factor;
	}

	double get_slow_duration() const
	{
		return slow_duration;
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
		//std::cout << "子弹基类渲染" << std::endl;
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
	BulletType bullet_type = BulletType::Arrow;
	bool armor_break = false;
	double slow_factor = 1.0;
	double slow_duration = 0;
private:
	bool is_collisional = true;
	bool is_vaild = true;
	double angle_animation = 0;
};

