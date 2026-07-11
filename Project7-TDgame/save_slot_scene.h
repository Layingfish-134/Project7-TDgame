#pragma once

#include "scene.h"
#include "scene_manager.h"
#include "ui_helpers.h"
#include "save_manager.h"

#include <string>
#include <vector>

class SaveSlotScene : public Scene
{
public:
	void on_enter(SDL_Renderer* renderer) override
	{
		refresh_buttons();
	}

	void on_input(const SDL_Event& event) override
	{
		if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
			SceneManager::instance()->goto_main_menu();

		if (event.type == SDL_MOUSEMOTION)
			update_hover(event.motion.x, event.motion.y);

		if (event.type == SDL_MOUSEBUTTONUP)
		{
			update_hover(event.button.x, event.button.y);
			if (hovered >= 0 && hovered < SaveManager::instance()->get_slot_count())
			{
				SaveManager::instance()->select_slot(hovered + 1);
				SceneManager::instance()->goto_level_select();
			}
			else if (hovered == back_button_index)
			{
				SceneManager::instance()->goto_main_menu();
			}
		}
	}

	void on_update(double delta) override {}

	void on_render(SDL_Renderer* renderer) override
	{
		SDL_SetRenderDrawColor(renderer, 30, 39, 45, 255);
		SDL_Rect bg = { 0, 0, 1280, 720 };
		SDL_RenderFillRect(renderer, &bg);

		UI::draw_text(renderer, u8"选择存档", 555, 70, { 255, 245, 204, 255 });
		for (int i = 0; i < (int)buttons.size(); i++)
			UI::draw_button(renderer, buttons[i], hovered == i);

		UI::draw_text(renderer, u8"存档只记录已完成关卡与解锁进度", 385, 620, { 180, 190, 180, 255 });
		UI::draw_text(renderer, u8"ESC 返回主菜单", 32, 672, { 180, 180, 170, 255 });
	}

private:
	std::vector<UI::Button> buttons;
	int hovered = -1;
	int back_button_index = -1;

	void refresh_buttons()
	{
		buttons.clear();

		for (int slot = 1; slot <= SaveManager::instance()->get_slot_count(); slot++)
		{
			std::string text = std::string(u8"存档 ") + std::to_string(slot);
			if (SaveManager::instance()->has_slot_data(slot))
			{
				text += std::string(u8"  解锁至关卡 ") + std::to_string(SaveManager::instance()->get_slot_highest_unlocked_level(slot));
				text += std::string(u8"  已通关 ") + std::to_string(SaveManager::instance()->get_slot_clear_count(slot));
			}
			else
			{
				text += u8"  [空]";
			}
			buttons.push_back({ { 360, 180 + (slot - 1) * 92, 560, 64 }, text });
		}

		back_button_index = (int)buttons.size();
		buttons.push_back({ { 490, 520, 300, 52 }, u8"返回主菜单" });
	}

	void update_hover(int x, int y)
	{
		hovered = -1;
		SDL_Point p = { x, y };
		for (int i = 0; i < (int)buttons.size(); i++)
			if (SDL_PointInRect(&p, &buttons[i].rect))
				hovered = i;
	}
};
