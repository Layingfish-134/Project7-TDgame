#pragma once
#include"manager.h"
#include"bullet.h"
#include"bullet_type.h"
#include"arrow_bullet.h"
#include"axe_bullet.h"
#include"shell_bullet.h"


#include<vector>
class BulletManager : public Manager<BulletManager>
{
	friend class Manager<BulletManager>;
public:
	typedef std::vector<Bullet*> BulletList;

public:
	void on_update(double delta)
	{
		for (Bullet* bullet : bullet_list)
		{
			bullet->on_update(delta);
		}

		bullet_list.erase(std::remove_if(
		bullet_list.begin(), bullet_list.end(), 
			[](const Bullet* bullet)
			{
				bool deletable = bullet->can_remove();
				if (deletable)
					delete bullet;
				return deletable;
			}),bullet_list.end());
	}

	void on_render(SDL_Renderer* renderer)
	{
		for (Bullet* bullet : bullet_list)
			bullet->on_render(renderer);
	}

public:
	void fire_bullet(BulletType type,Vector2 position,Vector2 velocity,double damage)//用于生成子弹对象
	{
		Bullet* bullet = nullptr;
		switch (type)
		{
		case Arrow:
			bullet = new ArrowBullet();
			break;
		case Axe:
			bullet = new AxeBullet();
			break;
		case Shell:
			bullet = new ShellBullet();
			break;
		default:
			bullet = new ArrowBullet();
			break;
		}

		bullet->set_position(position);
		bullet->set_damage(damage);
		bullet->set_velocity(velocity);
	}

public:
	BulletList& get_bullet_list()
	{
		return bullet_list;
	}

public:
	BulletManager() = default;
	~BulletManager()
	{
		for (Bullet* bullet : bullet_list)
			delete bullet;
	}

private:
	BulletList bullet_list;
};

