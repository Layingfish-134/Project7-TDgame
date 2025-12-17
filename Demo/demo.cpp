#define	SDL_MAIN_HANDLED

#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include"cJSON.h"

#include<SDL.h>
#include<SDL_image.h>
#include<SDL_mixer.h>
#include<SDL_ttf.h>
#include<SDL2_gfxPrimitives.h>

void test_json()
{
	std::ifstream file("test.json");
	if (!file.good())
	{
		std::cout << "open file json failed" << std::endl;
		return;
	}
	std::stringstream str_stream;
	str_stream << file.rdbuf();
	file.close();

	cJSON* json_root = cJSON_Parse(str_stream.str().c_str());

	cJSON* json_name = cJSON_GetObjectItem(json_root, "name");
	cJSON* json_age = cJSON_GetObjectItem(json_root, "age");
	cJSON* json_pets = cJSON_GetObjectItem(json_root, "pets");

	std::cout << "name: " << json_name->valuestring << std::endl;
	std::cout << "age: " << json_age->valueint << std::endl;

	cJSON* item = nullptr;
	cJSON_ArrayForEach(item, json_pets)
	{
		std::cout << "\t" << item->valuestring << std::endl;
	}
}

void test_csv()
{
	std::ifstream file("TTTT.csv");
	if (!file.good())
	{
		std::cout << "open file csv failed" << std::endl;
		return;
	}
	std::string str_line;
	while (std::getline(file, str_line))
	{
		std::string str_grid;
		std::stringstream str_eam(str_line);
		while (std::getline(str_eam, str_grid, ','))
		{
			std::cout << str_grid << " ";
		}
		std::cout << std::endl;
	}
	file.close();
	return;
}

int main()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	Mix_Init(MIX_INIT_MP3);
	TTF_Init();

	//test_json();
	test_csv();

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	SDL_Window* window = SDL_CreateWindow(u8"你好世界",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
		1280,720,SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Surface* sur_ava = IMG_Load("avatar.jpg");
	SDL_Texture* tex_ava = SDL_CreateTextureFromSurface(renderer, sur_ava);

	TTF_Font* font = TTF_OpenFont("ipix.ttf", 32);
	SDL_Color text_color = { 255,255,255,255 };
	SDL_Surface* sur_text = TTF_RenderUTF8_Blended(font, u8"你好，咩咩", text_color);
	SDL_Texture* tex_text = SDL_CreateTextureFromSurface(renderer, sur_text);
	const int fps1 = 60;

	Mix_Music* music = Mix_LoadMUS("music.mp3");
	Mix_FadeInMusic(music, -1, 1500);

	SDL_Event event;
	bool is_quit = false;

	SDL_Point pos_cursor = { 0,0 };
	SDL_Rect rect_ava;
	SDL_Rect rect_text;
	//x,y为矩形左上角


	rect_ava.w = sur_ava->w;
	rect_ava.h = sur_ava->h;

	rect_text.w = sur_text->w;
	rect_text.h = sur_text->h;

	Uint64 last_count = SDL_GetPerformanceCounter();
	Uint64 count_fre = SDL_GetPerformanceFrequency();

	while (!is_quit)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				is_quit = true;
				break;
			case SDL_MOUSEMOTION:
				pos_cursor.x = event.motion.x;
				pos_cursor.y = event.motion.y;
				break;
			default:
				break;
			}
		}

		Uint64 current_count = SDL_GetPerformanceCounter();
		double delta = (double)((current_count - last_count) / count_fre);
		last_count = current_count;
		if (delta*1000 < 1000.0 / fps1)
			SDL_Delay(1000.0/fps1 - delta*1000);

		//数据处理
		rect_ava.x = pos_cursor.x;
		rect_ava.y = pos_cursor.y;

		rect_text.x = pos_cursor.x;
		rect_text.y = pos_cursor.y;

		//渲染绘图

		SDL_SetRenderDrawColor(renderer, 0, 0,0, 255);
		SDL_RenderClear(renderer);

		SDL_RenderCopy(renderer, tex_ava, nullptr, &rect_ava);
		SDL_RenderCopy(renderer, tex_text, nullptr, &rect_text);
		filledCircleRGBA(renderer, pos_cursor.x, pos_cursor.y, 50, 225, 0, 0, 200);
		SDL_RenderPresent(renderer);

	}
	return 0;
}