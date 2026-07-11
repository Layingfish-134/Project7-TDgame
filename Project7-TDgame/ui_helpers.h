#pragma once

#include "resources_manager.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>

namespace UI
{
	struct Button
	{
		SDL_Rect rect = { 0 };
		std::string text;
		bool disabled = false;
	};

	inline void draw_text(SDL_Renderer* renderer, const std::string& text, int x, int y, const SDL_Color& color, ResID font_id = ResID::Font_Main)
	{
		const ResourcesManager::FontPool& font_pool = ResourcesManager::instance()->get_font_pool();
		auto iter_font = font_pool.find(font_id);
		if (iter_font == font_pool.end() || !iter_font->second)
			iter_font = font_pool.find(ResID::Font_Main);
		if (iter_font == font_pool.end() || !iter_font->second)
			return;

		TTF_Font* font = iter_font->second;
		SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
		if (!surface) return;

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_Rect rect_dst = { x, y, surface->w, surface->h };
		SDL_FreeSurface(surface);

		if (texture)
		{
			SDL_RenderCopy(renderer, texture, nullptr, &rect_dst);
			SDL_DestroyTexture(texture);
		}
	}

	inline void draw_centered_text(SDL_Renderer* renderer, const std::string& text, const SDL_Rect& bounds, const SDL_Color& color, ResID font_id = ResID::Font_Main)
	{
		const ResourcesManager::FontPool& font_pool = ResourcesManager::instance()->get_font_pool();
		auto iter_font = font_pool.find(font_id);
		if (iter_font == font_pool.end() || !iter_font->second)
			iter_font = font_pool.find(ResID::Font_Main);
		if (iter_font == font_pool.end() || !iter_font->second)
			return;

		TTF_Font* font = iter_font->second;
		int width = 0, height = 0;
		TTF_SizeUTF8(font, text.c_str(), &width, &height);
		draw_text(renderer, text, bounds.x + (bounds.w - width) / 2, bounds.y + (bounds.h - height) / 2, color, font_id);
	}

	inline bool point_in_rect(const SDL_Event& event, const SDL_Rect& rect)
	{
		SDL_Point point = { event.button.x, event.button.y };
		return SDL_PointInRect(&point, &rect);
	}

	inline bool draw_texture(SDL_Renderer* renderer, ResID res_id, const SDL_Rect& rect)
	{
		const ResourcesManager::TexturePool& tex_pool = ResourcesManager::instance()->get_texture_pool();
		auto iter = tex_pool.find(res_id);
		if (iter == tex_pool.end() || !iter->second)
			return false;

		SDL_RenderCopy(renderer, iter->second, nullptr, &rect);
		return true;
	}

	inline void draw_button(SDL_Renderer* renderer, const Button& button, bool hovered)
	{
		SDL_Color fill = button.disabled ? SDL_Color{ 58, 58, 62, 255 } :
			hovered ? SDL_Color{ 91, 113, 78, 255 } : SDL_Color{ 64, 74, 84, 255 };
		SDL_Color border = button.disabled ? SDL_Color{ 90, 90, 94, 255 } : SDL_Color{ 224, 222, 196, 255 };
		SDL_Color text = button.disabled ? SDL_Color{ 150, 150, 150, 255 } : SDL_Color{ 255, 255, 240, 255 };

		ResID texture = button.disabled ? ResID::Tex_UIButtonDisabled :
			hovered ? ResID::Tex_UIButtonHover : ResID::Tex_UIButtonIdle;
		if (!draw_texture(renderer, texture, button.rect))
		{
			SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
			SDL_RenderFillRect(renderer, &button.rect);
			SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
			SDL_RenderDrawRect(renderer, &button.rect);
		}
		draw_centered_text(renderer, button.text, button.rect, text);
	}
}
