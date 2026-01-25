#pragma once

#include"Vector2.h"
#include"timer.h"
#include"config_manager.h"
#include"resources_manager.h"


#include<SDL.h>
class CoinProp
{
public:
	CoinProp()
	{
		timer_jump.set_one_shot(true);
		timer_jump.set_wait_time(interval_jump);
		timer_jump.set_on_timeout([&]()
			{
				is_jumping = false;
			});

		timer_disappear.set_one_shot(true);
		timer_disappear.set_wait_time(interval_disappear);
		timer_disappear.set_on_timeout([&]()
			{
				is_vaild = false;
			});

		velocity.x =  ((rand() % 2 == 0) ? 1 : -1) * 2 * SIZE_TILE ;
		velocity.y = (-3) * SIZE_TILE;



	}
	~CoinProp() = default;
public:
	void set_position(const Vector2 pos)
	{
		position.x = pos.x;
		position.y = pos.y;
	}

	const Vector2& get_position() const
	{
		return position;
	}

	const Vector2& get_size() const
	{
		return size;
	}

	void set_invaild()
	{
		is_vaild = false;
		return;
	}

	bool can_remove()
	{
		return !is_vaild;
	}

public:
	void on_update(double delta)
	{
		timer_disappear.on_update(delta);
		timer_jump.on_update(delta);

		if(is_jumping)
		{
			velocity.y += gravity * delta;
		}
		else
		{
			velocity.x = 0;
			velocity.y = sin(SDL_GetTicks64() / 1000 * 4) * 30;
		}

		position += velocity * delta;


	}

	void on_render(SDL_Renderer* renderer)
	{
		static SDL_Rect rect = { 0,0 };
		rect.w = (int)size.x;
		rect.h = (int)size.y;

		SDL_Texture* tex_coin = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Coin)->second;
		rect.x = (int)(position.x - size.x / 2);
		rect.y = (int)(position.y - size.y / 2);
		SDL_RenderCopy(renderer, tex_coin, nullptr, &rect);

	}
private:
	Vector2 position;
	Vector2 velocity;//为了展现动画的蹦跳效果

	Timer timer_jump;
	Timer timer_disappear;

	bool is_vaild = true;
	bool is_jumping = true;

	double gravity = 490;
	double interval_jump = 0.75;
	Vector2 size = { 16,16 };
	double interval_disappear = 10;
};

