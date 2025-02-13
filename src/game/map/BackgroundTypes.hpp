#pragma once

#include <cstdint>

namespace bg {

    // Background type identifiers
    enum class BackgroundType : uint8_t 
    {
        GrassLand = 0,
        IceLand = 13
    };

    // Background constants
    struct BackgroundConstants 
    {
        static constexpr float ROTATION_ANIMATION_SPEED = 70.0f;
        static constexpr int MASK_POSITION_X = 240;
        static constexpr int MASK_POSITION_Y = 63;
        static constexpr int MASK_WIDTH = 256;
        static constexpr int MASK_HEIGHT = 128;
        static constexpr float NEW_BLOCK_VELOCITY = 200.0f;
        static constexpr float NEW_BLOCK_SCALE_VELOCITY = 60.0f;
    };

}