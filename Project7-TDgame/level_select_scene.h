#pragma once

#include "scene.h"
#include "scene_manager.h"
#include "ui_helpers.h"
#include "config_manager.h"
#include "save_manager.h"

#include <string>
#include <vector>

class LevelSelectScene : public Scene
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
			if (hovered >= 0 && hovered < (int)level_ids.size())
			{
				int level_id = level_ids[hovered];
				if (SaveManager::instance()->is_level_unlocked(level_id))
					SceneManager::instance()->goto_game(level_id);
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
		SDL_SetRenderDrawColor(renderer, 31, 42, 47, 255);
		SDL_Rect bg = { 0, 0, 1280, 720 };
		SDL_RenderFillRect(renderer, &bg);

		UI::draw_text(renderer, u8"选择关卡", 555, 58, { 255, 245, 204, 255 });
		const std::vector<ConfigManager::LevelInfo>& levels = ConfigManager::instance()->level_list;
		for (int i = 0; i < (int)level_ids.size() && i < (int)levels.size(); i++)
			render_level_card(renderer, levels[i], buttons[i].rect, hovered == i);

		if (back_button_index >= 0 && back_button_index < (int)buttons.size())
			UI::draw_button(renderer, buttons[back_button_index], hovered == back_button_index);
		UI::draw_text(renderer, u8"ESC 返回主菜单", 32, 672, { 180, 180, 170, 255 });
	}

private:
	std::vector<UI::Button> buttons;
	std::vector<int> level_ids;
	int hovered = -1;
	int back_button_index = -1;

	void refresh_buttons()
	{
		buttons.clear();
		level_ids.clear();

		const std::vector<ConfigManager::LevelInfo>& levels = ConfigManager::instance()->level_list;
		for (int i = 0; i < (int)levels.size(); i++)
		{
			const ConfigManager::LevelInfo& level = levels[i];
			SaveManager::LevelRecord record = SaveManager::instance()->get_level_record(level.id);
			bool unlocked = SaveManager::instance()->is_level_unlocked(level.id);

			buttons.push_back({ { 160, 132 + i * 128, 960, 112 }, level.name, !unlocked });
			level_ids.push_back(level.id);
		}

		back_button_index = (int)buttons.size();
		buttons.push_back({ { 490, 562, 300, 52 }, u8"返回主菜单" });
	}

	void render_level_card(SDL_Renderer* renderer, const ConfigManager::LevelInfo& level, const SDL_Rect& rect, bool hovered)
	{
		SaveManager::LevelRecord record = SaveManager::instance()->get_level_record(level.id);
		bool unlocked = SaveManager::instance()->is_level_unlocked(level.id);

		UI::draw_texture(renderer, hovered && unlocked ? ResID::Tex_UILevelCardHover : ResID::Tex_UILevelCardBg, rect);
		if (!unlocked)
			UI::draw_texture(renderer, ResID::Tex_UILevelLockedMask, rect);

		SDL_Color border = !unlocked ? SDL_Color{ 92, 92, 96, 255 } :
			record.cleared ? SDL_Color{ 248, 210, 105, 255 } : SDL_Color{ 224, 222, 196, 255 };
		SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
		SDL_RenderDrawRect(renderer, &rect);

		SDL_Rect preview = { rect.x + 18, rect.y + 16, 126, 80 };
		SDL_SetRenderDrawColor(renderer, 34, 44, 49, 255);
		SDL_RenderFillRect(renderer, &preview);
		SDL_SetRenderDrawColor(renderer, 104, 128, 103, 255);
		SDL_RenderDrawRect(renderer, &preview);
		UI::draw_centered_text(renderer, std::string("LV.") + std::to_string(level.id), preview, { 255, 245, 204, 255 }, ResID::Font_Small);

		SDL_Color main_text = unlocked ? SDL_Color{ 255, 245, 204, 255 } : SDL_Color{ 155, 155, 155, 255 };
		SDL_Color sub_text = unlocked ? SDL_Color{ 205, 215, 200, 255 } : SDL_Color{ 130, 130, 130, 255 };
		std::string title = level.name.empty() ? std::string(u8"关卡 ") + std::to_string(level.id) : level.name;
		int text_x = rect.x + 166;
		int meta_x = rect.x + 680;
		UI::draw_text(renderer, title, text_x, rect.y + 13, main_text);
		UI::draw_text(renderer, level.description.empty() ? u8"守住通往村庄的道路。" : level.description,
			text_x, rect.y + 47, sub_text, ResID::Font_Small);
		UI::draw_text(renderer, std::string(u8"目标: ") + (level.objective.empty() ? u8"清空所有波次" : level.objective),
			text_x, rect.y + 75, sub_text, ResID::Font_Small);

		std::string status = !unlocked ? u8"锁定" : record.cleared ? u8"已通关" : u8"未通关";
		UI::draw_text(renderer, std::string(u8"难度: ") + (level.difficulty.empty() ? u8"普通" : level.difficulty),
			meta_x, rect.y + 18, main_text, ResID::Font_Small);
		UI::draw_text(renderer, std::string(u8"状态: ") + status, meta_x, rect.y + 46, sub_text, ResID::Font_Small);
		if (record.cleared)
		{
			UI::draw_text(renderer, u8"最佳 HP/金币: " + std::to_string(record.best_home_hp) + " / " + std::to_string(record.best_coin),
				meta_x, rect.y + 74, sub_text, ResID::Font_Small);
		}
		else
		{
			UI::draw_text(renderer, u8"最佳 HP/金币: -- / --", meta_x, rect.y + 74, sub_text, ResID::Font_Small);
		}

		if (!unlocked)
		{
			SDL_Rect lock_rect = { preview.x + 35, preview.y + 12, 56, 56 };
			UI::draw_texture(renderer, ResID::Tex_UILockIcon, lock_rect);
		}
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
