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

RemotePlayer::RemotePlayer() : BasePlayer(), has_ice_block_(false)
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

        // 블록 초기화
        InitializeNextBlocks(blockType1, blockType2);

        // 게임 보드 초기화
        if (!InitializeGameBoard(Constants::Board::PLAYER_POSITION_X, Constants::Board::POSITION_Y))
        {
            LOGGER.Error("Failed to initialize remote player game board");
            return false;
        }

        // 컨트롤 블록 초기화
        if (!InitializeControlBlock())
        {
            LOGGER.Error("Failed to initialize remote player control block");
            return false;
        }

        // 뷰 초기화
        InitializeViews();
        if (interrupt_view_) 
        {
            interrupt_view_->SetPosition(Constants::Board::PLAYER_POSITION_X, 0);
        }

        // 게임 상태 초기화
        state_info_.currentPhase = GamePhase::Playing;
        state_info_.previousPhase = state_info_.currentPhase;
        game_state_ = GamePhase::Playing;
        prev_game_state_ = game_state_;
        play_time_ = 0.0f;
        total_score_ = 0;
        combo_count_ = 0;
        rest_score_ = 0;
        total_interrupt_block_count_ = 0;

        // 이벤트 발송 - 재시작 이벤트
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

    new_blocks_.emplace_back(std::move(next_block1));
    new_blocks_.emplace_back(std::move(next_block2));

    if (background_)
    {
        background_->SetPlayerNextBlock(new_blocks_[0]);
        background_->SetPlayerNextBlock(new_blocks_[1]);
    }
}

bool RemotePlayer::Restart(const std::span<const uint8_t>& blockType1, const std::span<const uint8_t>& blockType2)
{
    Reset();

    try
    {
        // 초기화
        InitializeNextBlocks(blockType1, blockType2);

        // 게임 보드 초기화
        if (!InitializeGameBoard(Constants::Board::PLAYER_POSITION_X, Constants::Board::POSITION_Y))
        {
            return false;
        }

        if (!InitializeControlBlock())
        {
            return false;
        }

        InitializeViews();

        // 게임 상태 초기화
        state_info_.currentPhase = GamePhase::Playing;
        state_info_.previousPhase = state_info_.currentPhase;
        game_state_ = GamePhase::Playing;        
        prev_game_state_ = game_state_;
        play_time_ = 0.0f;
        total_score_ = 0;
        combo_count_ = 0;
        rest_score_ = 0;
        total_interrupt_block_count_ = 0;

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
    switch (game_state_)
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
        game_state_ = GamePhase::Playing;
		state_info_.currentPhase = GamePhase::Playing;
        PlayNextBlock();
    }
}

void RemotePlayer::UpdateShatteringState()
{
    if (equal_block_list_.empty())
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

    for (auto it = equal_block_list_.begin(); it != equal_block_list_.end();)
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

    if (equal_block_list_.empty())
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

        CreateBlockClearEffect(std::shared_ptr<Block>(block, [](Block*) {}));
        RemoveBlock(block, pos_idx);

        x_index_list.push_back(pos_idx);
    }

    UpdateComboDisplay(pos);
    RemoveIceBlocks(x_index_list);

    // 그룹 제거
    group_it = equal_block_list_.erase(group_it);    
}

void RemotePlayer::UpdateComboDisplay(const SDL_FPoint& pos)
{
    if (combo_view_ && combo_count_ > 0)
    {
        combo_view_->UpdateComboCount(pos.x + Constants::Board::PLAYER_POSITION_X, pos.y, combo_count_);
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
        game_state_ = is_game_quit_ ? GamePhase::GameOver : GamePhase::Playing;
        prev_game_state_ = game_state_;

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
            game_state_ = GamePhase::Playing;
            prev_game_state_ = game_state_;

            state_info_.currentPhase = GamePhase::Playing;
            state_info_.previousPhase = state_info_.currentPhase;
        }
    }
}

void RemotePlayer::CreateNextBlock()
{
    // 원격 플레이어는 네트워크로부터 명령을 받아 블록을 생성하므로 구현 없음
}

