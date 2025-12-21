#pragma once
#include"manager.h"
#include"config_manager.h"
#include"resources_manager.h"

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

		ConfigManager* config = ConfigManager::instance();
		init_assert(config->load_game_config("config.json"), u8"游戏配置加载失败");
		init_assert(config->map.load_from_set("map.csv"), u8"游戏地图加载失败");
		init_assert(config->load_level_config("level.json"), u8"游戏关卡加载失败");

		window = SDL_CreateWindow(config->basic_template.window_title.c_str(), SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, 
			config->basic_template.window_width, config->basic_template.window_height, SDL_WINDOW_SHOWN);
		init_assert(window, u8"窗口创建失败");

		
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
		init_assert(renderer, u8"渲染器创建失败");

		//ResourcesManager* resources_manager = ResourcesManager::instance();
		init_assert(ResourcesManager::instance()->load_from_file(renderer), u8"游戏资源加载失败");

		init_assert(generate_tile_map_texture(), u8"生成瓦片地图纹理失败");

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

	SDL_Texture* tex_tile_map;

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
		ConfigManager* config = ConfigManager::instance();
		const SDL_Rect& rect_tile_map = config->rect_tile_map;
		SDL_RenderCopy(renderer, tex_tile_map, nullptr, &rect_tile_map);
	}

private:
	bool generate_tile_map_texture() //从瓦片库和地图数据得到实际的瓦片地图纹理
	{
		const Map& map = ConfigManager::instance()->map;
		const Tilemap& tile_map = map.get_tile_map();//瓦片地图数据
		SDL_Texture* tex_tile_set = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Tileset)->second;//瓦片库
		SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;//瓦片纹理绘制区域

		//得到瓦片库的方格数目
		int width_tex_tile_set, height_tex_tile_set;
		SDL_QueryTexture(tex_tile_set, nullptr, nullptr, &width_tex_tile_set, &height_tex_tile_set);
		int num_tile_single_line = (int)std::ceil((double)(width_tex_tile_set) / SIZE_TILE);


		//创建瓦片地图的纹理
		int width_tex_tile_map = map.get_width() * SIZE_TILE;
		int height_tex_tile_map = map.get_height() * SIZE_TILE;
		tex_tile_map = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET
			, width_tex_tile_map, height_tex_tile_map);
		if (!tex_tile_map) return false;

		//计算瓦片地图的绘制区域
		ConfigManager* config = ConfigManager::instance();
		rect_tile_map.x = (config->basic_template.window_width - width_tex_tile_map) / 2;
		rect_tile_map.y = (config->basic_template.window_height - height_tex_tile_map) / 2;
		rect_tile_map.w = width_tex_tile_map;
		rect_tile_map.h = height_tex_tile_map;


		//调整SDL渲染设置
		SDL_SetTextureBlendMode(tex_tile_map, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(renderer, tex_tile_map);

		for (int y = 0; y < map.get_height(); y++)
		{
			for (int x = 0; x < map.get_width(); x++)
			{
				const Tile& cur_tile = tile_map[y][x];
				const SDL_Rect rect_tile_dst =
				{
					x*SIZE_TILE,y*SIZE_TILE,
					SIZE_TILE,SIZE_TILE
				};
				SDL_Rect rect_tile_src =
				{
					(cur_tile.terrain % num_tile_single_line) * SIZE_TILE,
					(cur_tile.terrain / num_tile_single_line) * SIZE_TILE,
					SIZE_TILE,
					SIZE_TILE
				};

				SDL_RenderCopy(renderer, tex_tile_set, &rect_tile_src, &rect_tile_dst);

				if (cur_tile.decoration >= 0)
				{
					rect_tile_src =
					{
						(cur_tile.decoration % num_tile_single_line) * SIZE_TILE,
						(cur_tile.decoration / num_tile_single_line) * SIZE_TILE,
						SIZE_TILE,
						SIZE_TILE
					};
					SDL_RenderCopy(renderer, tex_tile_set, &rect_tile_src, &rect_tile_dst);
				}
			}
		}

		const SDL_Point& idx_home = map.get_home_idx();
		const SDL_Rect rect_home_dst = 
		{
              idx_home.x*SIZE_TILE,idx_home.y*SIZE_TILE,
			  SIZE_TILE,SIZE_TILE
		};

		SDL_RenderCopy(renderer,
			ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Home)->second, nullptr, &rect_home_dst);


		SDL_SetRenderTarget(renderer, nullptr);
		return true;
	}
};