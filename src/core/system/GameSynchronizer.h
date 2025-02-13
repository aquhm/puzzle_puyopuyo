// game/GameSynchronizer.hpp
#pragma once

#include <memory>
#include <queue>
#include "NetworkTypes.hpp"
#include "GameTypes.hpp"

class GameSynchronizer {
public:
    struct GameSnapshot {
        std::vector<BlockState> blocks;
        std::vector<ScoreInfo> scores;
        GameStatus gameStatus;
        float timestamp;
    };

    struct NetworkCommand {
        enum class Type {
            BlockMove,
            BlockMatch,
            ScoreUpdate,
            StateChange,
            GameSync
        };

        Type type;
        std::vector<uint8_t> data;
        float timestamp;
    };

    void queueCommand(NetworkCommand cmd);
    void update(float deltaTime);
    bool hasCommandsToProcess() const;
    NetworkCommand getNextCommand();

private:
    static constexpr float SYNC_INTERVAL = 0.1f;  // 100ms
    static constexpr float MAX_LAG_COMPENSATION = 0.15f;  // 150ms

    std::queue<NetworkCommand> commandQueue;
    float syncTimer{ 0.0f };

    void reconcileGameState(const GameSnapshot& serverState);
};