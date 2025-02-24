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


#include "../game/system/GameBoard.hpp"
#include "../game/view/InterruptBlockView.hpp"
#include "../game/view/ComboView.hpp"
#include "../game/view/ResultView.hpp"
#include "../game/system/GamePlayer.hpp"

#include "../game/block/GameGroupBlock.hpp"
#include "../game/block/IceBlock.hpp"
#include "../game/effect/BulletEffect.hpp"
#include "../game/effect/ExplosionEffect.hpp"
#include "../game/map/GameBackground.hpp"

#include "../texture/ImageTexture.hpp"



#include "../ui/EditBox.hpp"
#include "../ui/Button.hpp"

#include "../utils/Logger.hpp"

#include <format>
#include <random>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <span>
#include <string>
#include <functional>

#include <SdL3/SDL.h>
#include <SdL3/SDL_keyboard.h>
#include <SdL3/SDL_keycode.h>


GameState::GameState() 
{
    draw_objects_.reserve(100);
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
        if (!LoadResources() ||
            !InitializeComponents() ||
            !CreateUI()) {
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
    // ���� ���� �ʱ�ȭ
    if (!InitializeGameBoard()) 
    {
        return false;
    }

    // ���ͷ�Ʈ �� �ʱ�ȭ
    interrupt_view_ = std::make_unique<InterruptBlockView>();
    if (interrupt_view_)
    {
        interrupt_view_->SetPosition(Constants::Board::POSITION_X, 0);
    }
    else 
    {
        LOGGER.Error("Failed to create InterruptBlockView");
        return false;
    }

    // �޺� �� �ʱ�ȭ
    combo_view_ = std::make_unique<ComboView>();
    if (!combo_view_) 
    {
        LOGGER.Error("Failed to create ComboView");
        return false;
    }

    // ��� �� �ʱ�ȭ
    result_view_ = std::make_unique<ResultView>();
    if (!result_view_) 
    {
        LOGGER.Error("Failed to create ResultView");
        return false;
    }

    // ��Ʈ�� ��� �ʱ�ȭ
    control_block_ = std::make_unique<GameGroupBlock>();
    if (control_block_) 
    {
        control_block_->SetGameBlocks(block_list_);

        if (auto player = GAME_APP.GetPlayerManager().GetMyPlayer()) 
        {
            control_block_->SetPlayerID(player->GetId());
        }
    }
    else 
    {
        LOGGER.Error("Failed to create GameGroupBlock");
        return false;
    }

    return true;
}

bool GameState::InitializeGameBoard() 
{
    gameboard_ = std::make_unique<GameBoard>();
    if (!gameboard_) 
    {
        LOGGER.Error("Failed to create GameBoard");
        return false;
    }

    auto myPlayer = GAME_APP.GetPlayerManager().GetMyPlayer();

    // ���� ���� �ʱ�ȭ
    if (!gameboard_->Initialize(
        Constants::Board::POSITION_X,
        Constants::Board::POSITION_Y,
        block_list_,
        myPlayer ?
        myPlayer->GetId() : 0))
    {
        LOGGER.Error("Failed to initialize GameBoard");
        return false;
    }

    // ��� �ؽ�ó ����
    auto texture = ImageTexture::Create("PUYO/puyo_beta.png");
    if (!texture) 
    {
        LOGGER.Error("Failed to get block texture");
        return false;
    }

    gameboard_->SetBlockInfoTexture(texture);
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

    exit_button_->Init(buttonTexture,GAME_APP.GetWindowWidth() / 2.0f - 68.0f,GAME_APP.GetWindowHeight() / 2.0f + 30.0f, 136.0f, 49.0f);

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
    // ��� �ʱ�ȭ
    if (NETWORK.IsServer() || !NETWORK.IsRunning()) 
    {
        // TODO: �� �Ŵ��� ���� �� GetRandomMap() ����
        if (background_ = GAME_APP.GetMapManager().GetRandomMap(); background_->Initialize())
        {
            draw_objects_.push_back(background_.get());
        }
    }

    // �ʱ� ��� ����
    InitNextBlock();

#ifdef _APP_DEBUG_
    CreateBlocksFromFile();
#endif

    // ��Ʈ��ũ ���� �ʱ�ȭ
    if (NETWORK.IsRunning() && next_blocks_.size() == 2) 
    {
        // TODO: InitializeNetworkGame() ����
    }

    // UI ������Ʈ �ʱ�ȭ
    if (interrupt_view_) 
    {
        interrupt_view_->Initialize();
        draw_objects_.push_back(interrupt_view_.get());
    }

    if (gameboard_) 
    {
        gameboard_->ResetGroupBlock();
        gameboard_->SetState(BoardState::Normal);
        draw_objects_.push_back(gameboard_.get());
    }

    if (combo_view_) 
    {
        combo_view_->Initialize();
        draw_objects_.push_back(combo_view_.get());
    }

    if (result_view_) 
    {
        result_view_->Initialize();
        draw_objects_.push_back(result_view_.get());
    }

    if (chatbox_) chatbox_->SetVisible(true);
    if (restart_button_) restart_button_->SetVisible(false);
    if (exit_button_) exit_button_->SetVisible(false);

    // ���� ���� �ʱ�ȭ
    stateInfo_ = GameStateInfo{};
    scoreInfo_ = ScoreInfo{};
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

    // ���� ������Ʈ ����
    draw_objects_.clear();
    ice_blocks_.clear();

    if (background_) 
    {
        background_->Reset();
    }

    // ��� ����
    for (auto& block : next_blocks_) 
    {
        block.reset();
    }
    next_blocks_.clear();

    for (auto& block : block_list_) 
    {
        block.reset();
    }
    block_list_.clear();

    // �Ѿ� ����
    bullets_.clear();
    bullets_to_delete_.clear();

    // ���� ���� �ʱ�ȭ
    std::memset(board_blocks_, 0, sizeof(board_blocks_));

    SDL_StopTextInput(GAME_APP.GetWindow());
}

void GameState::Update(float deltaTime) 
{
    stateInfo_.playTime += deltaTime;

    // ���� ���� üũ
    if (stateInfo_.playTime >= Constants::Game::PLAY_START_DELAY &&
        !stateInfo_.isRunning &&
        stateInfo_.currentPhase == GamePhase::Playing &&
        background_ && background_->IsReadyGame())
    {
        stateInfo_.isRunning = true;
        if (NETWORK.IsRunning() && NETWORK.IsServer()) 
        {
            NETWORK.StartGame();
            CreateNextBlock();
        }
    }

    // ���� ���� ������Ʈ
    UpdateGameLogic(deltaTime);

    // ���� ������Ʈ ������Ʈ
    for (auto* obj : draw_objects_) 
    {
        if (obj) 
        {
            obj->Update(deltaTime);
        }
    }

    // ��� ������Ʈ
    for (auto& block : block_list_) 
    {
        if (block) 
        {
            block->Update(deltaTime);
        }
    }

    // �Ѿ� ������Ʈ
    BulletUpdate(deltaTime);

    // ��Ʈ��ũ �÷��̾� ������Ʈ
    if (game_player_ && NETWORK.IsRunning())
    {
        game_player_->Update(deltaTime);
    }

    GAME_APP.GetParticleManager().Update(deltaTime);

    // UI ������Ʈ
    if (chatbox_) 
    {
        chatbox_->Update(deltaTime);
    }
}

void GameState::UpdateGameLogic(float deltaTime) 
{
    switch (stateInfo_.currentPhase)
    {
    case GamePhase::Standing:
        if (result_view_) 
        {
            result_view_->Update(deltaTime);
        }
        break;

    case GamePhase::Playing:
        if (control_block_) 
        {
            control_block_->Update(deltaTime);
        }
        break;

    case GamePhase::IceBlocking:
        UpdateIceBlockPhase(deltaTime);
        break;

    case GamePhase::Shattering:
        UpdateShatteringPhase(deltaTime);
        break;

    case GamePhase::GameOver:
        UpdateGameOverPhase(deltaTime);
        break;
    }
}

void GameState::UpdateIceBlockPhase(float deltaTime) 
{
    if (block_list_.size() > 0) 
    {
        // ��� ����� ���� �������� üũ
        bool allStationary = true;
        for (const auto& block : block_list_) 
        {
            if (block->GetState() != BlockState::Stationary) 
            {
                allStationary = false;
                break;
            }
        }

        // ��� ����� ���� ���¸� ���� ��� ����
        if (allStationary) 
        {
            if (!IsGameOver()) 
            {
                stateInfo_.currentPhase = GamePhase::Playing;
                CreateNextBlock();
            }
        }
    }
}

void GameState::UpdateShatteringPhase(float deltaTime) 
{
    if (matched_blocks_.empty() == false) 
    {
        std::vector<SDL_FPoint> positions;
        std::list<SDL_Point> indexList;

        // ��Ī�� ��� �׷� ó��
        auto groupIter = matched_blocks_.begin();
        while (groupIter != matched_blocks_.end()) 
        {
            // �׷� �� ��� ����� PlayOut �������� Ȯ��
            bool allPlayedOut = std::all_of(groupIter->begin(), groupIter->end(),
                [](const Block* block) 
                {
                    return block->GetState() == BlockState::PlayOut; 
                });

            if (allPlayedOut) 
            {
                // ù ��° ������� �Ѿ� ����
                if (!groupIter->empty()) 
                {
                    CreateBullet(groupIter->front());
                }

                // �׷� �� ��� ��� ó��
                for (auto* block : *groupIter) 
                {
                    // ��ġ ���� ����
                    SDL_FPoint pos{ block->GetX(), block->GetY() };
                    SDL_Point idx{ block->GetPosIdx_X(), block->GetPosIdx_Y() };

                    // ��ƼŬ ����
                    auto particleContainer = std::make_unique<ExplosionContainer>();
                    particleContainer->SetBlockType(block->GetBlockType());
                    particleContainer->SetPlayerID(GAME_APP.GetPlayerManager().GetMyPlayer()->GetId());

                    GAME_APP.GetParticleManager().AddParticleContainer(std::move(particleContainer), pos);

                    // ���� ���忡�� ��� ����
                    board_blocks_[idx.y][idx.x] = nullptr;
                    block_list_.remove(std::shared_ptr<Block>(block));
                    indexList.push_back(idx);
                }

                // �޺� ǥ�� ������Ʈ
                if (combo_view_ && scoreInfo_.comboCount > 0 && !groupIter->empty()) 
                {
                    auto* firstBlock = groupIter->front();
                    combo_view_->UpdateComboCount(firstBlock->GetX(), firstBlock->GetY(), scoreInfo_.comboCount);
                }

                // ���� ��� ó��
                if (!ice_blocks_.empty()) 
                {
                    for (const auto& iceBlock : ice_blocks_) 
                    {
                        SDL_Point iceIdx{ iceBlock->GetPosIdx_X(), iceBlock->GetPosIdx_Y()};
                        board_blocks_[iceIdx.y][iceIdx.x] = nullptr;
                        block_list_.remove(iceBlock);
                        indexList.push_back(iceIdx);
                    }
                    ice_blocks_.clear();
                }

                groupIter = matched_blocks_.erase(groupIter);
            }
            else {
                ++groupIter;
            }
        }

        // ������ ��� ���� ��ϵ� ���� ���·� ����
        if (matched_blocks_.empty()) 
        {
            for (const auto& idx : indexList) 
            {
                for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; y++) 
                {
                    auto* block = board_blocks_[y][idx.x]
                        ;
                    if (block && y > idx.y) 
                    {
                        if (block->GetState() != BlockState::DownMoving) 
                        {
                            block->SetState(BlockState::DownMoving);
                        }
                    }
                }
            }
        }
    }
    else 
    {
        // ���� �� ���� üũ
        block_list_.sort(BlockComp());

        if (!block_list_.empty()) {
            // ��� ����� ���� �������� Ȯ��
            bool allStationary = std::all_of(block_list_.begin(), block_list_.end(),
                [](const auto& block) { return block->GetState() == BlockState::Stationary; });

            if (allStationary) {
                // ��� ��ũ ���� ������Ʈ
                for (const auto& block : block_list_) {
                    if (block->GetBlockType() != BlockType::Ice) {
                        block->SetLinkState();
                        UpdateLinkState(block.get());
                    }
                }

                // ���� ���� üũ �� ���� �ܰ� ó��
                if (!CheckGameBlockState()) 
                {
                    if (!stateInfo_.shouldQuit) 
                    {
                        if (scoreInfo_.totalInterruptBlockCount > 0 && !stateInfo_.isComboAttack && !stateInfo_.isDefending)
                        {
                            GenerateIceBlocks();
                        }
                        else {
                            CreateNextBlock();
                        }
                    }
                }
            }
        }
        else {
            if (stateInfo_.shouldQuit) {
                stateInfo_.currentPhase = GamePhase::Standing;
            }
            else {
                if (NETWORK.IsRunning()) 
                {
                    NETWORK.StopComboAttack();
                }

                stateInfo_.currentPhase = GamePhase::Playing;
                stateInfo_.previousPhase = GamePhase::Playing;

                if (scoreInfo_.totalInterruptBlockCount > 0 && !stateInfo_.isComboAttack && !stateInfo_.isDefending)
                {
                    GenerateIceBlocks();
                }
                else {
                    CreateNextBlock();
                }
            }
        }
    }
}

