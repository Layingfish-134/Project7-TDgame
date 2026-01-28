#pragma once
#include"manager.h"
#include"config_manager.h"
#include"resources_manager.h"
#include"enemy_manager.h"
#include"wave_manager.h"
#include"tower_manager.h"
#include"bullet_manager.h"
#include"player_manager.h"
#include"Statusbar.h"
#include"panel.h"
#include"place_panel.h"
#include"upgrade_panel.h"
#include"bannner.h"

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


		//TowerManager::instance()->place_tower(TowerType::Archer, { 5,0 });

		Mix_FadeInMusic(ResourcesManager::instance()->get_music_pool().find(ResID::Music_BGM)->second, -1, 1500);

		Uint64 last_counter = SDL_GetPerformanceCounter();
		const Uint64 counter_freq = SDL_GetPerformanceFrequency();

		while (!is_quit)
		{
			while (SDL_PollEvent(&event))
				on_input();

			Uint64 current_counter = SDL_GetPerformanceCounter();
			double delta = (double)(current_counter - last_counter) / counter_freq;
			last_counter = current_counter;
			if (delta * 1000 < 1000.0 / 60)
				SDL_Delay((Uint32)(1000.0 / 60 - delta * 1000));

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
		init_assert(config->map.load("map.csv"), u8"游戏地图加载失败");
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

		status_bar.set_position(15, 15);


		place_panel = new PlacePanel();
		upgrade_panel = new UpgradePanel();
		banner = new Banner();

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
	StatusBar status_bar;
	PlacePanel* place_panel = nullptr;
	UpgradePanel* upgrade_panel = nullptr;

	SDL_Texture* tex_tile_map = nullptr;

	Banner* banner = nullptr;
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
		static SDL_Point position_center;
		static SDL_Point idx_tile_selected;
		static ConfigManager* config_manager = ConfigManager::instance();


		switch (event.type)
		{
		case SDL_QUIT:
			is_quit = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (config_manager->is_game_over)
				break;
			if (get_cursor_idx_tile(idx_tile_selected, event.motion.x, event.motion.y))
			{
				get_selected_tile_center_position(position_center, idx_tile_selected);

				//std::cout << position_center.x << " " << position_center.y;
				if (check_click_home(idx_tile_selected))
				{
					upgrade_panel->set_idx_tile(idx_tile_selected);
					upgrade_panel->set_center_pos(position_center);
					upgrade_panel->show();
				}
				else if (can_place_tower(idx_tile_selected))
				{
					place_panel->set_idx_tile(idx_tile_selected);
					place_panel->set_center_pos(position_center);
					place_panel->show();
				}
			}
			break;
		default:
			break;
		}

		if (!config_manager->is_game_over)
		{
			place_panel->on_input(event);
			upgrade_panel->on_input(event);
			PlayerManager::instance()->on_input(event);
		}
	}

	void on_update(double delta)
	{
		static bool is_game_over_last_tick = false;
		static ConfigManager* config_instance = ConfigManager::instance();

		if (!config_instance->is_game_over)
		{
			//std::cout << "时间更新" << std::endl;
			
			WaveManager::instance()->on_update(delta);
			EnemyManager::instance()->on_update(delta);
			BulletManager::instance()->on_update(delta);
			TowerManager::instance()->on_update(delta);
			CoinManager::instance()->on_update(delta);
			PlayerManager::instance()->on_update(delta);

			place_panel->on_update(renderer);
			upgrade_panel->on_update(renderer);
			status_bar.on_update(renderer);

			return;
		}

		if (!is_game_over_last_tick && config_instance->is_game_over)//上一帧游戏没有结束但这一帧结束了，则当前为最后一帧
		{
			const ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();
			Mix_FadeOutMusic(1500);
			Mix_PlayChannel(-1, config_instance->is_game_win ?
				sound_pool.find(ResID::Sound_Win)->second : sound_pool.find(ResID::Sound_Loss)->second, 0);

		}
		is_game_over_last_tick = config_instance->is_game_over;

		banner->on_update(delta);
		if (banner->check_end_display())
		{
			is_quit = true;
		}
	}

	void on_render()
	{
		ConfigManager* config = ConfigManager::instance();
		const SDL_Rect& rect_tile_map = config->rect_tile_map;
		SDL_RenderCopy(renderer, tex_tile_map, nullptr, &rect_tile_map);

	
		EnemyManager::instance()->on_render(renderer);
		TowerManager::instance()->on_render(renderer);
		//std::cout << "should render bullet" << std::endl;
		BulletManager::instance()->on_render(renderer);
		CoinManager::instance()->on_render(renderer);
		PlayerManager::instance()->on_render(renderer);
		if (!config->is_game_over)
		{
			place_panel->on_render(renderer);
			upgrade_panel->on_render(renderer);
			status_bar.on_render(renderer);

			return;
		}

		int width_screen = 0;
		int height_screen = 0;
		SDL_GetWindowSizeInPixels(window, &width_screen, &height_screen);
		banner->set_position({ (double)width_screen / 2,(double)height_screen / 2 });
		banner->on_render(renderer);
	}

private:
	bool generate_tile_map_texture() //从瓦片库和地图数据得到实际的瓦片地图纹理
	{
		const Map& map = ConfigManager::instance()->map;
		const TileMap& tile_map = map.get_tile_map();//瓦片地图数据
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
					(cur_tile.terrian % num_tile_single_line) * SIZE_TILE,
					(cur_tile.terrian / num_tile_single_line) * SIZE_TILE,
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

private:
	bool check_click_home(SDL_Point& idx_tile_cursor_selected)
	{
		static const Map& map = ConfigManager::instance()->map;
		const SDL_Point& idx_home = map.get_home_idx();

		return idx_home.x == idx_tile_cursor_selected.x && idx_home.y == idx_tile_cursor_selected.y;
	}

	bool get_cursor_idx_tile(SDL_Point& idx_tile_cursor_selected, int screen_x, int screen_y)
	{
		static const Map& map = ConfigManager::instance()->map;
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		if (screen_x < rect_tile_map.x || screen_x  > rect_tile_map.x + rect_tile_map.w
			|| screen_y < rect_tile_map.y || screen_y  > rect_tile_map.y + rect_tile_map.h)
		{
			return false;
		}

		idx_tile_cursor_selected.x = std::min((int)((screen_x - rect_tile_map.x) / SIZE_TILE),(int)map.get_width() - 1);
		idx_tile_cursor_selected.y = std::min((int)((screen_y - rect_tile_map.y) / SIZE_TILE), (int)map.get_height() - 1);

		return true;
	}

	bool can_place_tower(const SDL_Point& idx_tile_selected) const
	{
		static const Map& map = ConfigManager::instance()->map;
		const Tile& selected_tile = map.get_tile_map()[idx_tile_selected.y][idx_tile_selected.x];

		return (selected_tile.decoration < 0) && (selected_tile.direction == Tile::Direction::None)
			&& (!selected_tile.has_tower);
	}

	void get_selected_tile_center_position(SDL_Point& pos,const SDL_Point& idx_tile_selected)
	{
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		pos.x = idx_tile_selected.x * SIZE_TILE + SIZE_TILE / 2;
		pos.y = idx_tile_selected.y * SIZE_TILE + SIZE_TILE / 2;

	}
};