#pragma once

#include "scene.h"
#include "scene_manager.h"
#include "ui_helpers.h"
#include "network_manager.h"

#include <sstream>
#include <vector>

class NetworkScene : public Scene
{
public:
	void on_enter(SDL_Renderer* renderer) override
	{
		keep_network = false;
		SDL_StartTextInput();
		buttons.clear();
		buttons.push_back({ { 420, 210, 440, 52 }, u8"创建房间 - RPG角色" });
		buttons.push_back({ { 420, 276, 440, 52 }, u8"创建房间 - 防御塔" });
		buttons.push_back({ { 420, 342, 440, 52 }, u8"加入房间 - RPG角色" });
		buttons.push_back({ { 420, 408, 440, 52 }, u8"加入房间 - 防御塔" });
		buttons.push_back({ { 420, 474, 440, 52 }, u8"开始联机关卡 1" });
		buttons.push_back({ { 420, 540, 440, 52 }, u8"返回主菜单" });
	}

	void on_exit() override
	{
		SDL_StopTextInput();
		if (!keep_network)
			NetworkManager::instance()->reset();
	}

	void on_input(const SDL_Event& event) override
	{
		if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
			SceneManager::instance()->goto_main_menu();

		if (event.type == SDL_TEXTINPUT)
		{
			host_ip += event.text.text;
			if (host_ip.size() > 32)
				host_ip.resize(32);
		}

		if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_BACKSPACE && !host_ip.empty())
			host_ip.pop_back();

		if (event.type == SDL_MOUSEMOTION)
			update_hover(event.motion.x, event.motion.y);

		if (event.type == SDL_MOUSEBUTTONUP)
		{
			update_hover(event.button.x, event.button.y);
			switch (hovered)
			{
			case 0:
				NetworkManager::instance()->start_host(7777, NetworkRole::Rpg);
				break;
			case 1:
				NetworkManager::instance()->start_host(7777, NetworkRole::Tower);
				break;
			case 2:
				NetworkManager::instance()->join_host(host_ip.empty() ? "127.0.0.1" : host_ip, 7777, NetworkRole::Rpg);
				break;
			case 3:
				NetworkManager::instance()->join_host(host_ip.empty() ? "127.0.0.1" : host_ip, 7777, NetworkRole::Tower);
				break;
			case 4:
				if (NetworkManager::instance()->hosting() && NetworkManager::instance()->roles_ready())
				{
					NetworkManager::instance()->set_start_level_id(1);
					keep_network = true;
					NetworkManager::instance()->send_line("START 1");
					SceneManager::instance()->goto_game(1, true, NetworkManager::instance()->get_role());
				}
				break;
			case 5:
				SceneManager::instance()->goto_main_menu();
				break;
			default:
				break;
			}
		}
	}

	void on_update(double delta) override
	{
		NetworkManager* network = NetworkManager::instance();
		network->on_update();

		std::string message;
		while (network->poll_message(message))
		{
			std::stringstream stream(message);
			std::string type;
			stream >> type;
			if (type == "ROLE")
			{
				std::string role_text;
				stream >> role_text;
				network->mark_peer_role(NetworkManager::role_from_text(role_text));
			}
			else if (type == "START")
			{
				int id = 1;
				stream >> id;
				network->set_start_level_id(id);
				keep_network = true;
				SceneManager::instance()->goto_game(id, true, network->get_role());
				return;
			}
			else if (type == "DISCONNECT")
			{
				network->reset();
			}
		}
	}

	void on_render(SDL_Renderer* renderer) override
	{
		SDL_SetRenderDrawColor(renderer, 28, 36, 48, 255);
		SDL_Rect bg = { 0, 0, 1280, 720 };
		SDL_RenderFillRect(renderer, &bg);

		UI::draw_text(renderer, u8"局域网联机原型", 520, 70, { 255, 245, 204, 255 });
		UI::draw_text(renderer, u8"加入地址:", 390, 150, { 220, 225, 210, 255 });
		SDL_Rect rect_ip = { 510, 144, 350, 42 };
		SDL_SetRenderDrawColor(renderer, 46, 54, 64, 255);
		SDL_RenderFillRect(renderer, &rect_ip);
		SDL_SetRenderDrawColor(renderer, 224, 222, 196, 255);
		SDL_RenderDrawRect(renderer, &rect_ip);
		UI::draw_text(renderer, host_ip.empty() ? "127.0.0.1" : host_ip, rect_ip.x + 12, rect_ip.y + 8, { 255, 255, 240, 255 });

		for (int i = 0; i < (int)buttons.size(); i++)
		{
			UI::Button button = buttons[i];
			if (i == 4)
				button.disabled = !NetworkManager::instance()->hosting() || !NetworkManager::instance()->roles_ready();
			UI::draw_button(renderer, button, hovered == i);
		}

		UI::draw_text(renderer, std::string(u8"状态: ") + NetworkManager::instance()->get_status(), 380, 625, { 190, 205, 190, 255 });
		UI::draw_text(renderer, std::string(u8"本机角色: ") + NetworkManager::role_to_text(NetworkManager::instance()->get_role())
			+ u8"  对方角色: " + NetworkManager::role_to_text(NetworkManager::instance()->get_peer_role()), 380, 650, { 190, 205, 190, 255 });
		if (!NetworkManager::instance()->get_last_error().empty())
			UI::draw_text(renderer, std::string(u8"错误: ") + NetworkManager::instance()->get_last_error(), 380, 680, { 230, 150, 150, 255 });
	}

private:
	std::vector<UI::Button> buttons;
	int hovered = -1;
	bool keep_network = false;
	std::string host_ip = "127.0.0.1";

	void update_hover(int x, int y)
	{
		hovered = -1;
		SDL_Point p = { x, y };
		for (int i = 0; i < (int)buttons.size(); i++)
			if (SDL_PointInRect(&p, &buttons[i].rect))
				hovered = i;
	}
};
