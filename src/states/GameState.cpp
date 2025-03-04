//#include "GameState.hpp"
//
//#include "../network/NetworkController.hpp"
////#include "../network/player/Player.hpp"
//#include "../network/packets/GamePackets.hpp"
//
//#include "../core/GameApp.hpp"
//#include "../core/common/constants/Constants.hpp"
//
//#include "../core/manager/ResourceManager.hpp"
//#include "../core/manager/PlayerManager.hpp"
//#include "../core/manager/ParticleManager.hpp"
//#include "../core/manager/MapManager.hpp"
//#include "../core/manager/StateManager.hpp"
//#include "../core/manager/FontManager.hpp"
//
//
//#include "../game/system/GameBoard.hpp"
//#include "../game/view/InterruptBlockView.hpp"
//#include "../game/view/ComboView.hpp"
//#include "../game/view/ResultView.hpp"
////#include "../game/system/GamePlayer.hpp"
//
//#include "../game/block/GameGroupBlock.hpp"
//#include "../game/block/IceBlock.hpp"
//#include "../game/effect/BulletEffect.hpp"
//#include "../game/effect/ExplosionEffect.hpp"
//#include "../game/map/GameBackground.hpp"
//#include "../game/system/BasePlayer.hpp"
//#include "../game/system/LocalPlayer.hpp"
//#include "../game/system/RemotePlayer.hpp"
//
//#include "../texture/ImageTexture.hpp"
//
//#include "../ui/EditBox.hpp"
//#include "../ui/Button.hpp"
//
//#include "../utils/Logger.hpp"
//
//#include <format>
//#include <random>
//#include <stdexcept>
//#include <algorithm>
//#include <fstream>
//#include <span>
//#include <string>
//#include <functional>
//
//#include <SdL3/SDL.h>
//#include <SdL3/SDL_keyboard.h>
//#include <SdL3/SDL_keycode.h>
//#include "../network/GameServer.hpp"
//#include <iostream>
//
//
//GameState::GameState() {
//    draw_objects_.reserve(100);
//    InitializePacketHandlers();
//}
//
//GameState::~GameState() = default;
//
//bool GameState::Init() {
//    if (initialized_) {
//        return false;
//    }
//
//    try {
//        if (!LoadResources() || !CreateUI()) {
//            return false;
//        }
//
//        // Initialize background
//        if (NETWORK.IsServer() || !NETWORK.IsRunning()) {
//            background_ = GAME_APP.GetMapManager().GetRandomMap();
//            if (background_ && background_->Initialize()) {
//                draw_objects_.push_back(background_.get());
//            }
//        }
//
//        // Create and initialize local player
//        local_player_ = std::make_shared<LocalPlayer>();
//
//        // Get player ID from player manager
//        if (auto player = GAME_APP.GetPlayerManager().GetMyPlayer()) 
//        {
//            local_player_id_ = player->GetId();
//            local_player_->SetPlayerId(local_player_id_);
//            local_player_->SetCharacterId(player->GetCharacterId());
//        }
//
//        if (!local_player_->Initialize(
//            Constants::Board::POSITION_X,
//            Constants::Board::POSITION_Y)) {
//            LOGGER.Error("Failed to initialize local player");
//            return false;
//        }
//
//        // Add local player components to render list
//        auto addComponent = [this](RenderableObject* obj) {
//            if (obj) draw_objects_.push_back(obj);
//            };
//
//        initialized_ = true;
//        return true;
//    }
//    catch (const std::exception& e) {
//        LOGGER.Error("Failed to initialize GameState: {}", e.what());
//        return false;
//    }
//}
//
//bool GameState::LoadResources() {
//    auto& resourceManager = GAME_APP.GetResourceManager();
//    try {
//        const std::vector<std::string> requiredTextures = {
//            "PUYO/puyo_beta.png",
//            "PUYO/Effect/effect.png",
//            "PUYO/Effect/attack_eff_mix_01.png",
//            "PUYO/rensa_font.png",
//            "PUYO/result.png"
//        };
//
//        for (const auto& path : requiredTextures) {
//            if (!resourceManager.GetResource<ImageTexture>(path)) {
//                throw std::runtime_error(std::format("Failed to load texture: {}", path));
//            }
//        }
//        return true;
//    }
//    catch (const std::exception& e) {
//        LOGGER.Error("Resource loading failed: {}", e.what());
//        return false;
//    }
//}
//
//bool GameState::CreateUI() {
//    // Initialize restart and exit buttons
//    restart_button_ = std::make_unique<Button>();
//    exit_button_ = std::make_unique<Button>();
//
//    if (!restart_button_ || !exit_button_) {
//        LOGGER.Error("Failed to create buttons");
//        return false;
//    }
//
//    auto buttonTexture = ImageTexture::Create("UI/BUTTON/button.png");
//    if (!buttonTexture) {
//        LOGGER.Error("Failed to get button texture");
//        return false;
//    }
//
//    // Set button positions and sizes
//    restart_button_->Init(buttonTexture,
//        GAME_APP.GetWindowWidth() / 2.0f - 68.0f,
//        GAME_APP.GetWindowHeight() / 2.0f - 20.0f,
//        136.0f, 49.0f);
//
//    exit_button_->Init(buttonTexture,
//        GAME_APP.GetWindowWidth() / 2.0f - 68.0f,
//        GAME_APP.GetWindowHeight() / 2.0f + 30.0f,
//        136.0f, 49.0f);
//
//    // Set button states and callbacks
//    SDL_FRect normalRect{ 0, 0, 136, 49 };
//    SDL_FRect hoverRect{ 0, 50, 136, 49 };
//
//    restart_button_->SetStateRect(Button::State::Normal, normalRect);
//    restart_button_->SetStateRect(Button::State::Hover, hoverRect);
//    restart_button_->SetEventCallback(Button::State::Down,
//        [this]() { return GameRestart(); });
//
//    normalRect = { 0, 100, 136, 49 };
//    hoverRect = { 0, 150, 136, 49 };
//
//    exit_button_->SetStateRect(Button::State::Normal, normalRect);
//    exit_button_->SetStateRect(Button::State::Hover, hoverRect);
//    exit_button_->SetEventCallback(Button::State::Down,
//        [this]() { return GameExit(); });
//
//    // Initialize chat box
//    chatbox_ = std::make_unique<EditBox>();
//    if (!chatbox_) {
//        LOGGER.Error("Failed to create EditBox");
//        return false;
//    }
//
//    chatbox_->Init((GAME_APP.GetWindowWidth() - 180) / 2.0f, 420.0f, 180.0f, 23.0f);
//    chatbox_->SetEventReturn([this]() { return SendChatMsg(); });
//
//    return true;
//}
//
//void GameState::Enter() 
//{
//    // Initialize network game
//    is_network_game_ = NETWORK.IsRunning();
//
//    // Initialize background
//    if (NETWORK.IsServer() || !NETWORK.IsRunning()) 
//    {
//        if (background_ = GAME_APP.GetMapManager().GetRandomMap();
//            background_->Initialize()) {
//            draw_objects_.push_back(background_.get());
//        }
//    }
//
//    // Initialize initial blocks for local player
//    // This would be part of the LocalPlayer initialization now
//
//    // Network game initialization
//    if (NETWORK.IsRunning() && local_player_) {
//        // Code for network initialization
//        // This would now use the LocalPlayer methods
//    }
//
//    if (chatbox_) chatbox_->SetVisible(true);
//    if (restart_button_) restart_button_->SetVisible(false);
//    if (exit_button_) exit_button_->SetVisible(false);
//
//    SDL_StartTextInput(GAME_APP.GetWindow());
//}
//
//void GameState::Leave() 
//{
//    if (chatbox_) 
//    {
//        chatbox_->ClearContent();
//        chatbox_->SetVisible(false);
//    }
//    if (restart_button_) restart_button_->SetVisible(false);
//    if (exit_button_) exit_button_->SetVisible(false);
//
//    draw_objects_.clear();
//
//    if (background_) 
//    {
//        background_->Reset();
//    }
//
//    if (local_player_) 
//    {
//        local_player_->Reset();
//    }
//
//    for (auto& [id, player] : remote_players_) 
//    {
//        player->Reset();
//    }
//
//    remote_players_.clear();
//
//    SDL_StopTextInput(GAME_APP.GetWindow());
//}
//
//void GameState::Update(float deltaTime) 
//{
//    if (background_) {
//        background_->Update(deltaTime);
//    }
//
//    if (local_player_) {
//        local_player_->Update(deltaTime);
//    }
//
//    for (auto& [id, player] : remote_players_) 
//    {
//        player->Update(deltaTime);
//    }
//
//    if (chatbox_) 
//    {
//        chatbox_->Update(deltaTime);
//    }
//
//    if (restart_button_) 
//    {
//        restart_button_->Update(deltaTime);
//    }
//
//    if (exit_button_) 
//    {
//        exit_button_->Update(deltaTime);
//    }
//
//    if (NETWORK.IsServer()) 
//    {
//        NETWORK.Update();
//    }
//}
//
//void GameState::Render() 
//{
//    if (!initialized_) 
//    {
//        return;
//    }
//
//    if (background_) 
//    {
//        background_->Render();
//    }
//
//    if (local_player_) 
//    {
//        local_player_->Render();
//    }
//
//    for (auto& [id, player] : remote_players_) 
//    {
//        player->Render();
//    }
//
//    // Render UI
//    RenderUI();
//}
//
//void GameState::RenderUI() 
//{
//    if (chatbox_) 
//    {
//        chatbox_->Render();
//    }
//
//    if (restart_button_) 
//    {
//        restart_button_->Render();
//    }
//
//    if (exit_button_) 
//    {
//        exit_button_->Render();
//    }
//}
//
//[[nodiscard]] const std::shared_ptr<RemotePlayer>& GameState::GetRemotePlayer(uint8_t playerId)
//{
//    auto it = remote_players_.find(playerId);
//
//    return (it != remote_players_.end()) ? it->second : nullptr;
//}
//
//void GameState::HandleEvent(const SDL_Event& event)
//{
//    if (!initialized_) 
//    {
//        return;
//    }
//
//    switch (event.type) {
//    case SDL_EVENT_MOUSE_MOTION:
//    case SDL_EVENT_MOUSE_BUTTON_DOWN:
//    case SDL_EVENT_MOUSE_BUTTON_UP:
//        HandleMouseInput(event);
//        break;
//
//    case SDL_EVENT_KEY_DOWN:
//        if (local_player_) 
//        {
//            local_player_->HandleKeyboardInput(event);
//        }
//
//        if (event.key.key == SDLK_RETURN || event.key.key == SDLK_BACKSPACE) 
//        {
//            if (chatbox_) 
//            {
//                chatbox_->HandleEvent(event);
//            }
//        }
//        break;
//
//    case SDL_EVENT_TEXT_INPUT:
//    case SDL_EVENT_TEXT_EDITING:
//        if (chatbox_) 
//        {
//            chatbox_->HandleEvent(event);
//        }
//        break;
//    }
//
//    if (local_player_) 
//    {
//        local_player_->HandleKeyboardState();
//    }
//}
//
//void GameState::HandleMouseInput(const SDL_Event& event) 
//{
//    if (restart_button_) 
//    {
//        restart_button_->HandleEvent(event);
//    }
//
//    if (exit_button_) 
//    {
//        exit_button_->HandleEvent(event);
//    }
//}
//
//bool GameState::GameRestart() 
//{
//    if (background_) 
//    {
//        background_->Reset();
//    }
//
//    if (local_player_) 
//    {
//        local_player_->Reset();
//    }
//
//    for (auto& [id, player] : remote_players_) 
//    {
//        player->Reset();
//    }
//
//    if (chatbox_) chatbox_->SetVisible(true);
//    if (restart_button_) restart_button_->SetVisible(false);
//    if (exit_button_) exit_button_->SetVisible(false);
//
//    // Initialize new blocks
//    // This would now be handled by the LocalPlayer
//
//    return true;
//}
//
//bool GameState::GameExit() 
//{
//    NETWORK.Shutdown();
//    GAME_APP.GetStateManager().RequestStateChange(StateManager::StateID::Login);
//    return true;
//}
//
//void GameState::GameQuit() 
//{
//    if (local_player_) 
//    {
//        local_player_->SetGameQuit(true);
//    }
//
//    if (restart_button_ && NETWORK.IsServer()) 
//    {
//        restart_button_->SetVisible(true);
//    }
//
//    if (exit_button_) 
//    {
//        exit_button_->SetVisible(true);
//    }
//}
//
//bool GameState::SendChatMsg() {
//    if (!chatbox_ || chatbox_->IsEmpty()) {
//        return false;
//    }
//
//    NETWORK.ChatMessage(chatbox_->GetText(TextType::UTF8));
//    return true;
//}
//
//void GameState::CreateRemotePlayer(const std::span<const uint8_t>& blocktype1,
//    const std::span<const uint8_t>& blocktype2,
//    uint8_t playerIdx,
//    uint8_t characterIdx) 
//{
//    auto remote_player = std::make_shared<RemotePlayer>();
//
//    if (remote_player->Initialize(Constants::Board::PLAYER_POSITION_X, Constants::Board::PLAYER_POSITION_Y)) 
//    {
//
//        remote_player->SetPlayerId(playerIdx);
//        remote_player->SetCharacterId(characterIdx);
//
//        remote_player->InitializeNextBlocks(blocktype1, blocktype2);
//
//        remote_players_[playerIdx] = remote_player;
//    }
//}
//
//void GameState::HandleNetworkMessage(uint8_t connectionId, std::span<const char> data, uint32_t length) {
//    packet_processor_.ProcessPacket(connectionId, data, length);
//}
//
//void GameState::InitializePacketHandlers() 
//{
//    packet_processor_.RegisterHandler<GameInitPacket>(
//        PacketType::InitializeGame,
//        [this](uint8_t connectionId, const GameInitPacket* packet) {
//            HandleGameInitialize(connectionId, packet);
//        }
//    );
//
//    packet_processor_.RegisterHandler<AddNewBlockPacket>(
//        PacketType::AddNewBlock,
//        [this](uint8_t connectionId, const AddNewBlockPacket* packet) {
//            HandleAddNewBlock(connectionId, packet);
//        }
//    );
//
//    packet_processor_.RegisterHandler<MoveBlockPacket>(
//        PacketType::UpdateBlockMove,
//        [this](uint8_t connectionId, const MoveBlockPacket* packet) {
//            HandleUpdateBlockMove(connectionId, packet);
//        }
//    );
//
//    packet_processor_.RegisterHandler<RotateBlockPacket>(
//        PacketType::UpdateBlockRotate,
//        [this](uint8_t connectionId, const RotateBlockPacket* packet) {
//            HandleBlockRotate(connectionId, packet);
//        }
//    );
//
//    packet_processor_.RegisterHandler<PacketBase>(
//        PacketType::StartGame,
//        [this](uint8_t connectionId, const PacketBase* packet) {
//            HandleStartGame();
//        }
//    );
//
//    packet_processor_.RegisterHandler<CheckBlockStatePacket>(
//        PacketType::CheckBlockState,
//        [this](uint8_t connectionId, const CheckBlockStatePacket* packet) {
//            HandleCheckBlockState(connectionId, packet);
//        }
//    );
//
//    packet_processor_.RegisterHandler<PacketBase>(
//        PacketType::GameOver,
//        [this](uint8_t connectionId, const PacketBase* packet) {
//            HandleGameOver();
//        }
//    );
//
//}
//
//void GameState::HandleGameInitialize(uint8_t connectionId, const GameInitPacket* packet) 
//{
//    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
//    if (!player) 
//    {
//        return;
//    }
//
//    background_ = GAME_APP.GetMapManager().CreateMap(packet->map_id);
//    
//    if (background_ && background_->Initialize()) 
//    {
//        draw_objects_.insert(draw_objects_.begin(), background_.get());
//
//        // Initialize background with local player's next blocks
//        if (local_player_) {
//            // Update background with player blocks
//            // This would now be handled in the LocalPlayer initialization
//        }
//    }
//
//    CreateRemotePlayer(packet->block1, packet->block2, packet->player_id, packet->character_id);
//}
//
//void GameState::HandleAddNewBlock(uint8_t connectionId, const AddNewBlockPacket* packet) 
//{
//    auto it = remote_players_.find(packet->player_id);
//    if (it != remote_players_.end()) 
//    {
//        it->second->AddNewBlock(packet->block_type);
//    }
//}
//
//void GameState::HandleUpdateBlockMove(uint8_t connectionId, const MoveBlockPacket* packet) 
//{
//    auto it = remote_players_.find(packet->player_id);
//    if (it != remote_players_.end()) 
//    {
//        it->second->HandleMoveBlock(packet->move_type, packet->position);
//    }
//}
//
//void GameState::HandleBlockRotate(uint8_t connectionId, const RotateBlockPacket* packet) 
//{
//    auto it = remote_players_.find(packet->player_id);
//    if (it != remote_players_.end()) 
//    {
//        it->second->HandleRotateBlock(packet->rotate_type, packet->is_horizontal_moving);
//    }
//}
//
//void GameState::HandleStartGame() 
//{
//    if (local_player_) 
//    {
//        local_player_->DestroyNextBlock();
//    }
//}
//
//void GameState::HandleCheckBlockState(uint8_t connectionId, const CheckBlockStatePacket* packet) 
//{
//    auto it = remote_players_.find(packet->player_id);
//    if (it != remote_players_.end()) 
//    {
//        it->second->CheckGameBlockState();
//    }
//}
//
//void GameState::HandleGameOver() 
//{
//    for (auto& [id, player] : remote_players_) 
//    {
//        player->LoseGame(true);
//    }
//}
//
//void GameState::Release() 
//{
//    // Release UI components
//    restart_button_.reset();
//    exit_button_.reset();
//    chatbox_.reset();
//
//    // Release players
//    local_player_.reset();
//    remote_players_.clear();
//
//    // Release background
//    background_.reset();
//
//    // Release game managers
//    GAME_APP.GetMapManager().Release();
//
//    // Stop text input
//    SDL_StopTextInput(GAME_APP.GetWindow());
//}


