#pragma once

#include "resources_manager.h"
#include "ui_helpers.h"

#include <SDL.h>
#include <functional>
#include <string>

class RemovePanel
{
public:
	typedef std::function<void(SDL_Point)> RemoveRequestCallback;

	void set_on_remove_requested(RemoveRequestCallback callback)
	{
		on_remove_requested = callback;
	}

	void show(SDL_Point idx_tile, SDL_Point center, double refund)
	{
		visible = true;
		hovered_index = -1;
		point_selected = idx_tile;
		center_pos = center;
		refund_coin = refund;
		update_layout();
	}

	void hide()
	{
		visible = false;
		hovered_index = -1;
	}

	bool is_visible() const
	{
		return visible;
	}

	void on_update(SDL_Renderer*)
	{
	}

	void on_render(SDL_Renderer* renderer)
	{
		if (!visible)
			return;

		UI::draw_texture(renderer, panel_texture(), panel_rect);
		UI::draw_text(renderer, u8"拆除防御塔?", panel_rect.x + 48, panel_rect.y + 13, color_title, ResID::Font_Small);
		UI::draw_text(renderer, std::string(u8"返还金币: $") + std::to_string((int)refund_coin),
			panel_rect.x + 24, panel_rect.y + 43, color_refund, ResID::Font_Small);
		UI::draw_centered_text(renderer, u8"确认拆除", confirm_rect, color_button, ResID::Font_Small);
		UI::draw_centered_text(renderer, u8"取消", cancel_rect, color_button, ResID::Font_Small);
	}

	void on_input(const SDL_Event& event)
	{
		if (!visible)
			return;

		if (event.type == SDL_MOUSEMOTION)
		{
			SDL_Point cursor = { event.motion.x, event.motion.y };
			if (SDL_PointInRect(&cursor, &confirm_rect))
				hovered_index = 0;
			else if (SDL_PointInRect(&cursor, &cancel_rect))
				hovered_index = 1;
			else
				hovered_index = -1;
		}
		else if (event.type == SDL_MOUSEBUTTONUP)
		{
			if (hovered_index == 0 && on_remove_requested)
				on_remove_requested(point_selected);
			hide();
		}
		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
		{
			hide();
		}
	}

private:
	void update_layout()
	{
		panel_rect = { center_pos.x + 38, center_pos.y - 56, 260, 112 };
		confirm_rect = { panel_rect.x + 24, panel_rect.y + 76, 96, 26 };
		cancel_rect = { panel_rect.x + 140, panel_rect.y + 76, 96, 26 };
	}

	ResID panel_texture() const
	{
		switch (hovered_index)
		{
		case 0: return ResID::Tex_UIRemoveTowerPanelHoverConfirm;
		case 1: return ResID::Tex_UIRemoveTowerPanelHoverCancel;
		default: return ResID::Tex_UIRemoveTowerPanel;
		}
	}

	bool visible = false;
	int hovered_index = -1;
	double refund_coin = 0;
	SDL_Point point_selected = { 0, 0 };
	SDL_Point center_pos = { 0, 0 };
	SDL_Rect panel_rect = { 0, 0, 0, 0 };
	SDL_Rect confirm_rect = { 0, 0, 0, 0 };
	SDL_Rect cancel_rect = { 0, 0, 0, 0 };
	RemoveRequestCallback on_remove_requested;
	const SDL_Color color_title = { 44, 31, 20, 255 };
	const SDL_Color color_refund = { 37, 74, 48, 255 };
	const SDL_Color color_button = { 42, 32, 24, 255 };
};
