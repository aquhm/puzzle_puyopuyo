#include "GamePlayer.hpp"

#include "../../core/manager/ResourceManager.hpp"
#include "../../core/manager/MapManager.hpp"
#include "../../core/manager/ParticleManager.hpp"
#include "../../core/manager/StateManager.hpp"
#include "../../core/manager/PlayerManager.hpp"

#include "../block/Block.hpp"
#include "../block/IceBlock.hpp"
#include "../block/GameGroupBlock.hpp"

#include "../system/GameBoard.hpp"
#include "../view/ComboView.hpp"
#include "../view/ResultView.hpp"
#include "../view/InterruptBlockView.hpp"

#include "../map/GameBackground.hpp"

#include "../effect/BulletEffect.hpp"
#include "../effect/ExplosionEffect.hpp"

#include "../../texture/ImageTexture.hpp"

#include "../../core/GameApp.hpp"
#include "../../core/GameUtils.hpp"

#include <stdexcept>
#include <algorithm>
#include <format>
#include <fstream>
#include <span>
#include <iostream>


GamePlayer::GamePlayer()
{
    draw_objects_.reserve(100);
}

GamePlayer::~GamePlayer()
{
    Release();
}

void GamePlayer::Release()
{
    Reset();

    combo_view_.reset();
    result_view_.reset();
    interrupt_block_view_.reset();
    game_board_.reset();
    control_block_.reset();
}

void GamePlayer::Reset()
{
    if (game_board_)
    {
        game_board_->ClearActiveGroupBlock();
    }

    if (control_block_)
    {
        control_block_->ResetBlock();
    }

    equal_block_list_.clear();
    ice_block_set_.clear();
    new_blocks_.clear();
    block_list_.clear();
    bullet_list_.clear();
    del_bullet_array_.clear();

    std::memset(board_blocks_, 0, sizeof(Block*) * Constants::Board::BOARD_Y_COUNT * Constants::Board::BOARD_X_COUNT);
}

bool GamePlayer::Initialize(const std::span<const uint8_t>& blocktype1, const std::span<const uint8_t>& blocktype2, uint8_t playerIdx, uint8_t characterIdx,
    const std::shared_ptr<GameBackground>& background)
{
    Reset();

    try
    {
        background_ = background;
        if (!background_)
        {
            throw std::runtime_error("Background is null");
        }
        
        InitializeNextBlocks(blocktype1, blocktype2);

        InitializeGameBoard();

        InitializeViews();

        InitializeControlBlock();

#ifdef _APP_DEBUG_
        //CreateBlocksFromFile();
#endif

        // Set initial game state
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
        SDL_Log("GamePlayer initialization failed: %s", e.what());
        return false;
    }
}

