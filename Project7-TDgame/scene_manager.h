#pragma once

#include "manager.h"
#include "network_manager.h"
#include "scene.h"

#include <SDL.h>

class SceneManager : public Manager<SceneManager>
{
	friend class Manager<SceneManager>;

public:
	void set_renderer(SDL_Renderer* renderer)
	{
		this->renderer = renderer;
	}

	void switch_to(Scene* next_scene)
	{
		if (is_dispatching)
		{
			delete pending_scene;
			pending_scene = next_scene;
			return;
		}

		switch_now(next_scene);
	}

	void switch_now(Scene* next_scene)
	{
		if (current_scene)
		{
			current_scene->on_exit();
			delete current_scene;
		}

		current_scene = next_scene;
		if (current_scene)
			current_scene->on_enter(renderer);
	}

	void on_input(const SDL_Event& event)
	{
		is_dispatching = true;
		if (current_scene)
			current_scene->on_input(event);
		is_dispatching = false;
		flush_pending_scene();
	}

	void on_update(double delta)
	{
		is_dispatching = true;
		if (current_scene)
			current_scene->on_update(delta);
		is_dispatching = false;
		flush_pending_scene();
	}

	void on_render(SDL_Renderer* renderer)
	{
		is_dispatching = true;
		if (current_scene)
			current_scene->on_render(renderer);
		is_dispatching = false;
		flush_pending_scene();
	}

	void request_quit()
	{
		quit_requested = true;
	}

	bool should_quit() const
	{
		return quit_requested;
	}

	void shutdown()
	{
		switch_to(nullptr);
	}

	void goto_main_menu();
	void goto_save_slots();
	void goto_level_select();
	void goto_game(int level_id, bool network_mode = false, NetworkRole role = NetworkRole::Offline);
	void goto_network_menu();

protected:
	SceneManager() = default;
	~SceneManager()
	{
		shutdown();
	}

private:
	SDL_Renderer* renderer = nullptr;
	Scene* current_scene = nullptr;
	Scene* pending_scene = nullptr;
	bool quit_requested = false;
	bool is_dispatching = false;

	void flush_pending_scene()
	{
		if (!pending_scene)
			return;

		Scene* next_scene = pending_scene;
		pending_scene = nullptr;
		switch_now(next_scene);
	}
};
