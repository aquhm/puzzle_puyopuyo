#pragma once

#include <SDL3/SDL.h>

namespace GameConstants {
    // Game board dimensions
    constexpr int BOARD_WIDTH = 6;
    constexpr int BOARD_HEIGHT = 13;

    // Time constants (in milliseconds)
    constexpr int SECOND = 1000;
    constexpr int MINUTE = 60 * SECOND;
    constexpr int HOUR = 60 * MINUTE;
    constexpr int DAY = 24 * HOUR;

    // Physics constants
    constexpr float PI = 3.141592f;
    constexpr float CIRCLE_ANGLE = 360.0f;
    constexpr float EPSILON = 0.001f;
    constexpr float GRAVITY = -9.8f;
    constexpr float ADD_VELOCITY = 10.0f;
}

// Direction enumeration
enum class Direction {
    None = -1,
    Left,
    Top,
    Right,
    Bottom
};