void GamePlayer::InitializeNextBlocks(const std::span<const uint8_t>& blocktype1, const std::span<const uint8_t>& blocktype2)
{
    auto next_block1 = std::make_unique<GroupBlock>();
    auto next_block2 = std::make_unique<GroupBlock>();

    if (!next_block1->Create(static_cast<BlockType>(blocktype1[0]), static_cast<BlockType>(blocktype1[1])) ||
        !next_block2->Create(static_cast<BlockType>(blocktype2[0]), static_cast<BlockType>(blocktype2[1])))
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

void GamePlayer::InitializeGameBoard()
{
    if (!game_board_)
    {
        game_board_ = std::make_unique<GameBoard>();
    }

    if (game_board_->Initialize(Constants::Board::PLAYER_POSITION_X, Constants::Board::PLAYER_POSITION_Y, block_list_) == false)
    {
        throw std::runtime_error("Failed to initialize game board");
    }

    auto texture = ImageTexture::Create("PUYO/puyo_beta.png");
    if (!texture)
    {
        throw std::runtime_error("Failed to load block texture");
    }

    game_board_->SetBlockInfoTexture(texture);
    game_board_->ClearActiveGroupBlock();
    game_board_->SetRenderTargetMark(false);
    draw_objects_.push_back(game_board_.get());
}

void GamePlayer::InitializeViews()
{
    // Initialize interrupt block view
    if (!interrupt_block_view_)
    {
        interrupt_block_view_ = std::make_unique<InterruptBlockView>();
    }

    if (!interrupt_block_view_->Initialize())
    {
        throw std::runtime_error("Failed to initialize interrupt block view");
    }

    interrupt_block_view_->SetPosition(Constants::Board::PLAYER_POSITION_X, 0);
    draw_objects_.push_back(interrupt_block_view_.get());

    // Initialize combo view
    if (!combo_view_)
    {
        combo_view_ = std::make_unique<ComboView>();
    }

    if (!combo_view_->Initialize())
    {
        throw std::runtime_error("Failed to initialize combo view");
    }

    draw_objects_.push_back(combo_view_.get());

    // Initialize result view
    if (!result_view_)
    {
        result_view_ = std::make_unique<ResultView>();
    }

    if (!result_view_->Initialize())
    {
        throw std::runtime_error("Failed to initialize result view");
    }

    draw_objects_.push_back(result_view_.get());
}

void GamePlayer::InitializeControlBlock()
{
    if (!control_block_)
    {
        control_block_ = std::make_unique<GameGroupBlock>();
    }

    if (control_block_)
    {
        control_block_->SetGameBlocks(&block_list_);
    }
    else
    {
        control_block_->ResetBlock();
    }
}

void GamePlayer::AddNewBlock(const std::span<const uint8_t, 2>& block_type)
{
    auto next_block = std::make_shared<GroupBlock>();
    if (!next_block->Create(static_cast<BlockType>(block_type[0]),
        static_cast<BlockType>(block_type[1])))
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

void GamePlayer::DestroyNextBlock()
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

        if (game_board_)
        {
            game_board_->CreateNewBlockInGame(control_block_);
            game_board_->SetRenderTargetMark(false);
        }
    }
}

void GamePlayer::MoveBlock(uint8_t move_type, float position)
{
    if (!control_block_ || control_block_->GetState() != BlockState::Playing)
    {
        return;
    }

    switch (static_cast<Constants::Direction>(move_type))
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

void GamePlayer::RotateBlock(uint8_t rotate_type, bool horizontal_moving)
{
    if (!control_block_ || control_block_->GetState() != BlockState::Playing)
    {
        return;
    }

    control_block_->SetEnableRotState(static_cast<RotateState>(rotate_type), horizontal_moving, true, false);
}

void GamePlayer::UpdateBlockPosition(float pos1, float pos2)
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

void GamePlayer::UpdateFallingBlock(uint8_t falling_idx, bool falling)
{
    if (control_block_ && control_block_->GetState() == BlockState::Playing)
    {
        control_block_->UpdateFallingBlock(falling_idx, falling);
    }
}

void GamePlayer::ChangeBlockState(uint8_t state)
{
    if (control_block_)
    {
        control_block_->SetState(static_cast<BlockState>(state));
    }
}

bool GamePlayer::PushBlockInGame(std::span<float> pos1, std::span<float> pos2)
{
    if (!control_block_ || control_block_->GetState() != BlockState::Stationary)
    {
        return false;
    }

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
            DestroyNextBlock();
        }

        return true;
    }

    return false;
}


void GamePlayer::UpdateLinkState(Block* block)
{
    if (!block || block->GetBlockType() == BlockType::Ice)
    {
        return;
    }

    const int x = block->GetPosIdx_X();
    const int y = block->GetPosIdx_Y();
    const auto block_type = block->GetBlockType();
    auto link_state = block->GetLinkState();

    // 방향 계산
    const std::array<std::pair<int, int>, 4> directions = { {
        {-1, 0},  // Left
        {1, 0},   // Right
        {0, 1},   // Top
        {0, -1}   // Bottom
    } };

    const std::array<LinkState, 4> link_states = { {
        LinkState::Right,   // Link to left neighbor
        LinkState::Left,    // Link to right neighbor
        LinkState::Bottom,  // Link to top neighbor
        LinkState::Top      // Link to bottom neighbor
    } };

    for (size_t i = 0; i < directions.size(); ++i)
    {
        const int check_x = x + directions[i].first;
        const int check_y = y + directions[i].second;

        // 경계 체크
        if (check_x < 0 || check_x >= Constants::Board::BOARD_X_COUNT || check_y < 0 || check_y >= Constants::Board::BOARD_Y_COUNT)
        {
            continue;
        }

        Block* check_block = board_blocks_[check_y][check_x];
        if (!check_block || check_block->GetState() != BlockState::Stationary)
        {
            continue;
        }

        if (block_type == check_block->GetBlockType())
        {
            // 현재 블록의 링크 상태 업데이트
            link_state = static_cast<LinkState>(static_cast<uint8_t>(link_state) | static_cast<uint8_t>(link_states[i]));
            block->SetLinkState(link_state);

            // 체크된 블록의 링크 상태 업데이트
            auto neighbor_link_state = check_block->GetLinkState();
            neighbor_link_state = static_cast<LinkState>(static_cast<uint8_t>(neighbor_link_state) | static_cast<uint8_t>(link_states[(i + 2) % 4]));
            check_block->SetLinkState(neighbor_link_state);
        }
    }
}

