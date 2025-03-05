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
    // 플레이어 초기화
    local_player_ = std::make_shared<LocalPlayer>();
    remote_player_ = std::make_shared<RemotePlayer>();

    // 뷰 컴포넌트 초기화
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

    // 로컬 플레이어에게 컴포넌트 설정
    local_player_->SetInterruptView(interrupt_view_);
    local_player_->SetComboView(combo_view_);
    local_player_->SetResultView(result_view_);

    return true;
}

bool GameState::CreateUI()
{
    // 재시작 버튼 초기화
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

    // 버튼 위치 및 크기 설정
    restart_button_->Init(buttonTexture, GAME_APP.GetWindowWidth() / 2.0f - 68.0f, GAME_APP.GetWindowHeight() / 2.0f - 20.0f, 136.0f, 49.0f);
    exit_button_->Init(buttonTexture, GAME_APP.GetWindowWidth() / 2.0f - 68.0f, GAME_APP.GetWindowHeight() / 2.0f + 30.0f, 136.0f, 49.0f);

    // 버튼 상태 설정
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

    return true;
}

void GameState::Enter()
{
    // 로컬 플레이어의 ID 설정
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

    if (restart_button_) restart_button_->SetVisible(false);
    if (exit_button_) exit_button_->SetVisible(false);

    CreateGamePlayer(std::span<const uint8_t>(), std::span<const uint8_t>(), localPlayerId_, characterId);
    ScheduleGameStart();
    // 상태 초기화
    shouldQuit_ = false;
    lastInputTime_ = SDL_GetTicks();

    SDL_StartTextInput(GAME_APP.GetWindow());    
}

void GameState::Leave()
{
    
    if (restart_button_) restart_button_->SetVisible(false);
    if (exit_button_) exit_button_->SetVisible(false);

    // 플레이어 정리
    if (local_player_) local_player_->Reset();
    if (remote_player_) remote_player_->Reset();

    // 배경 정리
    if (background_)
    {
        background_->Reset();
    }

    SDL_StopTextInput(GAME_APP.GetWindow());
}

void GameState::Update(float deltaTime)
{

    TIMER_SCHEDULER.Update();

    // 배경 업데이트
    if (background_)
    {
        background_->Update(deltaTime);
    }

    // 로컬 플레이어 업데이트
    if (local_player_)
    {
        local_player_->Update(deltaTime);
        local_player_->UpdateGameLogic(deltaTime);
    }

    // 원격 플레이어 업데이트
    if (remote_player_ && isNetworkGame_)
    {
        remote_player_->Update(deltaTime);
        remote_player_->UpdateGameState(deltaTime);
    }
}

void GameState::Render()
{
    if (!initialized_)
    {
        return;
    }

    // 배경 렌더링
    if (background_)
    {
        background_->Render();
    }

    // 플레이어 렌더링
    if (local_player_)
    {
        local_player_->Render();
    }

    if (remote_player_ && isNetworkGame_)
    {
        remote_player_->Render();
    }

    // UI 렌더링
    RenderUI();

#ifdef _DEBUG
    //RenderDebugInfo();
#endif
}