#include "GameState.hpp"

#include "../network/NetworkController.hpp"
#include "../network/player/Player.hpp"
#include "../network/packets/GamePackets.hpp"

#include "../core/GameApp.hpp"
#include "../core/common/constants/Constants.hpp"

#include "../core/manager/ResourceManager.hpp"
#include "../core/manager/PlayerManager.hpp"
#include "../core/manager/ParticleManager.hpp"
#include "../core/manager/MapManager.hpp"
#include "../core/manager/StateManager.hpp"
#include "../core/manager/FontManager.hpp"

#include "../game/map/GameBackground.hpp"
#include "../game/system/LocalPlayer.hpp"
#include "../game/system/RemotePlayer.hpp"
#include "../game/block/GameGroupBlock.hpp"

#include "../game/view/ComboView.hpp"
#include "../game/view/InterruptBlockView.hpp"
#include "../game/view/ResultView.hpp"

#include "../ui/EditBox.hpp"
#include "../ui/Button.hpp"

#include "../utils/Logger.hpp"
#include "../utils/TimerScheduler.hpp"

#include <format>
#include <stdexcept>
#include <algorithm>
#include <span>
#include <random>

GameState::GameState()
{
    InitializePacketHandlers();
}

GameState::~GameState() = default;

bool GameState::Init()
{
    if (initialized_)
    {
        return false;
    }

    try
    {
        if (LoadResources() == false || InitializeComponents() == false || CreateUI() == false)
        {
            return false;
        }

        initialized_ = true;
        return true;
    }
    catch (const std::exception& e)
    {
        LOGGER.Error("Failed to initialize GameState: {}", e.what());
        return false;
    }
}