int16_t GamePlayer::RecursionCheckBlock(int16_t x, int16_t y, int16_t direction, std::vector<Block*>& block_list)
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

bool GamePlayer::CheckGameBlockState()
{
    if (is_game_quit_)
    {
        game_state_ = GamePhase::Standing;
        return true;
    }

    const int block_count = static_cast<int>(block_list_.size());
    if (block_count < Constants::Game::MIN_MATCH_COUNT)
    {
        game_state_ = GamePhase::Playing;
        prev_game_state_ = GamePhase::Playing;
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
                equal_block_list_.push_back(BlockVector(
                    current_blocks.begin(),
                    current_blocks.end()
                ));
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

    // 매칭된 블록이 있는 경우
    if (!equal_block_list_.empty())
    {
        HandleMatchedBlocks();
        return true;
    }

    // 매칭된 블록이 없는 경우
    ResetMatchState();
    return false;
}

void GamePlayer::HandleMatchedBlocks()
{
    prev_game_state_ = game_state_;
    game_state_ = GamePhase::Shattering;

    if (prev_game_state_ == GamePhase::Shattering)
    {
        ++combo_count_;
    }
    else if (prev_game_state_ == GamePhase::Playing)
    {
        combo_count_ = 1;
    }

    // 매칭된 블록들을 파괴 상태로 설정
    for (auto& block_group : equal_block_list_)
    {
        for (auto& block : block_group)
        {
            block->SetState(BlockState::Destroying);
        }
    }

    CollectRemoveIceBlocks();
}

void GamePlayer::ResetMatchState()
{
    if (combo_count_ > 0)
    {
        combo_count_ = 0;
    }

    if (rest_score_ > 0)
    {
        rest_score_ = 0;
    }

    game_state_ = is_game_quit_ ? GamePhase::Standing : GamePhase::Playing;
    prev_game_state_ = game_state_;
}


void GamePlayer::AddInterruptBlock(int16_t count)
{
    total_interrupt_block_count_ += count;

    if (interrupt_block_view_)
    {
        interrupt_block_view_->UpdateInterruptBlock(total_interrupt_block_count_);
    }

    if (game_board_ && game_board_->GetState() != BoardState::Lose)
    {
        game_board_->SetState(BoardState::Damaging);
    }
}

void GamePlayer::AddInterruptBlock(uint8_t y_row_cnt, std::span<uint8_t> x_idx)
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

        if (interrupt_block_view_)
        {
            interrupt_block_view_->UpdateInterruptBlock(total_interrupt_block_count_);
        }

        game_state_ = GamePhase::IceBlocking;
    }
    catch (const std::exception& e)
    {
        SDL_Log("Failed to add interrupt blocks: %s", e.what());
    }
}

void GamePlayer::CreateFullRowInterruptBlocks(std::shared_ptr<ImageTexture>& texture)
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

void GamePlayer::CreatePartialRowInterruptBlocks(uint8_t y_row_cnt, std::span<uint8_t> x_idx, std::shared_ptr<ImageTexture>& texture)
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
    for (int x = 0; x < x_idx.size(); x++)
    {
        CreateSingleIceBlock(x_idx[x], y_row_cnt, texture);
    }

    total_interrupt_block_count_ -= static_cast<uint16_t>(y_row_cnt * Constants::Board::BOARD_X_COUNT + x_idx.size());
}

