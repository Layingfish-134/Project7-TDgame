#pragma once
#include"manager.h"
#include"config_manager.h"
#include"resources_manager.h"

#include<SDL_mixer.h>
class HomeManager : public Manager<HomeManager>
{
public:
	HomeManager()
	{
		num_hp = ConfigManager::instance()->num_init_hp;
	}
	~HomeManager() = default;
public:
	double get_home_hp()
	{
		return num_hp;
	}

	void decrease_hp(double val)
	{
		num_hp -= val;

		if (num_hp < 0)
			num_hp = 0;

		static const ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();

		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_HomeHurt)->second, 0);
	}
private:
	double num_hp;
};

