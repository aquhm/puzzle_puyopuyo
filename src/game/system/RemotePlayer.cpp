#include "RemotePlayer.hpp"

#include "../../core/manager/ResourceManager.hpp"
#include "../../core/manager/ParticleManager.hpp"
#include "../../core/GameApp.hpp"
#include "../../network/NetworkController.hpp"

#include "../block/Block.hpp"
#include "../block/IceBlock.hpp"
#include "../block/GameGroupBlock.hpp"
#include "../block/GroupBlock.hpp"

#include "../system/GameBoard.hpp"
#include "../view/InterruptBlockView.hpp"
#include "../view/ComboView.hpp"
#include "../view/ResultView.hpp"

#include "../map/GameBackground.hpp"
#include "../effect/BulletEffect.hpp"
#include "../../texture/ImageTexture.hpp"
#include "../../utils/Logger.hpp"

#include <algorithm>
#include <random>

RemotePlayer::RemotePlayer() : BasePlayer()
{
}

RemotePlayer::~RemotePlayer()
{
    Release();
}

bool RemotePlayer::Initialize(const std::span<const uint8_t>& blockType1, const std::span<const uint8_t>& blockType2,
    uint8_t playerIdx, uint16_t characterIdx, const std::shared_ptr<GameBackground>& background)
{
    Reset();

    try 
    {
        player_id_ = playerIdx;
        character_id_ = characterIdx;
        background_ = background;

        InitializeNextBlocks(blockType1, blockType2);

        if (!InitializeGameBoard(Constants::Board::PLAYER_POSITION_X, Constants::Board::POSITION_Y))
        {
            LOGGER.Error("Failed to initialize remote player game board");
            return false;
        }

        if (!InitializeControlBlock())
        {
            LOGGER.Error("Failed to initialize remote player control block");
            return false;
        }

        InitializeViews();
        if (interrupt_view_) 
        {
            interrupt_view_->SetPosition(Constants::Board::PLAYER_POSITION_X, 0);
        }

        state_info_.currentPhase = GamePhase::Playing;
        state_info_.previousPhase = state_info_.currentPhase;
		state_info_.playTime = 0.0f;

        NotifyEvent(std::make_shared<GameRestartEvent>(player_id_));

        return true;
    }
    catch (const std::exception& e) {
        LOGGER.Error("Error restarting remote player: {}", e.what());
        return false;
    }
}

void RemotePlayer::InitializeNextBlocks(const std::span<const uint8_t>& blockType1, const std::span<const uint8_t>& blockType2)
{
    auto next_block1 = std::make_unique<GroupBlock>();
    auto next_block2 = std::make_unique<GroupBlock>();

    if (!next_block1->Create(static_cast<BlockType>(blockType1[0]), static_cast<BlockType>(blockType1[1])) ||
        !next_block2->Create(static_cast<BlockType>(blockType2[0]), static_cast<BlockType>(blockType2[1])))
    {
        throw std::runtime_error("Failed to create next blocks");
    }

    next_block1->SetPosXY(Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_X, Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_Y);
    next_block1->SetScale(Constants::Block::SIZE, Constants::Block::SIZE);
    next_block2->SetPosition(Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_X, Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_Y);
    next_block2->SetScale(Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE, Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE);

    next_blocks_.emplace_back(std::move(next_block1));
    next_blocks_.emplace_back(std::move(next_block2));

    if (background_)
    {
        background_->SetPlayerNextBlock(next_blocks_[0]);
        background_->SetPlayerNextBlock(next_blocks_[1]);
    }
}

bool RemotePlayer::Restart(const std::span<const uint8_t>& blockType1, const std::span<const uint8_t>& blockType2)
{
    Reset();

    try
    {
        InitializeNextBlocks(blockType1, blockType2);

        if (!InitializeGameBoard(Constants::Board::PLAYER_POSITION_X, Constants::Board::POSITION_Y))
        {
            return false;
        }

        if (!InitializeControlBlock())
        {
            return false;
        }

        InitializeViews();

        state_info_.currentPhase = GamePhase::Playing;
        state_info_.previousPhase = state_info_.currentPhase;
		state_info_.playTime = 0.0f;

        return true;
    }
    catch (const std::exception& e)
    {
        LOGGER.Error("Failed to restart remote player: %s", e.what());
        return false;
    }
}

