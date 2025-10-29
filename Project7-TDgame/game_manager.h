#pragma once
#include"manager.h"

#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>

class GameManager : public Manager<GameManager>
{
	friend class Manager<GameManager>;

public:
	int run(int argc, char* argv[])
	{
		Uint64 last_count = SDL_GetPerformanceCounter();
		Uint64 count_fre = SDL_GetPerformanceFrequency();

		while (!is_quit)
		{
			while (SDL_PollEvent(&event))
				on_input();

			Uint64 cur_count = SDL_GetPerformanceCounter();
			double delta = (double)((cur_count - last_count) / count_fre);
			last_count = cur_count;
			if (delta * 1000 < 1000.0 / 60)
				SDL_Delay(1000.0 / 60 - delta*1000);

			on_update(delta);

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			on_render();
			SDL_RenderPresent(renderer);
		}

		return 0;
	}
protected:
	GameManager()
	{
		init_assert(!SDL_Init(SDL_INIT_EVERYTHING),u8"SDL初始化失败");
		init_assert(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG), u8"SDL_image初始化失败");
		init_assert(Mix_Init(MIX_INIT_MP3), u8"mixer初始化失败");
		init_assert(!TTF_Init(), u8"ttf初始化失败");

		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

		window = SDL_CreateWindow(u8"村庄保卫战", SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
		init_assert(window, u8"窗口创建失败");

		
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
		init_assert(renderer, u8"渲染器创建失败");


	}
	//在创建游戏管理器的时候即完成游戏的初始启动
	~GameManager()
	{
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		
		TTF_Quit();
		Mix_Quit();
		IMG_Quit();
		SDL_Quit();
	}
	//在游戏管理器销毁的时候即解放一切游戏资源
private:
	bool is_quit = false;
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

private:
	void on_input()
	{

	}

	void on_update(double delta)
	{

	}

	void on_render()
	{

	}
};