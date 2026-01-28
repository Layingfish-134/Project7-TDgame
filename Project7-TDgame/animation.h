#pragma once

#include"timer.h"
#include<SDL.h>

#include<vector>
#include<functional>
#include<iostream>

class Animation
{
public:
	Animation()
	{
		timer.set_one_shot(false);
		timer.set_on_timeout(
			[&]()
			{
				idx_frame++;
				if (idx_frame >= rect_src_list.size())
				{
					idx_frame = is_loop ? 0 : rect_src_list.size() - 1;
					if (!is_loop && on_finished)
						on_finished();
				}
			});
	}
	~Animation() = default;

public:
	typedef std::function<void()> PlayCallBack;

public:
	void reset()
	{
		timer.restart();
		idx_frame = 0;
	}

	void set_frame_data(SDL_Texture* texture,int num_x,int num_y,std::vector<int> idx_list)
	{
		int width_tex = 0, height_tex = 0;
		SDL_QueryTexture(texture, nullptr, nullptr, &width_tex, &height_tex);
		width_frame = width_tex / num_x;
		height_frame = height_tex / num_y;
		this->texture = texture;

		rect_src_list.resize(idx_list.size());
		for (size_t i = 0; i < idx_list.size(); i++)
		{
			int idx = idx_list[i];
			SDL_Rect& rect = rect_src_list[i];

			rect.x = (idx % num_x) * width_frame;
			rect.y = (idx / num_x) * height_frame;
			rect.w = width_frame;
			rect.h = height_frame;
		}
	}
	//输入参数为纹理，纹理长宽，需要取得的纹理列表

	void set_loop(bool flag)
	{
		is_loop = flag;
	}

	void set_interval(double val)
	{
		timer.set_wait_time(val);
	}

	void set_on_finished(PlayCallBack on_finished)
	{
		this->on_finished = on_finished;
	}

	void on_update(double delta)
	{
		timer.on_update(delta);
	}

	void on_render(SDL_Renderer* renderer,const SDL_Point& pos_dst,double angle = 0) const
	{
		static SDL_Rect rect_dst;


		rect_dst.x = pos_dst.x;
		rect_dst.y = pos_dst.y;
		rect_dst.h = height_frame;
		rect_dst.w = width_frame;
		//std::cout << pos_dst.x << " " << pos_dst.y << std::endl;
		SDL_RenderCopyEx(renderer, texture, &rect_src_list[idx_frame], &rect_dst, 
			angle, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
		//std::cout << "动画绘制" << std::endl;

	}
private:
	Timer timer;
	bool is_loop = true;
	size_t idx_frame = 0;
	SDL_Texture* texture = nullptr;
	PlayCallBack on_finished;
	std::vector<SDL_Rect> rect_src_list;
	int width_frame = 0;
	int height_frame = 0;
};

