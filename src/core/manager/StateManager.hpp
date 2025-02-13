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

    // ���� ������ ���� ������
    enum class StateID {
        Login,
        Room,
        CharSelect,
        Game,
        Max
    };


    StateManager();  // �⺻ ������ ����
    ~StateManager() override = default;

    // ����/�̵� ����
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    StateManager(StateManager&&) = delete;
    StateManager& operator=(StateManager&&) = delete;


    // IManager �������̽� ����
    [[nodiscard]] std::string_view GetName() const override
    {
        return "StateManager";
    }


    [[nodiscard]] bool Initialize() override;
    void Update(float deltaTime) override;
    void Release() override;
   
    // ���� ��ȯ ���� �޼���
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
