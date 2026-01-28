#pragma once

#include"resources_manager.h"
#include"config_manager.h"

#include<SDL.h>
#include<SDL_ttf.h>

#include<string>
class Panel
{
public:
	Panel()
	{
		tex_cursor = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_UISelectCursor)->second;
	}
	~Panel()
	{
		SDL_DestroyTexture(tex_text_background);
		SDL_DestroyTexture(tex_text_foreground);
	}

public:
	void show()
	{
		visiable = true;
	}

	void set_idx_tile(const SDL_Point& poi)
	{
		point_selected = poi;
	}

	void set_center_pos(const SDL_Point& pos)
	{
		center_pos.x = pos.x;
		center_pos.y = pos.y;
		//std::cout << "set: " << center_pos.x << " " << center_pos.y << std::endl;
	}

public:
	virtual void on_update(SDL_Renderer* renderer)
	{
		TTF_Font* font = ResourcesManager::instance()->get_font_pool().find(ResID::Font_Main)->second;
		if (hovered_tar == HoveredTarget::None)
			return;

		int val = 0;
		switch (hovered_tar)
		{
		case Panel::HoveredTarget::Top:
			val = val_top;
			break;
		case Panel::HoveredTarget::Left:
			val = val_left;
			break;
		case Panel::HoveredTarget::Right:
			val = val_right;
			break;
		}

		SDL_DestroyTexture(tex_text_background);
		tex_text_background = nullptr;
		SDL_DestroyTexture(tex_text_foreground);
		tex_text_foreground = nullptr;

		std::string str_value = val < 0 ? "Max" : std::to_string(val);
		SDL_Surface* suf_text_background = TTF_RenderText_Blended(font,str_value.c_str(),color_background);
		SDL_Surface* suf_text_foreground = TTF_RenderText_Blended(font, str_value.c_str(), color_foreground);

		width_text = suf_text_background->w;
		height_text = suf_text_background->h;
		tex_text_background = SDL_CreateTextureFromSurface(renderer, suf_text_background);
		tex_text_foreground = SDL_CreateTextureFromSurface(renderer, suf_text_foreground);

		SDL_FreeSurface(suf_text_background);
		SDL_FreeSurface(suf_text_foreground);
	}

	virtual void on_render(SDL_Renderer* renderer)
	{
		if (!visiable)
			return;
		SDL_Rect rect_dst_cursor;
		rect_dst_cursor.x = center_pos.x - SIZE_TILE / 2;
		rect_dst_cursor.y = center_pos.y - SIZE_TILE / 2;
		rect_dst_cursor.w = SIZE_TILE;
		rect_dst_cursor.h = SIZE_TILE;
		//sstd::cout <<"render_begin: " << center_pos.x << " +" << center_pos.y << std::endl;
		SDL_RenderCopy(renderer, tex_cursor, nullptr, &rect_dst_cursor);

		SDL_Rect rect_dst_panel = {
			center_pos.x - width / 2,
			center_pos.y - height / 2,
			width,height
		};

		//std::cout << "render_end: " << center_pos.x << " " << center_pos.y << std::endl;
		SDL_Texture* tex_panel = nullptr;
		switch (hovered_tar)
		{
		case Panel::HoveredTarget::None:
			tex_panel = tex_idle;
			break;
		case Panel::HoveredTarget::Top:
			tex_panel = tex_highlight_top;
			break;
		case Panel::HoveredTarget::Left:
			tex_panel = tex_highlight_left;
			break;
		case Panel::HoveredTarget::Right:
			tex_panel = tex_highlight_right;
			break;
		}

		SDL_RenderCopy(renderer, tex_panel, nullptr, &rect_dst_panel);

		if (hovered_tar == HoveredTarget::None)
			return;

		SDL_Rect rect_dst_text =
		{
			center_pos.x - width_text / 2 + offset_text_shadow.x,
			center_pos.y + height / 2 + offset_text_shadow.y,
			width_text,height_text
		};

		SDL_RenderCopy(renderer, tex_text_background, nullptr, &rect_dst_text);
		rect_dst_text.x -= offset_text_shadow.x;
		rect_dst_text.y -= offset_text_shadow.y;
		SDL_RenderCopy(renderer, tex_text_foreground, nullptr, &rect_dst_text);

		
	}

	void on_input(const SDL_Event& event)
	{
		if (!visiable)
			return;

		switch (event.type)
		{
		case SDL_MOUSEMOTION:
		{
			SDL_Point pos_cursor = { event.motion.x,event.motion.y };
			SDL_Rect rect_target = { 0,0,size_button,size_button };

			rect_target.x = center_pos.x - width / 2 + offset_top.x;
			rect_target.y = center_pos.y - height / 2 + offset_top.y;
			if (SDL_PointInRect(&pos_cursor, &rect_target))
			{
				hovered_tar = HoveredTarget::Top;
				return;
			}

			rect_target.x = center_pos.x - width / 2 + offset_left.x;
			rect_target.y = center_pos.y - height / 2 + offset_left.y;
			if (SDL_PointInRect(&pos_cursor, &rect_target))
			{
				hovered_tar = HoveredTarget::Left;
				return;
			}

			rect_target.x = center_pos.x - width / 2 + offset_right.x;
			rect_target.y = center_pos.y - height / 2 + offset_right.y;
			if (SDL_PointInRect(&pos_cursor, &rect_target))
			{
				hovered_tar = HoveredTarget::Right;
				return;
			}

			hovered_tar = HoveredTarget::None;
		}
			break;
		case SDL_MOUSEBUTTONUP:
		{
			switch (hovered_tar)
			{
			case Panel::HoveredTarget::Top:
				on_click_top();
				break;
			case Panel::HoveredTarget::Left:
				on_click_left();
				break;
			case Panel::HoveredTarget::Right:
				on_click_right();
				break;
			}

			visiable = false;
		}
			break;
		default:
			break;
		}
	}
protected:
	enum class HoveredTarget
	{
		None,
		Top,
		Left,
		Right
	};
protected:
	bool visiable = false;
	SDL_Point point_selected;
	SDL_Point center_pos = { 0,0 };
	SDL_Texture* tex_idle = nullptr;
	SDL_Texture* tex_highlight_top = nullptr;
	SDL_Texture* tex_highlight_left = nullptr;
	SDL_Texture* tex_highlight_right = nullptr;
	SDL_Texture* tex_cursor = nullptr;
	int val_top = 0; int val_left = 0; int val_right = 0;
	HoveredTarget hovered_tar = HoveredTarget::None;

protected:
	virtual void on_click_top() = 0;
	virtual void on_click_left() = 0;
	virtual void on_click_right() = 0;

private:
	const int size_button = 48;
	const int height = 144;
	const int width = 144;
	const SDL_Point offset_top = { 48,6 };
	const SDL_Point offset_left = { 8,80 };
	const SDL_Point offset_right = { 90,80 };
	const SDL_Point offset_text_shadow = { 3,3 };
	const SDL_Color color_background = { 175,175,175,255 };
	const SDL_Color color_foreground = { 255,255,255,255 };

	int width_text = 0;
	int height_text = 0;
	SDL_Texture* tex_text_background = nullptr;
	SDL_Texture* tex_text_foreground = nullptr;

};

