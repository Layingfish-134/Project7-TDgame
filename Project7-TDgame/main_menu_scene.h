#pragma once

#include "scene.h"
#include "scene_manager.h"
#include "ui_helpers.h"
#include "save_manager.h"

#include <vector>

class MainMenuScene : public Scene
{
public:
	void on_enter(SDL_Renderer* renderer) override
	{
		buttons.clear();
		buttons.push_back({ { 490, 250, 300, 52 }, u8"开始游戏" });
		buttons.push_back({ { 490, 318, 300, 52 }, u8"载入存档" });
		buttons.push_back({ { 490, 386, 300, 52 }, u8"局域网联机" });
		buttons.push_back({ { 490, 454, 300, 52 }, u8"退出" });
	}

	void on_input(const SDL_Event& event) override
	{
		if (event.type == SDL_MOUSEMOTION)
			update_hover(event.motion.x, event.motion.y);
		if (event.type == SDL_MOUSEBUTTONUP)
		{
			update_hover(event.button.x, event.button.y);
			switch (hovered)
			{
			case 0:
				SceneManager::instance()->goto_save_slots();
				break;
			case 1:
				SceneManager::instance()->goto_save_slots();
				break;
			case 2:
				SceneManager::instance()->goto_network_menu();
				break;
			case 3:
				SceneManager::instance()->request_quit();
				break;
			default:
				break;
			}
		}
	}

	void on_update(double delta) override {}

	void on_render(SDL_Renderer* renderer) override
	{
		SDL_SetRenderDrawColor(renderer, 32, 38, 42, 255);
		SDL_Rect bg = { 0, 0, 1280, 720 };
		SDL_RenderFillRect(renderer, &bg);

		const ResourcesManager::TexturePool& tex_pool = ResourcesManager::instance()->get_texture_pool();
		SDL_Rect home = { 586, 105, 108, 108 };
		SDL_RenderCopy(renderer, tex_pool.find(ResID::Tex_Home)->second, nullptr, &home);

		UI::draw_text(renderer, u8"村庄保卫战", 520, 62, { 255, 245, 204, 255 });
		UI::draw_text(renderer, u8"Tower Defense Demo", 505, 188, { 190, 205, 190, 255 });
		for (int i = 0; i < (int)buttons.size(); i++)
			UI::draw_button(renderer, buttons[i], hovered == i);

		std::string save_hint = SaveManager::instance()->uses_sqlite_runtime() ?
			u8"SQLite runtime: available" : u8"SQLite runtime: fallback save.db text mode";
		UI::draw_text(renderer, save_hint, 20, 680, { 160, 160, 160, 255 });
	}

private:
	std::vector<UI::Button> buttons;
	int hovered = -1;

	void update_hover(int x, int y)
	{
		hovered = -1;
		SDL_Point p = { x, y };
		for (int i = 0; i < (int)buttons.size(); i++)
			if (SDL_PointInRect(&p, &buttons[i].rect))
				hovered = i;
	}
};
