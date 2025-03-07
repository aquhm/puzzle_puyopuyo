#pragma once
#include <cstdint>

enum class PlayerEventType
{
    GameOver,    
    GameRestart,
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
        : BasePlayerEvent(player_id, PlayerEventType::GameRestart) {
    }
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
