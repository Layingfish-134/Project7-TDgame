#pragma once

#include "manager.h"
#include "config_manager.h"
#include "resources_manager.h"
#include "save_manager.h"
#include "scene_factory.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

class GameManager : public Manager<GameManager>
{
	friend class Manager<GameManager>;

public:
	int run(int argc, char* argv[])
	{
		SceneManager::instance()->goto_main_menu();

		Uint64 last_counter = SDL_GetPerformanceCounter();
		const Uint64 counter_freq = SDL_GetPerformanceFrequency();

		while (!SceneManager::instance()->should_quit())
		{
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
					SceneManager::instance()->request_quit();
				else
					SceneManager::instance()->on_input(event);
			}

			Uint64 current_counter = SDL_GetPerformanceCounter();
			double delta = (double)(current_counter - last_counter) / counter_freq;
			last_counter = current_counter;
			if (delta * 1000 < 1000.0 / 60)
				SDL_Delay((Uint32)(1000.0 / 60 - delta * 1000));

			SceneManager::instance()->on_update(delta);

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);
			SceneManager::instance()->on_render(renderer);
			SDL_RenderPresent(renderer);
		}

		SceneManager::instance()->shutdown();
		return 0;
	}

protected:
	GameManager()
	{
		init_assert(!SDL_Init(SDL_INIT_EVERYTHING), u8"SDL初始化失败");
		init_assert(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG), u8"SDL_image初始化失败");
		init_assert(Mix_Init(MIX_INIT_MP3), u8"mixer初始化失败");
		init_assert(!TTF_Init(), u8"ttf初始化失败");

		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

		ConfigManager* config = ConfigManager::instance();
		init_assert(config->load_game_config("config.json"), u8"游戏配置加载失败");

		window = SDL_CreateWindow(config->basic_template.window_title.c_str(), SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, config->basic_template.window_width, config->basic_template.window_height, SDL_WINDOW_SHOWN);
		init_assert(window, u8"窗口创建失败");

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
		init_assert(renderer, u8"渲染器创建失败");

		init_assert(ResourcesManager::instance()->load_from_file(renderer), u8"游戏资源加载失败");
		init_assert(config->load_level_manifest("resources/levels/levels.json"), u8"关卡列表加载失败");
		SaveManager::instance()->initialize("save.db");
		SceneManager::instance()->set_renderer(renderer);
	}

	~GameManager()
	{
		SceneManager::instance()->shutdown();
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		TTF_Quit();
		Mix_Quit();
		IMG_Quit();
		SDL_Quit();
	}

private:
	SDL_Event event;
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;

private:
	void init_assert(bool flag, const char* error_msg)
	{
		if (flag) return;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, u8"游戏错误", error_msg, window);
		exit(-1);
	}
};
