#pragma once
#include"panel.h"
#include"resources_manager.h"
#include"tower_manager.h"


class UpgradePanel : public Panel
{
public:
	UpgradePanel()
	{
		const ResourcesManager::TexturePool& tex_pool = ResourcesManager::instance()->get_texture_pool();

		tex_idle = tex_pool.find(ResID::Tex_UIUpgradeIdle)->second;
		tex_highlight_top = tex_pool.find(ResID::Tex_UIUpgradeHoveredTop)->second;
		tex_highlight_left = tex_pool.find(ResID::Tex_UIUpgradeHoveredLeft)->second;
		tex_highlight_right = tex_pool.find(ResID::Tex_UIUpgradeHoveredRight)->second;
	}
	~UpgradePanel();
public:
	void on_update(SDL_Renderer* renderer) override
	{
		static TowerManager* tower_manager = TowerManager::instance();
		val_top = (int)tower_manager->get_upgrade_tower_cost(TowerType::Axeman);
		val_left = (int)tower_manager->get_upgrade_tower_cost(TowerType::Archer);
		val_right = (int)tower_manager->get_upgrade_tower_cost(TowerType::Gunner);

		Panel::on_update(renderer);
	}

protected:
	void on_click_top() override
	{
		static CoinManager* coin_manager = CoinManager::instance();

		if (val_top > 0 && val_top < coin_manager->get_current_coin_num())
		{
			TowerManager::instance()->upgrade_tower(TowerType::Axeman);
			coin_manager->decrease_coin(val_top);
		}


	}
	void on_click_left() override
	{
		static CoinManager* coin_manager = CoinManager::instance();

		if (val_left > 0 && val_left < coin_manager->get_current_coin_num())
		{
			TowerManager::instance()->upgrade_tower(TowerType::Archer);
			coin_manager->decrease_coin(val_left);
		}

	}
	void on_click_right() override
	{
		static CoinManager* coin_manager = CoinManager::instance();

		if (val_right > 0 && val_right < coin_manager->get_current_coin_num())
		{
			TowerManager::instance()->upgrade_tower(TowerType::Gunner);
			coin_manager->decrease_coin(val_right);
		}

	}

private:

};

