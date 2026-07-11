#pragma once
#include"tower.h"
#include"resources_manager.h"

#include<algorithm>
#include<cmath>
#include<vector>

class BarracksTower : public Tower
{
public:
	BarracksTower()
	{
		tower_type = TowerType::Barracks;
		size.x = 48;
		size.y = 48;

		SDL_Texture* tex_barracks = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_BarracksLv1)->second;
		static std::vector<int> idx_list_idle_up = { 3, 4 };
		static std::vector<int> idx_list_idle_down = { 0, 1 };
		static std::vector<int> idx_list_idle_left = { 6, 7 };
		static std::vector<int> idx_list_idle_right = { 9, 10 };
		anim_idle_up.set_frame_data(tex_barracks, 3, 8, idx_list_idle_up);
		anim_idle_down.set_frame_data(tex_barracks, 3, 8, idx_list_idle_down);
		anim_idle_left.set_frame_data(tex_barracks, 3, 8, idx_list_idle_left);
		anim_idle_right.set_frame_data(tex_barracks, 3, 8, idx_list_idle_right);
		anim_fire_up.set_frame_data(tex_barracks, 3, 8, idx_list_idle_up);
		anim_fire_down.set_frame_data(tex_barracks, 3, 8, idx_list_idle_down);
		anim_fire_left.set_frame_data(tex_barracks, 3, 8, idx_list_idle_left);
		anim_fire_right.set_frame_data(tex_barracks, 3, 8, idx_list_idle_right);

		soldiers.resize(2);
	}

	void on_fire() override
	{
	}

	void on_update(double delta) override
	{
		Tower::on_update(delta);
		if (is_silenced())
			return;

		for (Soldier& soldier : soldiers)
			update_soldier(soldier, delta);
	}

	void on_render(SDL_Renderer* renderer) override
	{
		render_tower_body(renderer);
		for (const Soldier& soldier : soldiers)
			render_soldier(renderer, soldier);
	}

	struct SoldierSnapshot
	{
		double x = 0;
		double y = 0;
		double hp = 0;
		double max_hp = 0;
		int facing = 0;
		bool active = false;
		bool attacking = false;
		bool hurt = false;
		bool moving = false;
	};

	std::vector<SoldierSnapshot> get_soldier_snapshots() const
	{
		std::vector<SoldierSnapshot> snapshots;
		for (const Soldier& soldier : soldiers)
		{
			SoldierSnapshot snapshot;
			snapshot.x = soldier.position.x;
			snapshot.y = soldier.position.y;
			snapshot.hp = soldier.hp;
			snapshot.max_hp = soldier.max_hp;
			snapshot.facing = (int)soldier.facing;
			snapshot.active = soldier.active;
			snapshot.attacking = soldier.attacking;
			snapshot.hurt = soldier.hurt_timer > 0;
			snapshot.moving = soldier.moving;
			snapshots.push_back(snapshot);
		}
		return snapshots;
	}

