#include "StateManager.hpp"
#include "../../states/LoginState.hpp"
#include "../../states/RoomState.hpp"
#include "../../states/CharacterSelectState.hpp"
#include "../../states/GameState.hpp"
#include "../../utils/Logger.hpp"
#include <format>

StateManager::StateManager()
    : currentStateId_(StateID::Max)
    , currentState_(nullptr)
    , initialized_(false)
    , paused_(false)
{
}

bool StateManager::Initialize()
{
    if (initialized_) 
    {
        LOGGER.Warning("StateManager is already initialized");
        return true;
    }

    try 
    {
        InitializeStates();
        initialized_ = true;
        return true;
    }
    catch (const std::exception& e) 
    {
        LOGGER.Error("Failed to initialize StateManager: {}", e.what());
        return false;
    }
}

void StateManager::InitializeStates()
{
    // 각 상태 생성 및 초기화
    states_[StateID::Login] = std::make_shared<LoginState>();
    states_[StateID::Room] = std::make_shared<RoomState>();
    states_[StateID::CharSelect] = std::make_shared<CharacterSelectState>();
    states_[StateID::Game] = std::make_shared<GameState>();

    // 각 상태 초기화
    for (auto& [id, state] : states_) 
    {
        if (!state->Init()) 
        {
            throw std::runtime_error(std::format("Failed to initialize state: {}",static_cast<int>(id)));
        }
    }

    // 초기 상태 설정
    ChangeState(StateID::Login);
}

void StateManager::Update(float deltaTime)
{
    if (!initialized_ || paused_) 
    {
        return;
    }

    // 상태 변경 요청 처리
    ProcessStateChangeRequests();

    // 현재 상태 업데이트
    if (currentState_) 
    {
        currentState_->Update(deltaTime);
    }
}

void StateManager::ProcessStateChangeRequests()
{
    while (!stateChangeQueue_.empty()) 
    {
        StateID newState = stateChangeQueue_.front();
        ChangeState(newState);
        stateChangeQueue_.pop();
    }
}

void StateManager::Render()
{
    if (currentState_ != nullptr)
    {
        currentState_->Render();
    }
}

void StateManager::HandleEvent(const SDL_Event& event)
{
    if (!initialized_ || paused_ || !currentState_) 
    {
        return;
    }

    currentState_->HandleEvent(event);
}

void StateManager::HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length)
{
    if (!initialized_ || paused_ || !currentState_) 
    {
        return;
    }

    currentState_->HandleNetworkMessage(connectionId, message, length);
}

void StateManager::RequestStateChange(StateID newState)
{
    if (currentStateId_ == newState) 
    {
        return;
    }

    stateChangeQueue_.push(newState);
}

void StateManager::ChangeState(StateID newState)
{
    auto it = states_.find(newState);

    if (it == states_.end()) 
    {
        throw std::runtime_error(std::format("Invalid state requested: {}", static_cast<int>(newState)));
    }

    if (currentState_) 
    {
        currentState_->Leave();
    }

    currentStateId_ = newState;
    currentState_ = it->second;
    currentState_->Enter();

    LOGGER.Info("State changed to: {}", static_cast<int>(newState));
}

void StateManager::PauseCurrentState()
{
    if (!paused_ && currentState_) 
    {
        paused_ = true;
        // 필요한 경우 상태 일시 중지 로직 추가
    }
}

void StateManager::ResumeCurrentState()
{
    if (paused_ && currentState_) 
    {
        paused_ = false;
        // 필요한 경우 상태 재개 로직 추가
    }
}

void StateManager::Release()
{
    if (!initialized_) 
    {
        return;
    }

    if (currentState_) 
    {
        currentState_->Leave();
    }

    // 모든 상태들의 리소스 해제
    for (auto& [id, state] : states_) 
    {
        if (state) 
        {
            state->Leave();
            state->Release();
        }
    }

    // 컨테이너들 정리
    states_.clear();
    while (!stateChangeQueue_.empty()) 
    {
        stateChangeQueue_.pop();
    }

    currentState_.reset();
    currentStateId_ = StateID::Max;
    initialized_ = false;
    paused_ = false;
}