#pragma once

#include"resources_manager.h"
#include"config_manager.h"
#include"coin_manager.h"
#include"home_manager.h"
#include"player_manager.h"
#include<SDL.h>
#include<SDL2_gfxPrimitives.h>
#include<SDL_ttf.h>

#include<string>

class StatusBar
{
public:
	StatusBar() = default;
	~StatusBar() = default;
public:
	void set_position(int x,int y)//根据窗口坐标设置位置
	{
		position.x = x;
		position.y = y;
	}

	void on_render(SDL_Renderer* renderer)
	{
		SDL_Rect rect_dst;
		const ResourcesManager::TexturePool& tex_pool = ResourcesManager::instance()->get_texture_pool();
		SDL_Texture* tex_coin = tex_pool.find(ResID::Tex_UICoin)->second;
		SDL_Texture* tex_heart = tex_pool.find(ResID::Tex_UIHeart)->second;
		SDL_Texture* tex_home = tex_pool.find(ResID::Tex_UIHomeAvatar)->second;
		SDL_Texture* tex_player = tex_pool.find(ResID::Tex_UIPlayerAvatar)->second;

		//绘制房屋头像
		rect_dst.x = position.x;
		rect_dst.y = position.y;
		rect_dst.w = 78;
		rect_dst.h = 78;
		SDL_RenderCopy(renderer, tex_home, nullptr, &rect_dst);

		//绘制生命值
		const int wid_heart = 32;
		const int offset_per_heart = 2;
		for (int i = 0; i < HomeManager::instance()->get_home_hp(); i++)
		{
			rect_dst.x = position.x + 78 + 15 + i * (wid_heart + offset_per_heart);
			rect_dst.y = position.y;
			rect_dst.w = wid_heart;
			rect_dst.h = wid_heart;
			SDL_RenderCopy(renderer, tex_heart, nullptr, &rect_dst);
		}

		//绘制硬币图标与数量
		rect_dst.x = position.x + 78 + 15;
		rect_dst.y = position.y + 78 - 32;
		rect_dst.w = 32;
		rect_dst.h = 32;
		SDL_RenderCopy(renderer, tex_coin, nullptr, &rect_dst);

		
		
		//文字阴影
		rect_dst.x += 32 + 10 + offset_shadow.x;
		rect_dst.y = rect_dst.y + (32 - height_text) / 2 + offset_shadow.y;
		rect_dst.w = width_text; rect_dst.h = height_text;
		SDL_RenderCopy(renderer, tex_text_background, nullptr, &rect_dst);
		//文字
		rect_dst.x -= offset_shadow.x;
		rect_dst.y -= offset_shadow.y;
		SDL_RenderCopy(renderer, tex_text_foreground, nullptr, &rect_dst);
		

		//绘制玩家头像
		rect_dst.x = position.x + (78 - 65) / 2;
		rect_dst.y = position.y + 78 + 5;
		rect_dst.w = 65; rect_dst.h = 65;
		SDL_RenderCopy(renderer, tex_player, nullptr, &rect_dst);

		//绘制玩家能量条
		rect_dst.x = position.x + 78 + 15;
		rect_dst.y += 10;
		roundedBoxRGBA(renderer, rect_dst.x, rect_dst.y, rect_dst.x + width_mp, rect_dst.y + height_mp, 4,
			color_mp_bar_background.r, color_mp_bar_background.g, color_mp_bar_background.b, color_mp_bar_background.a);


		rect_dst.x += width_border_mp;
		rect_dst.y += width_border_mp;
		rect_dst.w = width_mp - 2 * width_border_mp;
		rect_dst.h = height_mp - 2 * width_border_mp;
		double process_mp = PlayerManager::instance()->get_current_mp() / 100;
		roundedBoxRGBA(renderer, rect_dst.x, rect_dst.y, rect_dst.x + (int)(rect_dst.w * process_mp), rect_dst.y + rect_dst.h,
			2,color_mp_bar_foreground.r, color_mp_bar_foreground.g, color_mp_bar_foreground.b, color_mp_bar_foreground.a);


	}

	void on_update(SDL_Renderer* renderer)
	{
		static TTF_Font* font = ResourcesManager::instance()->get_font_pool().find(ResID::Font_Main)->second;
		SDL_DestroyTexture(tex_text_background);
		tex_text_background = nullptr;
		SDL_DestroyTexture(tex_text_foreground);
		tex_text_foreground = nullptr;

		std::string str_coin_num = std::to_string((int)CoinManager::instance()->get_current_coin_num());
		SDL_Surface* suf_text_background = 
			TTF_RenderText_Blended(font, str_coin_num.c_str(), color_text_background);
		SDL_Surface* suf_text_foreground =
			TTF_RenderText_Blended(font, str_coin_num.c_str(), color_text_foreground);

		width_text = suf_text_background->w;
		height_text = suf_text_background->h;

		tex_text_background = SDL_CreateTextureFromSurface(renderer, suf_text_background);
		tex_text_foreground = SDL_CreateTextureFromSurface(renderer, suf_text_foreground);

		SDL_FreeSurface(suf_text_background);
		SDL_FreeSurface(suf_text_foreground);

	}
private:
	const int size_heart = 32;

	const int width_mp = 200;
	const int height_mp = 20;
	const int width_border_mp = 4;

	const SDL_Point offset_shadow = { 2,2 };

	const SDL_Color color_text_background = {175,175,175,255};
	const SDL_Color color_text_foreground = {255,255,255,255};
	const SDL_Color color_mp_bar_background = { 48,40,51,255 };
	const SDL_Color color_mp_bar_foreground = { 144,121,173,255 };

private:
	SDL_Point position;
	int width_text;
	int height_text;
	SDL_Texture* tex_text_background = nullptr;
	SDL_Texture* tex_text_foreground = nullptr;

};