void RemotePlayer::PlayNextBlock()
{
    if (new_blocks_.size() != 3 || !control_block_)
    {
        return;
    }

    auto first_block = new_blocks_.front();
    new_blocks_.pop_front();

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
    if (is_game_quit_)
    {
        game_state_ = GamePhase::GameOver;
        state_info_.currentPhase = GamePhase::Playing;

        return true;
    }

    const int block_count = static_cast<int>(block_list_.size());
    if (block_count < Constants::Game::MIN_MATCH_COUNT)
    {
        game_state_ = GamePhase::Playing;
        prev_game_state_ = GamePhase::Playing;

        state_info_.currentPhase = GamePhase::Playing;
        state_info_.previousPhase = state_info_.currentPhase;
        return false;
    }

    int current_count = 0;
    std::vector<Block*> current_blocks;

    // 게임 보드 순회하며 연결된 블록 체크
    for (int16_t y = 0; y < Constants::Board::BOARD_Y_COUNT; ++y)
    {
        for (int16_t x = 0; x < Constants::Board::BOARD_X_COUNT; ++x)
        {
            if (current_count == block_count)
            {
                break;
            }

            Block* current_block = board_blocks_[y][x];
            if (!current_block)
            {
                continue;
            }

            ++current_count;

            if (current_block->GetBlockType() == BlockType::Ice ||
                current_block->IsRecursionCheck())
            {
                continue;
            }

            current_block->SetRecursionCheck(true);
            current_blocks.clear();

            if (RecursionCheckBlock(x, y, -1, current_blocks) >= 3)
            {
                current_blocks.push_back(current_block);
                equal_block_list_.push_back(current_blocks);
            }
            else
            {
                current_block->SetRecursionCheck(false);
                for (auto* block : current_blocks)
                {
                    block->SetRecursionCheck(false);
                }
            }
        }
    }

    // 매치된 블록이 있는 경우
    if (equal_block_list_.empty() == false)
    {
        HandleMatchedBlocks();
        return true;
    }

    // 매치된 블록이 없는 경우
    ResetMatchState();
    return false;
}

void RemotePlayer::HandleMatchedBlocks()
{
    prev_game_state_ = game_state_;
    game_state_ = GamePhase::Shattering;    

    state_info_.previousPhase = state_info_.currentPhase;
    state_info_.currentPhase = GamePhase::Shattering;    

    if (prev_game_state_ == GamePhase::Shattering)
    {
        ++combo_count_;
    }
    else if (prev_game_state_ == GamePhase::Playing)
    {
        combo_count_ = 1;
    }

    // 매치된 블록들을 파괴 상태로 설정
    for (auto& block_group : equal_block_list_)
    {
        for (auto& block : block_group)
        {
            block->SetState(BlockState::Destroying);
        }
    }

    CollectRemoveIceBlocks();
}

void RemotePlayer::ResetMatchState()
{
    if (combo_count_ > 0)
    {
        combo_count_ = 0;
    }

    if (rest_score_ > 0)
    {
        rest_score_ = 0;
    }

    game_state_ = is_game_quit_ ? GamePhase::GameOver : GamePhase::Playing;
    prev_game_state_ = game_state_;
    
    state_info_.currentPhase = is_game_quit_ ? GamePhase::GameOver : GamePhase::Playing;
    state_info_.previousPhase = state_info_.currentPhase;
}

int16_t RemotePlayer::RecursionCheckBlock(int16_t x, int16_t y, int16_t direction, std::vector<Block*>& block_list)
{
    if (!board_blocks_[y][x])
    {
        return 0;
    }

    Block* current_block = board_blocks_[y][x];
    const auto block_type = current_block->GetBlockType();
    int16_t equal_count = 0;

    // 방향과 체크 방향 설정
    const std::array<std::pair<std::pair<int16_t, int16_t>, Constants::Direction>, 4> check_dirs = { {
        {{-1, 0}, Constants::Direction::Left},
        {{1, 0}, Constants::Direction::Right},
        {{0, 1}, Constants::Direction::Top},
        {{0, -1}, Constants::Direction::Bottom}
    } };

    for (const auto& [dir, check_direction] : check_dirs)
    {
        if (check_direction == static_cast<Constants::Direction>(direction))
        {
            continue;
        }

        const int16_t check_x = x + dir.first;
        const int16_t check_y = y + dir.second;

        // 경계 체크
        if (check_x < 0 || check_x >= Constants::Board::BOARD_X_COUNT || check_y < 0 || check_y >= Constants::Board::BOARD_Y_COUNT)
        {
            continue;
        }

        Block* check_block = board_blocks_[check_y][check_x];
        if (!check_block || check_block->IsRecursionCheck() || check_block->GetState() != BlockState::Stationary)
        {
            continue;
        }

        if (block_type == check_block->GetBlockType())
        {
            check_block->SetRecursionCheck(true);
            block_list.push_back(check_block);

            equal_count += 1 + RecursionCheckBlock(check_x, check_y,
                static_cast<int16_t>(static_cast<Constants::Direction>((static_cast<int>(check_direction) + 2) % 4)),
                block_list);
        }
    }

    return equal_count;
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

        if (!is_game_quit_ && !CheckGameBlockState() && game_state_ == GamePhase::Playing)
        {
            //DestroyNextBlock();
        }

        

        return true;
    }

    return false;
}

void RemotePlayer::AddInterruptBlock(uint8_t y_row_cnt, const std::span<const uint8_t>& x_idx)
{
    if (total_interrupt_block_count_ <= 0)
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
            interrupt_view_->UpdateInterruptBlock(total_interrupt_block_count_);
        }

        game_state_ = GamePhase::IceBlocking;

        state_info_.currentPhase = GamePhase::IceBlocking;
    }
    catch (const std::exception& e)
    {
        LOGGER.Error("Failed to add interrupt blocks: %s", e.what());
    }
}

