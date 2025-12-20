#pragma once
#include"enemy_type.h"

#include<vector>
class Wave
{
public:
	Wave() = default;
	~Wave() = default;
public:
	struct SpawnEvent
	{
		double interval = 0;
		int point = 1;
		EnemyType enemy_type = EnemyType::Silm;
	};
public:
	double interval = 0;
	double rewards = 0;
	std::vector<SpawnEvent> spawn_event_list;
};