bool GameState::LoadResources()
{
    auto& resourceManager = GAME_APP.GetResourceManager();
    try
    {
        const std::vector<std::string> requiredTextures =
        {
            "PUYO/puyo_beta.png",
            "PUYO/Effect/effect.png",
            "PUYO/Effect/attack_eff_mix_01.png",
            "PUYO/rensa_font.png",
            "PUYO/result.png"
        };

        for (const auto& path : requiredTextures)
        {
            if (!resourceManager.GetResource<ImageTexture>(path))
            {
                throw std::runtime_error(std::format("Failed to load texture: {}", path));
            }
        }
        return true;
    }
    catch (const std::exception& e)
    {
        LOGGER.Error("Resource loading failed: {}", e.what());
        return false;
    }
}

bool GameState::InitializeComponents()
{
    // �÷��̾� �ʱ�ȭ
    local_player_ = std::make_shared<LocalPlayer>();
    remote_player_ = std::make_shared<RemotePlayer>();

    // �� ������Ʈ �ʱ�ȭ
    interrupt_view_ = std::make_shared<InterruptBlockView>();
    if (!interrupt_view_ || !interrupt_view_->Initialize())
    {
        LOGGER.Error("Failed to create InterruptBlockView");
        return false;
    }
    interrupt_view_->SetPosition(Constants::Board::POSITION_X, 0);

    combo_view_ = std::make_shared<ComboView>();
    if (!combo_view_ || !combo_view_->Initialize())
    {
        LOGGER.Error("Failed to create ComboView");
        return false;
    }

    result_view_ = std::make_shared<ResultView>();
    if (!result_view_ || !result_view_->Initialize())
    {
        LOGGER.Error("Failed to create ResultView");
        return false;
    }

    // ���� �÷��̾�� ������Ʈ ����
    local_player_->SetInterruptView(interrupt_view_);
    local_player_->SetComboView(combo_view_);
    local_player_->SetResultView(result_view_);

    return true;
}