void GameState::UpdateGameOverPhase(float deltaTime) 
{    
    // ���� ���� �ִϸ��̼� �� UI ������Ʈ
    if (result_view_) 
    {
        result_view_->Update(deltaTime);
    }

    //TODO
   /* int numkeys;
    const bool* keyState = SDL_GetKeyboardState(&numkeys);
        
    while (true) 
    {
        SDL_PumpEvents();
        if (keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_SPACE])
        {
            if (NETWORK.IsServer())
            {
                GameRestart();
                break;
            }
        }
        else if (keyState[SDL_SCANCODE_ESCAPE])
        {
            GameExit();
            break;
        }
    }*/
}

void GameState::BulletUpdate(float deltaTime) 
{
    auto it = bullets_.begin();

    while (it != bullets_.end()) 
    {
        auto& bullet = *it;
        if (bullet) 
        {
            bullet->Update(deltaTime);

            if (!bullet->IsAlive()) 
            {
                bullets_to_delete_.push_back(bullet.get());
                it = bullets_.erase(it);
            }
            else 
            {
                ++it;
            }
        }
        else 
        {
            it = bullets_.erase(it);
        }
    }
}

void GameState::HandleEvent(const SDL_Event& event) 
{
    if (stateInfo_.shouldQuit) 
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
    if (stateInfo_.currentPhase != GamePhase::Playing) 
    {
        return;
    }

    switch (event.key.key) 
    {
    case SDLK_UP:
        if (control_block_ && control_block_->GetState() == BlockState::Playing) 
        {
            control_block_->Rotate();
        }
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

void GameState::HandleKeyboardState() 
{
    if (SDL_GetTicks() - lastInputTime_ < 40) 
    {
        return;
    }

    const bool* keyStates = SDL_GetKeyboardState(nullptr);    

    if (stateInfo_.currentPhase == GamePhase::Playing)
    {
        if (control_block_ && control_block_->GetState() == BlockState::Playing)
        {
            if (keyStates[SDL_SCANCODE_LEFT])
            {
                control_block_->MoveLeft();
            }

            if (keyStates[SDL_SCANCODE_RIGHT])
            {
                control_block_->MoveRight();
            }

            if (keyStates[SDL_SCANCODE_DOWN])
            {
                if (control_block_->GetAddForceVelocityY() <= 70.0f)
                {
                    control_block_->ForceAddVelocityY(10.0f);
                }
            }
        }      
    }
    else if (stateInfo_.currentPhase == GamePhase::GameOver)
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
    

    lastInputTime_ = SDL_GetTicks();
}

void GameState::CreateBullet(Block* block) 
{
    if (!block) 
    {
        LOGGER.Error("Attempt to create bullet with null block");
        return;
    }

    // ���� ���� ������ ��ǥ�� ���� ���� ��ġ ���
    SDL_FPoint startPos
    {
        Constants::Board::POSITION_X + Constants::Board::WIDTH_MARGIN + block->GetX() + Constants::Block::SIZE / 2,
        Constants::Board::POSITION_Y + block->GetY() + Constants::Block::SIZE / 2
    };

    SDL_FPoint endPos
    {
        stateInfo_.hasIceBlock ? Constants::Board::POSITION_X + (Constants::Board::WIDTH / 2) : GAME_APP.GetWindowWidth() - (Constants::Board::POSITION_X + (Constants::Board::WIDTH / 2)),
        Constants::Board::POSITION_Y
    };

    auto bullet = std::make_unique<BulletEffect>();
    if (!bullet->Initialize(startPos, endPos, block->GetBlockType())) 
    {
        LOGGER.Error("Failed to create bullet effect");
        return;
    }

    bullet->SetAttacking(!stateInfo_.hasIceBlock);
    bullets_.push_back(std::move(bullet));

    // ���� ��� ���� ��Ʈ��ũ ó��
    if (stateInfo_.hasIceBlock) 
    {
        NETWORK.DefenseInterruptBlock(
            scoreInfo_.addInterruptBlockCount,
            block->GetX(),
            block->GetY(),
            static_cast<uint8_t>(block->GetBlockType()));

        if (NETWORK.IsServer()) 
        {
            if (interrupt_view_) 
            {
                interrupt_view_->UpdateInterruptBlock(scoreInfo_.totalInterruptBlockCount);
            }
        }
    }
    else 
    {
        NETWORK.AttackInterruptBlock(
            scoreInfo_.addInterruptBlockCount,
            block->GetX(),
            block->GetY(),
            static_cast<uint8_t>(block->GetBlockType()));

        if (NETWORK.IsServer()) 
        {
            if (game_player_) 
            {
                game_player_->AddInterruptBlock(scoreInfo_.addInterruptBlockCount);
            }
        }
    }

    scoreInfo_.addInterruptBlockCount = 0;

    if (gameboard_ && gameboard_->GetState() != BoardState::Lose) {
        gameboard_->SetState(BoardState::Attacking);
    }
}

bool GameState::GameRestart() 
{
    // ���� ������ ����
    ice_blocks_.clear();
    if (background_) 
    {
        background_->Reset();
    }

    for (auto& block : next_blocks_) 
    {
        block.reset();
    }
    next_blocks_.clear();

    for (auto& block : block_list_) 
    {
        block.reset();
    }
    block_list_.clear();

    bullets_.clear();
    bullets_to_delete_.clear();

    // ���� ���� �ʱ�ȭ
    std::memset(board_blocks_, 0, sizeof(board_blocks_));

    // ������Ʈ �ʱ�ȭ
    if (control_block_) 
    {
        control_block_->ResetBlock();
    }

    if (gameboard_) 
    {
        gameboard_->ClearActiveGroupBlock();
        gameboard_->SetState(BoardState::Normal);
    }

    if (interrupt_view_) 
    {
        interrupt_view_->Initialize();
    }

    if (combo_view_) 
    {
        combo_view_->Initialize();
    }

    if (result_view_) 
    {
        result_view_->Initialize();
    }

    // UI ���� �ʱ�ȭ
    if (chatbox_) chatbox_->SetVisible(true);
    if (restart_button_) restart_button_->SetVisible(false);
    if (exit_button_) exit_button_->SetVisible(false);

    // ���� ���� �ʱ�ȭ
    stateInfo_ = GameStateInfo{};
    scoreInfo_ = ScoreInfo{};
    lastInputTime_ = SDL_GetTicks();

    // ���� ��� �ʱ�ȭ
    InitNextBlock();

#ifdef _APP_DEBUG_
    CreateBlocksFromFile();
#endif

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
    stateInfo_.shouldQuit = true;

    if (result_view_) {
        result_view_->UpdateResult(
            Constants::Board::POSITION_X + 20,
            100,
            true);
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

void GameState::InitNextBlock() 
{
    auto nextBlock1 = std::make_unique<GroupBlock>();
    auto nextBlock2 = std::make_unique<GroupBlock>();

    if (!nextBlock1->Create() || !nextBlock2->Create()) 
    {
        LOGGER.Error("Failed to create initial blocks");
        return;
    }

    nextBlock1->SetPosition(Constants::BlockPosition::NEXT_BLOCK_POS_X, Constants::BlockPosition::NEXT_BLOCK_POS_Y);

    nextBlock2->SetPosition(Constants::BlockPosition::NEXT_BLOCK_POS_SMALL_X, Constants::BlockPosition::NEXT_BLOCK_POS_SMALL_Y);
    nextBlock2->SetSize(Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE, Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE);

    next_blocks_.push_back(std::move(nextBlock1));
    next_blocks_.push_back(std::move(nextBlock2));

    if (background_) 
    {
        background_->Reset();
        background_->SetNextBlock(std::move(next_blocks_[0]));
        background_->SetNextBlock(std::move(next_blocks_[1]));
    }
}

void GameState::CreateNextBlock() 
{
    if (background_ && background_->IsChangingBlock()) 
    {
        return;
    }

    auto nextBlock = std::make_unique<GroupBlock>();
    if (!nextBlock->Create()) 
    {
        LOGGER.Error("Failed to create next block");
        return;
    }

    nextBlock->SetPosition(Constants::BlockPosition::NEXT_BLOCK_POS_SMALL_X, 100);
    nextBlock->SetSize(Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE, Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE);

    if (background_) 
    {
        next_blocks_.push_back(std::move(nextBlock));
        background_->SetNextBlock(std::move(nextBlock));
    }

    if (gameboard_) {
        gameboard_->SetRenderTargetMark(false);
    }

    // ��Ʈ��ũ ����ȭ
    if (NETWORK.IsRunning()) 
    {
        if (auto lastBlock = next_blocks_.back().get()) 
        {
            auto blocks = lastBlock->GetBlocks();
            std::array<uint8_t, 2> blockTypes{static_cast<uint8_t>(blocks[0]->GetBlockType()), static_cast<uint8_t>(blocks[1]->GetBlockType())};
            
            NETWORK.AddNewBlock(blockTypes);
        }
    }
}

void GameState::Render() 
{
    if (!initialized_) 
    {
        return;
    }

    for (const auto obj : draw_objects_) 
    {
        if (obj) 
        {
            obj->Render();
        }
    }

    if (NETWORK.IsRunning() && game_player_) 
    {
        game_player_->Render();
    }

    // 3. �Ѿ� ������
    for (const auto& bullet : bullets_) 
    {
        if (bullet) 
        {
            bullet->Render();
        }
    }

    RenderUI();

#ifdef _DEBUG
    RenderDebugInfo();
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
    if (!renderer) {
        return;
    }

    const SDL_Color debugColor{ 255, 255, 0, 255 };  // �����

    // ���� ���� ����
    std::string stateInfo = std::format("Phase: {}, Combo: {}, Score: {}",
        static_cast<int>(stateInfo_.currentPhase),
        scoreInfo_.comboCount,
        scoreInfo_.totalScore);

    // ��� ����
    std::string blockInfo = std::format("Active: {}, Ice: {}",
        block_list_.size(),
        ice_blocks_.size());

    // ����� �ؽ�Ʈ ������
    //RenderDebugText(stateInfo, 10, 10, debugColor);
    //RenderDebugText(blockInfo, 10, 30, debugColor);

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


bool GameState::FindMatchedBlocks(std::vector<std::vector<Block*>>& matchedGroups) 
{
    matchedGroups.clear();
    std::vector<Block*> currentGroup;
    int blockCount = 0;

    // ��� ��� ��ȸ�ϸ� ��Ī �˻�
    for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; y++) 
    {
        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++) 
        {
            if (blockCount == block_list_.size()) 
            {
                break;
            }

            Block* block = board_blocks_[y][x];
            if (!block || block->GetBlockType() == BlockType::Ice ||block->IsRecursionCheck()) 
            {
                continue;
            }

            blockCount++;
            block->SetRecursionCheck(true);
            currentGroup.clear();

            // ��������� ��Ī�Ǵ� ��� ã��
            if (RecursionCheckBlock(x, y, Constants::Direction::None, currentGroup) >= Constants::Game::MIN_MATCH_COUNT - 1)
            {
                currentGroup.push_back(block);
                matchedGroups.push_back(currentGroup);
            }
            else 
            {
                block->SetRecursionCheck(false);
                for (auto* matchedBlock : currentGroup) {
                    matchedBlock->SetRecursionCheck(false);
                }
            }
        }
    }

    return !matchedGroups.empty();
}

short GameState::RecursionCheckBlock(short x, short y, Constants::Direction direction,
    std::vector<Block*>& matchedBlocks)
{
    if (!board_blocks_[y][x]) {
        return 0;
    }

    Block* block = board_blocks_[y][x];
    BlockType blockType = block->GetBlockType();
    short matchCount = 0;

    // �� ���� �˻�
    const std::array<std::pair<Constants::Direction, std::pair<short, short>>, 4> directions = { {
        {Constants::Direction::Left,   {x - 1, y}},
        {Constants::Direction::Right,  {x + 1, y}},
        {Constants::Direction::Top,    {x, y + 1}},
        {Constants::Direction::Bottom, {x, y - 1}}
    } };

    for (const auto& [dir, pos] : directions) {
        if (direction == dir) {
            continue;
        }

        const auto [checkX, checkY] = pos;
        if (checkX < 0 || checkX >= Constants::Board::BOARD_X_COUNT ||
            checkY < 0 || checkY >= Constants::Board::BOARD_Y_COUNT) {
            continue;
        }

        Block* checkBlock = board_blocks_[checkY][checkX];
        if (!checkBlock || checkBlock->IsRecursionCheck() ||
            checkBlock->GetState() != BlockState::Stationary) {
            continue;
        }

        if (blockType == checkBlock->GetBlockType()) 
        {
            checkBlock->SetRecursionCheck(true);
            matchedBlocks.push_back(checkBlock);
            matchCount += 1 + RecursionCheckBlock(checkX, checkY, GameStateDetail::GetOppositeDirection(dir), matchedBlocks);
        }
    }

    return matchCount;
}

void GameState::UpdateTargetPosIdx() 
{
    if (!control_block_ || control_block_->GetState() != BlockState::Playing)
    {
        return;
    }

    auto& blocks = control_block_->GetBlocks();
    auto rotateState = control_block_->GetRotateState();

    size_t currentIndex = (rotateState == RotateState::Default) ? 1 : 0;
    std::array<BlockTargetMark, 2> markPositions{};
    int checkIdxX = -1, checkIdxY = -1;

    // ��ũ�� �߾ӿ� ǥ���ϱ� ���� ���� ��ġ ���
    float renderPos = (Constants::Block::SIZE - Constants::Board::BLOCK_MARK_SIZE) / 2.0f;

    for (int i = 0; i < 2; ++i) 
    {
        if (const auto& block = blocks[currentIndex]) 
        {
            int xIdx = block->GetPosIdx_X();
            BlockType blockType = block->GetBlockType();

            // ���� �������� �� ���� ã��
            for (int yIdx = 0; yIdx < Constants::Board::BOARD_Y_COUNT; ++yIdx) 
            {
                if (checkIdxX == xIdx && checkIdxY == yIdx) 
                {
                    continue;
                }

                if (board_blocks_[yIdx][xIdx] == nullptr) 
                {
                    markPositions[i].xPos = (xIdx * Constants::Block::SIZE) + renderPos;
                    markPositions[i].yPos = ((Constants::Board::BOARD_Y_COUNT - 2 - yIdx) * Constants::Block::SIZE) + renderPos;
                    markPositions[i].type = static_cast<uint8_t>(blockType);

                    checkIdxX = xIdx;
                    checkIdxY = yIdx;
                    break;
                }
            }

            // �ε��� ������Ʈ
            currentIndex = (rotateState == RotateState::Default) ? 0 : 1;
        }
    }

    // ���� ���忡 Ÿ�� ��ũ ������Ʈ
    if (gameboard_) 
    {
        gameboard_->UpdateTargetBlockMark(markPositions);
    }
}

void GameState::CreateGamePlayer(const std::span<const uint8_t>& blocktype1, const std::span<const uint8_t>& blocktype2, uint8_t playerIdx, uint8_t characterIdx)
{
    //mpGamePlayer = new GamePlayer;
    if (game_player_ == nullptr)
    {
        game_player_ = std::shared_ptr<GamePlayer>(new GamePlayer);
    }

    game_player_->Initialize(blocktype1, blocktype2, playerIdx, characterIdx, background_);
}

bool GameState::IsPossibleMove(int xIdx)
{
    return	
        board_blocks_[Constants::Board::BOARD_Y_COUNT - 2][xIdx] == NULL ||
        board_blocks_[Constants::Board::BOARD_Y_COUNT - 1][xIdx] == NULL ? true : false;
}

void GameState::UpdateLinkState(Block* block) 
{
    if (!block || block->GetBlockType() == BlockType::Ice) 
    {
        return;
    }

    int x = block->GetPosIdx_X();
    int y = block->GetPosIdx_Y();
    BlockType blockType = block->GetBlockType();
    uint8_t linkState = static_cast<uint8_t>(block->GetLinkState());

    // �� ���⺰ ���� ���� �˻�
    const std::array<std::pair<Constants::Direction, std::pair<int, int>>, 4> directions = { {
        {Constants::Direction::Left,   {x - 1, y}},
        {Constants::Direction::Right,  {x + 1, y}},
        {Constants::Direction::Top,    {x, y + 1}},
        {Constants::Direction::Bottom, {x, y - 1}}
    } };

    for (const auto& [dir, pos] : directions) {
        const auto [checkX, checkY] = pos;

        if (checkX >= 0 && checkX < Constants::Board::BOARD_X_COUNT &&
            checkY >= 0 && checkY < Constants::Board::BOARD_Y_COUNT)
        {
            Block* checkBlock = board_blocks_[checkY][checkX];
            if (checkBlock && checkBlock->GetState() == BlockState::Stationary &&
                checkBlock->GetBlockType() == blockType)
            {
                // ���� ���� ������Ʈ
                switch (dir) {
                case Constants::Direction::Left:
                    linkState |= static_cast<uint8_t>(LinkState::Left);
                    checkBlock->SetLinkState(static_cast<LinkState>(
                        static_cast<uint8_t>(checkBlock->GetLinkState()) |
                        static_cast<uint8_t>(LinkState::Right)));
                    break;
                case Constants::Direction::Right:
                    linkState |= static_cast<uint8_t>(LinkState::Right);
                    checkBlock->SetLinkState(static_cast<LinkState>(
                        static_cast<uint8_t>(checkBlock->GetLinkState()) |
                        static_cast<uint8_t>(LinkState::Left)));
                    break;
                case Constants::Direction::Top:
                    linkState |= static_cast<uint8_t>(LinkState::Top);
                    checkBlock->SetLinkState(static_cast<LinkState>(
                        static_cast<uint8_t>(checkBlock->GetLinkState()) |
                        static_cast<uint8_t>(LinkState::Bottom)));
                    break;
                case Constants::Direction::Bottom:
                    linkState |= static_cast<uint8_t>(LinkState::Bottom);
                    checkBlock->SetLinkState(static_cast<LinkState>(
                        static_cast<uint8_t>(checkBlock->GetLinkState()) |
                        static_cast<uint8_t>(LinkState::Top)));
                    break;
                }
            }
        }
    }

    block->SetLinkState(static_cast<LinkState>(linkState));
}

void GameState::CalculateScore() 
{
    // �޺� ���� ������Ʈ
    if (stateInfo_.previousPhase == GamePhase::Shattering) 
    {
        scoreInfo_.comboCount++;
    }
    else if (stateInfo_.previousPhase == GamePhase::Playing) 
    {
        scoreInfo_.comboCount = 1;
    }

    uint8_t linkBonus = 0;
    uint8_t blockCount = 0;
    uint8_t typeBonus = GetTypeBonus(matched_blocks_.size());
    short comboBonus = GetComboConstant(scoreInfo_.comboCount);

    // ��Ī�� ��ϵ鿡 ���� ���ʽ� ���
    for (const auto& group : matched_blocks_) {
        for (auto* block : group) {
            block->SetState(BlockState::Destroying);
        }

        linkBonus += GetLinkBonus(group.size());
        blockCount += static_cast<uint8_t>(group.size());
    }

    // ���� ���� ���
    int currentScore = ((blockCount * Constants::Game::Score::BASE_MATCH_SCORE) *
        (comboBonus + linkBonus + typeBonus + 1));

    // ���� ��� ī��Ʈ ���
    scoreInfo_.addInterruptBlockCount =
        (currentScore + scoreInfo_.restScore) / GetMargin();
    scoreInfo_.restScore =
        (currentScore + scoreInfo_.restScore) % GetMargin();
    scoreInfo_.totalScore += currentScore;

    // ���� ��� ���� ������Ʈ
    UpdateInterruptBlockState();
}

void GameState::UpdateInterruptBlockState() 
{
    if (scoreInfo_.totalInterruptBlockCount > 0) 
    {
        scoreInfo_.totalInterruptBlockCount -= scoreInfo_.addInterruptBlockCount;

        if (scoreInfo_.totalInterruptBlockCount < 0) 
        {
            scoreInfo_.addInterruptBlockCount = std::abs(scoreInfo_.totalInterruptBlockCount);
            stateInfo_.hasIceBlock = false;
            scoreInfo_.totalInterruptBlockCount = 0;

            if (NETWORK.IsServer()) 
            {
                if (scoreInfo_.addInterruptBlockCount > 0) 
                {
                    scoreInfo_.totalEnemyInterruptBlockCount += scoreInfo_.addInterruptBlockCount;
                }
            }
        }
        else 
        {
            stateInfo_.hasIceBlock = true;
        }
    }
    else 
    {
        if (NETWORK.IsServer()) 
        {
            if (scoreInfo_.addInterruptBlockCount > 0) 
            {
                scoreInfo_.totalEnemyInterruptBlockCount += scoreInfo_.addInterruptBlockCount;
            }
        }
        stateInfo_.hasIceBlock = false;
        scoreInfo_.totalInterruptBlockCount = 0;
    }
}

short GameState::GetComboConstant(uint8_t comboCount) const 
{
    if (comboCount <= 1) 
    {
        return 0;
    }

    if (comboCount <= 4) 
    {
        return static_cast<short>(std::pow(2, comboCount + 1));
    }

    if (comboCount <= Constants::Game::MAX_COMBO) 
    {
        return 32 * (comboCount - 3);
    }

    return 0;
}

uint8_t GameState::GetLinkBonus(size_t linkCount) const
{
    static const std::array<uint8_t, 8> LINK_BONUSES = { 0, 0, 0, 0, 2, 3, 4, 5 };

    if (linkCount <= 4) 
    {
        return 0;
    }
    else if (linkCount <= 10) 
    {
        return LINK_BONUSES[linkCount - 4];
    }
    else 
    {
        return Constants::Game::Score::MAX_LINK_BONUS;
    }
}

uint8_t GameState::GetTypeBonus(size_t count) const 
{
    static const std::array<uint8_t, 6> TYPE_BONUSES = 
    {
        0, 0, 3, 6, 12, Constants::Game::Score::MAX_TYPE_BONUS
    };
    return count < TYPE_BONUSES.size() ? TYPE_BONUSES[count] : TYPE_BONUSES.back();
}

uint8_t GameState::GetMargin() const 
{
    const float playTime = stateInfo_.playTime;

    // �ð��� ���� ���� �� ���
    for (const auto& margin : Constants::Game::SCORE_MARGINS) 
    {
        if (playTime <= margin.time) 
        {
            return margin.margin;
        }
    }

    return Constants::Game::SCORE_MARGINS[
        std::size(Constants::Game::SCORE_MARGINS) - 1].margin;
}

void GameState::UpdateComboState() 
{
    if (stateInfo_.previousPhase == GamePhase::Shattering) 
    {
        scoreInfo_.comboCount++;
    }
    else if (stateInfo_.previousPhase == GamePhase::Playing) 
    {
        scoreInfo_.comboCount = 1;
    }
}

void GameState::ResetComboState() 
{
    if (scoreInfo_.comboCount > 0) 
    {
        scoreInfo_.comboCount = 0;
        if (NETWORK.IsRunning()) 
        {
            NETWORK.StopComboAttack();
        }
    }

    if (scoreInfo_.restScore > 0) 
    {
        scoreInfo_.restScore = 0;
    }
}

void GameState::GenerateIceBlocks() 
{
    if (scoreInfo_.totalInterruptBlockCount <= 0 || stateInfo_.currentPhase != GamePhase::Playing) 
    {
        return;
    }

    auto texture = ImageTexture::Create("PUYO/puyo_beta.png");
    if (!texture) 
    {
        LOGGER.Error("Failed to get ice block texture");
        return;
    }

    const auto playerID = GAME_APP.GetPlayerManager().GetMyPlayer()->GetId();

    // ��� ���� ���� ���� ��� ����
    if (scoreInfo_.totalInterruptBlockCount > 30) 
    {
        GenerateLargeIceBlockGroup(texture, playerID);
    }
    else 
    {
        GenerateSmallIceBlockGroup(texture, playerID);
    }

    // UI ������Ʈ
    if (interrupt_view_) 
    {
        interrupt_view_->UpdateInterruptBlock(scoreInfo_.totalInterruptBlockCount);
    }

    HandlePhaseTransition(GamePhase::IceBlocking);
}

void GameState::GenerateLargeIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID)
{
    scoreInfo_.totalInterruptBlockCount -= 30;

    if (NETWORK.IsRunning()) 
    {
        NETWORK.AddInterruptBlock(5, 0, std::span<const uint8_t>());
    }

    // 5x6 ũ���� ���� ��� �׷� ����
    for (int y = 0; y < 5; y++) 
    {
        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++) 
        {
            auto iceBlock = std::make_shared<IceBlock>();
            InitializeIceBlock(iceBlock.get(), texture, x, y, playerID);
            block_list_.push_back(iceBlock);
            ice_blocks_.insert(iceBlock);
        }
    }
}

void GameState::GenerateSmallIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID)
{
    const auto yCnt = scoreInfo_.totalInterruptBlockCount / Constants::Board::BOARD_X_COUNT;
    const auto xCnt = scoreInfo_.totalInterruptBlockCount % Constants::Board::BOARD_X_COUNT;

    // ��ü �� ����
    for (int y = 0; y < yCnt; y++) 
    {
        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++) 
        {
            auto iceBlock = std::make_shared<IceBlock>();
            InitializeIceBlock(iceBlock.get(), texture, x, y, playerID);
            block_list_.push_back(iceBlock);
            ice_blocks_.insert(iceBlock);
        }
    }

    // ���� ��� ���� ��ġ
    if (xCnt > 0) 
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, Constants::Board::BOARD_X_COUNT - 1);

        std::set<int> positions;
        while (positions.size() < xCnt) 
        {
            positions.insert(dist(gen));
        }

        std::array<uint8_t, 5> xIdxList{};
        int idx = 0;
        for (int pos : positions) 
        {
            auto iceBlock = std::make_shared<IceBlock>();
            InitializeIceBlock(iceBlock.get(), texture, pos, yCnt, playerID);
            block_list_.push_back(iceBlock);
            ice_blocks_.insert(iceBlock);
            xIdxList[idx++] = static_cast<uint8_t>(pos);
        }

        if (NETWORK.IsRunning()) 
        {
            NETWORK.AddInterruptBlock(static_cast<uint8_t>(yCnt), static_cast<uint8_t>(xCnt), std::span<const uint8_t>(xIdxList));
        }
    }

    scoreInfo_.totalInterruptBlockCount = 0;
}