void GamePlayer::CreateSingleIceBlock(int x, int y, std::shared_ptr<ImageTexture>& texture)
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

    block_list_.push_back(ice_block);
}

void GamePlayer::AttackInterruptBlock(float x, float y, uint8_t type)
{
    const SDL_FPoint start_pos
    {
        Constants::Board::PLAYER_POSITION_X + Constants::Board::WIDTH_MARGIN +x + Constants::Block::SIZE / 2,
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

void GamePlayer::DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type)
{
    total_interrupt_block_count_ = max(0, total_interrupt_block_count_ - count);

    if (interrupt_block_view_)
    {
        interrupt_block_view_->UpdateInterruptBlock(total_interrupt_block_count_);
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

void GamePlayer::CollectRemoveIceBlocks()
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

void GamePlayer::CollectAdjacentIceBlocks(const std::shared_ptr<Block>& block)
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
            ice_block_set_.insert(std::shared_ptr<IceBlock>(ice_block));
        }
    }
}

void GamePlayer::UpdateInterruptBlock(int16_t count)
{
    total_interrupt_block_count_ = count;

    if (interrupt_block_view_)
    {
        interrupt_block_view_->UpdateInterruptBlock(count);
    }
}

[[nodiscard]] int16_t GamePlayer::GetComboConstant(uint8_t combo_count) const
{
    if (combo_count == 1)
    {
        return 0;
    }
    else if (combo_count <= 4)
    {
        return static_cast<int16_t>(std::pow(2, combo_count + 1));
    }
    else if (combo_count <= Constants::Game::MAX_COMBO)
    {
        return 32 * (combo_count - 3);
    }

    return 0;
}

[[nodiscard]] uint8_t GamePlayer::GetLinkBonus(uint8_t link_count) const
{
    static const std::array<std::pair<uint8_t, uint8_t>, 8> LINK_BONUS_TABLE = { {
        {4, 0},
        {5, 2},
        {6, 3},
        {7, 4},
        {8, 5},
        {9, 6},
        {10, 7},
        {11, 10}
    } };

    for (const auto& [count, bonus] : LINK_BONUS_TABLE)
    {
        if (link_count <= count)
        {
            return bonus;
        }
    }

    return Constants::Game::Score::MAX_LINK_BONUS;
}

[[nodiscard]] uint8_t GamePlayer::GetTypeCntBonus(uint8_t count) const
{
    static const std::array<std::pair<uint8_t, uint8_t>, 5> TYPE_BONUS_TABLE = { {
        {1, 0},
        {2, 3},
        {3, 6},
        {4, 12},
        {5, Constants::Game::Score::MAX_TYPE_BONUS}
    } };

    for (const auto& [type_count, bonus] : TYPE_BONUS_TABLE)
    {
        if (count == type_count)
        {
            return bonus;
        }
    }

    return 0;
}

[[nodiscard]] uint8_t GamePlayer::GetMargin() const
{
    for (const auto& margin : Constants::Game::SCORE_MARGINS)
    {
        if (play_time_ <= margin.time)
        {
            return margin.margin;
        }
    }

    return Constants::Game::SCORE_MARGINS[0].margin;
}

void GamePlayer::LoseGame(bool is_win)
{
    if (!is_win && game_board_)
    {
        game_board_->SetState(BoardState::Lose);
    }

    if (result_view_)
    {
        const float result_x = Constants::Board::PLAYER_POSITION_X + 20;
        const float result_y = 100;
        result_view_->UpdateResult(result_x, result_y, is_win);
    }

    game_state_ = GamePhase::Standing;
}

void GamePlayer::Update(float delta_time)
{
    play_time_ += delta_time;

    // 게임 오브젝트 업데이트
    for (auto draw_object : draw_objects_)
    {
        if (draw_object)
        {
            draw_object->Update(delta_time);
        }
    }

    // 블록 업데이트
    for (const auto& block : block_list_)
    {
        if (block)
        {
            block->Update(delta_time);
        }
    }

    // 게임 상태별 업데이트
    UpdateGameState(delta_time);

    // 투사체 및 파티클 업데이트
    UpdateBullets(delta_time);
 
    GAME_APP.GetParticleManager().Update(delta_time);
}

