#pragma once
#include"resources_manager.h"
#include"tower_manager.h"
#include"coin_manager.h"
#include"ui_helpers.h"

#include<functional>
#include<string>
#include<vector>

class UpgradePanel
{
public:
	typedef std::function<void(SDL_Point)> UpgradeRequestCallback;
	typedef std::function<void(SDL_Point, TowerSpecialization)> SpecializationRequestCallback;

	void set_on_upgrade_requested(UpgradeRequestCallback callback)
	{
		on_upgrade_requested = callback;
	}

	void set_on_specialization_requested(SpecializationRequestCallback callback)
	{
		on_specialization_requested = callback;
	}

	void show()
	{
		visible = true;
		hovered_index = -1;
	}

	void hide()
	{
		visible = false;
		hovered_index = -1;
		clear_tower_snapshot();
	}

	void set_tower_snapshot(TowerType type, int level, TowerSpecialization specialization, double coin)
	{
		snapshot.valid = true;
		snapshot.type = type;
		snapshot.level = level;
		snapshot.specialization = specialization;
		snapshot.coin = coin;
	}

	void clear_tower_snapshot()
	{
		snapshot.valid = false;
	}

	void set_idx_tile(const SDL_Point& idx)
	{
		point_selected = idx;
	}

	void set_center_pos(const SDL_Point& pos)
	{
		center_pos = pos;
		update_layout();
	}

	void on_update(SDL_Renderer*)
	{
	}

	void on_render(SDL_Renderer* renderer)
	{
		if (!visible)
			return;

		Tower* tower = TowerManager::instance()->find_tower(point_selected);
		if (!tower && !snapshot.valid)
			return;

		TowerType type = tower ? tower->get_tower_type() : snapshot.type;
		int level = tower ? tower->get_level() : snapshot.level;
		TowerSpecialization specialization = tower ? tower->get_specialization() : snapshot.specialization;

		UI::draw_texture(renderer, upgrade_panel_texture(), panel_rect);

		UI::draw_text(renderer, tower_name(type) + " Lv." + std::to_string(level + 1),
			panel_rect.x + 62, panel_rect.y + 15, color_title, ResID::Font_Small);
		render_tower_icon(renderer, type, level, specialization, { panel_rect.x + 20, panel_rect.y + 11, 36, 36 });

		render_upgrade_button(renderer, type, level, specialization);
		render_spec_button(renderer, type, level, specialization, 1);
		render_spec_button(renderer, type, level, specialization, 2);
	}

	void on_input(const SDL_Event& event)
	{
		if (!visible)
			return;

		if (event.type == SDL_MOUSEMOTION)
		{
			SDL_Point cursor = { event.motion.x, event.motion.y };
			hovered_index = -1;
			for (size_t i = 0; i < button_rects.size(); i++)
				if (SDL_PointInRect(&cursor, &button_rects[i]))
					hovered_index = (int)i;
		}
		else if (event.type == SDL_MOUSEBUTTONUP)
		{
			Tower* tower = TowerManager::instance()->find_tower(point_selected);
			bool has_context = tower || snapshot.valid;
			TowerType type = tower ? tower->get_tower_type() : snapshot.type;
			if (has_context && hovered_index == 0 && on_upgrade_requested)
				on_upgrade_requested(point_selected);
			else if (has_context && hovered_index == 1 && on_specialization_requested)
				on_specialization_requested(point_selected, first_spec(type));
			else if (has_context && hovered_index == 2 && on_specialization_requested)
				on_specialization_requested(point_selected, second_spec(type));
			visible = false;
		}
		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
		{
			visible = false;
		}
	}

private:
	void update_layout()
	{
		panel_rect = { center_pos.x + 38, center_pos.y - 110, 420, 220 };
		button_rects =
		{
			{ panel_rect.x + 18, panel_rect.y + 52, 384, 50 },
			{ panel_rect.x + 18, panel_rect.y + 116, 184, 72 },
			{ panel_rect.x + 218, panel_rect.y + 116, 184, 72 }
		};
	}

