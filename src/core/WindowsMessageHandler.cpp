#include "WindowsMessageHandler.hpp"

#include "common/constants/Constants.hpp"
#include "../network/GameClient.hpp"
#include "../network/NetworkController.hpp"

#include <SDL3/SDL_system.h>

void WindowsMessageHandler::SetupWindowsMessageHook() 
{
    SDL_SetWindowsMessageHook(MyWindowsMessageHook, this);
}

bool SDLCALL WindowsMessageHandler::MyWindowsMessageHook(void* userdata, MSG* msg) 
{
    WindowsMessageHandler* handler = static_cast<WindowsMessageHandler*>(userdata);

    HWND hwnd = msg->hwnd;
    UINT message = msg->message;
    WPARAM wParam = msg->wParam;
    LPARAM lParam = msg->lParam;

    switch (message)
    {
    case SDL_USEREVENT_SOCK:
        NETWORK.ProcessRecv(wParam, lParam);
        break;
    }

    return true;
}