void RemotePlayer::UpdateGameState(float deltaTime)
{
    switch (state_info_.currentPhase)
    {
    case GamePhase::GameOver:
        UpdateGameOverState(deltaTime);
        break;

    case GamePhase::Playing:
        UpdatePlayingState(deltaTime);
        break;

    case GamePhase::IceBlocking:
        UpdateIceBlockDowningState();
        break;

    case GamePhase::Shattering:
        UpdateShatteringState();
        break;

    default:
        break;
    }
}

void RemotePlayer::UpdateGameOverState(float deltaTime)
{
    if (result_view_)
    {
        result_view_->Update(deltaTime);
    }
}

void RemotePlayer::UpdatePlayingState(float deltaTime)
{
    if (control_block_)
    {
        control_block_->Update(deltaTime);
    }
}

void RemotePlayer::UpdateIceBlockDowningState()
{
    if (block_list_.empty())
    {
        return;
    }

    bool all_blocks_stationary = std::all_of(block_list_.begin(), block_list_.end(),
        [](const auto& block)
        {
            return block->GetState() == BlockState::Stationary;
        }
    );

    if (all_blocks_stationary)
    {
		state_info_.currentPhase = GamePhase::Playing;
        PlayNextBlock();
    }
}

void RemotePlayer::UpdateShatteringState()
{
    if (matched_blocks_.empty())
    {
        UpdateAfterBlocksCleared();
        return;
    }

    UpdateMatchedBlocks();
}

void RemotePlayer::UpdateMatchedBlocks()
{
    SDL_FPoint pos;
    SDL_Point pos_idx;
    std::list<SDL_Point> x_index_list;

    for (auto it = matched_blocks_.begin(); it != matched_blocks_.end();)
    {
        bool all_blocks_played_out = std::all_of(it->begin(), it->end(),
            [](const auto& block)
            {
                return block->GetState() == BlockState::PlayOut;
            }
        );

        if (all_blocks_played_out)
        {
            if (!it->empty())
            {
                auto firstBlock = it->front();
                CreateBullet(firstBlock);
            }

            HandleClearedBlockGroup(it, pos, pos_idx, x_index_list);
        }
        else
        {
            ++it;
        }
    }

    if (matched_blocks_.empty())
    {
        UpdateFallingBlocks(x_index_list);
    }
}


void RemotePlayer::HandleClearedBlockGroup(std::list<BlockVector>::iterator& group_it, SDL_FPoint& pos, SDL_Point& pos_idx, std::list<SDL_Point>& x_index_list)
{
    for (Block* block : *group_it)
    {
        if (!block) continue;

        pos = block->GetPosition();
        pos_idx = { block->GetPosIdx_X(), block->GetPosIdx_Y() };

        UpdateComboDisplay(pos);
        CreateBlockClearEffect(std::shared_ptr<Block>(block, [](Block*) {}));
        RemoveBlock(block, pos_idx);

        x_index_list.push_back(pos_idx);
    }
    
    RemoveIceBlocks(x_index_list);

    group_it = matched_blocks_.erase(group_it);    
}

void RemotePlayer::UpdateComboDisplay(const SDL_FPoint& pos)
{	
    if (combo_view_ && score_info_.comboCount > 0)
    {
        combo_view_->UpdateComboCount(pos.x + Constants::Board::PLAYER_POSITION_X, pos.y, score_info_.comboCount);
    }
}

void RemotePlayer::UpdateAfterBlocksCleared()
{
    block_list_.sort([](const auto& a, const auto& b)
        {
            return *a < *b;
        });

    if (block_list_.empty())
    {
        state_info_.currentPhase = is_game_quit_ ? GamePhase::GameOver : GamePhase::Playing;
        state_info_.previousPhase = state_info_.currentPhase;
        return;
    }

    bool all_blocks_stationary = std::all_of(block_list_.begin(), block_list_.end(),
        [](const auto& block)
        {
            return block->GetState() == BlockState::Stationary;
        }
    );

    if (all_blocks_stationary)
    {
        UpdateBlockLinks();

        if (CheckGameBlockState() == false && !is_game_quit_)
        {
            state_info_.currentPhase = GamePhase::Playing;
            state_info_.previousPhase = state_info_.currentPhase;
        }
    }
}

void RemotePlayer::CreateNextBlock()
{
}

