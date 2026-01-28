#pragma once
#include"Vector2.h"
#include"timer.h"
#include"config_manager.h"
#include"resources_manager.h"

#include<SDL.h>
class Banner
{
public:
	Banner()
	{
		size_foreground = { 646,215 };
		size_background = { 1282,219 };

		timer_display.set_one_shot(true);
		timer_display.set_wait_time(5);
		timer_display.set_on_timeout([&]()
			{
				is_end_display = true;
			});


	}
	~Banner() = default;

public:
	void on_update(double delta)
	{
		timer_display.on_update(delta);

		const ResourcesManager::TexturePool& tex_pool = ResourcesManager::instance()->get_texture_pool();
		ConfigManager* config = ConfigManager::instance();

		if (config->is_game_win)
		{
			tex_foreground = tex_pool.find(ResID::Tex_UIWinText)->second;
		}
		else
		{
			tex_foreground = tex_pool.find(ResID::Tex_UILossText)->second;
		}

		tex_background = tex_pool.find(ResID::Tex_UIGameOverBar)->second;
	}

	void on_render(SDL_Renderer* renderer)
	{
		static SDL_Rect rect_dst;
		rect_dst.x = position_center.x - size_background.x / 2;
		rect_dst.y = position_center.y - size_background.y / 2;
		rect_dst.w = size_background.x;
		rect_dst.h = size_background.y;

		SDL_RenderCopy(renderer, tex_background, nullptr, &rect_dst);


		rect_dst.x = position_center.x - size_foreground.x / 2;
		rect_dst.y = position_center.y - size_foreground.y / 2;
		rect_dst.w = size_foreground.x;
		rect_dst.h = size_foreground.y;

		SDL_RenderCopy(renderer, tex_foreground, nullptr, &rect_dst);


	}
public:
	void set_position(const Vector2& pos)
	{
		position_center = pos;
	}

	bool check_end_display()
	{
		return is_end_display;
	}
private:
	Vector2 position_center;
	Vector2 size_foreground;
	Vector2 size_background;

	SDL_Texture* tex_background;
	SDL_Texture* tex_foreground;

	Timer timer_display;
	bool is_end_display = false;
};

