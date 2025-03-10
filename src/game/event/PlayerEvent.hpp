#pragma once
#include <cstdint>

enum class PlayerEventType
{
    GameOver,    
    GameRestart,
    AddInterruptBlock,
    AttackInterruptBlock
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

    uint16_t GetX() const { return x_; }
    uint16_t GetY() const { return y_; }
    uint8_t GetType() const { return type_; }

private:
    float x_;
    float y_;
    uint8_t type_;
};

// 콤보 공격 이벤트
//class ComboAttackEvent : public BasePlayerEvent
//{
//public:
//    ComboAttackEvent(uint8_t playerId, uint8_t comboCount, int16_t damage)
//        : BasePlayerEvent(PlayerEventType::ComboAttack, playerId),
//        combo_count_(comboCount), damage_(damage) {
//    }
//
//    uint8_t GetComboCount() const { return combo_count_; }
//    int16_t GetDamage() const { return damage_; }
//
//private:
//    uint8_t combo_count_;
//    int16_t damage_;
//};
//
//// 방어 블록 이벤트
//class DefenseBlockEvent : public BasePlayerEvent
//{
//public:
//    DefenseBlockEvent(uint8_t playerId, int16_t blockCount)
//        : BasePlayerEvent(PlayerEventType::DefenseBlock, playerId),
//        block_count_(blockCount) {
//    }
//
//    int16_t GetBlockCount() const { return block_count_; }
//
//private:
//    int16_t block_count_;
//};
//
//// 점수 변경 이벤트
//class ScoreChangedEvent : public BasePlayerEvent
//{
//public:
//    ScoreChangedEvent(uint8_t playerId, int32_t scoreAmount, uint8_t comboCount)
//        : BasePlayerEvent(PlayerEventType::ScoreChanged, playerId),
//        score_amount_(scoreAmount), combo_count_(comboCount) {
//    }
//
//    int32_t GetScoreAmount() const { return score_amount_; }
//    uint8_t GetComboCount() const { return combo_count_; }
//
//private:
//    int32_t score_amount_;
//    uint8_t combo_count_;
//};