bool GameState::CreateUI()
{
    // ����� ��ư �ʱ�ȭ
    restart_button_ = std::make_unique<Button>();
    exit_button_ = std::make_unique<Button>();

    if (!restart_button_ || !exit_button_)
    {
        LOGGER.Error("Failed to create buttons");
        return false;
    }

    auto buttonTexture = ImageTexture::Create("UI/BUTTON/button.png");
    if (!buttonTexture)
    {
        LOGGER.Error("Failed to get button texture");
        return false;
    }

    // ��ư ��ġ �� ũ�� ����
    restart_button_->Init(buttonTexture, GAME_APP.GetWindowWidth() / 2.0f - 68.0f, GAME_APP.GetWindowHeight() / 2.0f - 20.0f, 136.0f, 49.0f);
    exit_button_->Init(buttonTexture, GAME_APP.GetWindowWidth() / 2.0f - 68.0f, GAME_APP.GetWindowHeight() / 2.0f + 30.0f, 136.0f, 49.0f);

    // ��ư ���� ����
    SDL_FRect normalRect{ 0, 0, 136, 49 };
    SDL_FRect hoverRect{ 0, 50, 136, 49 };

    restart_button_->SetStateRect(Button::State::Normal, normalRect);
    restart_button_->SetStateRect(Button::State::Hover, hoverRect);
    restart_button_->SetEventCallback(Button::State::Down,
        [this]()
        {
            return GameRestart();
        });

    normalRect = { 0, 100, 136, 49 };
    hoverRect = { 0, 150, 136, 49 };

    exit_button_->SetStateRect(Button::State::Normal, normalRect);
    exit_button_->SetStateRect(Button::State::Hover, hoverRect);
    exit_button_->SetEventCallback(Button::State::Down,
        [this]()
        {
            return GameExit();
        });

    // ä�� �ڽ� �ʱ�ȭ
    chatbox_ = std::make_unique<EditBox>();
    if (!chatbox_)
    {
        LOGGER.Error("Failed to create EditBox");
        return false;
    }

    chatbox_->Init((GAME_APP.GetWindowWidth() - 180) / 2.0f, 420.0f, 180.0f, 23.0f);
    chatbox_->SetEventReturn([this]() { return SendChatMsg(); });

    return true;
}