void GameState::InitializeIceBlock(IceBlock* block, const std::shared_ptr<ImageTexture>& texture, int x, int y, uint8_t playerID)
{
    if (!block) 
    {
        return;
    }

    // ��� �⺻ �Ӽ� ����
    block->SetBlockType(BlockType::Ice);
    block->SetLinkState(LinkState::Max);
    block->SetState(BlockState::DownMoving);
    block->SetBlockTex(texture);

    // �ε��� ��ġ ����
    block->SetPosIdx(x, y);

    // ���� ������ ��ġ ���
    float renderX = Constants::Board::WIDTH_MARGIN + Constants::Block::SIZE * x;
    float renderY = -Constants::Block::SIZE * (y + 1);  // ���������� ����
    block->SetPosition(renderX, renderY);

    // ũ�� ����
    block->SetSize(Constants::Block::SIZE, Constants::Block::SIZE);

    // �÷��̾� ID ����
    block->SetPlayerID(playerID);
}

void GameState::CollectRemoveIceBlocks() 
{
    if (block_list_.empty() || matched_blocks_.empty() || stateInfo_.currentPhase != GamePhase::Shattering) 
    {
        return;
    }

    // ��Ī�� ��� �ֺ��� ���� ��� ����
    for (const auto& group : matched_blocks_) 
    {
        for (auto* block : group) 
        {
            const int x = block->GetPosIdx_X();
            const int y = block->GetPosIdx_Y();

            // �ֺ� 4���� �˻�
            const std::array<std::pair<int, int>, 4> directions = 
            { {
                {x - 1, y}, {x + 1, y}, {x, y - 1}, {x, y + 1}
            } };

            for (const auto& [checkX, checkY] : directions) 
            {
                if (checkX >= 0 && checkX < Constants::Board::BOARD_X_COUNT && checkY >= 0 && checkY < Constants::Board::BOARD_Y_COUNT)
                {
                    if (auto* checkBlock = board_blocks_[checkY][checkX]) 
                    {
                        if (checkBlock->GetBlockType() == BlockType::Ice && checkBlock->GetState() == BlockState::Stationary)
                        {
                            if (auto iceBlock = dynamic_cast<IceBlock*>(checkBlock)) 
                            {
                                iceBlock->SetState(BlockState::Destroying);
                                ice_blocks_.insert(std::shared_ptr<IceBlock>(iceBlock));
                            }
                        }
                    }
                }
            }
        }
    }
}

