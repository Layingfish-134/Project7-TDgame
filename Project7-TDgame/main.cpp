#define SDL_MAIN_HANDLED

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include"game_manager.h"

static void set_working_directory_to_executable()
{
#ifdef _WIN32
	wchar_t path[MAX_PATH] = { 0 };
	if (GetModuleFileNameW(nullptr, path, MAX_PATH) == 0)
		return;

	for (int i = lstrlenW(path) - 1; i >= 0; --i)
	{
		if (path[i] == L'\\' || path[i] == L'/')
		{
			path[i] = L'\0';
			SetCurrentDirectoryW(path);
			return;
		}
	}
#endif
}

int main(int argc , char* argv[])
{
	set_working_directory_to_executable();
	return GameManager::instance()->run(argc,argv);
}