void GameState::Enter()
{
    // ���� �÷��̾��� ID ����
    localPlayerId_ = GAME_APP.GetPlayerManager().GetMyPlayer()->GetId();
    auto characterId = GAME_APP.GetPlayerManager().GetMyPlayer()->GetCharacterId();
    
    if (NETWORK.IsServer() || !NETWORK.IsRunning())
    {
        background_ = GAME_APP.GetMapManager().GetRandomMap();
        if (background_->Initialize())
        {
            isNetworkGame_ = NETWORK.IsRunning();
        }
    }

    // UI ������Ʈ �ʱ�ȭ
    if (chatbox_) chatbox_->SetVisible(true);
    if (restart_button_) restart_button_->SetVisible(false);
    if (exit_button_) exit_button_->SetVisible(false);

    CreateGamePlayer(std::span<const uint8_t>(), std::span<const uint8_t>(), localPlayerId_, characterId);
    ScheduleGameStart();
    // ���� �ʱ�ȭ
    shouldQuit_ = false;
    lastInputTime_ = SDL_GetTicks();

    SDL_StartTextInput(GAME_APP.GetWindow());    
}

void GameState::Leave()
{
    // UI ���� �ʱ�ȭ
    if (chatbox_)
    {
        chatbox_->ClearContent();
        chatbox_->SetVisible(false);
    }
    if (restart_button_) restart_button_->SetVisible(false);
    if (exit_button_) exit_button_->SetVisible(false);

    // �÷��̾� ����
    if (local_player_) local_player_->Reset();
    if (remote_player_) remote_player_->Reset();

    // ��� ����
    if (background_)
    {
        background_->Reset();
    }

    SDL_StopTextInput(GAME_APP.GetWindow());
}