void GameState::UpdateInterruptBlockView() 
{
    if (interrupt_view_) 
    {
        interrupt_view_->UpdateInterruptBlock(scoreInfo_.totalInterruptBlockCount);
    }
}

void GameState::HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length)
{
    if (length < sizeof(PacketBase)) {
        LOGGER.Warning("Received invalid network message size");
        return;
    }

    const auto* packet = reinterpret_cast<const PacketBase*>(message.data());

    // ��Ŷ ũ�� ����
    if (length != packet->GetSize()) 
    {
        LOGGER.Warning("Invalid packet size. Expected: {}, Actual: {}",
            packet->GetSize(), length);
        return;
    }

    switch (packet->GetType())
    {
    case PacketType::InitializeGame:
        if (const auto* initPacket = static_cast<const GameInitPacket*>(packet)) {
            HandleGameInitialize(connectionId, initPacket);
        }
        break;

    case PacketType::AddNewBlock:
        if (const auto* newBlockPacket = static_cast<const AddNewBlockPacket*>(packet)) {
            HandleAddNewBlock(connectionId, newBlockPacket);
        }
        break;

    case PacketType::UpdateBlockMove:
        if (const auto* movePacket = static_cast<const MoveBlockPacket*>(packet)) {
            HandleUpdateBlockMove(connectionId, movePacket);
        }
        break;

    case PacketType::UpdateBlockRotate:
        if (const auto* rotatePacket = static_cast<const RotateBlockPacket*>(packet)) {
            HandleBlockRotate(connectionId, rotatePacket);
        }
        break;

    case PacketType::StartGame:
        HandleStartGame();
        break;

    case PacketType::CheckBlockState:
        if (const auto* checkPacket = static_cast<const CheckBlockStatePacket*>(packet)) {
            HandleCheckBlockState(connectionId, checkPacket);
        }
        break;

    case PacketType::GameOver:
        HandleGameOver();
        break;

    default:
        LOGGER.Warning("Unknown packet type: {}", static_cast<int>(packet->GetType()));
        break;
    }
}

