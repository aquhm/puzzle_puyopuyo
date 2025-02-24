#pragma once

/**
 *
 * 설명: windows system message 수신을 위한 handler
 * 1. 기존 SDL_SYSWMEVENT가 deprecated되고 SDL_SetWindowsMessageHook으로 후킹 방식으로 변경
 * 2. 참고 https://github.com/libsdl-org/SDL/blob/main/docs/README-migration.md
 */

#include <SDL3/SDL.h>

class WindowsMessageHandler 
{
public:
    WindowsMessageHandler() = default;
    ~WindowsMessageHandler() = default;
        
    WindowsMessageHandler(const WindowsMessageHandler&) = delete;
    WindowsMessageHandler& operator=(const WindowsMessageHandler&) = delete;
    WindowsMessageHandler(WindowsMessageHandler&&) = delete;
    WindowsMessageHandler& operator=(WindowsMessageHandler&&) = delete;;

    void SetupWindowsMessageHook();

private:
    static bool SDLCALL MyWindowsMessageHook(void* userdata, MSG* msg);
};
