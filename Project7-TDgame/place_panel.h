#pragma once
#include"panel.h"
#include"tower_manager.h"
#include"coin_manager.h"


#include<SDL2_gfxPrimitives.h>
class PlacePanel : public Panel
{
public:
	PlacePanel()
	{
		const ResourcesManager::TexturePool& tex_pool = ResourcesManager::instance()->get_texture_pool();

		tex_idle = tex_pool.find(ResID::Tex_UIPlaceIdle)->second;
		tex_highlight_top = tex_pool.find(ResID::Tex_UIPlaceHoveredTop)->second;
		tex_highlight_left = tex_pool.find(ResID::Tex_UIPlaceHoveredLeft)->second;
		tex_highlight_right = tex_pool.find(ResID::Tex_UIPlaceHoveredRight)->second;

	}
	~PlacePanel() = default;
public:
	void on_update(SDL_Renderer* renderer) override
	{
		static TowerManager* tower_manager = TowerManager::instance();
		val_top = (int)tower_manager->get_place_tower_cost(TowerType::Axeman);
		val_left = (int)tower_manager->get_place_tower_cost(TowerType::Archer);
		val_right = (int)tower_manager->get_place_tower_cost(TowerType::Gunner);

		range_top = (int)tower_manager->get_tower_damage_range(TowerType::Axeman) * SIZE_TILE;
		range_left = (int)tower_manager->get_tower_damage_range(TowerType::Archer) * SIZE_TILE;
		range_right = (int)tower_manager->get_tower_damage_range(TowerType::Gunner) * SIZE_TILE;

		Panel::on_update(renderer);
	}

	void on_render(SDL_Renderer* renderer) override
	{
		if (!visiable)
			return;

		int region = 0;
		switch (hovered_tar)
		{
		case Panel::HoveredTarget::Top:
			region = range_top;
			break;
		case Panel::HoveredTarget::Left:
			region = range_left;
			break;
		case Panel::HoveredTarget::Right:
			region = range_right;
			break;
		}

		if (region > 0)
		{
			filledCircleRGBA(renderer, center_pos.x, center_pos.y, region,
				color_range_circle_content.r, color_range_circle_content.g, color_range_circle_content.b,
				color_range_circle_content.a);

			aacircleRGBA(renderer, center_pos.x, center_pos.y, region,
				color_range_circle_frame.r, color_range_circle_frame.g, color_range_circle_frame.b,
				color_range_circle_frame.a);
		}
		Panel::on_render(renderer);
	}
protected:
	void on_click_top() override
	{
		static CoinManager* coin_manager = CoinManager::instance();

		if (val_top <= coin_manager->get_current_coin_num())
		{
			TowerManager::instance()->place_tower(TowerType::Axeman, point_selected);
			coin_manager->decrease_coin(val_top);
		}

	}
	void on_click_left() override
	{
		static CoinManager* coin_manager = CoinManager::instance();

		if (val_left <= coin_manager->get_current_coin_num())
		{
			TowerManager::instance()->place_tower(TowerType::Archer, point_selected);
			//std::cout << "cost archer: " << val_left << std::endl;
			coin_manager->decrease_coin(val_left);
		}

	}
	void on_click_right() override
	{
		static CoinManager* coin_manager = CoinManager::instance();

		if (val_right <= coin_manager->get_current_coin_num())
		{
			TowerManager::instance()->place_tower(TowerType::Gunner, point_selected);
			coin_manager->decrease_coin(val_right);
		}

	}
	
private:
	int range_top = 0; int range_left = 0; int range_right = 0;
	const SDL_Color color_range_circle_frame = { 30,80,162,175 };
	const SDL_Color color_range_circle_content = { 0,149,217,75 };

};
