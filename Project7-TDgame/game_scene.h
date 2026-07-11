#pragma once

#include "scene.h"
#include "scene_manager.h"
#include "config_manager.h"
#include "resources_manager.h"
#include "enemy_manager.h"
#include "wave_manager.h"
#include "tower_manager.h"
#include "bullet_manager.h"
#include "player_manager.h"
#include "Statusbar.h"
#include "place_panel.h"
#include "upgrade_panel.h"
#include "remove_panel.h"
#include "bannner.h"
#include "save_manager.h"
#include "network_manager.h"
#include "ui_helpers.h"

#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class GameScene : public Scene
{
public:
	GameScene(int level_id, bool network_mode = false, NetworkRole role = NetworkRole::Offline)
		: level_id(level_id), network_mode(network_mode), local_role(role)
	{
	}

	~GameScene() override
	{
		on_exit();
	}

	void on_enter(SDL_Renderer* renderer) override
	{
		renderer_cached = renderer;
		ConfigManager* config = ConfigManager::instance();
		const ConfigManager::LevelInfo* level = config->get_level_info(level_id);
		if (!level)
			level = config->get_level_info(1);

		config->reset_runtime_state();
		config->current_level_id = level ? level->id : 1;
		if (level)
		{
			config->map.load(level->map_path);
			config->load_level_config(level->wave_path);
		}
		else
		{
			config->map.load("map.csv");
			config->load_level_config("level.json");
		}

		generate_tile_map_texture(renderer);

		EnemyManager::instance()->clear();
		EnemyManager::instance()->reset_stats();
		BulletManager::instance()->clear();
		TowerManager::instance()->clear();
		CoinManager::instance()->reset();
		HomeManager::instance()->reset();
		WaveManager::instance()->reset();
		PlayerManager::instance()->reset();
		level_elapsed_time = 0;
		settlement_calculated = false;
		settlement_score = 0;
		settlement_stars = 0;
		settlement_home_hp = 0;
		settlement_coin = 0;
		settlement_defeated = 0;
		settlement_leaked = 0;
		balance_logged = false;
		debug_coin_added = 0;
		debug_waves_skipped = 0;
		debug_forced_result = "";

		status_bar.set_position(15, 15);
		place_panel = new PlacePanel();
		upgrade_panel = new UpgradePanel();
		remove_panel = new RemovePanel();
		banner = new Banner();
		place_panel->set_on_place_requested([this](TowerType type, SDL_Point idx_tile)
			{
				request_place_tower(type, idx_tile);
			});
		upgrade_panel->set_on_upgrade_requested([this](SDL_Point idx_tile)
			{
				request_upgrade_tower(idx_tile);
			});
		upgrade_panel->set_on_specialization_requested([this](SDL_Point idx_tile, TowerSpecialization spec)
			{
				request_specialization(idx_tile, spec);
			});
		remove_panel->set_on_remove_requested([this](SDL_Point idx_tile)
			{
				request_remove_tower(idx_tile);
			});

		const ResourcesManager::MusicPool& music_pool = ResourcesManager::instance()->get_music_pool();
		Mix_FadeInMusic(music_pool.find(ResID::Music_BGM)->second, -1, 1500);
	}

	void on_exit() override
	{
		if (exited)
			return;
		exited = true;

		if (tex_tile_map)
		{
			SDL_DestroyTexture(tex_tile_map);
			tex_tile_map = nullptr;
		}
		delete place_panel;
		delete upgrade_panel;
		delete remove_panel;
		delete banner;
		place_panel = nullptr;
		upgrade_panel = nullptr;
		remove_panel = nullptr;
		banner = nullptr;

		EnemyManager::instance()->clear();
		BulletManager::instance()->clear();
		TowerManager::instance()->clear();
		CoinManager::instance()->clear_props();
		if (network_mode)
			NetworkManager::instance()->reset();
	}

	void on_input(const SDL_Event& event) override
	{
		ConfigManager* config_manager = ConfigManager::instance();

		if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
		{
			return_after_game();
			return;
		}

		if (config_manager->is_game_over)
		{
			if (settlement_ready && (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONUP))
				return_after_game();
			return;
		}

		if (handle_debug_input(event))
			return;

		if (!network_mode && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
		{
			paused = !paused;
			if (paused)
				PlayerManager::instance()->set_move_state(false, false, false, false);
			return;
		}

		if (paused)
			return;

		if (network_mode && !NetworkManager::instance()->hosting())
		{
			if (can_control_towers())
				process_tower_input(event);
			if (can_control_player())
				process_network_player_input(event);
			return;
		}

		if (can_control_towers())
			process_tower_input(event);

		if (can_control_player())
		{
			if (network_mode)
				process_network_player_input(event);
			else
				PlayerManager::instance()->on_input(event);
		}
	}

	void on_update(double delta) override
	{
		ConfigManager* config = ConfigManager::instance();
		if (network_mode)
		{
			NetworkManager::instance()->on_update();
			if (!config->is_game_over)
				level_elapsed_time += delta;
			if (NetworkManager::instance()->hosting())
				process_host_network_messages();
			else
			{
				process_client_network_messages();
				if (!config->is_game_over)
				{
					place_panel->on_update(renderer_cached);
					upgrade_panel->on_update(renderer_cached);
					remove_panel->on_update(renderer_cached);
					status_bar.on_update(renderer_cached);
					client_snapshot_timeout += delta;
					if (!NetworkManager::instance()->is_connected() ||
						(remote_world.has_snapshot && client_snapshot_timeout >= 2.0))
					{
						NetworkManager::instance()->reset();
						SceneManager::instance()->goto_network_menu();
						return;
					}
				}
				if (config->is_game_over)
				{
					process_game_over_once();
					banner->on_update(delta);
				}
				return;
			}
		}

		if (paused)
			return;

		if (!config->is_game_over)
		{
			level_elapsed_time += delta;
			WaveManager::instance()->on_update(delta);
			EnemyManager::instance()->on_update(delta);
			BulletManager::instance()->on_update(delta);
			TowerManager::instance()->on_update(delta);
			CoinManager::instance()->on_update(delta);
			PlayerManager::instance()->on_update(delta);

			place_panel->on_update(renderer_cached);
			upgrade_panel->on_update(renderer_cached);
			remove_panel->on_update(renderer_cached);
			status_bar.on_update(renderer_cached);

			if (network_mode && NetworkManager::instance()->is_connected())
			{
				network_snapshot_timer += delta;
				if (network_snapshot_timer >= 0.05)
				{
					network_snapshot_timer = 0;
					NetworkManager::instance()->send_line(build_snapshot_line());
				}
			}
			if (config->is_game_over)
				process_game_over_once();
			return;
		}

		process_game_over_once();
		banner->on_update(delta);

	}

	void on_render(SDL_Renderer* renderer) override
	{
		renderer_cached = renderer;
		if (network_mode && !NetworkManager::instance()->hosting() && remote_world.has_snapshot)
		{
			render_remote_world(renderer);
			return;
		}

		ConfigManager* config = ConfigManager::instance();
		const SDL_Rect& rect_tile_map = config->rect_tile_map;
		SDL_RenderCopy(renderer, tex_tile_map, nullptr, &rect_tile_map);

		EnemyManager::instance()->on_render(renderer);
		TowerManager::instance()->on_render(renderer);
		BulletManager::instance()->on_render(renderer);
		CoinManager::instance()->on_render(renderer);
		PlayerManager::instance()->on_render(renderer);

		if (!config->is_game_over)
		{
			place_panel->on_render(renderer);
			upgrade_panel->on_render(renderer);
			remove_panel->on_render(renderer);
			status_bar.on_render(renderer);
			render_wave_hint(renderer);
			if (paused)
				render_pause_overlay(renderer);
			return;
		}

		int width_screen = 0, height_screen = 0;
		SDL_GetRendererOutputSize(renderer, &width_screen, &height_screen);
		banner->set_position({ (double)width_screen / 2, (double)height_screen / 2 });
		banner->on_render(renderer);
		render_settlement_panel(renderer, width_screen, height_screen);
	}

private:
	int level_id = 1;
	bool network_mode = false;
	NetworkRole local_role = NetworkRole::Offline;
	bool exited = false;
	bool game_over_sound_played = false;
	bool saved_result = false;
	bool settlement_ready = false;
	bool network_game_over_sent = false;
	bool paused = false;
	double network_snapshot_timer = 0;
	double client_snapshot_timeout = 0;
	double level_elapsed_time = 0;
	bool settlement_calculated = false;
	int settlement_score = 0;
	int settlement_stars = 0;
	int settlement_home_hp = 0;
	int settlement_coin = 0;
	int settlement_defeated = 0;
	int settlement_leaked = 0;
	bool balance_logged = false;
	int debug_coin_added = 0;
	int debug_waves_skipped = 0;
	std::string debug_forced_result;
	bool net_move_left = false;
	bool net_move_right = false;
	bool net_move_up = false;
	bool net_move_down = false;

	StatusBar status_bar;
	PlacePanel* place_panel = nullptr;
	UpgradePanel* upgrade_panel = nullptr;
	RemovePanel* remove_panel = nullptr;
	Banner* banner = nullptr;
	SDL_Texture* tex_tile_map = nullptr;
	SDL_Renderer* renderer_cached = nullptr;

	struct RemoteEnemy
	{
		int type = 0;
		double x = 0;
		double y = 0;
		double hp = 0;
		double max_hp = 0;
		double vx = 0;
		double vy = 0;
		bool sketch = false;
		bool armored = false;
		bool intercepted = false;
	};

	struct RemoteActor
	{
		int type = 0;
		double x = 0;
		double y = 0;
		double vx = 0;
		double vy = 0;
		double hp = 0;
		double max_hp = 0;
		int facing = 0;
		int level = 0;
		int spec = 0;
		bool silenced = false;
		bool active = true;
		bool attacking = false;
		bool hurt = false;
		bool moving = false;
	};

	struct RemoteWorld
	{
		bool has_snapshot = false;
		int level_id = 1;
		int home_hp = 0;
		int coin = 0;
		int mp = 0;
		double player_x = 0;
		double player_y = 0;
		int player_facing = 0;
		bool player_flash = false;
		SDL_Rect player_flash_rect = { 0, 0, 0, 0 };
		bool player_impact = false;
		SDL_Rect player_impact_rect = { 0, 0, 0, 0 };
		bool game_over = false;
		bool game_win = false;
		int wave_index = 0;
		int wave_total = 0;
		bool wave_active = false;
		bool boss_warning = false;
		bool boss_alive = false;
		std::vector<RemoteEnemy> enemies;
		std::vector<RemoteActor> towers;
		std::vector<RemoteActor> soldiers;
		std::vector<RemoteActor> bullets;
		std::vector<RemoteActor> coins;
	} remote_world;

private:
	void return_after_game()
	{
		if (network_mode)
			SceneManager::instance()->goto_network_menu();
		else
			SceneManager::instance()->goto_level_select();
	}

	void process_game_over_once()
	{
		ConfigManager* config = ConfigManager::instance();
		if (!config->is_game_over)
			return;

		if (!game_over_sound_played)
		{
			const ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();
			Mix_FadeOutMusic(1500);
			Mix_PlayChannel(-1, config->is_game_win ?
				sound_pool.find(ResID::Sound_Win)->second : sound_pool.find(ResID::Sound_Loss)->second, 0);
			game_over_sound_played = true;
		}

		if (!network_mode && config->is_game_win && !saved_result)
		{
			SaveManager::instance()->complete_level(
				config->current_level_id,
				(int)HomeManager::instance()->get_home_hp(),
				(int)CoinManager::instance()->get_current_coin_num(),
				config->get_max_level_id());
			saved_result = true;
		}

		if (network_mode && NetworkManager::instance()->hosting() &&
			NetworkManager::instance()->is_connected() && !network_game_over_sent)
		{
			NetworkManager::instance()->send_line(config->is_game_win ? "GAME_OVER WIN" : "GAME_OVER LOSS");
			network_game_over_sent = true;
		}

		calculate_settlement_metrics();
		write_balance_log_once();
		settlement_ready = true;
	}

	bool handle_debug_input(const SDL_Event& event)
	{
		if (network_mode || event.type != SDL_KEYDOWN)
			return false;

		ConfigManager* config = ConfigManager::instance();
		switch (event.key.keysym.sym)
		{
		case SDLK_F5:
			CoinManager::instance()->increase_coin(100);
			debug_coin_added += 100;
			return true;
		case SDLK_F6:
			EnemyManager::instance()->clear();
			WaveManager::instance()->debug_skip_current_wave();
			debug_waves_skipped++;
			return true;
		case SDLK_F7:
			config->is_game_over = true;
			config->is_game_win = true;
			debug_forced_result = "win";
			process_game_over_once();
			return true;
		case SDLK_F8:
			config->is_game_over = true;
			config->is_game_win = false;
			debug_forced_result = "loss";
			process_game_over_once();
			return true;
		default:
			return false;
		}
	}

	bool can_control_player() const
	{
		return !network_mode || local_role == NetworkRole::Rpg || local_role == NetworkRole::Offline;
	}

	bool can_control_towers() const
	{
		return !network_mode || local_role == NetworkRole::Tower || local_role == NetworkRole::Offline;
	}

	void process_tower_input(const SDL_Event& event)
	{
		static SDL_Point position_center;
		static SDL_Point idx_tile_selected;
		ConfigManager* config_manager = ConfigManager::instance();

		if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (get_cursor_idx_tile(idx_tile_selected, event.button.x, event.button.y))
			{
				get_selected_tile_center_position(position_center, idx_tile_selected);
				Tower* tower = TowerManager::instance()->find_tower(idx_tile_selected);
				const RemoteActor* remote_tower = find_remote_tower(idx_tile_selected);
				if (event.button.button == SDL_BUTTON_RIGHT)
				{
					if (tower || remote_tower)
					{
						double refund = tower ? TowerManager::instance()->get_remove_tower_refund(tower) :
							TowerManager::instance()->get_remove_tower_refund((TowerType)remote_tower->type, remote_tower->level);
						place_panel->hide();
						upgrade_panel->hide();
						remove_panel->show(idx_tile_selected, position_center, refund);
					}
					else
					{
						remove_panel->hide();
					}
				}
				else if (event.button.button == SDL_BUTTON_LEFT && tower)
				{
					remove_panel->hide();
					upgrade_panel->clear_tower_snapshot();
					upgrade_panel->set_idx_tile(idx_tile_selected);
					upgrade_panel->set_center_pos(position_center);
					upgrade_panel->show();
				}
				else if (event.button.button == SDL_BUTTON_LEFT && remote_tower)
				{
					remove_panel->hide();
					upgrade_panel->set_tower_snapshot((TowerType)remote_tower->type, remote_tower->level,
						(TowerSpecialization)remote_tower->spec, remote_world.coin);
					upgrade_panel->set_idx_tile(idx_tile_selected);
					upgrade_panel->set_center_pos(position_center);
					upgrade_panel->show();
				}
				else if (event.button.button == SDL_BUTTON_LEFT && can_place_tower(idx_tile_selected))
				{
					remove_panel->hide();
					upgrade_panel->clear_tower_snapshot();
					place_panel->set_idx_tile(idx_tile_selected);
					place_panel->set_center_pos(position_center);
					place_panel->show();
				}
			}
		}

		if (!config_manager->is_game_over)
		{
			place_panel->on_input(event);
			upgrade_panel->on_input(event);
			remove_panel->on_input(event);
		}
	}

	void process_network_player_input(const SDL_Event& event)
	{
		bool changed = false;
		bool attack = false;
		bool skill = false;

		if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_a:
				net_move_left = true;
				changed = true;
				break;
			case SDLK_d:
				net_move_right = true;
				changed = true;
				break;
			case SDLK_w:
				net_move_up = true;
				changed = true;
				break;
			case SDLK_s:
				net_move_down = true;
				changed = true;
				break;
			case SDLK_j:
				attack = true;
				changed = true;
				break;
			case SDLK_k:
				skill = true;
				changed = true;
				break;
			default:
				break;
			}
		}
		else if (event.type == SDL_KEYUP)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_a:
				net_move_left = false;
				changed = true;
				break;
			case SDLK_d:
				net_move_right = false;
				changed = true;
				break;
			case SDLK_w:
				net_move_up = false;
				changed = true;
				break;
			case SDLK_s:
				net_move_down = false;
				changed = true;
				break;
			default:
				break;
			}
		}

		if (!changed)
			return;

		if (NetworkManager::instance()->hosting())
			apply_rpg_input(net_move_left, net_move_right, net_move_up, net_move_down, attack, skill);
		else if (NetworkManager::instance()->is_connected())
		{
			std::stringstream stream;
			stream << "INPUT_RPG " << NetworkManager::instance()->next_sequence() << " "
				<< (net_move_left ? 1 : 0) << " " << (net_move_right ? 1 : 0) << " "
				<< (net_move_up ? 1 : 0) << " " << (net_move_down ? 1 : 0) << " "
				<< (attack ? 1 : 0) << " " << (skill ? 1 : 0);
			NetworkManager::instance()->send_line(stream.str());
		}
	}

	void apply_rpg_input(bool left, bool right, bool up, bool down, bool attack, bool skill)
	{
		PlayerManager::instance()->set_move_state(left, right, up, down);
		if (attack)
			PlayerManager::instance()->trigger_normal_attack();
		if (skill)
			PlayerManager::instance()->trigger_skill();
	}

	void request_place_tower(TowerType type, SDL_Point idx_tile)
	{
		if (network_mode && !NetworkManager::instance()->hosting())
		{
			std::stringstream stream;
			stream << "CMD_PLACE " << NetworkManager::instance()->next_sequence() << " "
				<< (int)type << " " << idx_tile.x << " " << idx_tile.y;
			NetworkManager::instance()->send_line(stream.str());
			return;
		}

		try_place_tower(type, idx_tile);
	}

	void request_upgrade_tower(SDL_Point idx_tile)
	{
		if (network_mode && !NetworkManager::instance()->hosting())
		{
			std::stringstream stream;
			stream << "CMD_UPGRADE " << NetworkManager::instance()->next_sequence() << " "
				<< idx_tile.x << " " << idx_tile.y;
			NetworkManager::instance()->send_line(stream.str());
			return;
		}

		try_upgrade_tower(idx_tile);
	}

	void request_specialization(SDL_Point idx_tile, TowerSpecialization spec)
	{
		if (network_mode && !NetworkManager::instance()->hosting())
		{
			std::stringstream stream;
			stream << "CMD_SPEC " << NetworkManager::instance()->next_sequence() << " "
				<< idx_tile.x << " " << idx_tile.y << " " << (int)spec;
			NetworkManager::instance()->send_line(stream.str());
			return;
		}

		try_choose_specialization(idx_tile, spec);
	}

	void request_remove_tower(SDL_Point idx_tile)
	{
		if (network_mode && !NetworkManager::instance()->hosting())
		{
			std::stringstream stream;
			stream << "CMD_REMOVE " << NetworkManager::instance()->next_sequence() << " "
				<< idx_tile.x << " " << idx_tile.y;
			NetworkManager::instance()->send_line(stream.str());
			return;
		}

		try_remove_tower(idx_tile);
	}

	bool try_place_tower(TowerType type, SDL_Point idx_tile)
	{
		if (!can_place_tower(idx_tile))
			return false;

		double cost = TowerManager::instance()->get_place_tower_cost(type);
		if (cost > CoinManager::instance()->get_current_coin_num())
			return false;

		TowerManager::instance()->place_tower(type, idx_tile);
		CoinManager::instance()->decrease_coin(cost);
		return true;
	}

	bool try_upgrade_tower(SDL_Point idx_tile)
	{
		double cost = TowerManager::instance()->get_upgrade_tower_cost(idx_tile);
		if (cost <= 0 || cost > CoinManager::instance()->get_current_coin_num())
			return false;

		if (!TowerManager::instance()->upgrade_tower(idx_tile))
			return false;
		CoinManager::instance()->decrease_coin(cost);
		return true;
	}

	bool try_choose_specialization(SDL_Point idx_tile, TowerSpecialization spec)
	{
		if (!TowerManager::instance()->choose_specialization(idx_tile, spec))
			return false;
		return true;
	}

	bool try_remove_tower(SDL_Point idx_tile)
	{
		if (ConfigManager::instance()->is_game_over)
			return false;

		double refund = TowerManager::instance()->get_remove_tower_refund(idx_tile);
		if (!TowerManager::instance()->remove_tower(idx_tile))
			return false;

		if (refund > 0)
			CoinManager::instance()->increase_coin(refund);
		return true;
	}

	void process_host_network_messages()
	{
		std::string message;
		while (NetworkManager::instance()->poll_message(message))
		{
			std::stringstream stream(message);
			std::string type;
			stream >> type;

			if (type == "ROLE")
			{
				std::string role_text;
				stream >> role_text;
				NetworkManager::instance()->mark_peer_role(NetworkManager::role_from_text(role_text));
			}
			else if (type == "INPUT_RPG" && NetworkManager::instance()->get_peer_role() == NetworkRole::Rpg)
			{
				unsigned int sequence = 0;
				int left = 0, right = 0, up = 0, down = 0, attack = 0, skill = 0;
				stream >> sequence >> left >> right >> up >> down >> attack >> skill;
				apply_rpg_input(left != 0, right != 0, up != 0, down != 0, attack != 0, skill != 0);
			}
			else if (type == "CMD_PLACE" && NetworkManager::instance()->get_peer_role() == NetworkRole::Tower)
			{
				unsigned int sequence = 0;
				int tower_type = 0;
				SDL_Point idx_tile = { 0, 0 };
				stream >> sequence >> tower_type >> idx_tile.x >> idx_tile.y;
				try_place_tower((TowerType)tower_type, idx_tile);
			}
			else if (type == "CMD_UPGRADE" && NetworkManager::instance()->get_peer_role() == NetworkRole::Tower)
			{
				unsigned int sequence = 0;
				SDL_Point idx_tile = { 0, 0 };
				stream >> sequence >> idx_tile.x >> idx_tile.y;
				try_upgrade_tower(idx_tile);
			}
			else if (type == "CMD_SPEC" && NetworkManager::instance()->get_peer_role() == NetworkRole::Tower)
			{
				unsigned int sequence = 0;
				SDL_Point idx_tile = { 0, 0 };
				int spec = 0;
				stream >> sequence >> idx_tile.x >> idx_tile.y >> spec;
				try_choose_specialization(idx_tile, (TowerSpecialization)spec);
			}
			else if (type == "CMD_REMOVE" && NetworkManager::instance()->get_peer_role() == NetworkRole::Tower)
			{
				unsigned int sequence = 0;
				SDL_Point idx_tile = { 0, 0 };
				stream >> sequence >> idx_tile.x >> idx_tile.y;
				try_remove_tower(idx_tile);
			}
			else if (type == "DISCONNECT")
			{
				NetworkManager::instance()->reset();
				SceneManager::instance()->goto_network_menu();
				return;
			}
		}
	}

	void process_client_network_messages()
	{
		std::string message;
		while (NetworkManager::instance()->poll_message(message))
		{
			std::stringstream stream(message);
			std::string type;
			stream >> type;
			if (type == "SNAPSHOT")
				parse_snapshot(message);
			else if (type == "ROLE")
			{
				std::string role_text;
				stream >> role_text;
				NetworkManager::instance()->mark_peer_role(NetworkManager::role_from_text(role_text));
			}
			else if (type == "GAME_OVER")
			{
				std::string result;
				stream >> result;
				remote_world.game_over = true;
				remote_world.game_win = result == "WIN";
				ConfigManager::instance()->is_game_over = true;
				ConfigManager::instance()->is_game_win = remote_world.game_win;
			}
			else if (type == "DISCONNECT")
			{
				NetworkManager::instance()->reset();
				SceneManager::instance()->goto_network_menu();
				return;
			}
		}
	}

	std::string build_snapshot_line()
	{
		std::stringstream stream;
		ConfigManager* config = ConfigManager::instance();
		PlayerManager* player = PlayerManager::instance();
		const Vector2& player_pos = player->get_position();
		const SDL_Rect& flash_rect = player->get_flash_hitbox();
		const SDL_Rect& impact_rect = player->get_impact_hitbox();
		stream << "SNAPSHOT " << config->current_level_id << " "
			<< (int)HomeManager::instance()->get_home_hp() << " "
			<< (int)CoinManager::instance()->get_current_coin_num() << " "
			<< (int)player->get_current_mp() << " "
			<< (int)player_pos.x << " " << (int)player_pos.y << " "
			<< (int)player->get_facing() << " "
			<< (player->is_releasing_flash_now() ? 1 : 0) << " "
			<< flash_rect.x << " " << flash_rect.y << " " << flash_rect.w << " " << flash_rect.h << " "
			<< (player->is_releasing_impact_now() ? 1 : 0) << " "
			<< impact_rect.x << " " << impact_rect.y << " " << impact_rect.w << " " << impact_rect.h << " "
			<< (config->is_game_over ? 1 : 0) << " " << (config->is_game_win ? 1 : 0) << " "
			<< WaveManager::instance()->get_current_wave_index() << " "
			<< WaveManager::instance()->get_total_wave_count() << " "
			<< (WaveManager::instance()->is_wave_active() ? 1 : 0) << " "
			<< (WaveManager::instance()->is_current_boss_pending() ? 1 : 0) << " "
			<< (EnemyManager::instance()->has_enemy_type(EnemyType::Boss) ? 1 : 0);

		EnemyManager::EnemyList& enemies = EnemyManager::instance()->get_enemy_list();
		stream << " ENEMIES " << enemies.size();
		for (Enemy* enemy : enemies)
		{
			const Vector2& pos = enemy->get_position();
			const Vector2& velocity = enemy->get_velocity();
			stream << " " << (int)enemy->get_enemy_type() << " " << (int)pos.x << " " << (int)pos.y
				<< " " << (int)enemy->get_hp() << " " << (int)enemy->get_max_hp()
				<< " " << (int)velocity.x << " " << (int)velocity.y
				<< " " << (enemy->is_showing_sketch() ? 1 : 0)
				<< " " << (enemy->is_armored() ? 1 : 0)
				<< " " << (enemy->is_intercepted() ? 1 : 0);
		}

		TowerManager::TowerList& towers = TowerManager::instance()->get_tower_list();
		stream << " TOWERS " << towers.size();
		for (Tower* tower : towers)
		{
			const Vector2& pos = tower->get_position();
			stream << " " << (int)tower->get_tower_type() << " " << (int)pos.x << " " << (int)pos.y
				<< " " << (int)tower->get_facing()
				<< " " << tower->get_level()
				<< " " << (int)tower->get_specialization()
				<< " " << (tower->is_silenced() ? 1 : 0);
		}

		std::vector<BarracksTower::SoldierSnapshot> soldier_snapshots;
		for (Tower* tower : towers)
		{
			BarracksTower* barracks = dynamic_cast<BarracksTower*>(tower);
			if (!barracks)
				continue;
			std::vector<BarracksTower::SoldierSnapshot> snapshots = barracks->get_soldier_snapshots();
			soldier_snapshots.insert(soldier_snapshots.end(), snapshots.begin(), snapshots.end());
		}
		stream << " SOLDIERS " << soldier_snapshots.size();
		for (const BarracksTower::SoldierSnapshot& soldier : soldier_snapshots)
		{
			stream << " " << (int)soldier.x << " " << (int)soldier.y
				<< " " << (int)soldier.hp
				<< " " << (int)soldier.max_hp
				<< " " << soldier.facing
				<< " " << (soldier.active ? 1 : 0)
				<< " " << (soldier.attacking ? 1 : 0)
				<< " " << (soldier.hurt ? 1 : 0)
				<< " " << (soldier.moving ? 1 : 0);
		}

		BulletManager::BulletList& bullets = BulletManager::instance()->get_bullet_list();
		stream << " BULLETS " << bullets.size();
		for (Bullet* bullet : bullets)
		{
			const Vector2& pos = bullet->get_position();
			const Vector2& velocity = bullet->get_velocity();
			stream << " " << (int)bullet->get_bullet_type() << " " << (int)pos.x << " " << (int)pos.y
				<< " " << (int)velocity.x << " " << (int)velocity.y
				<< " " << (bullet->can_collided() ? 1 : 0);
		}

		CoinManager::CoinPropList& coins = CoinManager::instance()->get_coin_list();
		stream << " COINS " << coins.size();
		for (CoinProp* coin : coins)
		{
			const Vector2& pos = coin->get_position();
			stream << " " << (int)pos.x << " " << (int)pos.y;
		}

		return stream.str();
	}

	void parse_snapshot(const std::string& message)
	{
		client_snapshot_timeout = 0;
		std::stringstream stream(message);
		std::string token;
		stream >> token;
		stream >> remote_world.level_id >> remote_world.home_hp >> remote_world.coin >> remote_world.mp
			>> remote_world.player_x >> remote_world.player_y >> remote_world.player_facing
			>> remote_world.player_flash
			>> remote_world.player_flash_rect.x >> remote_world.player_flash_rect.y
			>> remote_world.player_flash_rect.w >> remote_world.player_flash_rect.h
			>> remote_world.player_impact
			>> remote_world.player_impact_rect.x >> remote_world.player_impact_rect.y
			>> remote_world.player_impact_rect.w >> remote_world.player_impact_rect.h
			>> remote_world.game_over >> remote_world.game_win
			>> remote_world.wave_index >> remote_world.wave_total >> remote_world.wave_active
			>> remote_world.boss_warning >> remote_world.boss_alive;

		remote_world.enemies.clear();
		remote_world.towers.clear();
		remote_world.soldiers.clear();
		remote_world.bullets.clear();
		remote_world.coins.clear();

		size_t count = 0;
		stream >> token >> count;
		for (size_t i = 0; i < count; i++)
		{
			RemoteEnemy enemy;
			stream >> enemy.type >> enemy.x >> enemy.y >> enemy.hp >> enemy.max_hp
				>> enemy.vx >> enemy.vy >> enemy.sketch >> enemy.armored >> enemy.intercepted;
			remote_world.enemies.push_back(enemy);
		}

		stream >> token >> count;
		for (size_t i = 0; i < count; i++)
		{
			RemoteActor tower;
			stream >> tower.type >> tower.x >> tower.y >> tower.facing >> tower.level >> tower.spec >> tower.silenced;
			remote_world.towers.push_back(tower);
		}

		stream >> token >> count;
		for (size_t i = 0; i < count; i++)
		{
			RemoteActor soldier;
			stream >> soldier.x >> soldier.y >> soldier.hp >> soldier.max_hp >> soldier.facing
				>> soldier.active >> soldier.attacking >> soldier.hurt >> soldier.moving;
			remote_world.soldiers.push_back(soldier);
		}

		stream >> token >> count;
		for (size_t i = 0; i < count; i++)
		{
			RemoteActor bullet;
			stream >> bullet.type >> bullet.x >> bullet.y >> bullet.vx >> bullet.vy >> bullet.active;
			remote_world.bullets.push_back(bullet);
		}

		stream >> token >> count;
		for (size_t i = 0; i < count; i++)
		{
			RemoteActor coin;
			stream >> coin.x >> coin.y;
			remote_world.coins.push_back(coin);
		}

		remote_world.has_snapshot = true;
		ConfigManager::instance()->is_game_over = remote_world.game_over;
		ConfigManager::instance()->is_game_win = remote_world.game_win;
	}

	void render_remote_world(SDL_Renderer* renderer)
	{
		const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		SDL_RenderCopy(renderer, tex_tile_map, nullptr, &rect_tile_map);

		for (const RemoteEnemy& enemy : remote_world.enemies)
			render_enemy_snapshot(renderer, enemy);
		for (const RemoteActor& tower : remote_world.towers)
			render_tower_snapshot(renderer, tower);
		for (const RemoteActor& soldier : remote_world.soldiers)
			render_soldier_snapshot(renderer, soldier);
		for (const RemoteActor& bullet : remote_world.bullets)
			render_bullet_snapshot(renderer, bullet);
		for (const RemoteActor& coin : remote_world.coins)
			render_whole_texture_center(renderer, ResID::Tex_Coin, coin.x, coin.y, 16, 16);

		render_player_snapshot(renderer);
		render_player_effects_snapshot(renderer);
		render_remote_status_bar(renderer);
		render_wave_hint(renderer, true);

		if (!ConfigManager::instance()->is_game_over && can_control_towers())
		{
			place_panel->on_render(renderer);
			upgrade_panel->on_render(renderer);
			remove_panel->on_render(renderer);
		}

		if (ConfigManager::instance()->is_game_over)
		{
			process_game_over_once();
			int width_screen = 0, height_screen = 0;
			SDL_GetRendererOutputSize(renderer, &width_screen, &height_screen);
			banner->set_position({ (double)width_screen / 2, (double)height_screen / 2 });
			banner->on_render(renderer);
			render_settlement_panel(renderer, width_screen, height_screen);
		}
	}

	void calculate_settlement_metrics()
	{
		if (settlement_calculated)
			return;

		ConfigManager* config = ConfigManager::instance();
		const ConfigManager::LevelInfo* level = config->get_level_info(config->current_level_id);
		settlement_home_hp = network_mode && !NetworkManager::instance()->hosting() ? remote_world.home_hp : (int)HomeManager::instance()->get_home_hp();
		settlement_coin = network_mode && !NetworkManager::instance()->hosting() ? remote_world.coin : (int)CoinManager::instance()->get_current_coin_num();
		settlement_defeated = EnemyManager::instance()->get_total_defeated();
		settlement_leaked = EnemyManager::instance()->get_total_leaked();

		int target_time = level ? level->target_time : 120;
		int star_home_hp = level ? level->star_home_hp : 7;
		int star_coin = level ? level->star_coin : 120;
		int time_bonus = (std::max)(0, target_time - (int)level_elapsed_time) * 8;

		settlement_stars = 0;
		if (config->is_game_win)
		{
			settlement_stars = 1;
			if (settlement_home_hp >= star_home_hp)
				settlement_stars++;
			if ((int)level_elapsed_time <= target_time || settlement_coin >= star_coin)
				settlement_stars++;
		}

		settlement_score = config->is_game_win ? 500 : 0;
		settlement_score += settlement_home_hp * 100;
		settlement_score += settlement_coin * 3;
		settlement_score += settlement_defeated * 25;
		settlement_score += time_bonus;
		settlement_score -= settlement_leaked * 60;
		settlement_score = (std::max)(0, settlement_score);
		settlement_calculated = true;
	}

	void write_balance_log_once()
	{
		if (network_mode || balance_logged)
			return;

		calculate_settlement_metrics();

		const char* path = "balance_log.txt";
		bool write_header = false;
		{
			std::ifstream existing(path);
			write_header = !existing.good() || existing.peek() == std::ifstream::traits_type::eof();
		}

		std::ofstream file(path, std::ios::app);
		if (!file.good())
			return;

		if (write_header)
		{
			file << "level_id,level_name,result,elapsed_seconds,home_hp,coin,spawned,defeated,leaked,boss_appeared,boss_summons,score,stars,debug_coin_added,debug_waves_skipped,debug_forced_result\n";
		}

		ConfigManager* config = ConfigManager::instance();
		const ConfigManager::LevelInfo* level = config->get_level_info(config->current_level_id);
		std::string level_name = level && !level->name.empty() ? level->name : std::string("Level ") + std::to_string(config->current_level_id);
		for (char& ch : level_name)
			if (ch == ',')
				ch = ' ';

		file << config->current_level_id << ","
			<< level_name << ","
			<< (config->is_game_win ? "win" : "loss") << ","
			<< (int)level_elapsed_time << ","
			<< settlement_home_hp << ","
			<< settlement_coin << ","
			<< EnemyManager::instance()->get_total_spawned() << ","
			<< settlement_defeated << ","
			<< settlement_leaked << ","
			<< (EnemyManager::instance()->has_boss_appeared() ? 1 : 0) << ","
			<< EnemyManager::instance()->get_total_boss_summons() << ","
			<< settlement_score << ","
			<< settlement_stars << ","
			<< debug_coin_added << ","
			<< debug_waves_skipped << ","
			<< (debug_forced_result.empty() ? "none" : debug_forced_result)
			<< "\n";

		balance_logged = true;
	}

	std::string format_time(double seconds) const
	{
		int total_seconds = (std::max)(0, (int)seconds);
		int minutes = total_seconds / 60;
		int remain = total_seconds % 60;
		std::stringstream stream;
		stream << minutes << ":";
		if (remain < 10)
			stream << "0";
		stream << remain;
		return stream.str();
	}

	void render_settlement_panel(SDL_Renderer* renderer, int width_screen, int height_screen)
	{
		calculate_settlement_metrics();

		ConfigManager* config = ConfigManager::instance();
		const ConfigManager::LevelInfo* level = config->get_level_info(config->current_level_id);
		SDL_Rect panel = { width_screen / 2 - 310, height_screen / 2 + 78, 620, 205 };
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		if (!UI::draw_texture(renderer, ResID::Tex_UIResultPanelBg, panel))
		{
			SDL_SetRenderDrawColor(renderer, 30, 35, 42, 226);
			SDL_RenderFillRect(renderer, &panel);
			SDL_SetRenderDrawColor(renderer, 224, 222, 196, 255);
			SDL_RenderDrawRect(renderer, &panel);
		}

		std::string title = level && !level->name.empty() ? level->name : std::string(u8"关卡 ") + std::to_string(config->current_level_id);
		SDL_Rect result_icon = { panel.x + 36, panel.y + 28, 64, 64 };
		UI::draw_texture(renderer, config->is_game_win ? ResID::Tex_UIResultVictory : ResID::Tex_UIResultDefeat, result_icon);
		UI::draw_text(renderer, config->is_game_win ? u8"胜利结算" : u8"失败结算",
			panel.x + 116, panel.y + 24, config->is_game_win ? SDL_Color{ 255, 245, 204, 255 } : SDL_Color{ 230, 170, 160, 255 });
		UI::draw_text(renderer, title, panel.x + 116, panel.y + 54, { 205, 215, 200, 255 });
		render_settlement_stars(renderer, panel.x + 430, panel.y + 32, config->is_game_win ? settlement_stars : 0);

		int left_x = panel.x + 46;
		int right_x = panel.x + 340;
		int row_y = panel.y + 106;
		UI::draw_text(renderer, u8"耗时: " + format_time(level_elapsed_time), left_x, row_y, { 235, 235, 220, 255 });
		UI::draw_text(renderer, u8"家园血量: " + std::to_string(settlement_home_hp), right_x, row_y, { 235, 235, 220, 255 });
		UI::draw_text(renderer, u8"金币: " + std::to_string(settlement_coin), left_x, row_y + 34, { 235, 235, 220, 255 });
		UI::draw_text(renderer, u8"击退 / 漏怪: " + std::to_string(settlement_defeated) + " / " + std::to_string(settlement_leaked),
			right_x, row_y + 34, { 235, 235, 220, 255 });
		UI::draw_text(renderer, u8"评分: " + std::to_string(settlement_score), left_x, row_y + 68, { 255, 245, 204, 255 });

		SDL_Rect prompt_rect = { panel.x, panel.y + panel.h - 36, panel.w, 30 };
		UI::draw_centered_text(renderer,
			network_mode ? u8"按任意键或点击鼠标返回联机菜单" : u8"按任意键或点击鼠标返回选关",
			prompt_rect, { 255, 245, 204, 255 });
	}

	void render_settlement_stars(SDL_Renderer* renderer, int x, int y, int stars)
	{
		for (int i = 0; i < 3; i++)
		{
			SDL_Rect star_rect = { x + i * 42, y, 32, 32 };
			UI::draw_texture(renderer, i < stars ? ResID::Tex_UIStarFull : ResID::Tex_UIStarEmpty, star_rect);
		}
	}

	void render_wave_hint(SDL_Renderer* renderer, bool remote = false)
	{
		ConfigManager* config = ConfigManager::instance();
		if (config->is_game_over)
			return;

		int total = remote ? remote_world.wave_total : WaveManager::instance()->get_total_wave_count();
		if (total <= 0)
			return;

		int wave_index = remote ? remote_world.wave_index : WaveManager::instance()->get_current_wave_index();
		bool active = remote ? remote_world.wave_active : WaveManager::instance()->is_wave_active();
		bool boss_warning = remote ? remote_world.boss_warning : WaveManager::instance()->is_current_boss_pending();
		bool boss_alive = remote ? remote_world.boss_alive : EnemyManager::instance()->has_enemy_type(EnemyType::Boss);

		int current = (std::min)(wave_index + 1, total);
		std::string line_1 = std::string(u8"波次 ") + std::to_string(current) + " / " + std::to_string(total);
		std::string line_2;
		if (boss_alive)
			line_2 = u8"Boss 已出现";
		else if (boss_warning)
			line_2 = u8"警告: Boss 即将出现";
		else if (active)
			line_2 = u8"敌袭进行中";
		else
		{
			int remaining = (int)std::ceil(WaveManager::instance()->get_next_wave_remaining_time());
			if (remote)
				line_2 = u8"下一波准备中";
			else
				line_2 = std::string(u8"下一波: ") + std::to_string((std::max)(0, remaining)) + "s";
		}

		SDL_Rect panel = { 470, 18, 340, 66 };
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		if (boss_warning || boss_alive)
			SDL_SetRenderDrawColor(renderer, 94, 42, 38, 220);
		else
			SDL_SetRenderDrawColor(renderer, 30, 36, 42, 200);
		SDL_RenderFillRect(renderer, &panel);
		SDL_SetRenderDrawColor(renderer, boss_warning || boss_alive ? 255 : 224, boss_warning || boss_alive ? 190 : 222, 130, 255);
		SDL_RenderDrawRect(renderer, &panel);
		UI::draw_centered_text(renderer, line_1, { panel.x, panel.y + 5, panel.w, 26 }, { 255, 245, 204, 255 });
		UI::draw_centered_text(renderer, line_2, { panel.x, panel.y + 34, panel.w, 26 },
			boss_warning || boss_alive ? SDL_Color{ 255, 220, 150, 255 } : SDL_Color{ 205, 215, 200, 255 });
	}

	void render_pause_overlay(SDL_Renderer* renderer)
	{
		int width_screen = 0, height_screen = 0;
		SDL_GetRendererOutputSize(renderer, &width_screen, &height_screen);

		SDL_Rect overlay = { 0, 0, width_screen, height_screen };
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 115);
		SDL_RenderFillRect(renderer, &overlay);

		SDL_Rect text_rect = { width_screen / 2 - 170, height_screen / 2 - 30, 340, 60 };
		UI::draw_centered_text(renderer, u8"已暂停  空格继续", text_rect, { 255, 245, 204, 255 });
	}

	void render_player_snapshot(SDL_Renderer* renderer)
	{
		bool attacking = remote_world.player_flash || remote_world.player_impact;
		int anim_offset = (int)((SDL_GetTicks64() / (attacking ? 100 : 120)) % (attacking ? 2 : 4));
		int frame = attacking ? 24 + anim_offset : 8 + anim_offset;
		switch ((Facing)remote_world.player_facing)
		{
		case Facing::Down:
			frame = (attacking ? 16 : 0) + anim_offset;
			break;
		case Facing::Up:
			frame = (attacking ? 20 : 4) + anim_offset;
			break;
		case Facing::Left:
			frame = (attacking ? 24 : 8) + anim_offset;
			break;
		case Facing::Right:
			frame = (attacking ? 28 : 12) + anim_offset;
			break;
		}
		render_sheet_frame_center(renderer, ResID::Tex_Player, 4, 8, frame, remote_world.player_x, remote_world.player_y, 96, 96);
	}

	void render_player_effects_snapshot(SDL_Renderer* renderer)
	{
		if (remote_world.player_flash)
			render_player_effect_snapshot(renderer, true, remote_world.player_flash_rect);
		if (remote_world.player_impact)
			render_player_effect_snapshot(renderer, false, remote_world.player_impact_rect);
	}

	void render_player_effect_snapshot(SDL_Renderer* renderer, bool flash, const SDL_Rect& dst)
	{
		if (dst.w <= 0 || dst.h <= 0)
			return;

		ResID texture = flash ? ResID::Tex_EffectFlash_Right : ResID::Tex_EffectImpact_Right;
		int columns = 1;
		int rows = 5;
		int anim_offset = (int)((SDL_GetTicks64() / 100) % 5);
		int frame = anim_offset;

		switch ((Facing)remote_world.player_facing)
		{
		case Facing::Up:
			texture = flash ? ResID::Tex_EffectFlash_Up : ResID::Tex_EffectImpact_Up;
			columns = 5;
			rows = 1;
			frame = anim_offset;
			break;
		case Facing::Down:
			texture = flash ? ResID::Tex_EffectFlash_Down : ResID::Tex_EffectImpact_Down;
			columns = 5;
			rows = 1;
			frame = 4 - anim_offset;
			break;
		case Facing::Left:
			texture = flash ? ResID::Tex_EffectFlash_Left : ResID::Tex_EffectImpact_Left;
			columns = 1;
			rows = 5;
			frame = 4 - anim_offset;
			break;
		case Facing::Right:
			texture = flash ? ResID::Tex_EffectFlash_Right : ResID::Tex_EffectImpact_Right;
			columns = 1;
			rows = 5;
			frame = anim_offset;
			break;
		}

		render_sheet_frame_rect(renderer, texture, columns, rows, frame, dst);
	}

	void render_remote_status_bar(SDL_Renderer* renderer)
	{
		static const SDL_Point position = { 15, 15 };
		static const int avatar_home_size = 78;
		static const int avatar_player_size = 65;
		static const int icon_size = 32;
		static const int heart_gap = 2;
		static const int panel_gap = 15;
		static const int width_mp = 200;
		static const int height_mp = 20;
		static const int width_border_mp = 4;

		const ResourcesManager::TexturePool& tex_pool = ResourcesManager::instance()->get_texture_pool();
		SDL_Texture* tex_coin = tex_pool.find(ResID::Tex_UICoin)->second;
		SDL_Texture* tex_heart = tex_pool.find(ResID::Tex_UIHeart)->second;
		SDL_Texture* tex_home = tex_pool.find(ResID::Tex_UIHomeAvatar)->second;
		SDL_Texture* tex_player = tex_pool.find(ResID::Tex_UIPlayerAvatar)->second;

		SDL_Rect rect_dst = { position.x, position.y, avatar_home_size, avatar_home_size };
		SDL_RenderCopy(renderer, tex_home, nullptr, &rect_dst);

		int heart_count = (std::max)(0, remote_world.home_hp);
		for (int i = 0; i < heart_count; i++)
		{
			rect_dst.x = position.x + avatar_home_size + panel_gap + i * (icon_size + heart_gap);
			rect_dst.y = position.y;
			rect_dst.w = icon_size;
			rect_dst.h = icon_size;
			SDL_RenderCopy(renderer, tex_heart, nullptr, &rect_dst);
		}

		rect_dst.x = position.x + avatar_home_size + panel_gap;
		rect_dst.y = position.y + avatar_home_size - icon_size;
		rect_dst.w = icon_size;
		rect_dst.h = icon_size;
		SDL_RenderCopy(renderer, tex_coin, nullptr, &rect_dst);
		UI::draw_text(renderer, std::to_string(remote_world.coin),
			rect_dst.x + icon_size + 10, rect_dst.y + 2, { 255, 255, 240, 255 });

		rect_dst.x = position.x + (avatar_home_size - avatar_player_size) / 2;
		rect_dst.y = position.y + avatar_home_size + 5;
		rect_dst.w = avatar_player_size;
		rect_dst.h = avatar_player_size;
		SDL_RenderCopy(renderer, tex_player, nullptr, &rect_dst);

		int bar_x = position.x + avatar_home_size + panel_gap;
		int bar_y = rect_dst.y + 10;
		roundedBoxRGBA(renderer, bar_x, bar_y, bar_x + width_mp, bar_y + height_mp, 4, 48, 40, 51, 255);

		double process_mp = (std::max)(0, (std::min)(remote_world.mp, 100)) / 100.0;
		roundedBoxRGBA(renderer,
			bar_x + width_border_mp,
			bar_y + width_border_mp,
			bar_x + width_border_mp + (int)((width_mp - 2 * width_border_mp) * process_mp),
			bar_y + height_mp - width_border_mp,
			2, 144, 121, 173, 255);

		UI::draw_text(renderer, u8"联机同步中", bar_x, bar_y + height_mp + 10, { 255, 245, 204, 255 });
	}

	void render_enemy_snapshot(SDL_Renderer* renderer, const RemoteEnemy& enemy)
	{
		ResID texture = ResID::Tex_Slime;
		int columns = 6;
		int frame_count = 6;
		int render_size = 48;
		switch ((EnemyType)enemy.type)
		{
		case EnemyType::Silm:
			texture = enemy.sketch ? ResID::Tex_SlimeSketch : ResID::Tex_Slime;
			columns = 6;
			frame_count = 6;
			break;
		case EnemyType::KingSilm:
			texture = enemy.sketch ? ResID::Tex_KingSlimeSketch : ResID::Tex_KingSlime;
			columns = 6;
			frame_count = 6;
			break;
		case EnemyType::Skeleton:
			texture = enemy.sketch ? ResID::Tex_SkeletonSketch : ResID::Tex_Skeleton;
			columns = 5;
			frame_count = 5;
			break;
		case EnemyType::Goblin:
			texture = enemy.sketch ? ResID::Tex_GoblinSketch : ResID::Tex_Goblin;
			columns = 5;
			frame_count = 5;
			break;
		case EnemyType::GoblinPriest:
			texture = enemy.sketch ? ResID::Tex_GoblinPriestSketch : ResID::Tex_GoblinPriest;
			columns = 5;
			frame_count = 5;
			break;
		case EnemyType::Boss:
			texture = enemy.sketch ? ResID::Tex_BossSketch : ResID::Tex_Boss;
			columns = 6;
			frame_count = 6;
			render_size = 72;
			break;
		case EnemyType::Silencer:
			texture = enemy.sketch ? ResID::Tex_SilencerCast : ResID::Tex_SilencerWalk;
			columns = 6;
			frame_count = 6;
			break;
		case EnemyType::Armored:
			texture = enemy.sketch ? ResID::Tex_ArmoredHit : ResID::Tex_ArmoredWalk;
			columns = 6;
			frame_count = 6;
			break;
		}
		int frame = enemy_direction_base_frame(columns, enemy.vx, enemy.vy) + (int)((SDL_GetTicks64() / 140) % frame_count);
		render_sheet_frame_center(renderer, texture, columns, 4, frame, enemy.x, enemy.y, render_size, render_size);
		render_enemy_hp_bar(renderer, enemy, render_size);
		render_enemy_status_snapshot(renderer, enemy, render_size);
	}

	void render_tower_snapshot(SDL_Renderer* renderer, const RemoteActor& tower)
	{
		ResID texture = ResID::Tex_Archer;
		int columns = 3;
		int frame = 0;
		switch ((TowerType)tower.type)
		{
		case TowerType::Archer:
			texture = ResID::Tex_Archer;
			columns = 3;
			frame = tower_idle_frame((TowerType)tower.type, (Facing)tower.facing, (int)((SDL_GetTicks64() / 180) % 2));
			break;
		case TowerType::Axeman:
			texture = ResID::Tex_Axeman;
			columns = 3;
			frame = tower_idle_frame((TowerType)tower.type, (Facing)tower.facing, (int)((SDL_GetTicks64() / 180) % 2));
			break;
		case TowerType::Gunner:
			texture = ResID::Tex_Gunner;
			columns = 4;
			frame = tower_idle_frame((TowerType)tower.type, (Facing)tower.facing, (int)((SDL_GetTicks64() / 180) % 2));
			break;
		case TowerType::Barracks:
			texture = barracks_texture_for_snapshot(tower);
			columns = 3;
			frame = (int)((SDL_GetTicks64() / 180) % 2);
			break;
		}
		render_sheet_frame_center(renderer, texture, columns, 8, frame, tower.x, tower.y, 48, 48);
		if (tower.silenced)
		{
			SDL_Rect dst = { (int)(tower.x - 12), (int)(tower.y - 42), 24, 24 };
			UI::draw_texture(renderer, ResID::Tex_UIStatusSilence, dst);
		}
	}

	void render_soldier_snapshot(SDL_Renderer* renderer, const RemoteActor& soldier)
	{
		if (!soldier.active)
			return;

		ResID texture = ResID::Tex_BarracksSoldierIdle;
		if (soldier.hurt)
			texture = ResID::Tex_BarracksSoldierHit;
		else if (soldier.attacking)
			texture = ResID::Tex_BarracksSoldierAttack;
		else if (soldier.moving)
			texture = ResID::Tex_BarracksSoldierWalk;

		int frame = soldier_base_frame((Facing)soldier.facing) + (int)((SDL_GetTicks64() / 110) % 6);
		render_sheet_frame_center(renderer, texture, 6, 4, frame, soldier.x, soldier.y, 32, 32);
		if (soldier.attacking)
			render_soldier_attack_effect_snapshot(renderer, soldier);
		render_soldier_hp_bar_snapshot(renderer, soldier);
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

	void render_soldier_attack_effect_snapshot(SDL_Renderer* renderer, const RemoteActor& soldier)
	{
		Vector2 effect_pos = { soldier.x, soldier.y };
		switch ((Facing)soldier.facing)
		{
		case Facing::Down: effect_pos.y += 18; break;
		case Facing::Up: effect_pos.y -= 18; break;
		case Facing::Right: effect_pos.x += 20; break;
		case Facing::Left: effect_pos.x -= 20; break;
		}

		int frame = (int)((SDL_GetTicks64() / 70) % 6);
		render_sheet_frame_center(renderer, ResID::Tex_EffectSoldierSlash, 6, 1, frame, effect_pos.x, effect_pos.y, 32, 32);
	}

	void render_soldier_hp_bar_snapshot(SDL_Renderer* renderer, const RemoteActor& soldier)
	{
		if (soldier.max_hp <= 0 || soldier.hp >= soldier.max_hp)
			return;

		double ratio = (std::max)(0.0, (std::min)(soldier.hp / soldier.max_hp, 1.0));
		SDL_Rect rect = { (int)(soldier.x - 18), (int)(soldier.y - 26), (int)(36 * ratio), 5 };
		SDL_SetRenderDrawColor(renderer, 202, 232, 166, 255);
		SDL_RenderFillRect(renderer, &rect);
		rect.w = 36;
		SDL_SetRenderDrawColor(renderer, 85, 114, 91, 255);
		SDL_RenderDrawRect(renderer, &rect);
	}

	void render_bullet_snapshot(SDL_Renderer* renderer, const RemoteActor& bullet)
	{
		ResID texture = ResID::Tex_BulletArrow;
		int columns = 2;
		int rows = 1;
		int frame_count = 2;
		bool rotated = false;
		int width = 48;
		int height = 48;
		switch ((BulletType)bullet.type)
		{
		case BulletType::Arrow:
			texture = ResID::Tex_BulletArrow;
			columns = 2;
			rows = 1;
			frame_count = 2;
			rotated = true;
			break;
		case BulletType::Axe:
			texture = ResID::Tex_BulletAxe;
			columns = 4;
			rows = 2;
			frame_count = 8;
			break;
		case BulletType::Shell:
			texture = bullet.active ? ResID::Tex_BulletShell : ResID::Tex_EffectExplode;
			columns = bullet.active ? 2 : 5;
			rows = 1;
			frame_count = bullet.active ? 2 : 5;
			width = bullet.active ? 48 : 96;
			height = bullet.active ? 48 : 96;
			break;
		}
		int frame = (int)((SDL_GetTicks64() / 100) % frame_count);
		double angle = rotated ? std::atan2(bullet.vy, bullet.vx) * 180.0 / 3.14159265358979323846 : 0;
		render_sheet_frame_center_ex(renderer, texture, columns, rows, frame, bullet.x, bullet.y, width, height, angle);
	}

	int enemy_direction_base_frame(int columns, double vx, double vy) const
	{
		if (std::abs(vx) >= std::abs(vy))
			return vx >= 0 ? columns * 2 : columns * 3;
		return vy >= 0 ? 0 : columns;
	}

	void render_enemy_hp_bar(SDL_Renderer* renderer, const RemoteEnemy& enemy, int render_size)
	{
		if (enemy.max_hp <= 0 || enemy.hp >= enemy.max_hp)
			return;

		static const int offset_y = 2;
		static const int width_hp_bar = 40;
		static const int height_hp_bar = 8;
		static const SDL_Color color_border = { 116, 185, 124, 255 };
		static const SDL_Color color_content = { 226, 255, 194, 255 };

		double hp_ratio = (std::max)(0.0, (std::min)(enemy.hp / enemy.max_hp, 1.0));
		SDL_Rect rect =
			{
				(int)(enemy.x - width_hp_bar / 2),
			(int)(enemy.y - render_size / 2 - height_hp_bar - offset_y),
			(int)(width_hp_bar * hp_ratio),
			height_hp_bar
		};
		SDL_SetRenderDrawColor(renderer, color_content.r, color_content.g, color_content.b, color_content.a);
		SDL_RenderFillRect(renderer, &rect);

		rect.w = width_hp_bar;
		SDL_SetRenderDrawColor(renderer, color_border.r, color_border.g, color_border.b, color_border.a);
		SDL_RenderDrawRect(renderer, &rect);
	}

	void render_enemy_status_snapshot(SDL_Renderer* renderer, const RemoteEnemy& enemy, int render_size)
	{
		int icon_x = (int)(enemy.x - 20);
		int icon_y = (int)(enemy.y - render_size / 2 - 12);
		if (enemy.armored)
		{
			SDL_Rect dst = { icon_x, icon_y, 16, 16 };
			UI::draw_texture(renderer, ResID::Tex_UIStatusArmor, dst);
		}
		if (enemy.intercepted)
		{
			SDL_Rect dst = { icon_x + 18, icon_y, 16, 16 };
			UI::draw_texture(renderer, ResID::Tex_UIStatusIntercept, dst);
		}
	}

	ResID barracks_texture_for_snapshot(const RemoteActor& tower) const
	{
		TowerSpecialization spec = (TowerSpecialization)tower.spec;
		if (spec == TowerSpecialization::BarracksShieldWall)
			return ResID::Tex_BarracksShieldWall;
		if (spec == TowerSpecialization::BarracksVanguard)
			return ResID::Tex_BarracksVanguard;
		if (tower.level >= 6)
			return ResID::Tex_BarracksLv3;
		if (tower.level >= 3)
			return ResID::Tex_BarracksLv2;
		return ResID::Tex_BarracksLv1;
	}

	int tower_idle_frame(TowerType type, Facing facing, int anim_offset) const
	{
		anim_offset = anim_offset % 2;
		switch (type)
		{
		case TowerType::Archer:
			switch (facing)
			{
			case Facing::Up: return 3 + anim_offset;
			case Facing::Left: return 6 + anim_offset;
			case Facing::Right: return 9 + anim_offset;
			case Facing::Down: return 0 + anim_offset;
			}
			break;
		case TowerType::Axeman:
			switch (facing)
			{
			case Facing::Up: return 3 + anim_offset;
			case Facing::Left: return 9 + anim_offset;
			case Facing::Right: return 6 + anim_offset;
			case Facing::Down: return 0 + anim_offset;
			}
			break;
		case TowerType::Gunner:
			switch (facing)
			{
			case Facing::Up: return 4 + anim_offset;
			case Facing::Left: return 12 + anim_offset;
			case Facing::Right: return 8 + anim_offset;
			case Facing::Down: return 0 + anim_offset;
			}
			break;
		case TowerType::Barracks:
			return anim_offset;
		}
		return anim_offset;
	}

	void render_sheet_frame_rect(SDL_Renderer* renderer, ResID res_id, int columns, int rows, int frame, const SDL_Rect& dst)
	{
		SDL_Texture* texture = ResourcesManager::instance()->get_texture_pool().find(res_id)->second;
		int tex_width = 0, tex_height = 0;
		SDL_QueryTexture(texture, nullptr, nullptr, &tex_width, &tex_height);
		SDL_Rect src =
		{
			(frame % columns) * (tex_width / columns),
			(frame / columns) * (tex_height / rows),
			tex_width / columns,
			tex_height / rows
		};
		SDL_RenderCopy(renderer, texture, &src, &dst);
	}

	void render_sheet_frame_center_ex(SDL_Renderer* renderer, ResID res_id, int columns, int rows, int frame, double x, double y, int width, int height, double angle, SDL_RendererFlip flip = SDL_FLIP_NONE)
	{
		SDL_Texture* texture = ResourcesManager::instance()->get_texture_pool().find(res_id)->second;
		int tex_width = 0, tex_height = 0;
		SDL_QueryTexture(texture, nullptr, nullptr, &tex_width, &tex_height);
		SDL_Rect src =
		{
			(frame % columns) * (tex_width / columns),
			(frame / columns) * (tex_height / rows),
			tex_width / columns,
			tex_height / rows
		};
		SDL_Rect dst = { (int)(x - width / 2), (int)(y - height / 2), width, height };
		SDL_RenderCopyEx(renderer, texture, &src, &dst, angle, nullptr, flip);
	}

	void render_sheet_frame_center(SDL_Renderer* renderer, ResID res_id, int columns, int rows, int frame, double x, double y, int width, int height)
	{
		SDL_Texture* texture = ResourcesManager::instance()->get_texture_pool().find(res_id)->second;
		int tex_width = 0, tex_height = 0;
		SDL_QueryTexture(texture, nullptr, nullptr, &tex_width, &tex_height);
		SDL_Rect src =
		{
			(frame % columns) * (tex_width / columns),
			(frame / columns) * (tex_height / rows),
			tex_width / columns,
			tex_height / rows
		};
		SDL_Rect dst = { (int)(x - width / 2), (int)(y - height / 2), width, height };
		SDL_RenderCopy(renderer, texture, &src, &dst);
	}

	void render_whole_texture_center(SDL_Renderer* renderer, ResID res_id, double x, double y, int width, int height)
	{
		SDL_Texture* texture = ResourcesManager::instance()->get_texture_pool().find(res_id)->second;
		SDL_Rect dst = { (int)(x - width / 2), (int)(y - height / 2), width, height };
		SDL_RenderCopy(renderer, texture, nullptr, &dst);
	}

	bool generate_tile_map_texture(SDL_Renderer* renderer)
	{
		const Map& map = ConfigManager::instance()->map;
		const TileMap& tile_map = map.get_tile_map();
		SDL_Texture* tex_tile_set = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Tileset)->second;
		SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		int width_tex_tile_set = 0, height_tex_tile_set = 0;
		SDL_QueryTexture(tex_tile_set, nullptr, nullptr, &width_tex_tile_set, &height_tex_tile_set);
		int num_tile_single_line = (int)std::ceil((double)(width_tex_tile_set) / SIZE_TILE);

		int width_tex_tile_map = (int)map.get_width() * SIZE_TILE;
		int height_tex_tile_map = (int)map.get_height() * SIZE_TILE;
		tex_tile_map = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width_tex_tile_map, height_tex_tile_map);
		if (!tex_tile_map) return false;

		ConfigManager* config = ConfigManager::instance();
		rect_tile_map.x = (config->basic_template.window_width - width_tex_tile_map) / 2;
		rect_tile_map.y = (config->basic_template.window_height - height_tex_tile_map) / 2;
		rect_tile_map.w = width_tex_tile_map;
		rect_tile_map.h = height_tex_tile_map;

		SDL_SetTextureBlendMode(tex_tile_map, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(renderer, tex_tile_map);

		for (int y = 0; y < map.get_height(); y++)
		{
			for (int x = 0; x < map.get_width(); x++)
			{
				const Tile& cur_tile = tile_map[y][x];
				const SDL_Rect rect_tile_dst = { x * SIZE_TILE, y * SIZE_TILE, SIZE_TILE, SIZE_TILE };
				SDL_Rect rect_tile_src =
				{
					(cur_tile.terrian % num_tile_single_line) * SIZE_TILE,
					(cur_tile.terrian / num_tile_single_line) * SIZE_TILE,
					SIZE_TILE,
					SIZE_TILE
				};

				SDL_RenderCopy(renderer, tex_tile_set, &rect_tile_src, &rect_tile_dst);
				if (cur_tile.decoration >= 0)
				{
					rect_tile_src =
					{
						(cur_tile.decoration % num_tile_single_line) * SIZE_TILE,
						(cur_tile.decoration / num_tile_single_line) * SIZE_TILE,
						SIZE_TILE,
						SIZE_TILE
					};
					SDL_RenderCopy(renderer, tex_tile_set, &rect_tile_src, &rect_tile_dst);
				}
			}
		}

		const SDL_Point& idx_home = map.get_home_idx();
		const SDL_Rect rect_home_dst = { idx_home.x * SIZE_TILE, idx_home.y * SIZE_TILE, SIZE_TILE, SIZE_TILE };
		SDL_RenderCopy(renderer, ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Home)->second, nullptr, &rect_home_dst);

		SDL_SetRenderTarget(renderer, nullptr);
		return true;
	}

	bool check_click_home(const SDL_Point& idx_tile_cursor_selected)
	{
		static const Map& map = ConfigManager::instance()->map;
		const SDL_Point& idx_home = map.get_home_idx();
		return idx_home.x == idx_tile_cursor_selected.x && idx_home.y == idx_tile_cursor_selected.y;
	}

	bool get_cursor_idx_tile(SDL_Point& idx_tile_cursor_selected, int screen_x, int screen_y)
	{
		static const Map& map = ConfigManager::instance()->map;
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		if (screen_x < rect_tile_map.x || screen_x > rect_tile_map.x + rect_tile_map.w ||
			screen_y < rect_tile_map.y || screen_y > rect_tile_map.y + rect_tile_map.h)
			return false;

		idx_tile_cursor_selected.x = (std::min)((int)((screen_x - rect_tile_map.x) / SIZE_TILE), (int)map.get_width() - 1);
		idx_tile_cursor_selected.y = (std::min)((int)((screen_y - rect_tile_map.y) / SIZE_TILE), (int)map.get_height() - 1);
		return true;
	}

	bool can_place_tower(const SDL_Point& idx_tile_selected) const
	{
		static const Map& map = ConfigManager::instance()->map;
		if (idx_tile_selected.x < 0 || idx_tile_selected.y < 0 ||
			idx_tile_selected.x >= (int)map.get_width() || idx_tile_selected.y >= (int)map.get_height())
			return false;
		if (find_remote_tower(idx_tile_selected))
			return false;

		const Tile& selected_tile = map.get_tile_map()[idx_tile_selected.y][idx_tile_selected.x];
		return (selected_tile.decoration < 0) && (selected_tile.direction == Tile::Direction::None) &&
			(selected_tile.special_flag < 0) && (!selected_tile.has_tower);
	}

	const RemoteActor* find_remote_tower(const SDL_Point& idx_tile_selected) const
	{
		if (!network_mode || NetworkManager::instance()->hosting() || !remote_world.has_snapshot)
			return nullptr;

		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		for (const RemoteActor& tower : remote_world.towers)
		{
			int tower_tile_x = (int)((tower.x - rect_tile_map.x) / SIZE_TILE);
			int tower_tile_y = (int)((tower.y - rect_tile_map.y) / SIZE_TILE);
			if (tower_tile_x == idx_tile_selected.x && tower_tile_y == idx_tile_selected.y)
				return &tower;
		}
		return nullptr;
	}

	void get_selected_tile_center_position(SDL_Point& pos, const SDL_Point& idx_tile_selected)
	{
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		pos.x = rect_tile_map.x + idx_tile_selected.x * SIZE_TILE + SIZE_TILE / 2;
		pos.y = rect_tile_map.y + idx_tile_selected.y * SIZE_TILE + SIZE_TILE / 2;
	}
};
