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
    // �� ���� ���� �� �ʱ�ȭ
    states_[StateID::Login] = std::make_shared<LoginState>();
    states_[StateID::Room] = std::make_shared<RoomState>();
    states_[StateID::CharSelect] = std::make_shared<CharacterSelectState>();
    states_[StateID::Game] = std::make_shared<GameState>();

    // �� ���� �ʱ�ȭ
    for (auto& [id, state] : states_) 
    {
        if (!state->Init()) 
        {
            throw std::runtime_error(std::format("Failed to initialize state: {}",static_cast<int>(id)));
        }
    }

    // �ʱ� ���� ����
    ChangeState(StateID::Login);
}

void StateManager::Update(float deltaTime)
{
    if (!initialized_ || paused_) 
    {
        return;
    }

    // ���� ���� ��û ó��
    ProcessStateChangeRequests();

    // ���� ���� ������Ʈ
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
        // �ʿ��� ��� ���� �Ͻ� ���� ���� �߰�
    }
}

void StateManager::ResumeCurrentState()
{
    if (paused_ && currentState_) 
    {
        paused_ = false;
        // �ʿ��� ��� ���� �簳 ���� �߰�
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

    // ��� ���µ��� ���ҽ� ����
    for (auto& [id, state] : states_) 
    {
        if (state) 
        {
            state->Leave();
            state->Release();
        }
    }

    // �����̳ʵ� ����
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