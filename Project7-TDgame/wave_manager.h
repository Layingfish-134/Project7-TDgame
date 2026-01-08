#pragma once
#include"manager.h"
#include"timer.h"
#include"config_manager.h"
#include"enemy_manager.h"
#include"coin_manager.h"

#include<vector>
#include<iostream>
class WaveManager : public Manager<WaveManager>
{
	friend class Manager<WaveManager>;
public:
	WaveManager()
	{
		std::vector<Wave>& wave_list = ConfigManager::instance()->wave_list;

		timer_start_wave.set_one_shot(true);
		timer_start_wave.set_wait_time(wave_list[0].interval);
		timer_start_wave.set_on_timeout([&]()
			{
				is_wave_start = true;
				timer_spawn_enemy.set_wait_time(wave_list[idx_wave].spawn_event_list[0].interval);
				//std::cout << "波次开始" << std::endl;
				timer_spawn_enemy.restart();
			});

		timer_spawn_enemy.set_one_shot(true);
		timer_spawn_enemy.set_on_timeout([&]()
			{
				std::vector<Wave::SpawnEvent>& spawn_event_list = wave_list[idx_wave].spawn_event_list;
				const Wave::SpawnEvent& spawn_event = spawn_event_list[idx_spawn_event];
				//std::cout << "生成敌人计时器触发" << std::endl;
				EnemyManager::instance()->spawn_enemy(spawn_event.enemy_type, spawn_event.point);
				//生成这一个事件的敌人

				idx_spawn_event++;
				//累加生成事件索引
				if (idx_spawn_event >= spawn_event_list.size())
				{
					is_cur_wave_spawned_last_enemy = true;
					return;
				}
				//本波次的生成事件已经全部结束

				timer_spawn_enemy.set_wait_time(spawn_event_list[idx_spawn_event].interval);
				timer_spawn_enemy.restart();
				//用下一次生成事件的数据重置定时器
			});
	}
	~WaveManager() = default;
public:
	void on_update(double delta)
	{
		static ConfigManager* config_instance = ConfigManager::instance();


		if (config_instance->is_game_over)
			return;
		//游戏已经结束，不做更新

		if (!is_wave_start)
			timer_start_wave.on_update(delta);
		else
			timer_spawn_enemy.on_update(delta);

		if (is_cur_wave_spawned_last_enemy && EnemyManager::instance()->check_enemy_all_killed())
		{
			//当前波次最后一个敌人已经生成并且敌人已经被清空，意味着当前波次结束,
			const std::vector<Wave>& wave_list = config_instance->wave_list;

			CoinManager::instance()->increase_coin(wave_list[idx_wave].rewards);
			//奖励金币
			idx_wave++;
			if (idx_wave >= wave_list.size())//说明所有波次清空，游戏胜利
			{
				config_instance->is_game_over = true;
				config_instance->is_game_win = true;
			}
			else //游戏尚未结束，用下一个波次的数据初始化
			{
				is_wave_start = false;
				is_cur_wave_spawned_last_enemy = false;
				idx_spawn_event = 0;

				timer_start_wave.set_wait_time(wave_list[idx_wave].interval);
				timer_start_wave.restart();
			}
		}
	}
private:
	//游戏包含多个wave，一个wave包含多个spawn
	int idx_wave = 0;
	int idx_spawn_event = 0;
	Timer timer_start_wave;
	Timer timer_spawn_enemy;
	bool is_wave_start = false; //标志这一个波次开始
	bool is_cur_wave_spawned_last_enemy = false; //标志一个波次的最后一个敌人生成完毕
};

