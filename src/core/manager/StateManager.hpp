#pragma once
// StateManager.hpp
#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <queue>
#include "../../states/GameState.hpp"
#include "IManager.hpp"


class StateManager final : public IManager {
public:

    // 상태 관리를 위한 열거형
    enum class StateID {
        Login,
        Room,
        CharSelect,
        Game,
        Max
    };


    StateManager();  // 기본 생성자 선언
    ~StateManager() override = default;

    // 복사/이동 방지
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    StateManager(StateManager&&) = delete;
    StateManager& operator=(StateManager&&) = delete;


    // IManager 인터페이스 구현
    [[nodiscard]] std::string_view GetName() const override
    {
        return "StateManager";
    }


    [[nodiscard]] bool Initialize() override;
    void Update(float deltaTime) override;
    void Release() override;
   
    // 상태 전환 관련 메서드
    void requestStateChange(StateID newState);

    [[nodiscard]] std::shared_ptr<BaseState> getCurrentState() const;
    [[nodiscard]] StateID getCurrentStateID() const { return currentStateId_; }

private:

    void changeState(StateID newState);
    void initializeStates();

    std::unordered_map<StateID, std::shared_ptr<BaseState>> states_;
    std::queue<StateID> stateChangeQueue_;

    std::shared_ptr<BaseState> currentState_;

    StateID currentStateId_{ StateID::Max };
    bool initialized_{ false };
};
