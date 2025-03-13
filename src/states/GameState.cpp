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
#include "../game/system/GameBoard.hpp"

GameState::GameState()
{
    InitializePacketHandlers();
}

GameState::~GameState()
{
    Release();
}

bool GameState::Init()
{
    if (initialized_)
    {
        return false;
    }

    try
    {
        if (LoadResources() == false || CreatePlayers() == false || CreateUI() == false)
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

bool GameState::CreatePlayers()
{
    local_player_ = std::make_shared<LocalPlayer>();
    remote_player_ = std::make_shared<RemotePlayer>();

    return true;
}

bool GameState::CreateUI()
{
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

    restart_button_->Init(buttonTexture, GAME_APP.GetWindowWidth() / 2.0f - 68.0f, GAME_APP.GetWindowHeight() / 2.0f - 20.0f, 136.0f, 49.0f);
    exit_button_->Init(buttonTexture, GAME_APP.GetWindowWidth() / 2.0f - 68.0f, GAME_APP.GetWindowHeight() / 2.0f + 30.0f, 136.0f, 49.0f);

    restart_button_->SetStateRect(Button::State::Normal, { 0, 0, 136, 49 });
    restart_button_->SetStateRect(Button::State::Hover, { 0, 50, 136, 49 });
    restart_button_->SetEventCallback(Button::State::Down,
        [this]()
        {
            return GameRestart();
        });

    exit_button_->SetStateRect(Button::State::Normal, { 0, 100, 136, 49 });
    exit_button_->SetStateRect(Button::State::Hover, { 0, 150, 136, 49 });
    exit_button_->SetEventCallback(Button::State::Down,
        [this]()
        {
            return GameExit();
        });

    return true;
}

void GameState::Enter()
{
    local_player_id_ = GAME_APP.GetPlayerManager().GetMyPlayer()->GetId();
    auto characterId = GAME_APP.GetPlayerManager().GetMyPlayer()->GetCharacterId();
    
    if (NETWORK.IsServer() || !NETWORK.IsRunning())
    {
        background_ = GAME_APP.GetMapManager().GetRandomMap();
        if (background_->Initialize())
        {
            is_network_game_ = NETWORK.IsRunning();
        }
    }

    if (restart_button_) restart_button_->SetVisible(false);
    if (exit_button_) exit_button_->SetVisible(false);

    CreateGamePlayer(std::span<const uint8_t>(), std::span<const uint8_t>(), local_player_id_, characterId);
    ScheduleGameStart();

    should_quit_ = false;
    lastInputTime_ = SDL_GetTicks();

    SDL_StartTextInput(GAME_APP.GetWindow());    
}

void GameState::Leave()
{
    if (local_player_)
    {
        local_player_->RemoveEventListener(this);
    }

    if (remote_player_)
    {
        remote_player_->RemoveEventListener(this);
    }

    if (restart_button_) restart_button_->SetVisible(false);
    if (exit_button_) exit_button_->SetVisible(false);

    if (local_player_) local_player_->Reset();
    if (remote_player_) remote_player_->Reset();

    if (background_)
    {
        background_->Reset();
    }

    SDL_StopTextInput(GAME_APP.GetWindow());
}

void GameState::Update(float deltaTime)
{
    TIMER_SCHEDULER.Update();

    if (background_)
    {
        background_->Update(deltaTime);
    }

    if (local_player_)
    {
        local_player_->Update(deltaTime);
        local_player_->UpdateGameLogic(deltaTime);
    }

    if (remote_player_ && is_network_game_)
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

    if (background_)
    {
        background_->Render();
    }

    if (local_player_)
    {
        local_player_->Render();
    }

    if (remote_player_ && is_network_game_)
    {
        remote_player_->Render();
    }

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

    for (int x = 0; x <= Constants::Board::BOARD_X_COUNT; ++x)
    {
        float xPos = Constants::Board::POSITION_X + Constants::Board::WIDTH_MARGIN + x * Constants::Block::SIZE;

        SDL_RenderLine(renderer,
            xPos,
            Constants::Board::POSITION_Y,
            xPos,
            Constants::Board::POSITION_Y + Constants::Board::HEIGHT);
    }

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
    if (should_quit_)
    {
        return;
    }

    if (local_player_ && local_player_->GetGameState() == GamePhase::Playing)
    {
        switch (event.key.key)
        {
        case SDLK_UP:
            local_player_->RotateBlock(0, false);
            break;
        case SDLK_1 :
            local_player_->SetTotalInterruptBlockCount(4);
			local_player_->GetInterruptView()->UpdateInterruptBlock(4);
            break;
        case SDLK_2:
            local_player_->SetTotalInterruptBlockCount(40);
            local_player_->GetInterruptView()->UpdateInterruptBlock(40);
            break;
        case SDLK_3:
            local_player_->SetTotalInterruptBlockCount(100);
            local_player_->GetInterruptView()->UpdateInterruptBlock(100);
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

    if (local_player_)
    {
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
        else if (local_player_->GetGameState() == GamePhase::GameOver && should_quit_)
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
    if (!local_player_ || !remote_player_)
    {
        LOGGER.Error("Cannot restart game: players not initialized");
        return false;
    }

    try
    {
        Reset();

        auto success = local_player_->Restart();
        if (!success)
        {
            LOGGER.Error("Failed to restart local player");
            return false;
        }

        if (NETWORK.IsServer())
        {
            const auto& next_blocks = local_player_->GetNextBlock();
            if (next_blocks.size() >= 2)
            {
                std::array<uint8_t, 2> block_type1{};
                std::array<uint8_t, 2> block_type2{};

                auto first_block = next_blocks[0].get();
                if (first_block) {
                    auto blocks = first_block->GetBlocks();
                    if (blocks.size() >= 2) {
                        block_type1[0] = static_cast<uint8_t>(blocks[0]->GetBlockType());
                        block_type1[1] = static_cast<uint8_t>(blocks[1]->GetBlockType());
                    }
                }

                auto second_block = next_blocks[1].get();
                if (second_block) {
                    auto blocks = second_block->GetBlocks();
                    if (blocks.size() >= 2) {
                        block_type2[0] = static_cast<uint8_t>(blocks[0]->GetBlockType());
                        block_type2[1] = static_cast<uint8_t>(blocks[1]->GetBlockType());
                    }
                }

                NETWORK.ReStartGame(block_type1, block_type2);
            }
        }
        else
        {
            const auto& next_blocks = local_player_->GetNextBlock();
            std::array<uint8_t, 2> block_type1{};
            std::array<uint8_t, 2> block_type2{};

            auto& first_blocks = next_blocks[0]->GetBlocks();
            block_type1[0] = static_cast<uint8_t>(first_blocks[0]->GetBlockType());
            block_type1[1] = static_cast<uint8_t>(first_blocks[1]->GetBlockType());

            auto& second_blocks = next_blocks[1]->GetBlocks();
            block_type2[0] = static_cast<uint8_t>(second_blocks[0]->GetBlockType());
            block_type2[1] = static_cast<uint8_t>(second_blocks[1]->GetBlockType());

            NETWORK.ReStartGame(block_type1, block_type2);
        }

        if (restart_button_) restart_button_->SetVisible(false);
        if (exit_button_) exit_button_->SetVisible(false);

        should_quit_ = false;
        lastInputTime_ = SDL_GetTicks();

        ScheduleGameStart();
        return true;
    }
    catch (const std::exception& e)
    {
        LOGGER.Error("Error during game restart: {}", e.what());
        return false;
    }
}

bool GameState::GameExit()
{
    NETWORK.Shutdown();
    GAME_APP.GetStateManager().RequestStateChange(StateManager::StateID::Login);
    return true;
}

void GameState::GameQuit()
{
    should_quit_ = true;

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

    packet_processor_.RegisterHandler<GameOverPacket>(
        PacketType::GameOver,
        [this](uint8_t connectionId, const GameOverPacket* packet) {
            HandleGameOver();
        }
    );

    packet_processor_.RegisterHandler<AttackInterruptPacket>(
        PacketType::AttackInterruptBlock,
        [this](uint8_t connectionId, const AttackInterruptPacket* packet) {
            HandleAttackInterrupt(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<AddInterruptBlockPacket>(
        PacketType::AddInterruptBlock,
        [this](uint8_t connectionId, const AddInterruptBlockPacket* packet) {
            HandleAddInterruptBlock(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<RestartGamePacket>(
        PacketType::RestartGame,
        [this](uint8_t connectionId, const RestartGamePacket* packet) {
            HandleRestart(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<DefenseInterruptPacket>(
        PacketType::DefenseInterruptBlock,
        [this](uint8_t connectionId, const DefenseInterruptPacket* packet) {
            HandleDefenseInterrupt(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<DefenseResultInterruptBlockCountPacket>(
        PacketType::DefenseResultInterruptBlockCount,
        [this](uint8_t connectionId, const DefenseResultInterruptBlockCountPacket* packet) {
            HandleDefenseResultInterruptBlockCount(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<AttackResultPlayerInterruptBlocCountPacket>(
        PacketType::AttackResultPlayerInterruptBlocCount,
        [this](uint8_t connectionId, const AttackResultPlayerInterruptBlocCountPacket* packet) {
            HandleAttackResultPlayerInterruptBlocCount(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<SyncBlockPositionYPacket>(
        PacketType::SyncBlockPositionY,
        [this](uint8_t connectionId, const SyncBlockPositionYPacket* packet) {
            HandleSyncBlockPositionY(connectionId, packet);
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

    background_ = GAME_APP.GetMapManager().CreateMap(packet->map_id);    

    if (background_ && background_->Initialize())
    {
        local_player_->SetBackGround(background_);

        auto& next_blocks = local_player_->GetNextBlock();
        background_->SetNextBlock(next_blocks[0]);
        background_->SetNextBlock(next_blocks[1]);
        
        std::span<const uint8_t> blockType1(packet->block1);
        std::span<const uint8_t> blockType2(packet->block2);

        CreateGamePlayer(blockType1, blockType2, packet->player_id, packet->character_id);

        ScheduleGameStart();
    }
}

void GameState::CreateGamePlayer(const std::span<const uint8_t>& blockType1, const std::span<const uint8_t>& blockType2,
    uint8_t playerIdx, uint16_t characterIdx)
{
    if (playerIdx == GAME_APP.GetPlayerManager().GetMyPlayer()->GetId())
    {
        if (local_player_->Initialize(blockType1, blockType2, playerIdx, characterIdx, background_) == false)
        {
            LOGGER.Error("Failed to initialize local player");
        }

        local_player_->AddEventListener(this);
        
    }
    else
    {
        if (remote_player_->Initialize(blockType1, blockType2, playerIdx, characterIdx, background_) == false)
        {
            LOGGER.Error("Failed to initialize remote player");
        }

        remote_player_->AddEventListener(this);

        is_network_game_ = true;
    }
}

void GameState::HandleAddNewBlock(uint8_t connectionId, const AddNewBlockPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    if (player->GetId() != local_player_id_ && remote_player_)
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

    if (player->GetId() != local_player_id_ && remote_player_)
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

    if (player->GetId() != local_player_id_ && remote_player_)
    {
        remote_player_->RotateBlock(packet->rotate_type, packet->is_horizontal_moving);
    }
}

void GameState::HandleStartGame()
{
    if (local_player_)
    {
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

    if (player->GetId() != local_player_id_ && remote_player_)
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

    if (player->GetId() != local_player_id_ && remote_player_)
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

    if (player->GetId() != local_player_id_ && remote_player_)
    {
        std::span<const float, 2> pos1{ packet->position1 };
        std::span<const float, 2> pos2{ packet->position2 };

        //LOGGER.Info("GameState::HandlePushBlockInGame playerID_({}) pos1: {} pos2: {}", remote_player_->GetPlayerID(), pos1, pos2);

        remote_player_->PushBlockInGame(pos1, pos2);
    }
}

void GameState::HandleAddInterruptBlock(uint8_t connectionId, const AddInterruptBlockPacket* packet)
{
    LOGGER.Info("GameState::HandleAddInterruptBlock y_row_count {} x_count {} ", packet->y_row_count, packet->x_count);

    if (auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id))
    {
        if (remote_player_)
        {
            const std::span<const uint8_t> xIndicies{ packet->x_indices.data(), packet->x_count};
            remote_player_->AddInterruptBlock(packet->y_row_count, xIndicies);
        }
    }
}

void GameState::HandleAttackInterrupt(uint8_t connectionId, const AttackInterruptPacket* packet)
{
    if (auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id))
    {
        if (local_player_)
        {
            //LOGGER.Info("GameState::HandleAttackInterrupt count {} ", packet->count);
            local_player_->AddInterruptBlock(packet->count);
            local_player_->SetComboAttackState(true);
        }

        if (remote_player_)
        {
            remote_player_->AttackInterruptBlock(packet->position_x, packet->position_y, packet->block_type);
            remote_player_->UpdateInterruptBlock(0);
        }
    }
}

void GameState::HandleRestart(uint8_t connectionId, const RestartGamePacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    if (player->GetId() != local_player_id_ && remote_player_)
    {
        std::span<const uint8_t> blockType1(packet->block1);
        std::span<const uint8_t> blockType2(packet->block2);

        GameRestart();

        remote_player_->Restart(blockType1, blockType2);
    }
}

void GameState::HandleDefenseInterrupt(uint8_t connectionId, const DefenseInterruptPacket* packet)
{
    if (auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id))
    {
        if (player->GetId() != local_player_id_ && remote_player_)
        {
            remote_player_->DefenseInterruptBlockCount(packet->count, packet->position_x, packet->position_y, packet->block_type);
        }
    }
}

void GameState::HandleDefenseResultInterruptBlockCount(uint8_t connectionId, const DefenseResultInterruptBlockCountPacket* packet)
{
	auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
	if (!player)
	{
		return;
	}
	if (local_player_)
	{
		//local_player_->SetTotalInterruptBlockCount(packet->count);
		local_player_->UpdateInterruptBlock(packet->count);
	}
}

void GameState::HandleAttackResultPlayerInterruptBlocCount(uint8_t connectionId, const AttackResultPlayerInterruptBlocCountPacket* packet)
{
    if (auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id))
    {
        if (remote_player_)
        {
            remote_player_->UpdateInterruptBlock(packet->count);
            remote_player_->SetGameBoardState(BoardState::Damaging);
        }

        if (local_player_)
        {
            local_player_->UpdateInterruptBlock(packet->attackerCount);
        }
    }
}

void GameState::HandleStopCombo(uint8_t connectionId, const StopComboPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player)
    {
        return;
    }

    if (local_player_)
    {
        local_player_->SetComboAttackState(false);
    }
}

void GameState::HandleSyncBlockPositionY(uint8_t connectionId, const SyncBlockPositionYPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id))
    {
        if (player->GetId() != local_player_id_ && remote_player_)
        {
            remote_player_->SyncPositionY(packet->position_y, packet->velocity);
        }
    }
}

void GameState::HandleLose(uint8_t connectionId, const LoseGamePacket* packet)
{
    if (auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id))
    {
        remote_player_->LoseGame(false);
        local_player_->LoseGame(true);
    }
}

void GameState::HandleGameOver()
{
    GameQuit();
}

void GameState::HandleSystemEvent(const SDL_Event& event)
{
}

void GameState::Reset()
{
    if (background_)
    {
        background_->Reset();
    }

    if (local_player_)
    {
        local_player_->Reset();
    }

    if (remote_player_)
    {
        remote_player_->Reset();
    }
}

void GameState::Release()
{
    Leave();

    if (restart_button_) {
        restart_button_->Release();
        restart_button_.reset();
    }

    if (exit_button_) {
        exit_button_->Release();
        exit_button_.reset();
    }

    if (background_) {
        background_->Release();
        background_.reset();
    }

    if (local_player_) {
        local_player_->Release();
        local_player_.reset();
    }

    if (remote_player_) {
        remote_player_->Release();
        remote_player_.reset();
    }

    initialized_ = false;
    is_network_game_ = false;

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

void GameState::OnPlayerEvent(const std::shared_ptr<BasePlayerEvent>& event)
{
    switch (event->GetType())
    {
    case PlayerEventType::GameOver:
    {
        HandlePlayerGameOver(std::dynamic_pointer_cast<GameOverEvent>(event));
        break;
    }
    case PlayerEventType::GameRestart:
    {
        HandleGameRestart(std::dynamic_pointer_cast<GameRestartEvent>(event));
        break;
    }
    case PlayerEventType::AddInterruptBlock:
    {
        HandleAddInterruptBlock(std::dynamic_pointer_cast<AddInterruptBlockEvent>(event));
        break;
    }
    case PlayerEventType::AttackInterruptBlock:
    {
        HandleAttackInterruptBlock(std::dynamic_pointer_cast<AttackInterruptBlockEvent>(event));
        break;
    }
    case PlayerEventType::DefenseBlock:
    {
        HandleDefenseBlock(std::dynamic_pointer_cast<DefenseBlockEvent>(event));
        break;
    }
    }
}

void GameState::HandlePlayerGameOver(const std::shared_ptr<GameOverEvent>& event)
{
    if (remote_player_)
    {
        remote_player_->LoseGame(true);
    }

    exit_button_->SetVisible(true);

    if (NETWORK.IsServer())
    {
		restart_button_->SetVisible(true);
    }
}

void GameState::HandleGameRestart(const std::shared_ptr<GameRestartEvent>& event)
{    
}

void GameState::HandleAddInterruptBlock(const std::shared_ptr<AddInterruptBlockEvent>& event)
{
    if (remote_player_)
    {
        remote_player_->AddInterruptBlock(event->GetCount());
    }
}

void GameState::HandleAttackInterruptBlock(const std::shared_ptr<AttackInterruptBlockEvent>& event)
{
    if (remote_player_)
    {
        remote_player_->AttackInterruptBlock(event->GetX(), event->GetY(), event->GetType());
		remote_player_->UpdateInterruptBlock(0);
    }
}

void GameState::HandleDefenseBlock(const std::shared_ptr<DefenseBlockEvent>& event)
{
    if (remote_player_)
    {
        remote_player_->DefenseInterruptBlockCount(event->GetBlockCount(), event->GetX(), event->GetY(), event->GetType());
    }
}