void RemotePlayer::PlayNextBlock()
{
    if (next_blocks_.size() != 3 || !control_block_)
    {
        return;
    }

    auto first_block = next_blocks_.front();
    next_blocks_.pop_front();

    if (first_block)
    {
        control_block_->ResetBlock();
        control_block_->SetGroupBlock(first_block.get());
        control_block_->SetState(BlockState::Playing);
        control_block_->SetEnableRotState(RotateState::Default, false, false);

        //LOGGER.Info("RemotePlayer.PlayNextBlock");

        if (game_board_)
        {
            game_board_->CreateNewBlockInGame(control_block_);
            game_board_->SetRenderTargetMark(false);
        }
    }
}

bool RemotePlayer::CheckGameBlockState()
{
    if (state_info_.shouldQuit)
    {
		HandlePhaseTransition(GamePhase::GameOver);
        return true;
    }

    const int block_count = static_cast<int>(block_list_.size());
    if (block_count < Constants::Game::MIN_MATCH_COUNT)
    {
        HandlePhaseTransition(GamePhase::Playing);
        return false;
    }

    int current_count = 0;
    std::vector<Block*> current_blocks;

    matched_blocks_.clear();

    if (FindMatchedBlocks(matched_blocks_))
    {
        HandlePhaseTransition(GamePhase::Shattering);
        UpdateComboState();
        CollectRemoveIceBlocks();

        // 매치된 블록들 파괴 상태로 설정
        for (const auto& group : matched_blocks_)
        {
            for (auto* block : group)
            {
                block->SetState(BlockState::Destroying);
            }
        }

        return true;
    }
    else
    {
        ResetComboState();
    }

    HandlePhaseTransition(state_info_.shouldQuit ? GamePhase::GameOver : GamePhase::Playing);

    return false;
}


void RemotePlayer::MoveBlock(uint8_t moveType, float position)
{
    if (!control_block_ || control_block_->GetState() != BlockState::Playing)
    {
        return;
    }

    switch (static_cast<Constants::Direction>(moveType))
    {
    case Constants::Direction::Left:
        control_block_->MoveLeft(false);
        break;

    case Constants::Direction::Right:
        control_block_->MoveRight(false);
        break;

    case Constants::Direction::Bottom:
        control_block_->ForceAddVelocityY(Constants::GroupBlock::ADD_VELOCITY, false);
        break;

    default:
        break;
    }
}

void RemotePlayer::RotateBlock(uint8_t rotateType, bool horizontalMoving)
{
    if (!control_block_ || control_block_->GetState() != BlockState::Playing)
    {
        return;
    }

    control_block_->SetEnableRotState(static_cast<RotateState>(rotateType), horizontalMoving, true, false);
}

void RemotePlayer::UpdateBlockPosition(float pos1, float pos2)
{
    if (!control_block_)
    {
        return;
    }

    std::array<std::shared_ptr<Block>, 2> blocks = control_block_->GetBlocks();

    if (blocks[0])
    {
        blocks[0]->SetY(pos1);
    }

    if (blocks[1])
    {
        blocks[1]->SetY(pos2);
    }
}

void RemotePlayer::UpdateFallingBlock(uint8_t fallingIdx, bool falling)
{
    if (control_block_ && control_block_->GetState() == BlockState::Playing)
    {
        control_block_->UpdateFallingBlock(fallingIdx, falling);
    }
}

void RemotePlayer::ChangeBlockState(uint8_t state)
{
    if (control_block_)
    {
        control_block_->SetState(static_cast<BlockState>(state));
    }
}

bool RemotePlayer::PushBlockInGame(const std::span<const float>& pos1, const std::span<const float>& pos2)
{
    if (!control_block_ || control_block_->GetState() != BlockState::Stationary)
    {
        return false;
    }

    //LOGGER.Info("RemotePlayer.PushBlockInGame pos1 : {} pos2: {} ", pos1, pos2);

    auto blocks = control_block_->GetBlocks();
    if (blocks[0] && blocks[1])
    {
        blocks[0]->SetPosition(pos1[0], pos1[1]);
        blocks[1]->SetPosition(pos2[0], pos2[1]);

        int x_idx_0 = static_cast<int>(pos1[0] / Constants::Block::SIZE);
        int x_idx_1 = static_cast<int>(pos2[0] / Constants::Block::SIZE);

        int y_idx_0 = (Constants::Board::BOARD_Y_COUNT - 2) - static_cast<int>(pos1[1] / Constants::Block::SIZE);
        int y_idx_1 = (Constants::Board::BOARD_Y_COUNT - 2) - static_cast<int>(pos2[1] / Constants::Block::SIZE);

        blocks[0]->SetPosIdx(x_idx_0, y_idx_0);
        blocks[1]->SetPosIdx(x_idx_1, y_idx_1);

        block_list_.push_back(blocks[0]);
        board_blocks_[y_idx_0][x_idx_0] = blocks[0].get();
        UpdateLinkState(blocks[0].get());

        block_list_.push_back(blocks[1]);
        board_blocks_[y_idx_1][x_idx_1] = blocks[1].get();
        UpdateLinkState(blocks[1].get());

        control_block_->ResetBlock();

        block_list_.sort([](const auto& a, const auto& b) { return *a < *b; });

        if (game_board_)
        {
            game_board_->ClearActiveGroupBlock();
        }

        if (!is_game_quit_ && !CheckGameBlockState() && state_info_.currentPhase == GamePhase::Playing)
        {
            //DestroyNextBlock();
        }

        

        return true;
    }

    return false;
}