void RemotePlayer::AddInterruptBlockCnt(short cnt, float x, float y, unsigned char type)
{
    // 방해 블록 카운트 증가
    total_interrupt_block_count_ += cnt;
    score_info_.totalInterruptBlockCount += cnt;

    // 콤보 공격 상태 설정
    state_info_.isComboAttack = true;

    // UI 업데이트
    if (interrupt_view_)
    {
        interrupt_view_->UpdateInterruptBlock(total_interrupt_block_count_);
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
    total_interrupt_block_count_ -= 30;
}

void RemotePlayer::CreatePartialRowInterruptBlocks(uint8_t y_row_cnt, const std::span<const uint8_t>& x_idx, std::shared_ptr<ImageTexture>& texture)
{
    // Complete rows
    for (int y = 0; y < y_row_cnt; y++)
    {
        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++)
        {
            CreateSingleIceBlock(x, y, texture);
        }
    }

    // Partial row
    for (int i = 0; i < x_idx.size(); i++)
    {
        CreateSingleIceBlock(x_idx[i], y_row_cnt, texture);
    }

    total_interrupt_block_count_ -= static_cast<uint16_t>(y_row_cnt * Constants::Board::BOARD_X_COUNT + x_idx.size());
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
    ice_block_set_.insert(ice_block);
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
    total_interrupt_block_count_ = std::max<uint16_t>(0, total_interrupt_block_count_ - count);

    if (interrupt_view_)
    {
        interrupt_view_->UpdateInterruptBlock(total_interrupt_block_count_);
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

void RemotePlayer::CollectRemoveIceBlocks()
{
    if (block_list_.empty() || equal_block_list_.empty() || game_state_ != GamePhase::Shattering)
    {
        return;
    }

    for (const auto& clear_group : equal_block_list_)
    {
        for (const auto& block : clear_group)
        {
            if (!block)
            {
                continue;
            }

            CollectAdjacentIceBlocks(block);
        }
    }

    // Set all collected ice blocks to destroying state
    for (const auto& ice_block : ice_block_set_)
    {
        ice_block->SetState(BlockState::Destroying);
    }
}

void RemotePlayer::CollectAdjacentIceBlocks(Block* block)
{
    const int x = block->GetPosIdx_X();
    const int y = block->GetPosIdx_Y();

    // 방향과 체크 방향 설정
    const std::array<std::pair<int, int>, 4> directions = { {
        {-1, 0},  // Left
        {1, 0},   // Right
        {0, 1},   // Top
        {0, -1}   // Bottom
    } };

    for (const auto& [dx, dy] : directions)
    {
        const int check_x = x + dx;
        const int check_y = y + dy;

        if (check_x < 0 || check_x >= Constants::Board::BOARD_X_COUNT ||
            check_y < 0 || check_y >= Constants::Board::BOARD_Y_COUNT)
        {
            continue;
        }

        Block* check_block = board_blocks_[check_y][check_x];
        if (!check_block ||
            check_block->GetState() != BlockState::Stationary ||
            check_block->GetBlockType() != BlockType::Ice)
        {
            continue;
        }

        if (auto ice_block = dynamic_cast<IceBlock*>(check_block))
        {
            auto found = std::find_if(block_list_.begin(), block_list_.end(),
                [ice_block](const auto& block) {
                    return block.get() == ice_block;
                });

            if (found != block_list_.end()) {
                ice_block_set_.insert(std::static_pointer_cast<IceBlock>(*found));
            }
        }
    }
}

void RemotePlayer::RemoveIceBlocks(std::list<SDL_Point>& x_index_list)
{
    if (ice_block_set_.empty())
    {
        return;
    }

    for (const auto& ice_block : ice_block_set_)
    {
        SDL_Point pos_idx{ ice_block->GetPosIdx_X(), ice_block->GetPosIdx_Y() };

        board_blocks_[pos_idx.y][pos_idx.x] = nullptr;
        block_list_.remove(ice_block);
        x_index_list.push_back(pos_idx);
    }

    ice_block_set_.clear();
}

void RemotePlayer::CalculateIceBlockCount()
{
    if (prev_game_state_ == GamePhase::Shattering)
    {
        ++combo_count_;
    }
    else if (prev_game_state_ == GamePhase::Playing)
    {
        combo_count_ = 1;
    }

    for (auto& block_group : equal_block_list_)
    {
        for (auto& block : block_group)
        {
            block->SetState(BlockState::Destroying);
        }
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
        new_blocks_.push_back(next_block);
        background_->SetPlayerNextBlock(next_block);
    }
}

void RemotePlayer::Release()
{
    equal_block_list_.clear();

    ReleaseContainer(del_bullet_array_);
    ReleaseContainer(ice_block_set_);
    ReleaseContainer(new_blocks_);

    BasePlayer::Release();
}

void RemotePlayer::Reset()
{
    equal_block_list_.clear();

    ReleaseContainer(del_bullet_array_);
    ReleaseContainer(ice_block_set_);
    ReleaseContainer(new_blocks_);

    BasePlayer::Reset();
}