void GameState::Update(float deltaTime)
{

    TIMER_SCHEDULER.Update();

    // ��� ������Ʈ
    if (background_)
    {
        background_->Update(deltaTime);
    }

    // ���� �÷��̾� ������Ʈ
    if (local_player_)
    {
        local_player_->Update(deltaTime);
        local_player_->UpdateGameLogic(deltaTime);
    }

    // ���� �÷��̾� ������Ʈ
    if (remote_player_ && isNetworkGame_)
    {
        remote_player_->Update(deltaTime);
        remote_player_->UpdateGameState(deltaTime);
    }

    // UI ������Ʈ
    if (chatbox_)
    {
        chatbox_->Update(deltaTime);
    }
}

void GameState::Render()
{
    if (!initialized_)
    {
        return;
    }

    // ��� ������
    if (background_)
    {
        background_->Render();
    }

    // �÷��̾� ������
    if (local_player_)
    {
        local_player_->Render();
    }

    if (remote_player_ && isNetworkGame_)
    {
        remote_player_->Render();
    }

    // UI ������
    RenderUI();

#ifdef _DEBUG
    //RenderDebugInfo();
#endif
}

void GameState::RenderUI()
{
    if (chatbox_)
    {
        chatbox_->Render();
    }

    if (restart_button_)
    {
        restart_button_->Render();
    }

    if (exit_button_)
    {
        exit_button_->Render();
    }
}

#ifdef _DEBUG
void GameState::RenderDebugInfo()
{
    auto renderer = GAME_APP.GetRenderer();
    if (!renderer)
    {
        return;
    }

    RenderDebugGrid();
}

void GameState::RenderDebugGrid()
{
    auto renderer = GAME_APP.GetRenderer();
    if (!renderer)
    {
        return;
    }

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

    // ������
    for (int x = 0; x <= Constants::Board::BOARD_X_COUNT; ++x)
    {
        float xPos = Constants::Board::POSITION_X + Constants::Board::WIDTH_MARGIN + x * Constants::Block::SIZE;

        SDL_RenderLine(renderer,
            xPos,
            Constants::Board::POSITION_Y,
            xPos,
            Constants::Board::POSITION_Y + Constants::Board::HEIGHT);
    }

    // ����
    for (int y = 0; y <= Constants::Board::BOARD_Y_COUNT; ++y)
    {
        float yPos = Constants::Board::POSITION_Y + y * Constants::Block::SIZE;

        SDL_RenderLine(renderer,
            Constants::Board::POSITION_X + Constants::Board::WIDTH_MARGIN,
            yPos,
            Constants::Board::POSITION_X + Constants::Board::WIDTH_MARGIN +
            Constants::Board::WIDTH,
            yPos);
    }
}
#endif

void GameState::HandleEvent(const SDL_Event& event)
{
    if (shouldQuit_)
    {
        return;
    }

    switch (event.type)
    {
    case SDL_EVENT_MOUSE_MOTION:
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        HandleMouseInput(event);
        break;

    case SDL_EVENT_KEY_DOWN:
        HandleKeyboardInput(event);
        break;

    case SDL_EVENT_TEXT_INPUT:
    case SDL_EVENT_TEXT_EDITING:
        if (chatbox_)
        {
            chatbox_->HandleEvent(event);
        }
        break;
    }

    // Ű���� ���� ó��
    HandleKeyboardState();
}

void GameState::HandleMouseInput(const SDL_Event& event)
{
    if (restart_button_)
    {
        restart_button_->HandleEvent(event);
    }

    if (exit_button_)
    {
        exit_button_->HandleEvent(event);
    }
}

void GameState::HandleKeyboardInput(const SDL_Event& event)
{
    // ���� �÷��̾�� Ű���� �Է� ����
    if (local_player_ && local_player_->GetGameState() == GamePhase::Playing)
    {
        switch (event.key.key)
        {
        case SDLK_UP:
            local_player_->RotateBlock(0, false);
            break;

        case SDLK_RETURN:
        case SDLK_BACKSPACE:
            if (chatbox_)
            {
                chatbox_->HandleEvent(event);
            }
            break;
        }
    }
}

