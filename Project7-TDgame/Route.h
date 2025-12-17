#pragma once

#include"Tile.h"
#include<SDL.h>
#include<vector>


class Route
{

public:
	typedef std::vector<SDL_Point> IdxList;
public:
	Route() = default;
	Route(const Tilemap& tilemap, SDL_Point idx_origin)
	{
		SDL_Point idx_next = idx_origin;
		size_t wid_map = tilemap[0].size();
		size_t hei_map = tilemap.size();
		//通过tilemap遍历得到路径数组
		while (true)
		{
			if (idx_next.x >= wid_map || idx_next.y >= hei_map)
				break;
			//判定索引出界
			if (tile_already_exist(idx_next))
				break;
			//判定循环路径--不允许存在

			idx_list.push_back(idx_next);

			const Tile& tile = tilemap[idx_next.y][idx_next.x];
			if (tile.special_flag == 0)
				break;

			//得到下一个路径
			bool is_next_exist = true;
			switch (tile.direction)
			{
			case Tile::Direction::Up:
				idx_next.y--;
				break;
			case Tile::Direction::Down:
				idx_next.y++;
				break;
			case Tile::Direction::Left:
				idx_next.x--;
				break;
			case Tile::Direction::Right:
				idx_next.x++;
				break;
			default:
				is_next_exist = false;	
				break;
			}

			if (!is_next_exist)
				break;

		}
	}
	~Route() = default;

public:
	const IdxList& get_route() const
	{
		return idx_list;
	}
private:
	IdxList idx_list;
	
private:
	bool tile_already_exist(SDL_Point idx)
	{
		for (int i = 0; i < idx_list.size(); i++)
		{
			if (idx.x == idx_list[i].x && idx.y == idx_list[i].y)
			{
				return	true;
			}
		}

		return false;
	}
};