private:
	struct Soldier
	{
		Vector2 position;
		double hp = 0;
		double max_hp = 0;
		double attack_cooldown = 0;
		double hurt_timer = 0;
		double enemy_hit_cooldown = 0;
		double respawn_timer = 0;
		Facing facing = Facing::Down;
		bool active = false;
		bool attacking = false;
		bool moving = false;
	};

	void update_soldier(Soldier& soldier, double delta)
	{
		if (!soldier.active)
		{
			soldier.respawn_timer -= delta;
			if (soldier.respawn_timer <= 0)
				spawn_soldier(soldier);
			return;
		}

		soldier.hurt_timer = (std::max)(0.0, soldier.hurt_timer - delta);
		soldier.enemy_hit_cooldown -= delta;
		soldier.attack_cooldown -= delta;
		soldier.moving = false;
		Enemy* target = find_soldier_target(soldier.position);
		if (!target)
		{
			soldier.attacking = false;
			move_soldier_toward(soldier, get_guard_position(), delta);
			return;
		}

		double distance = (target->get_position() - soldier.position).length();
		update_facing(soldier, target->get_position() - soldier.position);
		if (distance > get_soldier_attack_range())
		{
			soldier.attacking = false;
			move_soldier_toward(soldier, target->get_position(), delta);
			return;
		}

		soldier.attacking = true;
		target->apply_intercept(0.4);
		if (distance <= target->get_attack_range() && soldier.enemy_hit_cooldown <= 0)
		{
			soldier.hp -= (std::max)(1.0, target->get_attack_damage());
			soldier.hurt_timer = 0.18;
			soldier.enemy_hit_cooldown = 0.55;
		}
		if (soldier.attack_cooldown <= 0)
		{
			bool armor_break = get_specialization() == TowerSpecialization::BarracksVanguard;
			target->decrease_hp(get_attack_damage(), armor_break);
			soldier.attack_cooldown = get_attack_interval();
		}

		if (soldier.hp <= 0)
		{
			soldier.active = false;
			soldier.attacking = false;
			soldier.respawn_timer = get_respawn_time();
		}
	}

	void spawn_soldier(Soldier& soldier)
	{
		soldier.position = get_guard_position();
		soldier.max_hp = get_soldier_hp();
		soldier.hp = soldier.max_hp;
		soldier.attack_cooldown = 0;
		soldier.enemy_hit_cooldown = 0;
		soldier.hurt_timer = 0;
		soldier.facing = Facing::Down;
		soldier.active = true;
		soldier.attacking = false;
		soldier.moving = false;
	}

	Enemy* find_soldier_target(const Vector2& position)
	{
		Enemy* target = nullptr;
		double best_process = -1;
		double range = ConfigManager::instance()->barracks_template.view_range[get_level()] * SIZE_TILE;
		for (Enemy* enemy : EnemyManager::instance()->get_enemy_list())
		{
			if (enemy->can_remove())
				continue;
			if ((enemy->get_position() - position).length() > range)
				continue;
			if (enemy->get_route_process() > best_process)
			{
				target = enemy;
				best_process = enemy->get_route_process();
			}
		}
		return target;
	}

	void move_soldier_toward(Soldier& soldier, const Vector2& target, double delta)
	{
		Vector2 diff = target - soldier.position;
		if (diff.length() < 3)
			return;
		update_facing(soldier, diff);
		soldier.position += diff.normalize() * 95 * delta;
		soldier.moving = true;
	}

	void update_facing(Soldier& soldier, const Vector2& direction)
	{
		if (std::abs(direction.x) >= std::abs(direction.y))
			soldier.facing = direction.x >= 0 ? Facing::Right : Facing::Left;
		else
			soldier.facing = direction.y >= 0 ? Facing::Down : Facing::Up;
	}

	Vector2 get_guard_position() const
	{
		Vector2 guard = get_position();
		guard.y += 24;
		return guard;
	}

	double get_soldier_hp() const
	{
		double hp = 30 + get_level() * 8;
		if (get_specialization() == TowerSpecialization::BarracksShieldWall)
			hp *= 1.8;
		return hp;
	}

	double get_respawn_time() const
	{
		return get_specialization() == TowerSpecialization::BarracksVanguard ? 1.8 : 3.0;
	}

	double get_soldier_attack_range() const
	{
		return ConfigManager::instance()->barracks_template.soldier_attack_range[get_level()] * SIZE_TILE;
	}

	ResID get_tower_texture() const
	{
		if (get_specialization() == TowerSpecialization::BarracksShieldWall)
			return ResID::Tex_BarracksShieldWall;
		if (get_specialization() == TowerSpecialization::BarracksVanguard)
			return ResID::Tex_BarracksVanguard;
		if (get_level() >= 6)
			return ResID::Tex_BarracksLv3;
		if (get_level() >= 3)
			return ResID::Tex_BarracksLv2;
		return ResID::Tex_BarracksLv1;
	}

	void render_tower_body(SDL_Renderer* renderer) const
	{
		int frame = 0 + (int)((SDL_GetTicks64() / 180) % 2);
		render_sheet_frame_center(renderer, get_tower_texture(), 3, 8, frame, get_position().x, get_position().y, 48, 48);
		if (is_silenced())
		{
			SDL_Rect dst = { (int)(get_position().x - 12), (int)(get_position().y - 42), 24, 24 };
			SDL_RenderCopy(renderer, ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_UIStatusSilence)->second, nullptr, &dst);
		}
	}

	void render_soldier(SDL_Renderer* renderer, const Soldier& soldier) const
	{
		if (!soldier.active)
			return;
		ResID texture = ResID::Tex_BarracksSoldierIdle;
		if (soldier.hurt_timer > 0)
			texture = ResID::Tex_BarracksSoldierHit;
		else if (soldier.attacking)
			texture = ResID::Tex_BarracksSoldierAttack;
		else if (soldier.moving)
			texture = ResID::Tex_BarracksSoldierWalk;

		int frame = soldier_base_frame(soldier.facing) + (int)((SDL_GetTicks64() / 110) % 6);
		render_sheet_frame_center(renderer, texture, 6, 4, frame, soldier.position.x, soldier.position.y, 32, 32);
		if (soldier.attacking)
			render_soldier_attack_effect(renderer, soldier);
		render_soldier_hp_bar(renderer, soldier);
	}

	int soldier_base_frame(Facing facing) const
	{
		switch (facing)
		{
		case Facing::Down: return 0;
		case Facing::Up: return 6;
		case Facing::Right: return 12;
		case Facing::Left: return 18;
		}
		return 0;
	}

	void render_soldier_attack_effect(SDL_Renderer* renderer, const Soldier& soldier) const
	{
		Vector2 effect_pos = soldier.position;
		switch (soldier.facing)
		{
		case Facing::Down: effect_pos.y += 18; break;
		case Facing::Up: effect_pos.y -= 18; break;
		case Facing::Right: effect_pos.x += 20; break;
		case Facing::Left: effect_pos.x -= 20; break;
		}

		int frame = (int)((SDL_GetTicks64() / 70) % 6);
		render_sheet_frame_center(renderer, ResID::Tex_EffectSoldierSlash, 6, 1, frame, effect_pos.x, effect_pos.y, 32, 32);
	}

	void render_soldier_hp_bar(SDL_Renderer* renderer, const Soldier& soldier) const
	{
		if (soldier.max_hp <= 0 || soldier.hp >= soldier.max_hp)
			return;
		double ratio = (std::max)(0.0, (std::min)(soldier.hp / soldier.max_hp, 1.0));
		SDL_Rect rect = { (int)(soldier.position.x - 18), (int)(soldier.position.y - 26), (int)(36 * ratio), 5 };
		SDL_SetRenderDrawColor(renderer, 202, 232, 166, 255);
		SDL_RenderFillRect(renderer, &rect);
		rect.w = 36;
		SDL_SetRenderDrawColor(renderer, 85, 114, 91, 255);
		SDL_RenderDrawRect(renderer, &rect);
	}

	void render_sheet_frame_center(SDL_Renderer* renderer, ResID res_id, int columns, int rows, int frame, double x, double y, int width, int height) const
	{
		SDL_Texture* texture = ResourcesManager::instance()->get_texture_pool().find(res_id)->second;
		int tex_width = 0, tex_height = 0;
		SDL_QueryTexture(texture, nullptr, nullptr, &tex_width, &tex_height);
		SDL_Rect src = { (frame % columns) * (tex_width / columns), (frame / columns) * (tex_height / rows), tex_width / columns, tex_height / rows };
		SDL_Rect dst = { (int)(x - width / 2), (int)(y - height / 2), width, height };
		SDL_RenderCopy(renderer, texture, &src, &dst);
	}

	std::vector<Soldier> soldiers;
};
