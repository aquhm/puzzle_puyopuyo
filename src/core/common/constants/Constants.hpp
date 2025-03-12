#pragma once

#include <cmath>
#include <array>
#include <numbers>
#include <SDL3/SDL.h>
#include <limits>
#include <cfloat>

#define SDL_USEREVENT_SOCK		WM_USER + 1


namespace Constants
{
    enum class PlayerType : uint8_t
    {
        Local,
        Remote
    };

    enum class Direction
    {
        None = -1,
        Left,
        Top,
        Right,
        Bottom
    };

    inline namespace Game
    {
        constexpr float PLAY_START_DELAY = 2.0f;
        constexpr float MATCH_ANIMATION_DURATION = 0.5f;

        constexpr int MAX_COMBO = 19;
        constexpr int MIN_MATCH_COUNT = 4;

        constexpr float DEFAULT_DROP_SPEED = 1.0f;
        constexpr float FAST_DROP_SPEED = 10.0f;

        constexpr struct TimeMargin
        {
            float time;
            uint8_t margin;
        }
        SCORE_MARGINS[] =
        {
            {96.0f,  70},
            {112.0f, 52},
            {128.0f, 34},
            {144.0f, 25},
            {160.0f, 16},
            {176.0f, 12},
            {192.0f, 8},
            {208.0f, 6},
            {224.0f, 4},
            {240.0f, 3},
            {256.0f, 2},
            { FLT_MAX, 1 }
        };

        inline namespace Score 
        {
            constexpr int BASE_MATCH_SCORE = 10;
            constexpr int COMBO_MULTIPLIER_BASE = 2;
            constexpr int MAX_LINK_BONUS = 10;
            constexpr int MAX_TYPE_BONUS = 24;
        }

        inline namespace CharacterSelect
        {
            constexpr int CHARACTER_GRID_HEIGHT = 4;
            constexpr int CHARACTER_GRID_WIDTH = 7;
        }
    }

    inline namespace Math
    {
        constexpr double PI = std::numbers::pi;
        constexpr float PI_F = std::numbers::pi_v<float>;
        constexpr float DEGREES_TO_RADIANS = PI_F / 180.0f;
        constexpr float RADIANS_TO_DEGREES = 180.0f / PI_F;
        constexpr float EPSILON = 0.001f;
        constexpr float CIRCLE_ANGLE = 360.0f;

        constexpr int SECOND = 1000;
        constexpr int MINUTE = 60 * SECOND;
        constexpr int HOUR = 60 * MINUTE;
        constexpr int DAY = 24 * HOUR;
    }

    inline namespace Time
    {
        constexpr int SECOND = 1000;
        constexpr int MINUTE = 60 * SECOND;
        constexpr int HOUR = 60 * MINUTE;
        constexpr int DAY = 24 * HOUR;
        constexpr float FRAME_TIME = 1.0f / 60.0f;
    }

    inline namespace Block
    {
        constexpr float SIZE = 31.0f;                
        constexpr float GRAVITY = 9.8f;              
        constexpr float CHANGE_TIME = 0.2f;          

        constexpr float EFFECT_COMPRESS_TIME = 0.1f; 
        constexpr float EFFECT_EXPAND_TIME = 0.15f;  
        constexpr float EFFECT_DESTROY_TIME = 0.5f;  

        constexpr int DESTROY_DELTA_SIZE = 21;       
        constexpr float DESTROY_POS_VELOCITY = (DESTROY_DELTA_SIZE * 0.5f) / EFFECT_DESTROY_TIME;

        constexpr float DESTROY_EXPAND_TIME = 0.5f;  
        constexpr int DESTROY_EXPAND_DELTA_SIZE = 10;
        constexpr float DESTROY_EXPAND_POS_VELOCITY = (DESTROY_EXPAND_DELTA_SIZE * 0.5f) / DESTROY_EXPAND_TIME;

        constexpr int SHATTERING_DOWN_SPEED = 50;    

        constexpr float DOWN_VELOCITY = 0.05f;               // 블록 하강 속도
        constexpr float ROTATE_VELOCITY = 500.0f;           // 블록 회전 속도
        constexpr float HORIZONTAL_VELOCITY = SIZE / 90.0f; // 수평 이동 속도
    }

    inline namespace GroupBlock
    {
        constexpr int NEXT_BLOCK_POS_X = 16;
        constexpr int NEXT_BLOCK_POS_Y = 10;
        constexpr int NEXT_BLOCK_POS_SMALL_X = 42;
        constexpr int NEXT_BLOCK_POS_SMALL_Y = 75;

        constexpr int NEXT_PLAYER_BLOCK_POS_X = 111;
        constexpr int NEXT_PLAYER_BLOCK_POS_Y = 10;
        constexpr int NEXT_PLAYER_BLOCK_POS_SMALL_X = 94;
        constexpr int NEXT_PLAYER_BLOCK_POS_SMALL_Y = 75;

        constexpr int NEXT_BLOCK_SMALL_SIZE = 21;
        constexpr int COUNT = 2;

        constexpr float ADD_VELOCITY = 10.0f;
    }

