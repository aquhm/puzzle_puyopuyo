#pragma once
#include <cstdint>

enum class PlayerEventType
{
    GameOver,    
    GameRestart,
    AddInterruptBlock,
    AttackInterruptBlock,
    DefenseBlock
};

class BasePlayerEvent
{
public:
    explicit BasePlayerEvent(PlayerEventType type, uint8_t playerId)
        : type_(type), player_id_(playerId) {
    }

    virtual ~BasePlayerEvent() = default;

    PlayerEventType GetType() const { return type_; }
    uint8_t GetPlayerId() const { return player_id_; }

private:
    PlayerEventType type_;
    uint8_t player_id_;
};

class GameOverEvent : public BasePlayerEvent
{
public:
    GameOverEvent(uint8_t playerId, bool isWin)
        : BasePlayerEvent(PlayerEventType::GameOver, playerId), is_win_(isWin) {
    }

    bool IsWin() const { return is_win_; }

private:
    bool is_win_;
};

class GameRestartEvent : public BasePlayerEvent
{
public:
    explicit GameRestartEvent(uint8_t player_id)
        : BasePlayerEvent(PlayerEventType::GameRestart, player_id) {
    }
};

class AddInterruptBlockEvent : public BasePlayerEvent
{
public:
    explicit AddInterruptBlockEvent(uint8_t player_id, uint16_t count)
        : BasePlayerEvent(PlayerEventType::AddInterruptBlock, player_id), count_(count) {
    }

    uint16_t GetCount() const { return count_; }

private:
	int16_t count_;
};

class AttackInterruptBlockEvent : public BasePlayerEvent
{
public:
    explicit AttackInterruptBlockEvent(uint8_t player_id, float x, float y, uint8_t type)
        : BasePlayerEvent(PlayerEventType::AttackInterruptBlock, player_id), x_(x), y_(y), type_(type) {
    }

    float GetX() const { return x_; }
    float GetY() const { return y_; }
    uint8_t GetType() const { return type_; }

private:
    float x_;
    float y_;
    uint8_t type_;
};

// 방어 블록 이벤트
class DefenseBlockEvent : public BasePlayerEvent
{
public:
    DefenseBlockEvent(uint8_t playerId, int16_t blockCount, float x, float y, uint8_t type)
        : BasePlayerEvent(PlayerEventType::DefenseBlock, playerId),
        block_count_(blockCount), x_(x), y_(y), type_(type) {
    }

    int16_t GetBlockCount() const { return block_count_; }
    float GetX() const { return x_; }
    float GetY() const { return y_; }
    uint8_t GetType() const { return type_; }

private:
    int16_t block_count_;
    float x_;
    float y_;
    uint8_t type_;
};