#pragma once
#include"Tile.h"
#include"Route.h"
#include<SDL.h>

#include<fstream>
#include<sstream>
#include<string>
#include<iostream>
#include<unordered_map>

class Map
{
public:
	Map() = default;
	~Map() = default;

public:
	typedef std::unordered_map<int, Route> SpawnerRoutePool;
public:
	int get_width() const
	{
		if (tile_map.empty())
			return 0;

		return tile_map[0].size();
	}
	int get_height() const
	{
		if (tile_map.empty())
			return 0;

		return tile_map.size();
	}

	const Tilemap& get_tile_map() const
	{
		return tile_map;
	}

	const SDL_Point& get_home_idx() const
	{
		return idx_home;
	}

	const SpawnerRoutePool& get_route_pool() const
	{
		return spawner_route_pool;
	}

	void place_tower(const SDL_Point& idx)
	{
		tile_map[idx.y][idx.x].has_tower = true;
	}
private:
	//解析csv文件得到瓦片地图
	bool load_from_set(const std::string& path)
	{
		std::ifstream file(path);

		if (!file.good())
		{
			return false;
		}

		Tilemap tile_temp_map;
		//读取地图时的中间量

		int idx_x = -1;
		int idx_y = -1;
		//x标志表格列号，y标志表格行号
		//从file中读取行

		std::string str_line;
		while (std::getline(file, str_line))
		{
			//读取到行之后需要消除首尾空白字符
			str_line = trim_string(str_line);
			if (str_line.empty())
				continue;
			//跳过空行

			idx_x = -1;
			idx_y++;//行序列递增
			tile_temp_map.emplace_back();//创建空行
			//解析单行，以逗号为分隔符
			std::stringstream str_stream(str_line);
			std::string str_tile;//存储单个单元格内容
			while (std::getline(str_stream, str_tile, ','))
			{
				idx_x++;
				tile_temp_map[idx_y].emplace_back();
				//在y处，创建空列，也就是一个格子

				Tile& tile = tile_temp_map[idx_y].back();
				load_from_tile(tile, str_tile);
				//从字符串读取数据
			}
		}

		file.close();
		if (tile_temp_map.empty() || tile_temp_map[0].empty())
			return false;

		tile_map = tile_temp_map;
		//地图初始化完毕，构建洋流图缓存
		generate_map_cache();
		return true;

	}
private:
	void generate_map_cache()
	{
		for (int y = 0; y < get_height(); y++)
		{
			for (int x = 0; x < get_width(); x++)
			{
				const Tile& tile = tile_map[y][x];
				if (tile.special_flag < 0)
					continue;

				if (tile.special_flag == 0)
				{
					idx_home.x = x;
					idx_home.y = y;
				}
				else
				{
					Route temp_route(tile_map, { x,y });
					spawner_route_pool[tile.special_flag] = temp_route;
				}
			}
		}
	}
	//生成地图缓存
private:
	Tilemap tile_map;
	SDL_Point idx_home = { 0 };
	SpawnerRoutePool spawner_route_pool;

private:
	//工具函数
	std::string trim_string(const std::string& str)
	{
		size_t begin_not = str.find_first_not_of(" \t");
		if (begin_not == std::string::npos)
			return "";

		size_t end_not = str.find_last_not_of(" \t");
		size_t range_not = end_not - begin_not + 1;
		return str.substr(begin_not, range_not);
	}

	void load_from_tile(Tile& tile,const std::string& str)
	{
		//消除单元格内空白字符
		std::string str_tidy = trim_string(str);
		std::stringstream str_stream(str);
		std::string str_value;
		std::vector<int> values;
		while (std::getline(str_stream, str_value, '\\'))
		{
			int value = -1;
			try
			{
				value = std::stoi(str_value);
			}
			catch (const std::invalid_argument&)
			{
				value = -1;
			}
			values.push_back(value);
		}

		//考虑地图层数不全与单层数据不合法的问题
		tile.terrain = (values.size() < 1 || values[0] < 0) ?  0 : values[0];
		tile.decoration = (values.size() < 2 || values[1] < -1) ? -1 : values[1];
		tile.direction = (Tile::Direction)(values.size() < 3 || values[2] < 0 ? 0 : values[2]);
		tile.special_flag = (values.size() < 4 || values[3] < -1 ? -1 : values[3]);
		return;
	}
};
