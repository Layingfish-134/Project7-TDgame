#pragma once

#include "scene_manager.h"
#include "main_menu_scene.h"
#include "save_slot_scene.h"
#include "level_select_scene.h"
#include "game_scene.h"
#include "network_scene.h"

inline void SceneManager::goto_main_menu()
{
	switch_to(new MainMenuScene());
}

inline void SceneManager::goto_save_slots()
{
	switch_to(new SaveSlotScene());
}

inline void SceneManager::goto_level_select()
{
	switch_to(new LevelSelectScene());
}

inline void SceneManager::goto_game(int level_id, bool network_mode, NetworkRole role)
{
	switch_to(new GameScene(level_id, network_mode, role));
}

inline void SceneManager::goto_network_menu()
{
	switch_to(new NetworkScene());
}
