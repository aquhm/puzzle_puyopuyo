#pragma once
#include <SDL3/SDL_rect.h>


namespace RectUtils
{
    inline void ConvertFRectToRect(const SDL_FRect& frect, SDL_Rect* rect)
    {
        if (rect) 
        {
            rect->x = static_cast<int>(frect.x);
            rect->y = static_cast<int>(frect.y);
            rect->w = static_cast<int>(frect.w);
            rect->h = static_cast<int>(frect.h);
        }
    }    
}