void GamePlayer::UpdateGameState(float delta_time)
{
    switch (game_state_)
    {
    case GamePhase::Standing:
        UpdateStandingState(delta_time);
        break;

    case GamePhase::Playing:
        UpdatePlayingState(delta_time);
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

void GamePlayer::UpdateStandingState(float delta_time)
{
    if (result_view_)
    {
        result_view_->Update(delta_time);
    }
}

void GamePlayer::UpdatePlayingState(float delta_time)
{
    if (control_block_)
    {
        control_block_->Update(delta_time);
    }
}

void GamePlayer::UpdateIceBlockDowningState()
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
    }
}

void GamePlayer::UpdateShatteringState()
{
    if (equal_block_list_.empty())
    {
        UpdateAfterBlocksCleared();
        return;
    }

    UpdateMatchedBlocks();
}

void GamePlayer::UpdateMatchedBlocks()
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

void GamePlayer::HandleClearedBlockGroup(std::list<BlockVector>::iterator& group_it, SDL_FPoint& pos, SDL_Point& pos_idx, std::list<SDL_Point>& x_index_list)
{
    for (const auto& block : *group_it)
    {
        pos = block->GetPosition();
        pos_idx = { block->GetPosIdx_X(), block->GetPosIdx_Y() };

        CreateBlockClearEffect(block);
        RemoveBlock(block, pos_idx);
        x_index_list.push_back(pos_idx);
    }

    UpdateComboDisplay(pos);
    RemoveIceBlocks(x_index_list);
    group_it = equal_block_list_.erase(group_it);
}

void GamePlayer::CreateBlockClearEffect(const std::shared_ptr<Block>& block)
{
    auto particle_container = std::make_unique<ExplosionContainer>();
    particle_container->SetBlockType(block->GetBlockType());
    particle_container->SetPlayerID(0);

    GAME_APP.GetParticleManager().AddParticleContainer(std::move(particle_container), block->GetPosition());
}

void GamePlayer::RemoveBlock(const std::shared_ptr<Block>& block, const SDL_Point& pos_idx)
{
    board_blocks_[pos_idx.y][pos_idx.x] = nullptr;
    block_list_.remove(block);
}

void GamePlayer::UpdateComboDisplay(const SDL_FPoint& pos)
{
    if (combo_view_ && combo_count_ > 0)
    {
        combo_view_->UpdateComboCount(pos.x + Constants::Board::PLAYER_POSITION_X, pos.y, combo_count_);
    }
}

void GamePlayer::UpdateFallingBlocks(const std::list<SDL_Point>& x_index_list)
{
    for (const auto& pos : x_index_list)
    {
        for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; y++)
        {
            if (Block* block = board_blocks_[y][pos.x])
            {
                if (block->GetState() == BlockState::DownMoving)
                {
                    continue;
                }

                if (y > pos.y)
                {
                    block->SetState(BlockState::DownMoving);
                }
            }
        }
    }
}

void GamePlayer::UpdateAfterBlocksCleared()
{
    block_list_.sort([](const auto& a, const auto& b) 
        { 
            return *a < *b; 
        });


    if (block_list_.empty())
    {
        game_state_ = is_game_quit_ ? GamePhase::Standing : GamePhase::Playing;
        prev_game_state_ = game_state_;
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

        if (!CheckGameBlockState() && !is_game_quit_)
        {
            game_state_ = GamePhase::Playing;
            prev_game_state_ = game_state_;
        }
    }
}

void GamePlayer::UpdateBlockLinks()
{
    for (const auto& block : block_list_)
    {
        if (block->GetBlockType() != BlockType::Ice)
        {
            block->SetLinkState();
            UpdateLinkState(block.get());
        }
    }
}

void GamePlayer::UpdateBullets(float delta_time)
{
    auto bullet_it = bullet_list_.begin();
    while (bullet_it != bullet_list_.end())
    {
        if (auto bullet = *bullet_it)
        {
            bullet->Update(delta_time);

            if (!bullet->IsAlive())
            {
                bullet_it = bullet_list_.erase(bullet_it);
            }
            else
            {
                ++bullet_it;
            }
        }
    }
}

