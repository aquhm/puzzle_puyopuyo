//#pragma once
//
//#include <functional>
//#include <unordered_map>
//#include "GameTypes.hpp"
//
//class GameStateTransition {
//public:
//    using TransitionCallback = std::function<void()>;
//
//    struct TransitionRule {
//        GameStatus nextState;
//        TransitionCallback onTransition;
//        bool needsConfirmation;
//    };
//
//    void addTransition(GameStatus from, GameStatus to,
//        TransitionCallback callback = nullptr,
//        bool needsConfirmation = false);
//
//    bool canTransition(GameStatus from, GameStatus to) const;
//    void executeTransition(GameStatus& current, GameStatus to);
//
//private:
//    std::unordered_map
//        GameStatus,
//        std::unordered_map<GameStatus, TransitionRule>
//    > transitions;
//};