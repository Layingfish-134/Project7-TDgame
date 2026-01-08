#pragma once
#include<vector>

#define SIZE_TILE 48
//一个瓦片占据48px

struct Tile
{
	enum class Direction
	{
		None = 0,
		Up,
		Down,
		Left,
		Right
	};
	int terrian = 0;
	int decoration = -1;
	int special_flag = -1;
	Direction direction = Direction::None;
	//绘图四层设置
	bool has_tower = false;
	//游戏逻辑设置
};

typedef std::vector<std::vector<Tile>> TileMap;