    namespace Board
    {
        constexpr int BOARD_X_COUNT = 6;
        constexpr int BOARD_Y_COUNT = 13;

        constexpr float POSITION_X = 30;
        constexpr float POSITION_Y = 32;

        constexpr float PLAYER_POSITION_X = 416;
        constexpr float PLAYER_POSITION_Y = 32;

        constexpr int WIDTH = 194;
        constexpr int HEIGHT = 372;

        constexpr float WIDTH_MARGIN = 4;

        constexpr float NEW_BLOCK_POS_X = WIDTH_MARGIN + Block::SIZE * 2;
        constexpr float NEW_BLOCK_POS_Y = 0;

        constexpr float NEW_BLOCK_ANIM_SPEED = 0.15f;

        constexpr float BLOCK_MARK_POS_X = 480;
        constexpr float BLOCK_MARK_POS_Y = 225;
        constexpr float BLOCK_MARK_SIZE = 15;

        constexpr float ATTACK_SPEED = 350.0f;
        constexpr float ROTATE_SPEED = 3000.0f;
        constexpr float CURVE_SPEED = 2.0f;
    }

    inline namespace Window
    {
        constexpr int DEFAULT_WIDTH = 640;
        constexpr int DEFAULT_HEIGHT = 448;
        constexpr int MIN_WIDTH = 640;
        constexpr int MIN_HEIGHT = 448;
    }

    inline namespace Background
    {
        constexpr int MASK_WIDTH = 256;
        constexpr int MASK_HEIGHT = 128;
        constexpr int MASK_POSITION_X = 240;
        constexpr int MASK_POSITION_Y = 63;
        constexpr float NEW_BLOCK_VELOCITY = 100.0f;
        constexpr float NEW_BLOCK_SCALE_VELOCITY = 50.0f;
    }


    inline namespace Network
    {
        constexpr int NETWORK_EVENT_CODE = 1;

        constexpr int NET_PORT = 9000;

        constexpr int PACKET_SIZE_LEN = sizeof(unsigned int);
        constexpr int MAX_PACKET_SIZE = 256;
        constexpr int PACKET_DATA_SIZE_LEN = MAX_PACKET_SIZE - PACKET_SIZE_LEN;
        constexpr int MAX_WORKERTHREAD = 1;
        constexpr int MAX_CLIENT = 4;
        constexpr int MAX_RINGBUFSIZE = 1024;

        constexpr int CLIENT_BUF_SIZE = 256;
        constexpr int MAX_CHAT_LEN = 100;
    }

    inline namespace BulletEffect
    {
        constexpr int SIZE = 18;
        constexpr float BULLET_AMPLITUDE = 100.0f;       // 발사체 진폭 크기
        constexpr float BULLET_FORCE = 1500.0f;          // 발사체 기본 힘
        constexpr float SHOCK_EXPAND_TIME = 0.3f;
        constexpr float SHOCK_EXPAND_DELTA_SIZE = 100.0f;
        constexpr float SHOCK_EXPAND_POS_VELOCITY = (SHOCK_EXPAND_DELTA_SIZE * 0.5f) / SHOCK_EXPAND_TIME;
    }

    inline namespace Particle
    {
        // 블록 폭파 이펙트 관련 상수(ExplosionParticle)
        namespace Explosion
        {
            constexpr int SIZE = 18;
            constexpr int TEXTURE_POS_X = 161;
            constexpr int TEXTURE_POS_Y = 353;
            constexpr int TEXTURE_SPACING = 14;
            constexpr float GRAVITY = 98.0f;
            constexpr float DEFAULT_LIFETIME = 5.0f;
            constexpr size_t PARTICLE_COUNT = 10;
        }
    }   

    namespace DirectionInfo 
    {
        // 방향 오프셋 (순서: 좌, 우, 상, 하)
        inline static const std::array<std::pair<int, int>, 4> OFFSETS = 
        {
            std::make_pair(-1, 0),  // Left
            std::make_pair(1, 0),   // Right 
            std::make_pair(0, 1),   // Top
            std::make_pair(0, -1)   // Bottom
        };

        // 방향과 오프셋 (방향 정보 포함)
        inline static const std::array<std::pair<Constants::Direction, std::pair<int, int>>, 4> DIRECTION_OFFSETS =
        {
            std::make_pair(Constants::Direction::Left, std::make_pair(-1, 0)),
            std::make_pair(Constants::Direction::Right, std::make_pair(1, 0)),
            std::make_pair(Constants::Direction::Top, std::make_pair(0, 1)),
            std::make_pair(Constants::Direction::Bottom, std::make_pair(0, -1))
        };

        // 위치와 방향에서 새 위치 계산
        inline std::pair<int, int> GetNewPosition(int x, int y, Constants::Direction dir) 
        {
            switch (dir) 
            {
            case Constants::Direction::Left:
                return { x - 1, y };
            case Constants::Direction::Right:
                return { x + 1, y };
            case Constants::Direction::Top:
                return { x, y + 1 };
            case Constants::Direction::Bottom:
                return { x, y - 1 };
            default:
                return { x, y };
            }
        }
    }
}