void GameState::HandleGameInitialize(uint8_t connectionId, const GameInitPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (!player) 
    {
        return;
    }

    // �� ����
    background_ = GAME_APP.GetMapManager().CreateMap(packet->map_id);

    if (background_ && background_->Initialize()) 
    {
        draw_objects_.insert(draw_objects_.begin(), background_.get());

        if (!next_blocks_.empty()) 
        {
            background_->SetNextBlock(std::move(next_blocks_[0]));
            background_->SetNextBlock(std::move(next_blocks_[1]));
        }
    }

    // �÷��̾� ����
    CreateGamePlayer(packet->block1, packet->block2, packet->player_id, packet->character_id);
}

void GameState::HandleAddNewBlock(uint8_t connectionId, const AddNewBlockPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (player && game_player_)
    {
        game_player_->AddNewBlock(packet->block_type);
    }
}

void GameState::HandleUpdateBlockMove(uint8_t connectionId, const MoveBlockPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (player && game_player_)
    {
        game_player_->UpdateBlockPosition(packet->move_type, packet->position);
    }
}

void GameState::HandleBlockRotate(uint8_t connectionId, const RotateBlockPacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);

    if (player && game_player_) 
    {
        game_player_->RotateBlock(packet->rotate_type, packet->is_horizontal_moving);
    }
}