void GameState::RenderUI()
{

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

    // 수직선
    for (int x = 0; x <= Constants::Board::BOARD_X_COUNT; ++x)
    {
        float xPos = Constants::Board::POSITION_X + Constants::Board::WIDTH_MARGIN + x * Constants::Block::SIZE;

        SDL_RenderLine(renderer,
            xPos,
            Constants::Board::POSITION_Y,
            xPos,
            Constants::Board::POSITION_Y + Constants::Board::HEIGHT);
    }

    // 수평선
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
    }

    // 키보드 상태 처리
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
    // 로컬 플레이어에게 키보드 입력 전달
    if (local_player_ && local_player_->GetGameState() == GamePhase::Playing)
    {
        switch (event.key.key)
        {
        case SDLK_UP:
            local_player_->RotateBlock(0, false);
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

    // 로컬 플레이어에게 키보드 상태 전달
    if (local_player_)
    {
        // 로컬 플레이어가 게임 중일 때 처리
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
    // 게임 상태 초기화
    if (local_player_ && remote_player_)
    {
        // 초기 블록 타입 생성 (서버만 수행)
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

            // 로컬 플레이어 재시작
            local_player_->Restart(blockType1, blockType2);

            // 네트워크로 재시작 패킷 전송
           /* RestartGamePacket packet;
            packet.player_id = GAME_APP.GetPlayerManager().GetMyPlayer()->GetId();
            memcpy(packet.block1, blockType1.data(), 2);
            memcpy(packet.block2, blockType2.data(), 2);

            NETWORK.ReStartGame(packet);*/
        }
    }

    // 배경 초기화
    if (background_)
    {
        background_->Reset();
    }

    // UI 상태 초기화
    if (restart_button_) restart_button_->SetVisible(false);
    if (exit_button_) exit_button_->SetVisible(false);

    // 게임 상태 초기화
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
            HandleChangeBlockState(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<PushBlockPacket>(
        PacketType::PushBlockInGame,
        [this](uint8_t connectionId, const PushBlockPacket* packet) {
            HandlePushBlockInGame(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<LoseGamePacket>(
        PacketType::LoseGame,
        [this](uint8_t connectionId, const LoseGamePacket* packet) {
            HandleLose(connectionId, packet);
        }
    );    

    packet_processor_.RegisterHandler<StopComboPacket>(
        PacketType::StopComboAttack,
        [this](uint8_t connectionId, const StopComboPacket* packet) {
            HandleStopCombo(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<PacketBase>(
        PacketType::GameOver,
        [this](uint8_t connectionId, const PacketBase* packet) {
            HandleGameOver();
        }
    );
}

void GameState::HandleGameInitialize(uint8_t connectionId, const GameInitPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    // 배경 맵 생성
    background_ = GAME_APP.GetMapManager().CreateMap(packet->map_id);    

    if (background_ && background_->Initialize())
    {
        local_player_->SetBackGround(background_);

        // 로컬 플레이어 블록 정보 맵에 설정
        auto next_blocks = local_player_->GetNextBlock();
        background_->Reset();
        background_->SetNextBlock(next_blocks[0]);
        background_->SetNextBlock(next_blocks[1]);

        // 플레이어 생성
        std::span<const uint8_t> blockType1(packet->block1);
        std::span<const uint8_t> blockType2(packet->block2);

        CreateGamePlayer(blockType1, blockType2, packet->player_id, packet->character_id);

        ScheduleGameStart();
    }
}

void GameState::CreateGamePlayer(const std::span<const uint8_t>& blocktype1, const std::span<const uint8_t>& blocktype2,
    uint8_t playerIdx, uint8_t characterIdx)
{
    // 로컬 플레이어인 경우
    if (playerIdx == GAME_APP.GetPlayerManager().GetMyPlayer()->GetId())
    {
        if (!local_player_->Initialize(blocktype1, blocktype2, playerIdx, characterIdx, background_))
        {
            LOGGER.Error("Failed to initialize local player");
        }
        
    }
    // 원격 플레이어인 경우
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

    // 원격 플레이어의 블록 추가
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

    // 원격 플레이어의 블록 이동
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

    // 원격 플레이어의 블록 회전
    if (player->GetId() != localPlayerId_ && remote_player_)
    {
        remote_player_->RotateBlock(packet->rotate_type, packet->is_horizontal_moving);
    }
}

void GameState::HandleStartGame()
{
    // 게임 시작 처리
    if (local_player_)
    {
        // 로컬 플레이어 다음 블록 생성
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

    // 원격 플레이어의 블록 상태 체크
    if (player->GetId() != localPlayerId_ && remote_player_)
    {
        remote_player_->CheckGameBlockState();
    }
}

void GameState::HandleChangeBlockState(uint8_t connectionId, const ChangeBlockStatePacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    if (player->GetId() != localPlayerId_ && remote_player_)
    {
        remote_player_->ChangeBlockState(packet->state);
    }
}

void GameState::HandlePushBlockInGame(uint8_t connectionId, const PushBlockPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    if (player->GetId() != localPlayerId_ && remote_player_)
    {
        std::span<const float, 2> pos1{ packet->position1 };
        std::span<const float, 2> pos2{ packet->position2 };

        //LOGGER.Info("GameState::HandlePushBlockInGame playerID_({}) pos1: {} pos2: {}", remote_player_->GetPlayerID(), pos1, pos2);

        remote_player_->PushBlockInGame(pos1, pos2);
    }
}

void GameState::HandleStopCombo(uint8_t connectionId, const StopComboPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    if (player->GetId() != localPlayerId_ && remote_player_)
    {
        remote_player_->SetComboAttackState(false);
    }
}

void GameState::HandleLose(uint8_t connectionId, const LoseGamePacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    if (player->GetId() != localPlayerId_ && remote_player_)
    {
        remote_player_->LoseGame(false);
    }
}

void GameState::HandleGameOver()
{
    GameQuit();
}

void GameState::HandleSystemEvent(const SDL_Event& event)
{
    //TODO: 시스템 이벤트 처리
}

void GameState::Release()
{
    Leave();

    restart_button_.reset();
    exit_button_.reset();

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
    // 2초 후에 게임 시작 로직 실행
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