void GameState::HandleKeyboardState()
{
    if (SDL_GetTicks() - lastInputTime_ < 40)
    {
        return;
    }

    const bool* keyStates = SDL_GetKeyboardState(nullptr);

    // ���� �÷��̾�� Ű���� ���� ����
    if (local_player_)
    {
        // ���� �÷��̾ ���� ���� �� ó��
        if (local_player_->GetGameState() == GamePhase::Playing)
        {
            if (keyStates[SDL_SCANCODE_LEFT])
            {
                local_player_->MoveBlock(static_cast<uint8_t>(Constants::Direction::Left), 0);
            }

            if (keyStates[SDL_SCANCODE_RIGHT])
            {
                local_player_->MoveBlock(static_cast<uint8_t>(Constants::Direction::Right), 0);
            }

            if (keyStates[SDL_SCANCODE_DOWN])
            {
                local_player_->MoveBlock(static_cast<uint8_t>(Constants::Direction::Bottom), 0);
            }
        }
        else if (local_player_->GetGameState() == GamePhase::Standing && shouldQuit_)
        {
            if (keyStates[SDL_SCANCODE_RETURN] || keyStates[SDL_SCANCODE_SPACE])
            {
                if (NETWORK.IsServer())
                {
                    GameRestart();
                }
            }
            else if (keyStates[SDL_SCANCODE_ESCAPE])
            {
                GameExit();
            }
        }
    }

    lastInputTime_ = SDL_GetTicks();
}

bool GameState::GameRestart()
{
    // ���� ���� �ʱ�ȭ
    if (local_player_ && remote_player_)
    {
        // �ʱ� ��� Ÿ�� ���� (������ ����)
        if (NETWORK.IsServer())
        {
            std::array<uint8_t, 2> blockType1 = {
                static_cast<uint8_t>(rand() % 5 + 1),
                static_cast<uint8_t>(rand() % 5 + 1)
            };
            std::array<uint8_t, 2> blockType2 = {
                static_cast<uint8_t>(rand() % 5 + 1),
                static_cast<uint8_t>(rand() % 5 + 1)
            };

            // ���� �÷��̾� �����
            local_player_->Restart(blockType1, blockType2);

            // ��Ʈ��ũ�� ����� ��Ŷ ����
           /* RestartGamePacket packet;
            packet.player_id = GAME_APP.GetPlayerManager().GetMyPlayer()->GetId();
            memcpy(packet.block1, blockType1.data(), 2);
            memcpy(packet.block2, blockType2.data(), 2);

            NETWORK.ReStartGame(packet);*/
        }
    }

    // ��� �ʱ�ȭ
    if (background_)
    {
        background_->Reset();
    }

    // UI ���� �ʱ�ȭ
    if (chatbox_) chatbox_->SetVisible(true);
    if (restart_button_) restart_button_->SetVisible(false);
    if (exit_button_) exit_button_->SetVisible(false);

    // ���� ���� �ʱ�ȭ
    shouldQuit_ = false;
    lastInputTime_ = SDL_GetTicks();

    return true;
}

bool GameState::GameExit()
{
    NETWORK.Shutdown();
    GAME_APP.GetStateManager().RequestStateChange(StateManager::StateID::Login);
    return true;
}

void GameState::GameQuit()
{
    shouldQuit_ = true;

    if (local_player_)
    {
        local_player_->SetGameQuit();
        local_player_->LoseGame(false);
    }

    if (NETWORK.IsServer() && restart_button_)
    {
        restart_button_->SetVisible(true);
    }

    if (exit_button_)
    {
        exit_button_->SetVisible(true);
    }
}

bool GameState::SendChatMsg()
{
    if (!chatbox_ || chatbox_->IsEmpty())
    {
        return false;
    }

    NETWORK.ChatMessage(chatbox_->GetText(TextType::UTF8));
    return true;
}

void GameState::HandleNetworkMessage(uint8_t connectionId, std::span<const char> data, uint32_t length)
{
    packet_processor_.ProcessPacket(connectionId, data, length);
}

