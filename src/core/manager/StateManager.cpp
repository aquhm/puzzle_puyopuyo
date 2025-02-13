// StateManager.cpp
#include "StateManager.hpp"
#include <stdexcept>
#include "../../states/LoginState.hpp"
#include "../../states/RoomState.hpp"
#include "../../states/CharacterSelectState.hpp"

StateManager::StateManager() 
    : currentStateId_(StateID::Max)
    , currentState_(nullptr)
    , initialized_(false)
{
    
}

void StateManager::initializeStates() 
{
    // 스마트 포인터로 상태들 생성
    states_[StateID::Login] = std::make_shared<LoginState>();
    states_[StateID::Room] = std::make_shared<RoomState>();
    states_[StateID::CharSelect] = std::make_shared<CharacterSelectState>();
    states_[StateID::Game] = std::make_shared<GameState>();

    // 초기 상태 설정
    changeState(StateID::Login);
}

void StateManager::requestStateChange(StateID newState) 
{
    if (currentStateId_ == newState) 
    {
        return;
    }
    stateChangeQueue_.push(newState);
}

bool StateManager::Initialize()
{
    initializeStates();

    return true;
}

void StateManager::Update(float deltaTime)
{
    // 상태 변경 요청 처리
    while (stateChangeQueue_.empty() == false) 
    {
        StateID newState = stateChangeQueue_.front();
        changeState(newState);
        stateChangeQueue_.pop();
    }

    // 현재 상태 업데이트
    if (currentState_ != nullptr) 
    {
        currentState_->Update(deltaTime);
    }
}

void StateManager::Release()
{
    if (currentState_ != nullptr) 
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

    // 현재 상태 관련 변수들 리셋
    //currentState_->reset();
    currentStateId_ = StateID::Max;
    initialized_ = false;

}

void StateManager::changeState(StateID newState) 
{
    if (currentState_) 
    {
        currentState_->Leave();
    }

    auto it = states_.find(newState);
    
    if (it == states_.end()) 
    {
        throw std::runtime_error("Invalid state requested");
    }

    currentStateId_ = newState;
    currentState_ = it->second;
    currentState_->Enter();
}

std::shared_ptr<BaseState> StateManager::getCurrentState() const 
{
    return currentState_;
}