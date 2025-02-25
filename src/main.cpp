/**
 * 
 * 설명: 프로그램 진입점
 * 1. SDL_MAIN_USE_CALLBACKS 1 define 활성화시 이벤트 콜백 기반 메커니즘으로 동작
 * 2. <SDL3/SDL_main.h>이 포함되어야 합니다.
 * 3. 반환: SDL_APP_CONTINUE(성공), SDL_APP_FAILURE(실패)
 * 4. https://github.com/libsdl-org/SDL/blob/main/docs/README-migration.md
 * 
 */
#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL_main.h>
#include "./core/GameApp.hpp"


SDL_AppResult SDL_AppInit(void** appState, int argc, char* argv[])
{
	if (!GAME_APP.Initialize()) 
	{
		return SDL_APP_FAILURE;
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appState, SDL_Event* event)
{
	if (event->type == SDL_EVENT_QUIT) 
	{
		GAME_APP.GetInstance().SetGameRunning(false);
		return SDL_APP_SUCCESS;
	}

	GAME_APP.HandleEvents(*event);
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appState)
{
	GAME_APP.MainLoop();

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appState, SDL_AppResult result)
{
	GAME_APP.Release();
}

//int main(int argc, char* args[])
//{
//	if (GAME_APP.Initialize())
//	{
//		GAME_APP.MainLoop();
//	}
//
//	GAME_APP.Release();
//
//	return 0;
//}