void GameState::HandleStartGame() 
{
    CreateNextBlock();
}

void GameState::HandleCheckBlockState(uint8_t connectionId, const CheckBlockStatePacket* packet)
{
    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
    if (player && game_player_) 
    {
        game_player_->CheckGameBlockState();
    }
}

//void GameState::HandleComboUpdate(uint8_t connectionId, const char* payload, uint32_t size)
//{
//    const auto* packet = reinterpret_cast<const net::ComboPacket*>(
//        payload);
//
//    auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id);
//    if (player && game_player_) 
//    {
//        game_player_->UpdateCombo(packet->combo_count, packet->combo_position_x, packet->combo_position_y, packet->is_continue);
//    }
//}

bool GameState::SendChatMsg() 
{
    if (!chatbox_ || chatbox_->IsEmpty()) 
    {
        return false;
    }

    NETWORK.ChatMessage(chatbox_->GetText(TextType::UTF8));
    return true;
}

//MARK
//void GameState::SendNewPlayerInfo(const std::shared_ptr<net::Player>& newPlayer) 
//{
//    if (!newPlayer) {
//        return;
//    }
//
//    net::AddPlayerPacket packet;
//    packet.player_id = newPlayer->GetId();
//    packet.character_id = newPlayer->GetCharacterId();
//
//    auto& playerManager = GAME_APP.GetPlayerManager();
//
//    for (const auto& [_, player] : playerManager.GetPlayers()) 
//    {
//        if (player != newPlayer && player != playerManager.GetMyPlayer() && player->GetNetInfo())
//        {
//            NETWORK.SendToClient(player->GetNetInfo(), packet);
//        }
//    }
//}
//
//void GameState::SendPlayerListInfo(const std::shared_ptr<net::Player>& newPlayer)
//{
//    if (!newPlayer || !newPlayer->GetNetInfo()) 
//    {
//        return;
//    }
//
//    auto& playerManager = GAME_APP.GetPlayerManager();
//
//    for (const auto& [_, player] : playerManager.GetPlayers()) 
//    {
//        if (player != newPlayer) 
//        {
//            net::PlayerInfoPacket packet;
//            packet.player_id = player->GetId();
//            packet.character_id = player->GetCharacterId();
//
//            NETWORK.SendToClient(newPlayer->GetNetInfo(), packet);
//        }
//    }
//}

void GameState::CalculateIceBlockCount() 
{
    // �޺� ���¿� ���� ī��Ʈ ������Ʈ
    if (stateInfo_.previousPhase == GamePhase::Shattering) 
    {
        stateInfo_.defenseCount++;
    }
    else if (stateInfo_.previousPhase == GamePhase::Playing) 
    {
        stateInfo_.defenseCount = 1;
    }

    uint8_t linkBonus = 0;
    uint8_t blockCount = 0;
    uint8_t typeBonus = GetTypeBonus(matched_blocks_.size());
    short comboBonus = GetComboConstant(scoreInfo_.comboCount);

    // ��Ī�� ��� �׷캰 ���ʽ� ���
    for (const auto& group : matched_blocks_) {
        linkBonus += GetLinkBonus(group.size());
        blockCount += static_cast<uint8_t>(group.size());
    }

    // ���� ���� ���
    int currentScore = ((blockCount * 10) * (comboBonus + linkBonus + typeBonus + 1));

    // ���� ��� ī��Ʈ ���
    scoreInfo_.addInterruptBlockCount = (currentScore + scoreInfo_.restScore) / GetMargin();
    scoreInfo_.restScore = currentScore % GetMargin();
    scoreInfo_.totalScore += currentScore;

    // ���� ��� ���� ������Ʈ
    if (scoreInfo_.totalInterruptBlockCount > 0) {
        scoreInfo_.totalInterruptBlockCount -= scoreInfo_.addInterruptBlockCount;

        if (scoreInfo_.totalInterruptBlockCount < 0) {
            scoreInfo_.addInterruptBlockCount = std::abs(scoreInfo_.totalInterruptBlockCount);
            stateInfo_.hasIceBlock = false;
            scoreInfo_.totalInterruptBlockCount = 0;

            if (NETWORK.IsServer()) {
                scoreInfo_.totalEnemyInterruptBlockCount += scoreInfo_.addInterruptBlockCount;
            }
        }
        else {
            stateInfo_.hasIceBlock = true;
        }
    }
}

void GameState::ProcessMatchedBlocks() {
    if (matched_blocks_.empty()) {
        return;
    }

    bool allProcessed = true;
    for (const auto& group : matched_blocks_) {
        for (auto* block : group) {
            if (block->GetState() != BlockState::PlayOut) {
                allProcessed = false;
                break;
            }
        }
        if (!allProcessed) break;
    }

    if (allProcessed) {
        // ���� ��� �� ����Ʈ ����
        for (const auto& group : matched_blocks_) {
            if (!group.empty()) {
                CreateBullet(group[0]);  // ��ǥ ������� �Ѿ� ����
            }

            // ��� ���� ó��
            for (auto* block : group) {
                auto posX = block->GetPosIdx_X();
                auto posY = block->GetPosIdx_Y();

                // ��ƼŬ ����
                SDL_FPoint pos{ block->GetX(), block->GetY() };
                // TODO: ��ƼŬ �Ŵ��� ���� �ʿ�
                //PARTICLE_MANAGER.CreateBlockDestroyEffect(pos, block->GetBlockType());

                // ���� ���忡�� ����
                board_blocks_[posY][posX] = nullptr;
                auto it = std::find(block_list_.begin(), block_list_.end(),
                    std::shared_ptr<Block>(block));
                if (it != block_list_.end()) {
                    block_list_.erase(it);
                }
            }
        }

        matched_blocks_.clear();
        UpdateBlockPositions();
    }
}

void GameState::HandlePhaseTransition(GamePhase newPhase) {
    if (stateInfo_.currentPhase == newPhase) {
        return;
    }

    stateInfo_.previousPhase = stateInfo_.currentPhase;
    stateInfo_.currentPhase = newPhase;

    switch (newPhase) {
    case GamePhase::Shattering:
        ProcessBlockMatching();
        break;

    case GamePhase::IceBlocking:
        GenerateIceBlocks();
        break;

    case GamePhase::GameOver:
        HandleGameOver();
        break;
    }
}

bool GameState::CheckGameBlockState() 
{
    if (gameboard_) {
        gameboard_->SetRenderTargetMark(false);
    }

    // ���� ���� üũ
    if (stateInfo_.shouldQuit) {
        if (NETWORK.IsRunning()) {
            NETWORK.StopComboAttack();
        }
        stateInfo_.currentPhase = GamePhase::Standing;
        return true;
    }

    // �ּ� ��� �� üũ
    if (block_list_.size() < Constants::Game::MIN_MATCH_COUNT) {
        ResetComboState();
        stateInfo_.currentPhase = GamePhase::Playing;
        stateInfo_.previousPhase = GamePhase::Playing;
        return false;
    }

    // ��Ī ��� ã��
    if (FindMatchedBlocks(matched_blocks_)) {
        stateInfo_.previousPhase = stateInfo_.currentPhase;
        HandlePhaseTransition(GamePhase::Shattering);

        // ���� ���
        CalculateIceBlockCount();

        // ���� ��� ó��
        if (!stateInfo_.isDefending) {
            CollectRemoveIceBlocks();
        }

        return true;
    }
    else 
    {
        if (stateInfo_.defenseCount >= 1 && !stateInfo_.isComboAttack) 
        {
            stateInfo_.isDefending = false;
            stateInfo_.defenseCount = 0;
        }

        if (scoreInfo_.comboCount > 0) 
        {
            scoreInfo_.comboCount = 0;
            if (NETWORK.IsRunning()) 
            {
                NETWORK.StopComboAttack();
            }
        }

        if (scoreInfo_.restScore > 0) {
            scoreInfo_.restScore = 0;
        }
    }

    // ���� ���� üũ
    if (IsGameOver()) 
    {
        return true;
    }

    // ���� �ܰ� ó��
    if (stateInfo_.shouldQuit) 
    {
        stateInfo_.currentPhase = GamePhase::Standing;
    }
    else 
    {
        stateInfo_.currentPhase = GamePhase::Playing;
        stateInfo_.previousPhase = GamePhase::Playing;

        if (scoreInfo_.totalInterruptBlockCount > 0 && !stateInfo_.isComboAttack && !stateInfo_.isDefending)
        {
            GenerateIceBlocks();
        }
        else {
            CreateNextBlock();
        }
    }

    return false;
}

