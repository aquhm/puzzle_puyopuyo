#pragma once
/*
 *
 * 설명: BaseState를 구현한 State들을 관리하는 Manager Class
 *
 */

#include "IManager.hpp"
#include "../../states/BaseState.hpp"
#include "../IEventHandler.hpp"
#include "../IRenderable.hpp"

#include <memory>
#include <unordered_map>
#include <string>
#include <concepts>
#include <queue>


class StateManager final : public IManager, public IEventHandler, public IRenderable
{
public:

    enum class StateID
    {
        Login,
        Room,
        CharSelect,
        Game,
        Max
    };
    
    using StateMap = std::unordered_map<StateID, std::shared_ptr<BaseState>>;    

    StateManager();
    ~StateManager() override = default;

    // 복사/이동 방지
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    StateManager(StateManager&&) = delete;
    StateManager& operator=(StateManager&&) = delete;


    [[nodiscard]] std::string_view GetName() const override { return "StateManager"; }
    [[nodiscard]] bool Initialize() override;
    void Update(float deltaTime) override;
    void Release() override;
    void Render() override;

    
    void HandleEvent(const SDL_Event& event) override;
    void HandleNetworkMessage(uint8_t connectionId, std::span<const char> message, uint32_t length);

    
    void RequestStateChange(StateID newState);
    void PauseCurrentState();
    void ResumeCurrentState();
    
    [[nodiscard]] const std::shared_ptr<BaseState>& GetCurrentState() const { return current_state_; }
    [[nodiscard]] StateID GetCurrentStateID() const { return current_state_id_; }
    template<std::derived_from<BaseState> T>
    [[nodiscard]] const std::shared_ptr<T>& GetState(StateID stateId) const;

public:
    void ChangeState(StateID newState);
    void InitializeStates();
    void ProcessStateChangeRequests();
    
    StateMap states_;
    std::queue<StateID> state_change_queue_;
    std::shared_ptr<BaseState> current_state_;
    StateID current_state_id_{ StateID::Max };
    bool initialized_{ false };
    bool paused_{ false };
};


template<std::derived_from<BaseState> T>
[[nodiscard]] const std::shared_ptr<T>& StateManager::GetState(StateID stateId) const
{
    auto it = states_.find(stateId);
    if (it != states_.end()) 
    {
        return std::dynamic_pointer_cast<T>(it->second);
    }
    return nullptr;
}