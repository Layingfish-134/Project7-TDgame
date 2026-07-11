#pragma once

#include <SDL.h>

class Scene
{
public:
	virtual ~Scene() = default;

	virtual void on_enter(SDL_Renderer* renderer) {}
	virtual void on_exit() {}
	virtual void on_input(const SDL_Event& event) = 0;
	virtual void on_update(double delta) = 0;
	virtual void on_render(SDL_Renderer* renderer) = 0;
};