void GameState::UpdateBlockPositions() {
    // �� ������ ó��
    for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++) {
        int emptySpaces = 0;

        // �Ʒ��������� ���� �˻�
        for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; y++) {
            if (board_blocks_[y][x] == nullptr) {
                emptySpaces++;
            }
            else if (emptySpaces > 0) {
                // �� ������ ������ ����� �Ʒ��� �̵�
                Block* block = board_blocks_[y][x];
                board_blocks_[y][x] = nullptr;
                board_blocks_[y - emptySpaces][x] = block;

                // ����� �ε����� ���� ������Ʈ
                block->SetPosIdx(x, y - emptySpaces);
                block->SetState(BlockState::DownMoving);
            }
        }
    }
}

void GameState::CheckGameOverCondition() {
    // ��� �߾� 4ĭ üũ
    bool isGameOver = false;
    for (int x = 2; x <= 3; x++) {
        if (board_blocks_[Constants::Board::BOARD_Y_COUNT - 1][x] != nullptr ||
            board_blocks_[Constants::Board::BOARD_Y_COUNT - 2][x] != nullptr)
        {
            isGameOver = true;
            break;
        }
    }

    if (isGameOver) {
        HandleGameOver();
    }
}

void GameState::HandleGameOver() 
{
    if (gameboard_) 
    {
        if (NETWORK.IsRunning()) 
        {
            NETWORK.LoseGame();
        }
        gameboard_->SetState(BoardState::Lose);
    }

    if (result_view_) 
    {
        result_view_->UpdateResult(Constants::Board::POSITION_X + 20, 100, false);
    }

    if (game_player_) 
    {
        game_player_->LoseGame(true);
        game_player_->SetGameQuit();
    }

    if (exit_button_) 
    {
        exit_button_->SetVisible(true);
    }

    if (NETWORK.IsServer() && restart_button_) 
    {
        restart_button_->SetVisible(true);
    }

    HandlePhaseTransition(GamePhase::Standing);
}


void GameState::DestroyNextBlock() 
{
    if (next_blocks_.size() == 3) 
    {
        auto nextBlock = std::move(next_blocks_.front());
        next_blocks_.pop_front();

        if (nextBlock && control_block_) 
        {
            control_block_->ResetBlock();
            control_block_->SetGroupBlock(nextBlock.get());
            control_block_->SetState(BlockState::Playing);
            control_block_->SetEnableRotState(RotateState::Default, false, false, false);

            if (gameboard_) 
            {
                gameboard_->CreateNewBlockInGame(std::move(control_block_));
            }

            UpdateTargetPosIdx();
        }
    }
}

void GameState::ProcessBlockMatching() 
{
    matched_blocks_.clear();
    std::vector<Block*> currentGroup;
    bool hasMatches = false;

    for (short y = 0; y < Constants::Board::BOARD_Y_COUNT; y++) 
    {
        for (short x = 0; x < Constants::Board::BOARD_X_COUNT; x++) 
        {
            Block* block = board_blocks_[y][x];

            if (!block || block->GetBlockType() == BlockType::Ice || block->IsRecursionCheck()) 
            {
                continue;
            }

            block->SetRecursionCheck(true);
            currentGroup.clear();

            // ����� ��� �˻�
            if (RecursionCheckBlock(x, y, Constants::Direction::None, currentGroup) >=
                Constants::Game::MIN_MATCH_COUNT - 1)
            {
                currentGroup.push_back(block);
                matched_blocks_.push_back(currentGroup);
                hasMatches = true;
            }
            else 
            {
                block->SetRecursionCheck(false);
                for (auto* matchedBlock : currentGroup) 
                {
                    matchedBlock->SetRecursionCheck(false);
                }
            }
        }
    }

    if (hasMatches) 
    {
        // ��Ī�� ��ϵ� �ı� ���·� ����
        for (const auto& group : matched_blocks_) 
        {
            for (auto* block : group) 
            {
                block->SetState(BlockState::Destroying);
            }
        }

        UpdateBlockLinks();
    }
}

//void GameState::UpdateBlockLinks()
//{
//    for (auto& block : block_list_)
//    {
//        if (block && block->GetBlockType() != BlockType::Ice)
//        {
//            block->SetLinkState(LinkState::Max);
//        }
//    }
//
//    // �� ����� ��ũ ���� ������Ʈ
//    for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; y++)
//    {
//        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++)
//        {
//            Block* block = board_blocks_[y][x];
//            if (!block || block->GetBlockType() == BlockType::Ice)
//            {
//                continue;
//            }
//
//            BlockType blockType = block->GetBlockType();
//            uint8_t linkState = 0;
//
//            // �� ���� �˻�
//            const std::array<std::pair<Constants::Direction, std::pair<int, int>>, 4> directions = { {
//                {Constants::Direction::Left,   {x - 1, y}},
//                {Constants::Direction::Right,  {x + 1, y}},
//                {Constants::Direction::Top,    {x, y + 1}},
//                {Constants::Direction::Bottom, {x, y - 1}}
//            } };
//
//            for (const auto& [dir, pos] : directions) {
//                const auto [checkX, checkY] = pos;
//
//                if (checkX >= 0 && checkX < Constants::Board::BOARD_X_COUNT &&
//                    checkY >= 0 && checkY < Constants::Board::BOARD_Y_COUNT)
//                {
//                    Block* neighborBlock = board_blocks_[checkY][checkX];
//                    if (neighborBlock &&
//                        neighborBlock->GetBlockType() == blockType &&
//                        neighborBlock->GetState() == BlockState::Stationary)
//                    {
//                        // ���⿡ ���� ��ũ ���� ������Ʈ
//                        switch (dir) {
//                        case Constants::Direction::Left:
//                            linkState |= static_cast<uint8_t>(LinkState::Left);
//                            neighborBlock->SetLinkState(static_cast<LinkState>(
//                                static_cast<uint8_t>(neighborBlock->GetLinkState()) |
//                                static_cast<uint8_t>(LinkState::Right)));
//                            break;
//                        case Constants::Direction::Right:
//                            linkState |= static_cast<uint8_t>(LinkState::Right);
//                            neighborBlock->SetLinkState(static_cast<LinkState>(
//                                static_cast<uint8_t>(neighborBlock->GetLinkState()) |
//                                static_cast<uint8_t>(LinkState::Left)));
//                            break;
//                        case Constants::Direction::Top:
//                            linkState |= static_cast<uint8_t>(LinkState::Top);
//                            neighborBlock->SetLinkState(static_cast<LinkState>(
//                                static_cast<uint8_t>(neighborBlock->GetLinkState()) |
//                                static_cast<uint8_t>(LinkState::Bottom)));
//                            break;
//                        case Constants::Direction::Bottom:
//                            linkState |= static_cast<uint8_t>(LinkState::Bottom);
//                            neighborBlock->SetLinkState(static_cast<LinkState>(
//                                static_cast<uint8_t>(neighborBlock->GetLinkState()) |
//                                static_cast<uint8_t>(LinkState::Top)));
//                            break;
//                        }
//                    }
//                }
//            }
//
//            block->SetLinkState(static_cast<LinkState>(linkState));
//        }
//    }
//}

void GameState::HandleSystemEvent(const SDL_Event & event)
{
    //TODO
    /*if (event.type != SDL_SYSWMEVENT)
    {
        return;
    }

#ifdef _WIN32
    const auto& msg = event.syswm.msg->msg.win;
    switch (msg.msg)
    {
    case SDL_USEREVENT_SOCK:
    {
        if (NETWORK.IsClient())
        {
            NETWORK.ProcessRecv(msg.wParam, msg.lParam);
        }
    }
    break;

    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        GAME_APP.Quit();
        break;
    }
#endif*/
}