void RemotePlayer::AddInterruptBlock(uint8_t y_row_cnt, const std::span<const uint8_t>& x_idx)
{
    state_info_.hasIceBlock = score_info_.totalInterruptBlockCount > 0;
    if (state_info_.hasIceBlock == false)
    {
        return;
    }

    auto texture = ImageTexture::Create("PUYO/puyo_beta.png");
    if (!texture)
    {
        throw std::runtime_error("Failed to load block texture for interrupt blocks");
    }

    try
    {
        if (y_row_cnt == 5)
        {
            CreateFullRowInterruptBlocks(texture);
        }
        else
        {
            CreatePartialRowInterruptBlocks(y_row_cnt, x_idx, texture);
        }        

        if (interrupt_view_)
        {
            interrupt_view_->UpdateInterruptBlock(score_info_.totalInterruptBlockCount);
        }

        state_info_.hasIceBlock = score_info_.totalInterruptBlockCount > 0;
        state_info_.currentPhase = GamePhase::IceBlocking;
    }
    catch (const std::exception& e)
    {
        LOGGER.Error("Failed to add interrupt blocks: %s", e.what());
    }
}

void RemotePlayer::AddInterruptBlockCnt(short cnt, float x, float y, unsigned char type)
{
    score_info_.totalInterruptBlockCount += cnt;
    state_info_.isComboAttack = true;
    state_info_.hasIceBlock = score_info_.totalInterruptBlockCount > 0;

    if (interrupt_view_)
    {
        interrupt_view_->UpdateInterruptBlock(score_info_.totalInterruptBlockCount);
    }

    if (game_board_ && game_board_->GetState() != BoardState::Lose)
    {
        game_board_->SetState(BoardState::Attacking);
    }
}

void RemotePlayer::CreateFullRowInterruptBlocks(std::shared_ptr<ImageTexture>& texture)
{
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++)
        {
            CreateSingleIceBlock(x, y, texture);
        }
    }
    score_info_.totalInterruptBlockCount -= 30;
}

void RemotePlayer::CreatePartialRowInterruptBlocks(uint8_t y_row_cnt, const std::span<const uint8_t>& x_idx, std::shared_ptr<ImageTexture>& texture)
{
    for (int y = 0; y < y_row_cnt; y++)
    {
        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++)
        {
            CreateSingleIceBlock(x, y, texture);
        }
    }

    for (int i = 0; i < x_idx.size(); i++)
    {
        CreateSingleIceBlock(x_idx[i], y_row_cnt, texture);
    }

    score_info_.totalInterruptBlockCount -= static_cast<uint16_t>(y_row_cnt * Constants::Board::BOARD_X_COUNT + x_idx.size());
}

void RemotePlayer::CreateSingleIceBlock(int x, int y, std::shared_ptr<ImageTexture>& texture)
{
    auto ice_block = std::make_shared<IceBlock>();
    ice_block->SetBlockType(BlockType::Ice);
    ice_block->SetLinkState(LinkState::LeftRightTopBottom);
    ice_block->SetState(BlockState::DownMoving);
    ice_block->SetBlockTex(texture);
    ice_block->SetPosIdx(x, y);

    const float x_pos = x * Constants::Block::SIZE + Constants::Board::WIDTH_MARGIN;
    const float y_pos = -Constants::Block::SIZE * (y + 1);
    ice_block->SetPosition(x_pos, y_pos);
    ice_block->SetScale(Constants::Block::SIZE, Constants::Block::SIZE);
    ice_block->SetPlayerID(player_id_);

    block_list_.push_back(ice_block);
}

