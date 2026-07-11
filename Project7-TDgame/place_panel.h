#pragma once
#include"resources_manager.h"
#include"tower_manager.h"
#include"coin_manager.h"
#include"ui_helpers.h"

#include<SDL2_gfxPrimitives.h>
#include<functional>
#include<string>
#include<vector>

class PlacePanel
{
public:
	typedef std::function<void(TowerType, SDL_Point)> PlaceRequestCallback;

	void set_on_place_requested(PlaceRequestCallback callback)
	{
		on_place_requested = callback;
	}

	void show()
	{
		visible = true;
		hovered_index = -1;
	}

	void hide()
	{
		visible = false;
		hovered_index = -1;
	}

	void set_idx_tile(const SDL_Point& idx)
	{
		point_selected = idx;
	}

	void set_center_pos(const SDL_Point& pos)
	{
		center_pos = pos;
		update_layout();
	}

	void on_update(SDL_Renderer*)
	{
	}

	void on_render(SDL_Renderer* renderer)
	{
		if (!visible)
			return;

		render_range(renderer);
		UI::draw_texture(renderer, build_panel_texture(), panel_rect);
		UI::draw_text(renderer, u8"建造", panel_rect.x + 18, panel_rect.y + 14, color_title, ResID::Font_Small);
		for (size_t i = 0; i < options.size(); i++)
			render_button(renderer, (int)i);
	}

	void on_input(const SDL_Event& event)
	{
		if (!visible)
			return;

		if (event.type == SDL_MOUSEMOTION)
		{
			SDL_Point cursor = { event.motion.x, event.motion.y };
			hovered_index = -1;
			for (size_t i = 0; i < button_rects.size(); i++)
				if (SDL_PointInRect(&cursor, &button_rects[i]))
					hovered_index = (int)i;
		}
		else if (event.type == SDL_MOUSEBUTTONUP)
		{
			if (hovered_index >= 0 && hovered_index < (int)options.size() && on_place_requested)
				on_place_requested(options[hovered_index].type, point_selected);
			visible = false;
		}
		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
		{
			visible = false;
		}
	}

private:
	struct BuildOption
	{
		TowerType type;
		const char* name;
	};

	void update_layout()
	{
		panel_rect = { center_pos.x + 38, center_pos.y - 94, 420, 188 };
		button_rects.clear();
		for (int i = 0; i < 4; i++)
		{
			int col = i % 2;
			int row = i / 2;
			button_rects.push_back({ panel_rect.x + 24 + col * 198, panel_rect.y + 48 + row * 58, 174, 52 });
		}
	}

	void render_button(SDL_Renderer* renderer, int index)
	{
		const BuildOption& option = options[index];
		SDL_Rect rect = button_rects[index];
		double cost = TowerManager::instance()->get_place_tower_cost(option.type);
		bool affordable = cost <= CoinManager::instance()->get_current_coin_num();
		SDL_Color text_color = affordable ? color_primary : color_disabled;

		if (!affordable)
		{
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(renderer, 18, 20, 24, 115);
			SDL_RenderFillRect(renderer, &rect);
		}

		render_tower_icon(renderer, option.type, { rect.x + 12, rect.y + 7, 38, 38 });
		UI::draw_text(renderer, option.name, rect.x + 58, rect.y + 6, text_color, ResID::Font_Small);
		UI::draw_text(renderer, "$" + std::to_string((int)cost), rect.x + 58, rect.y + 29, text_color, ResID::Font_Small);
	}

	ResID build_panel_texture() const
	{
		switch (hovered_index)
		{
		case 0: return ResID::Tex_UIBuildPanelHover0;
		case 1: return ResID::Tex_UIBuildPanelHover1;
		case 2: return ResID::Tex_UIBuildPanelHover2;
		case 3: return ResID::Tex_UIBuildPanelHover3;
		default: return ResID::Tex_UIBuildPanel;
		}
	}

	void render_tower_icon(SDL_Renderer* renderer, TowerType type, const SDL_Rect& dst)
	{
		switch (type)
		{
		case TowerType::Archer:
			render_sheet_icon(renderer, ResID::Tex_Archer, 3, 8, 0, dst);
			break;
		case TowerType::Axeman:
			render_sheet_icon(renderer, ResID::Tex_Axeman, 3, 8, 0, dst);
			break;
		case TowerType::Gunner:
			render_sheet_icon(renderer, ResID::Tex_Gunner, 4, 8, 0, dst);
			break;
		case TowerType::Barracks:
			render_sheet_icon(renderer, ResID::Tex_BarracksLv1, 3, 8, 0, dst);
			break;
		}
	}

	void render_sheet_icon(SDL_Renderer* renderer, ResID res_id, int columns, int rows, int frame, const SDL_Rect& dst)
	{
		SDL_Texture* texture = ResourcesManager::instance()->get_texture_pool().find(res_id)->second;
		int tex_width = 0, tex_height = 0;
		SDL_QueryTexture(texture, nullptr, nullptr, &tex_width, &tex_height);
		SDL_Rect src = { (frame % columns) * (tex_width / columns), (frame / columns) * (tex_height / rows), tex_width / columns, tex_height / rows };
		SDL_RenderCopy(renderer, texture, &src, &dst);
	}

	void render_range(SDL_Renderer* renderer)
	{
		if (hovered_index < 0 || hovered_index >= (int)options.size())
			return;
		int radius = (int)(TowerManager::instance()->get_tower_damage_range(options[hovered_index].type) * SIZE_TILE);
		if (radius <= 0)
			return;
		filledCircleRGBA(renderer, center_pos.x, center_pos.y, radius, 0, 149, 217, 45);
		aacircleRGBA(renderer, center_pos.x, center_pos.y, radius, 30, 80, 162, 145);
	}

	bool visible = false;
	int hovered_index = -1;
	SDL_Point point_selected = { 0, 0 };
	SDL_Point center_pos = { 0, 0 };
	SDL_Rect panel_rect = { 0, 0, 0, 0 };
	std::vector<SDL_Rect> button_rects;
	PlaceRequestCallback on_place_requested;
	const SDL_Color color_title = { 104, 77, 49, 255 };
	const SDL_Color color_primary = { 82, 70, 49, 255 };
	const SDL_Color color_disabled = { 151, 139, 112, 255 };
	std::vector<BuildOption> options =
	{
		{ TowerType::Archer, "Archer" },
		{ TowerType::Axeman, "Axeman" },
		{ TowerType::Gunner, "Gunner" },
		{ TowerType::Barracks, "Barracks" }
	};
};