void GameState::HandleNetworkCommand(const std::string_view & command) 
{
    if (command.empty()) 
    {
        return;
    }

    try 
    {
        struct NetworkCommand 
        {
            std::string_view action;
            std::string_view data;
        };

        auto parseCommand = [](const std::string_view& cmd) -> NetworkCommand {
            auto pos = cmd.find(':');
            if (pos == std::string_view::npos) {
                return { cmd, {} };
            }
            return {
                cmd.substr(0, pos),
                cmd.substr(pos + 1)
            };
            };

        auto cmd = parseCommand(command);

        if (cmd.action == "MOVE") 
        {
            if (control_block_ && control_block_->GetState() == BlockState::Playing)
            {
                if (cmd.data == "LEFT") 
                {
                    control_block_->MoveLeft();
                }
                else if (cmd.data == "RIGHT") 
                {
                    control_block_->MoveRight();
                }
                else if (cmd.data == "DOWN") 
                {
                    control_block_->ForceAddVelocityY(10.0f);
                }
            }
        }
        else if (cmd.action == "ROTATE") 
        {
            if (control_block_ && control_block_->GetState() == BlockState::Playing)
            {
                control_block_->Rotate();
            }
        }
        else if (cmd.action == "STATE") 
        {
            if (cmd.data == "CHECK") 
            {
                CheckGameBlockState();
            }
            else if (cmd.data == "UPDATE") 
            {
                UpdateBlockPositions();
            }
        }
        else if (cmd.action == "COMBO") 
        {            
            auto comboData = parseCommand(cmd.data);
            if (comboData.action == "START") 
            {
                stateInfo_.isComboAttack = true;
            }
            else if (comboData.action == "END") 
            {
                stateInfo_.isComboAttack = false;
                NETWORK.StopComboAttack();
            }
        }
        else if (cmd.action == "INTERRUPT") 
        {
            auto interruptData = parseCommand(cmd.data);
            if (interruptData.action == "ADD") 
            {
                try 
                {
                    auto count = std::stoi(std::string(interruptData.data));
                    scoreInfo_.totalInterruptBlockCount += count;
                    UpdateInterruptBlockView();
                }
                catch (const std::exception& e) 
                {
                    LOGGER.Error("Failed to parse interrupt count: {}", e.what());
                }
            }
        }
        else if (cmd.action == "GAME") 
        {
            if (cmd.data == "START") 
            {
                CreateNextBlock();
            }
            else if (cmd.data == "OVER") 
            {
                HandleGameOver();
            }
            else if (cmd.data == "RESTART") 
            {
                GameRestart();
            }
        }
    }
    catch (const std::exception& e) 
    {
        LOGGER.Error("Failed to process network command: {}", e.what());
    }
}


bool GameState::IsGameOver() 
{
    // ��� �߾� 4ĭ�� ä���� �ִ��� Ȯ��
    if (board_blocks_[Constants::Board::BOARD_Y_COUNT - 1][2] != nullptr ||
        board_blocks_[Constants::Board::BOARD_Y_COUNT - 1][3] != nullptr ||
        board_blocks_[Constants::Board::BOARD_Y_COUNT - 2][2] != nullptr ||
        board_blocks_[Constants::Board::BOARD_Y_COUNT - 2][3] != nullptr)
    {
        // ���� ���� ó��
        if (gameboard_) 
        {
            if (NETWORK.IsRunning()) 
            {
                NETWORK.LoseGame();
            }
            gameboard_->SetState(BoardState::Lose);
        }

        if (result_view_) 
        {
            result_view_->UpdateResult(Constants::Board::POSITION_X + 20, 100, false);
        }

        if (game_player_) 
        {
            game_player_->LoseGame(true);
            game_player_->SetGameQuit();
        }

        if (exit_button_) 
        {
            exit_button_->SetVisible(true);
        }

        if (NETWORK.IsServer() && restart_button_) 
        {
            restart_button_->SetVisible(true);
        }

        stateInfo_.currentPhase = GamePhase::Standing;
        return true;
    }

    return false;
}

void GameState::UpdateBlockLinks() 
{
    // ��� Ȱ�� ��ϵ��� ���� ���� ������Ʈ
    for (auto& block : block_list_) 
    {
        if (!block || block->GetBlockType() == BlockType::Ice) 
        {
            continue;
        }

        int x = block->GetPosIdx_X();
        int y = block->GetPosIdx_Y();
        BlockType blockType = block->GetBlockType();
        uint8_t linkState = 0;

        // �ֺ� 4���� �˻縦 ���� ���� �迭
        const std::array<std::pair<Constants::Direction, std::pair<int, int>>, 4> directions = { {
            {Constants::Direction::Left,   {x - 1, y}},    // ����
            {Constants::Direction::Right,  {x + 1, y}},    // ������
            {Constants::Direction::Top,    {x, y + 1}},    // ��
            {Constants::Direction::Bottom, {x, y - 1}}     // �Ʒ�
        } };

        // �� ���⺰ ���� �˻�
        for (const auto& [dir, pos] : directions) 
        {
            const auto [checkX, checkY] = pos;

            // ���� ���� üũ
            if (checkX >= 0 && checkX < Constants::Board::BOARD_X_COUNT &&
                checkY >= 0 && checkY < Constants::Board::BOARD_Y_COUNT)
            {
                Block* neighborBlock = board_blocks_[checkY][checkX];
                if (neighborBlock &&
                    neighborBlock->GetBlockType() == blockType &&
                    neighborBlock->GetState() == BlockState::Stationary)
                {
                    // ���⿡ ���� ��ũ ���� ������Ʈ
                    switch (dir) 
                    {
                    case Constants::Direction::Left:
                        linkState |= static_cast<uint8_t>(LinkState::Left);
                        neighborBlock->SetLinkState(static_cast<LinkState>(static_cast<uint8_t>(neighborBlock->GetLinkState()) |static_cast<uint8_t>(LinkState::Right)));
                        break;

                    case Constants::Direction::Right:
                        linkState |= static_cast<uint8_t>(LinkState::Right);
                        neighborBlock->SetLinkState(static_cast<LinkState>(static_cast<uint8_t>(neighborBlock->GetLinkState()) | static_cast<uint8_t>(LinkState::Left)));
                        break;

                    case Constants::Direction::Top:
                        linkState |= static_cast<uint8_t>(LinkState::Top);
                        neighborBlock->SetLinkState(static_cast<LinkState>(static_cast<uint8_t>(neighborBlock->GetLinkState()) | static_cast<uint8_t>(LinkState::Bottom)));
                        break;

                    case Constants::Direction::Bottom:
                        linkState |= static_cast<uint8_t>(LinkState::Bottom);
                        neighborBlock->SetLinkState(static_cast<LinkState>(static_cast<uint8_t>(neighborBlock->GetLinkState()) | static_cast<uint8_t>(LinkState::Top)));
                        break;
                    }
                }
            }
        }

        // ���� ��ũ ���� ����
        block->SetLinkState(static_cast<LinkState>(linkState));
    }
}

void GameState::DefenseInterruptBlockCount(short cnt, float x, float y, unsigned char type)
{
    if (NETWORK.IsServer())
    {
        scoreInfo_.totalEnemyInterruptBlockCount -= cnt;	// Ŭ���� ���غ�� ���� ����

        if (game_player_)	// Ŭ��  ���غ�� UI ���� �� �߻�ü ����
        {
            game_player_->DefenseInterruptBlockCount(cnt, x, y, type);
        }
    }
    else
    {


    }
}

void GameState::AddInterruptBlockCount(short cnt, float x, float y, unsigned char type)
{
    if (NETWORK.IsServer())
    {
        stateInfo_.isComboAttack = true;

        scoreInfo_.totalInterruptBlockCount += cnt;

        if (interrupt_view_)
        {
            interrupt_view_->UpdateInterruptBlock(scoreInfo_.totalInterruptBlockCount);
        }

        if (game_player_)
        {
            game_player_->AttackInterruptBlock(x, y, type);

            scoreInfo_.totalEnemyInterruptBlockCount = 0;

            game_player_->UpdateInterruptBlock(0);
        }
    }   
}

void GameState::Release() 
{
    Leave();

    restart_button_.reset();
    exit_button_.reset();
    chatbox_.reset();

    interrupt_view_.reset();
    gameboard_.reset();
    combo_view_.reset();
    result_view_.reset();
    control_block_.reset();

    GAME_APP.GetMapManager().Release();

    SDL_StopTextInput(GAME_APP.GetWindow());
}

void GameState::CreateBlocksFromFile()
{
    std::ifstream file("puyo.txt");
    if (!file)
    {
        throw std::runtime_error("Failed to open puyo.txt");
    }

    auto texture = ImageTexture::Create("PUYO/puyo_beta.png");
    if (!texture)
    {
        throw std::runtime_error("Failed to load block texture");
    }

    std::string line;
    for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; y++)
    {
        std::getline(file, line);
        if (!file)
        {
            throw std::runtime_error("Failed to read line from file");
        }

        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++)
        {
            if (x >= line.length())
            {
                break;
            }

            int type = line[x] - '0';
            if (type <= 0)
            {
                continue;
            }

            std::shared_ptr<Block> block = (type == static_cast<int>(BlockType::Ice)) ?
                std::make_shared<IceBlock>() : std::make_shared<Block>();

            float x_pos = x * Constants::Block::SIZE + Constants::Board::WIDTH_MARGIN;
            float y_pos = (y - 1) * Constants::Block::SIZE;

            block->SetBlockType(static_cast<BlockType>(type));
            block->SetPosIdx(x, Constants::Board::BOARD_Y_COUNT - 1 - y);
            block->SetPosition(x_pos, y_pos);
            block->SetSize(Constants::Block::SIZE, Constants::Block::SIZE);
            block->SetState(BlockState::Stationary);
            block->SetBlockTex(texture);

            board_blocks_[Constants::Board::BOARD_Y_COUNT - 1 - y][x] = block.get();
            block_list_.push_back(block);

            UpdateLinkState(block.get());
        }
    }

    block_list_.sort([](const auto& a, const auto& b) { return *a < *b; });
}
