#pragma once
#include"bullet.h"
#include"resources_manager.h"


class ShellBullet : public Bullet
{
public:
	ShellBullet()
	{
		static SDL_Texture* tex_shell = ResourcesManager::instance()->
			get_texture_pool().find(ResID::Tex_BulletShell)->second;

		static SDL_Texture* tex_explode = ResourcesManager::instance()->
			get_texture_pool().find(ResID::Tex_EffectExplode)->second;


		static std::vector<int> idx_list_shell = { 0,1 };
		static std::vector<int> idx_list_explode = {0,1,2,3,4};

		animation.set_loop(true);
		animation.set_interval(0.1);
		animation.set_frame_data(tex_shell, 2, 1, idx_list_shell);

		animation.set_loop(false);
		animation.set_interval(0.1);
		animation.set_frame_data(tex_explode, 5, 1, idx_list_explode);
		animation.set_on_finished([&]()
			{
				this->set_invaild();
			});

		can_rotated = false;
		size.x = 48, size.y = 48;
		damage_range = 96;

	}
	~ShellBullet() = default;
public:
	void on_collide(Enemy* enemy) override
	{
		static const ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ShellHit)->second, 0);

		this->disable_collided();
	}

	void on_update(double delta) override
	{
		if (can_collided())
		{
			Bullet::on_update(delta);
			return;
		}
		
		animation_explode.on_update(delta);
	}

	void on_render(SDL_Renderer* renderer) override
	{
		if (can_collided())
		{
			Bullet::on_render(renderer);
			return;
		}
		
		static SDL_Point point_explode;
		point_explode.x = (int)(position.x - 96 / 2);
		point_explode.y = (int)(position.y - 96 / 2);
		animation_explode.on_render(renderer, point_explode);
	}
private:
	Animation animation_explode;
};