void RemotePlayer::AttackInterruptBlock(float x, float y, uint8_t type)
{
    const SDL_FPoint start_pos
    {
        Constants::Board::PLAYER_POSITION_X + Constants::Board::WIDTH_MARGIN + x + Constants::Block::SIZE / 2,
        Constants::Board::PLAYER_POSITION_Y + y + Constants::Block::SIZE / 2
    };

    const SDL_FPoint end_pos
    {
        Constants::Board::POSITION_X + (Constants::Board::WIDTH / 2),
        Constants::Board::POSITION_Y
    };

    auto bullet = std::make_shared<BulletEffect>();
    if (!bullet->Initialize(start_pos, end_pos, static_cast<BlockType>(type)))
    {
        return;
    }

    bullet->SetAttacking(true);
    bullet_list_.push_back(bullet);

    if (game_board_ && game_board_->GetState() != BoardState::Lose)
    {
        game_board_->SetState(BoardState::Attacking);
    }
}

void RemotePlayer::DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type)
{
    score_info_.totalInterruptBlockCount = std::max<uint16_t>(0, score_info_.totalInterruptBlockCount - count);
    state_info_.hasIceBlock = score_info_.totalInterruptBlockCount > 0;    

    if (interrupt_view_)
    {
        interrupt_view_->UpdateInterruptBlock(score_info_.totalInterruptBlockCount);
    }

    const SDL_FPoint start_pos
    {
        Constants::Board::PLAYER_POSITION_X + Constants::Board::WIDTH_MARGIN + x + Constants::Block::SIZE / 2,
        Constants::Board::PLAYER_POSITION_Y + y + Constants::Block::SIZE / 2
    };

    const SDL_FPoint end_pos
    {
        Constants::Board::PLAYER_POSITION_X + (Constants::Board::WIDTH / 2),
        Constants::Board::PLAYER_POSITION_Y
    };

    auto bullet = std::make_shared<BulletEffect>();
    if (!bullet->Initialize(start_pos, end_pos, static_cast<BlockType>(type)))
    {
        return;
    }

    bullet->SetAttacking(false);
    bullet_list_.push_back(bullet);

    if (game_board_ && game_board_->GetState() != BoardState::Lose)
    {
        game_board_->SetState(BoardState::Attacking);
    }
}

void RemotePlayer::AddNewBlock(const std::span<const uint8_t, 2>& block_type)
{
    auto next_block = std::make_shared<GroupBlock>();
    if (!next_block->Create(static_cast<BlockType>(block_type[0]), static_cast<BlockType>(block_type[1])))
    {
        throw std::runtime_error("Failed to create next block");
    }

    next_block->SetPosXY(Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_X, 100);
    next_block->SetScale(Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE, Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE);

    if (background_)
    {
        next_blocks_.push_back(next_block);
        background_->SetPlayerNextBlock(next_block);
    }
}

void RemotePlayer::Release()
{
    matched_blocks_.clear();

    ReleaseContainer(next_blocks_);

    BasePlayer::Release();
}

void RemotePlayer::Reset()
{
    matched_blocks_.clear();

    ReleaseContainer(next_blocks_);

    BasePlayer::Reset();
}

void RemotePlayer::CreateBullet(Block* block)
{
    if (!block)
    {
        LOGGER.Error("CreateBullet: block is null");
        return;
    }

    SDL_FPoint startPos
    {
        Constants::Board::PLAYER_POSITION_X + Constants::Board::WIDTH_MARGIN + block->GetX() + Constants::Block::SIZE / 2,
        Constants::Board::POSITION_Y + block->GetY() + Constants::Block::SIZE / 2
    };

    SDL_FPoint endPos;
    if (state_info_.hasIceBlock)
    {
        endPos =
        {
            GAME_APP.GetWindowWidth() - (Constants::Board::POSITION_X + (Constants::Board::WIDTH / 2)),
            Constants::Board::POSITION_Y
        };
    }
    else
    {
        endPos =
        {
            Constants::Board::POSITION_X + (Constants::Board::WIDTH / 2),
            Constants::Board::POSITION_Y
        };
    }

    auto bullet = std::make_shared<BulletEffect>();
    if (!bullet->Initialize(startPos, endPos, block->GetBlockType()))
    {
        LOGGER.Error("Failed to create bullet effect");
        return;
    }

    bullet->SetAttacking(!state_info_.hasIceBlock);
    bullet_list_.push_back(bullet);

    if (game_board_ && game_board_->GetState() != BoardState::Lose)
    {
        game_board_->SetState(BoardState::Attacking);
    }
}