	void render_upgrade_button(SDL_Renderer* renderer, TowerType type, int level, TowerSpecialization specialization)
	{
		SDL_Rect rect = button_rects[0];
		double cost = TowerManager::instance()->get_upgrade_tower_cost(type, level);
		bool enabled = cost > 0 && cost <= get_available_coin();
		render_disabled_overlay(renderer, rect, enabled);
		render_tower_icon(renderer, type, level, specialization, { rect.x + 13, rect.y + 8, 38, 38 });
		std::string label = cost < 0 ? "MAX" : std::string(u8"升级 $") + std::to_string((int)cost);
		UI::draw_text(renderer, label, rect.x + 62, rect.y + 3,
			enabled ? color_primary : color_disabled, ResID::Font_Small);
		UI::draw_text(renderer, enabled ? u8"提升当前防御塔等级" : u8"金币不足或已满级", rect.x + 62, rect.y + 26,
			enabled ? color_secondary : color_disabled, ResID::Font_Small);
	}

	void render_spec_button(SDL_Renderer* renderer, TowerType type, int level, TowerSpecialization current_specialization, int slot)
	{
		SDL_Rect rect = button_rects[slot];
		TowerSpecialization spec = slot == 1 ? first_spec(type) : second_spec(type);
		bool enabled = can_choose_specialization(level, current_specialization);
		render_disabled_overlay(renderer, rect, enabled);
		render_spec_icon(renderer, spec, { rect.x + 8, rect.y + 8, 40, 40 });
		UI::draw_text(renderer, spec_name(spec), rect.x + 56, rect.y + 7,
			enabled ? color_primary : color_disabled, ResID::Font_Small);
		UI::draw_text(renderer, enabled ? u8"专精" : spec_state_text(current_specialization), rect.x + 56, rect.y + 31,
			enabled ? color_secondary : color_disabled, ResID::Font_Small);
	}

	ResID upgrade_panel_texture() const
	{
		switch (hovered_index)
		{
		case 0: return ResID::Tex_UITowerUpgradePanelHoverUpgrade;
		case 1: return ResID::Tex_UITowerUpgradePanelHoverSpecLeft;
		case 2: return ResID::Tex_UITowerUpgradePanelHoverSpecRight;
		default: return ResID::Tex_UITowerUpgradePanel;
		}
	}