void GamePlayer::Render()
{
    // 게임 오브젝트 렌더링
    for (auto* draw_object : draw_objects_)
    {
        if (draw_object)
        {
            draw_object->Render();
        }
    }

    // 투사체 렌더링
    for (const auto& bullet : bullet_list_)
    {
        if (bullet)
        {
            bullet->Render();
        }
    }
}

void GamePlayer::CalculateIceBlockCount()
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

void GamePlayer::CreateBullet(Block* block)
{
    if (!block)
    {
        SDL_Log("CreateBullet: block is null");
        return;
    }

    SDL_FPoint start_pos
    {
        Constants::Board::POSITION_X + Constants::Board::WIDTH_MARGIN + block->GetX() + Constants::Block::SIZE / 2,
        Constants::Board::POSITION_Y + block->GetY() + Constants::Block::SIZE / 2    
    };

    SDL_FPoint end_pos;
    if (has_ice_block_)
    {
        end_pos = 
        {
            Constants::Board::POSITION_X + (Constants::Board::WIDTH / 2),
            Constants::Board::POSITION_Y
        };
    }
    else
    {
        end_pos = 
        {
            GAME_APP.GetWindowWidth() - (Constants::Board::POSITION_X + (Constants::Board::WIDTH / 2)),
            Constants::Board::POSITION_Y
        };
    }

    auto bullet = std::make_shared<BulletEffect>();
    if (!bullet->Initialize(start_pos, end_pos, block->GetBlockType()))
    {
        return;
    }

    bullet_list_.push_back(bullet);
}


void GamePlayer::CreateBlocksFromFile()
{
    std::string currentPath = std::filesystem::current_path().string();
    std::cout << "현재 작업 디렉토리: " << currentPath << std::endl;

    std::ifstream file("./bin/puyo.txt");
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
            block->SetScale(Constants::Block::SIZE, Constants::Block::SIZE);
            block->SetState(BlockState::Stationary);
            block->SetBlockTex(texture);

            board_blocks_[Constants::Board::BOARD_Y_COUNT - 1 - y][x] = block.get();
            block_list_.push_back(block);

            UpdateLinkState(block.get());
        }
    }

    block_list_.sort([](const auto& a, const auto& b) { return *a < *b; });
}

bool GamePlayer::Restart(const std::span<const uint8_t>& block_type1, const std::span<const uint8_t>& block_type2)
{
    Reset();

    try
    {
        // Initialize next blocks
        auto next_block1 = std::make_shared<GroupBlock>();
        auto next_block2 = std::make_shared<GroupBlock>();

        if (!next_block1->Create(static_cast<BlockType>(block_type1[0]), static_cast<BlockType>(block_type1[1])) ||
            !next_block2->Create(static_cast<BlockType>(block_type2[0]), static_cast<BlockType>(block_type2[1])))
        {
            throw std::runtime_error("Failed to create next blocks");
        }

        next_block1->SetPosition(Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_X, Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_Y);
        next_block2->SetPosition(Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_X, Constants::GroupBlock::NEXT_PLAYER_BLOCK_POS_SMALL_Y);
        next_block2->SetScale(Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE, Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE);

        new_blocks_.push_back(std::move(next_block1));
        new_blocks_.push_back(std::move(next_block2));

        if (game_board_)
        {
            if (!game_board_->Initialize(Constants::Board::PLAYER_POSITION_X,
                Constants::Board::PLAYER_POSITION_Y,
                block_list_))
            {
                throw std::runtime_error("Failed to initialize game board");
            }

            game_board_->ClearActiveGroupBlock();
            game_board_->SetState(BoardState::Normal);
            game_board_->SetRenderTargetMark(false);
        }

        // Reset UI components
        if (interrupt_block_view_) interrupt_block_view_->Initialize();
        if (combo_view_) combo_view_->Initialize();
        if (result_view_) result_view_->Initialize();
        if (control_block_) control_block_->ResetBlock();

#ifdef _APP_DEBUG_
        //CreateBlocksFromFile();
#endif

        // Reset game state
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
        SDL_Log("Failed to restart game: %s", e.what());
        return false;
    }
}

void GamePlayer::RemoveIceBlocks(std::list<SDL_Point>& x_index_list)
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