void GameState::InitializePacketHandlers()
{
    packet_processor_.RegisterHandler<GameInitPacket>(
        PacketType::InitializeGame,
        [this](uint8_t connectionId, const GameInitPacket* packet) {
            HandleGameInitialize(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<AddNewBlockPacket>(
        PacketType::AddNewBlock,
        [this](uint8_t connectionId, const AddNewBlockPacket* packet) {
            HandleAddNewBlock(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<MoveBlockPacket>(
        PacketType::UpdateBlockMove,
        [this](uint8_t connectionId, const MoveBlockPacket* packet) {
            HandleUpdateBlockMove(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<RotateBlockPacket>(
        PacketType::UpdateBlockRotate,
        [this](uint8_t connectionId, const RotateBlockPacket* packet) {
            HandleBlockRotate(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<PacketBase>(
        PacketType::StartGame,
        [this](uint8_t connectionId, const PacketBase* packet) {
            HandleStartGame();
        }
    );

    packet_processor_.RegisterHandler<CheckBlockStatePacket>(
        PacketType::CheckBlockState,
        [this](uint8_t connectionId, const CheckBlockStatePacket* packet) {
            HandleCheckBlockState(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<ChangeBlockStatePacket>(
        PacketType::ChangeBlockState,
        [this](uint8_t connectionId, const ChangeBlockStatePacket* packet) {
            HandleChangeBlockStatePacket(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<PacketBase>(
        PacketType::GameOver,
        [this](uint8_t connectionId, const PacketBase* packet) {
            HandleGameOver();
        }
    );

    // ������ ��Ŷ �ڵ鷯 ��� - �߰� ��� �ʿ�� ����
}

void GameState::HandleGameInitialize(uint8_t connectionId, const GameInitPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    // ��� �� ����
    background_ = GAME_APP.GetMapManager().CreateMap(packet->map_id);    

    if (background_ && background_->Initialize())
    {
        local_player_->SetBackGround(background_);

        // ���� �÷��̾� ��� ���� �ʿ� ����
        auto next_blocks = local_player_->GetNextBlock();
        background_->Reset();
        background_->SetNextBlock(next_blocks[0]);
        background_->SetNextBlock(next_blocks[1]);

        // �÷��̾� ����
        std::span<const uint8_t> blockType1(packet->block1);
        std::span<const uint8_t> blockType2(packet->block2);

        CreateGamePlayer(blockType1, blockType2, packet->player_id, packet->character_id);

        ScheduleGameStart();
    }
}

void GameState::CreateGamePlayer(const std::span<const uint8_t>& blocktype1, const std::span<const uint8_t>& blocktype2,
    uint8_t playerIdx, uint8_t characterIdx)
{
    // ���� �÷��̾��� ���
    if (playerIdx == GAME_APP.GetPlayerManager().GetMyPlayer()->GetId())
    {
        if (!local_player_->Initialize(blocktype1, blocktype2, playerIdx, characterIdx, background_))
        {
            LOGGER.Error("Failed to initialize local player");
        }
        
    }
    // ���� �÷��̾��� ���
    else
    {
        if (!remote_player_->Initialize(blocktype1, blocktype2, playerIdx, characterIdx, background_))
        {
            LOGGER.Error("Failed to initialize remote player");
        }

        isNetworkGame_ = true;
    }
}

void GameState::HandleAddNewBlock(uint8_t connectionId, const AddNewBlockPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    // ���� �÷��̾��� ��� �߰�
    if (player->GetId() != localPlayerId_ && remote_player_)
    {
        remote_player_->AddNewBlock(packet->block_type);
    }
}

void GameState::HandleUpdateBlockMove(uint8_t connectionId, const MoveBlockPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    // ���� �÷��̾��� ��� �̵�
    if (player->GetId() != localPlayerId_ && remote_player_)
    {
        remote_player_->MoveBlock(packet->move_type, packet->position);
    }
}

void GameState::HandleBlockRotate(uint8_t connectionId, const RotateBlockPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    // ���� �÷��̾��� ��� ȸ��
    if (player->GetId() != localPlayerId_ && remote_player_)
    {
        remote_player_->RotateBlock(packet->rotate_type, packet->is_horizontal_moving);
    }
}

void GameState::HandleStartGame()
{
    // ���� ���� ó��
    if (local_player_)
    {
        // ���� �÷��̾� ���� ��� ����
        local_player_->CreateNextBlock();
    }
}

void GameState::HandleCheckBlockState(uint8_t connectionId, const CheckBlockStatePacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    // ���� �÷��̾��� ��� ���� üũ
    if (player->GetId() != localPlayerId_ && remote_player_)
    {
        remote_player_->CheckGameBlockState();
    }
}

void GameState::HandleChangeBlockStatePacket(uint8_t connectionId, const ChangeBlockStatePacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    // ���� �÷��̾��� ��� ���� üũ
    if (player->GetId() != localPlayerId_ && remote_player_)
    {
        remote_player_->ChangeBlockState(packet->state);
    }
}

void GameState::HandleGameOver()
{
    GameQuit();
}

void GameState::HandleSystemEvent(const SDL_Event& event)
{
    //TODO: �ý��� �̺�Ʈ ó��
}

void GameState::Release()
{
    Leave();

    restart_button_.reset();
    exit_button_.reset();
    chatbox_.reset();

    background_.reset();
    local_player_.reset();
    remote_player_.reset();

    initialized_ = false;
    isNetworkGame_ = false;

    GAME_APP.GetMapManager().Release();

    SDL_StopTextInput(GAME_APP.GetWindow());
}

Block* (*GameState::GetGameBlocks(uint8_t playerId))[Constants::Board::BOARD_X_COUNT]
{
    if (local_player_->GetPlayerID() == playerId)
    {
        return local_player_->GetGameBlocks();
    }
    else
    {
        return remote_player_->GetGameBlocks();
    }
}

void GameState::ScheduleGameStart() 
{
    // 2�� �Ŀ� ���� ���� ���� ����
    TIMER_SCHEDULER.ScheduleTask(Constants::Game::PLAY_START_DELAY, 
        [this]()
        {
            if (NETWORK.IsRunning() && NETWORK.IsServer())
            {
                NETWORK.StartGame();
                local_player_->CreateNextBlock();
            }

            if (local_player_->IsRunning() == false)
            {
                local_player_->SetRunning(true);
            }            
        });
}