	void render_disabled_overlay(SDL_Renderer* renderer, const SDL_Rect& rect, bool enabled)
	{
		if (enabled)
			return;
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 18, 20, 24, 115);
		SDL_RenderFillRect(renderer, &rect);
	}

	std::string tower_name(TowerType type) const
	{
		switch (type)
		{
		case TowerType::Archer: return "Archer";
		case TowerType::Axeman: return "Axeman";
		case TowerType::Gunner: return "Gunner";
		case TowerType::Barracks: return "Barracks";
		}
		return "Tower";
	}

	void render_tower_icon(SDL_Renderer* renderer, TowerType type, int level, TowerSpecialization specialization, const SDL_Rect& dst)
	{
		switch (type)
		{
		case TowerType::Archer:
			render_sheet_icon(renderer, ResID::Tex_Archer, 3, 8, 0, dst);
			break;
		case TowerType::Axeman:
			render_sheet_icon(renderer, ResID::Tex_Axeman, 3, 8, 0, dst);
			break;
		case TowerType::Gunner:
			render_sheet_icon(renderer, ResID::Tex_Gunner, 4, 8, 0, dst);
			break;
		case TowerType::Barracks:
			render_sheet_icon(renderer, barracks_texture(level, specialization), 3, 8, 0, dst);
			break;
		}
	}

	ResID barracks_texture(int level, TowerSpecialization specialization) const
	{
		if (specialization == TowerSpecialization::BarracksShieldWall)
			return ResID::Tex_BarracksShieldWall;
		if (specialization == TowerSpecialization::BarracksVanguard)
			return ResID::Tex_BarracksVanguard;
		if (level >= 6)
			return ResID::Tex_BarracksLv3;
		if (level >= 3)
			return ResID::Tex_BarracksLv2;
		return ResID::Tex_BarracksLv1;
	}

	void render_spec_icon(SDL_Renderer* renderer, TowerSpecialization spec, const SDL_Rect& dst)
	{
		UI::draw_texture(renderer, spec_icon(spec), dst);
	}

	ResID spec_icon(TowerSpecialization spec) const
	{
		switch (spec)
		{
		case TowerSpecialization::ArcherRapidFire: return ResID::Tex_UISpecArcherRapidFire;
		case TowerSpecialization::ArcherPiercingArmor: return ResID::Tex_UISpecArcherPiercingArmor;
		case TowerSpecialization::AxemanStrongSlow: return ResID::Tex_UISpecAxemanStrongSlow;
		case TowerSpecialization::AxemanCleave: return ResID::Tex_UISpecAxemanCleave;
		case TowerSpecialization::GunnerBarrage: return ResID::Tex_UISpecGunnerBarrage;
		case TowerSpecialization::GunnerShotgun: return ResID::Tex_UISpecGunnerShotgun;
		case TowerSpecialization::BarracksShieldWall: return ResID::Tex_UISpecBarracksShieldWall;
		case TowerSpecialization::BarracksVanguard: return ResID::Tex_UISpecBarracksVanguard;
		default: return ResID::Tex_UISpecArcherRapidFire;
		}
	}

	void render_sheet_icon(SDL_Renderer* renderer, ResID res_id, int columns, int rows, int frame, const SDL_Rect& dst)
	{
		SDL_Texture* texture = ResourcesManager::instance()->get_texture_pool().find(res_id)->second;
		int tex_width = 0, tex_height = 0;
		SDL_QueryTexture(texture, nullptr, nullptr, &tex_width, &tex_height);
		SDL_Rect src = { (frame % columns) * (tex_width / columns), (frame / columns) * (tex_height / rows), tex_width / columns, tex_height / rows };
		SDL_RenderCopy(renderer, texture, &src, &dst);
	}

	TowerSpecialization first_spec(TowerType type) const
	{
		switch (type)
		{
		case TowerType::Archer: return TowerSpecialization::ArcherRapidFire;
		case TowerType::Axeman: return TowerSpecialization::AxemanStrongSlow;
		case TowerType::Gunner: return TowerSpecialization::GunnerBarrage;
		case TowerType::Barracks: return TowerSpecialization::BarracksShieldWall;
		}
		return TowerSpecialization::None;
	}

	TowerSpecialization second_spec(TowerType type) const
	{
		switch (type)
		{
		case TowerType::Archer: return TowerSpecialization::ArcherPiercingArmor;
		case TowerType::Axeman: return TowerSpecialization::AxemanCleave;
		case TowerType::Gunner: return TowerSpecialization::GunnerShotgun;
		case TowerType::Barracks: return TowerSpecialization::BarracksVanguard;
		}
		return TowerSpecialization::None;
	}

	std::string spec_name(TowerSpecialization spec) const
	{
		switch (spec)
		{
		case TowerSpecialization::ArcherRapidFire: return u8"速射";
		case TowerSpecialization::ArcherPiercingArmor: return u8"破甲";
		case TowerSpecialization::AxemanStrongSlow: return u8"强缓";
		case TowerSpecialization::AxemanCleave: return u8"顺劈";
		case TowerSpecialization::GunnerBarrage: return u8"炮幕";
		case TowerSpecialization::GunnerShotgun: return u8"霰弹";
		case TowerSpecialization::BarracksShieldWall: return u8"盾墙";
		case TowerSpecialization::BarracksVanguard: return u8"先锋";
		default: return "-";
		}
	}

	bool can_choose_specialization(int level, TowerSpecialization specialization) const
	{
		return level >= specialization_level && specialization == TowerSpecialization::None;
	}

	double get_available_coin() const
	{
		return snapshot.valid ? snapshot.coin : CoinManager::instance()->get_current_coin_num();
	}

	std::string spec_state_text(TowerSpecialization specialization) const
	{
		if (specialization != TowerSpecialization::None)
			return u8"已选择";
		return u8"Lv.3";
	}

	struct TowerSnapshot
	{
		bool valid = false;
		TowerType type = TowerType::Archer;
		int level = 0;
		TowerSpecialization specialization = TowerSpecialization::None;
		double coin = 0;
	};

	bool visible = false;
	int hovered_index = -1;
	SDL_Point point_selected = { 0, 0 };
	SDL_Point center_pos = { 0, 0 };
	SDL_Rect panel_rect = { 0, 0, 0, 0 };
	std::vector<SDL_Rect> button_rects;
	TowerSnapshot snapshot;
	UpgradeRequestCallback on_upgrade_requested;
	SpecializationRequestCallback on_specialization_requested;
	static const int specialization_level = 2;
	const SDL_Color color_title = { 54, 38, 24, 255 };
	const SDL_Color color_primary = { 44, 31, 20, 255 };
	const SDL_Color color_secondary = { 37, 74, 48, 255 };
	const SDL_Color color_disabled = { 94, 80, 65, 255 };
};
