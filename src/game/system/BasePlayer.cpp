#include "BasePlayer.hpp"

#include "../../core/manager/ResourceManager.hpp"
#include "../../core/manager/ParticleManager.hpp"
#include "../../core/manager/PlayerManager.hpp"
#include "../../core/manager/StateManager.hpp"

#include "../block/Block.hpp"
#include "../block/IceBlock.hpp"
#include "../block/GameGroupBlock.hpp"
#include "../block/GroupBlock.hpp"

#include "../event/IPlayerEventListener.hpp"
#include "../system/GameBoard.hpp"
#include "../view/InterruptBlockView.hpp"
#include "../view/ComboView.hpp"
#include "../view/ResultView.hpp"

#include "../map/GameBackground.hpp"

#include "../effect/BulletEffect.hpp"
#include "../effect/ExplosionEffect.hpp"

#include "../../texture/ImageTexture.hpp"
#include "../../core/GameApp.hpp"
#include "../../network/NetworkController.hpp"
#include "../../network/player/Player.hpp"

#include "../../utils/Logger.hpp"

#include <stdexcept>
#include <algorithm>
#include <format>
#include <iostream>
#include <fstream>

BasePlayer::BasePlayer()
{
    draw_objects_.reserve(100);
}

BasePlayer::~BasePlayer()
{
    Release();
}

void BasePlayer::Reset()
{
    if (game_board_) 
    {
        game_board_->Release();
        game_board_->ClearActiveGroupBlock();
    }

    if (interrupt_view_) 
    {
        interrupt_view_->Release();
    }

    if (combo_view_) 
    {
        combo_view_->Release();
    }

    if (result_view_) 
    {
        result_view_->Release();
    }

    if (control_block_) {
        control_block_->Release();
    }

    ReleaseContainer(block_list_);
    ReleaseContainer(bullet_list_);
    ReleaseContainer(draw_objects_);

    draw_objects_.clear();

    std::memset(board_blocks_, 0, sizeof(Block*) * Constants::Board::BOARD_Y_COUNT * Constants::Board::BOARD_X_COUNT);
    
    score_info_.reset();
    state_info_ = GameStateInfo{};
	state_info_.currentPhase = GamePhase::Playing;
	state_info_.previousPhase = GamePhase::Playing;
	state_info_.playTime = 0.0f;
    is_game_quit_ = false;
}

void BasePlayer::Release()
{
    Reset();

    if (game_board_) {
        game_board_->Release();
        game_board_.reset();
    }

    if (interrupt_view_) {
        interrupt_view_->Release();
        interrupt_view_.reset();
    }

    if (combo_view_) {
        combo_view_->Release();
        combo_view_.reset();
    }

    if (result_view_) {
        result_view_->Release();
        result_view_.reset();
    }

    if (control_block_) {
        control_block_->Release();
        control_block_.reset();
    }

    background_.reset();

    draw_objects_.clear();
}

void BasePlayer::Update(float deltaTime)
{
    state_info_.playTime += deltaTime;

    for (auto* obj : draw_objects_)
    {
        if (obj)
        {
            obj->Update(deltaTime);
        }
    }

    for (auto& block : block_list_)
    {
        if (block)
        {
            block->Update(deltaTime);
        }
    }

    UpdateBullets(deltaTime);

    GAME_APP.GetParticleManager().Update(deltaTime);
}

void BasePlayer::Render()
{
    for (auto obj : draw_objects_)
    {
        if (obj && obj->IsVisible())
        {
            obj->Render();
        }
    }

    for (const auto& bullet : bullet_list_)
    {
        if (bullet)
        {
            bullet->Render();
        }
    }
}

bool BasePlayer::InitializeGameBoard(float posX, float posY)
{
    if (!game_board_)
    {
        game_board_ = std::make_unique<GameBoard>();
    }

    if (!game_board_->Initialize(posX, posY, block_list_, player_id_))
    {
        LOGGER.Error("Failed to initialize game board for player {}", player_id_);
        return false;
    }

    auto texture = ImageTexture::Create("PUYO/puyo_beta.png");
    if (!texture)
    {
        LOGGER.Error("Failed to load block texture");
        return false;
    }

    game_board_->SetBlockInfoTexture(texture);
    game_board_->SetRenderTargetMark(false);
    draw_objects_.push_back(game_board_.get());

    return true;
}

bool BasePlayer::InitializeViews()
{
    interrupt_view_ = std::make_shared<InterruptBlockView>();    
    interrupt_view_->Initialize();

    float boardPosX = (player_id_ == GAME_APP.GetPlayerManager().GetMyPlayer()->GetId()) ?
        Constants::Board::POSITION_X : Constants::Board::PLAYER_POSITION_X;
    
    interrupt_view_->SetPosition(boardPosX, 0);
    draw_objects_.push_back(interrupt_view_.get());

    combo_view_ = std::make_shared<ComboView>();
    combo_view_->Initialize();
    draw_objects_.push_back(combo_view_.get());    

    result_view_ = std::make_shared<ResultView>();
    result_view_->Initialize();
    draw_objects_.push_back(result_view_.get());

    return true;
}

bool BasePlayer::InitializeControlBlock()
{
    if (!control_block_)
    {
        control_block_ = std::make_shared<GameGroupBlock>();
    }

    if (control_block_)
    {
        control_block_->SetGameBlocks(&block_list_);
        control_block_->SetPlayerID(player_id_);
        //control_block_->ResetBlock();
        return true;
    }

    return false;
}

void BasePlayer::UpdateLinkState(Block* block)
{
    if (!block || block->GetBlockType() == BlockType::Ice)
    {
        return;
    }

    int x = block->GetPosIdx_X();
    int y = block->GetPosIdx_Y();
    BlockType blockType = block->GetBlockType();
    uint8_t linkState = static_cast<uint8_t>(block->GetLinkState());

    const std::array<std::pair<Constants::Direction, std::pair<int, int>>, 4> directions = { {
        {Constants::Direction::Left,   {x - 1, y}},
        {Constants::Direction::Right,  {x + 1, y}},
        {Constants::Direction::Top,    {x, y + 1}},
        {Constants::Direction::Bottom, {x, y - 1}}
    } };

    for (const auto& [dir, pos] : directions)
    {
        const auto [checkX, checkY] = pos;

        if (checkX >= 0 && checkX < Constants::Board::BOARD_X_COUNT &&
            checkY >= 0 && checkY < Constants::Board::BOARD_Y_COUNT)
        {
            Block* checkBlock = board_blocks_[checkY][checkX];
            if (checkBlock && checkBlock->GetState() == BlockState::Stationary && checkBlock->GetBlockType() == blockType)
            {
                auto checkBlockLinkState = static_cast<uint8_t>(checkBlock->GetLinkState());
                switch (dir)
                {
                case Constants::Direction::Left:
                    linkState |= static_cast<uint8_t>(LinkState::Right);
                    checkBlock->SetLinkState(static_cast<LinkState>(checkBlockLinkState | static_cast<uint8_t>(LinkState::Left)));
                    break;
                case Constants::Direction::Right:
                    linkState |= static_cast<uint8_t>(LinkState::Left);
                    checkBlock->SetLinkState(static_cast<LinkState>(checkBlockLinkState | static_cast<uint8_t>(LinkState::Right)));
                    break;
                case Constants::Direction::Top:
                    linkState |= static_cast<uint8_t>(LinkState::Bottom);
                    checkBlock->SetLinkState(static_cast<LinkState>(checkBlockLinkState | static_cast<uint8_t>(LinkState::Top)));
                    break;
                case Constants::Direction::Bottom:
                    linkState |= static_cast<uint8_t>(LinkState::Top);
                    checkBlock->SetLinkState(static_cast<LinkState>(checkBlockLinkState | static_cast<uint8_t>(LinkState::Bottom)));
                    break;
                }
            }
        }
    }

    block->SetLinkState(static_cast<LinkState>(linkState));
}

void BasePlayer::CreateBullet(Block* block)
{
    if (!block)
    {
        LOGGER.Error("CreateBullet: block is null");
        return;
    }

    float boardPosX = (player_id_ == GAME_APP.GetPlayerManager().GetMyPlayer()->GetId()) ?
        Constants::Board::POSITION_X : Constants::Board::PLAYER_POSITION_X;

    SDL_FPoint startPos
    {
        boardPosX + Constants::Board::WIDTH_MARGIN + block->GetX() + Constants::Block::SIZE / 2,
        Constants::Board::POSITION_Y + block->GetY() + Constants::Block::SIZE / 2
    };

    SDL_FPoint endPos;
    if (state_info_.hasIceBlock)
    {
        endPos =
        {
            GAME_APP.GetWindowWidth() - (boardPosX + (Constants::Board::WIDTH / 2)),
            Constants::Board::POSITION_Y
        };
    }
    else
    {
        endPos =
        {
            boardPosX + (Constants::Board::WIDTH / 2),
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

void BasePlayer::UpdateBullets(float delta_time)
{
    auto it = bullet_list_.begin();
    while (it != bullet_list_.end())
    {
        if (auto& bullet = *it)
        {
            bullet->Update(delta_time);

            if (!bullet->IsAlive())
            {
                bullet->Release();
                it = bullet_list_.erase(it);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            it = bullet_list_.erase(it);
        }
    }
}

void BasePlayer::CreateBlockClearEffect(const std::shared_ptr<Block>& block)
{
    //LOGGER.Info("BasePlayer::CreateBlockClearEffect player_id_ = {}", player_id_);

    auto particle_container = std::make_unique<ExplosionContainer>();
    particle_container->SetBlockType(block->GetBlockType());
    particle_container->SetPlayerID(player_id_);

    GAME_APP.GetParticleManager().AddParticleContainer(std::move(particle_container), block->GetPosition());
}

void BasePlayer::RemoveBlock(Block* block, const SDL_Point& pos_idx)
{
    if (!block) return;

    if (pos_idx.x >= 0 && pos_idx.x < Constants::Board::BOARD_X_COUNT &&
        pos_idx.y >= 0 && pos_idx.y < Constants::Board::BOARD_Y_COUNT)
    {
        board_blocks_[pos_idx.y][pos_idx.x] = nullptr;
    }

    auto it = std::find_if(block_list_.begin(), block_list_.end(),
        [block](const std::shared_ptr<Block>& ptr) {
            return ptr.get() == block;
        });

    if (it != block_list_.end()) 
    
    {
        (*it)->Release();
        block_list_.erase(it);
    }
}

void BasePlayer::UpdateFallingBlocks(const std::list<SDL_Point>& x_index_list)
{
    for (const auto& pos : x_index_list)
    {
        for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; y++)
        {
            if (Block* block = board_blocks_[y][pos.x]; block != nullptr)
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

void BasePlayer::UpdateBlockLinks()
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

int16_t BasePlayer::GetComboConstant(uint8_t combo_count) const
{
    if (combo_count <= 1)
    {
        return 0;
    }

    if (combo_count <= 4)
    {
        return static_cast<int16_t>(std::pow(2, combo_count + 1));
    }

    if (combo_count <= Constants::Game::MAX_COMBO)
    {
        return 32 * (combo_count - 3);
    }

    return 0;
}

uint8_t BasePlayer::GetLinkBonus(size_t link_count) const
{
    static const std::array<uint8_t, 8> LINK_BONUSES = { 0, 0, 0, 0, 2, 3, 4, 5 };

    if (link_count <= 4)
    {
        return 0;
    }
    else if (link_count <= 10)
    {
        return LINK_BONUSES[link_count - 4];
    }
    else
    {
        return Constants::Game::Score::MAX_LINK_BONUS;
    }
}

uint8_t BasePlayer::GetTypeBonus(size_t count) const
{
    static const std::array<uint8_t, 6> TYPE_BONUSES =
    {
        0, 0, 3, 6, 12, Constants::Game::Score::MAX_TYPE_BONUS
    };
    return count < TYPE_BONUSES.size() ? TYPE_BONUSES[count] : TYPE_BONUSES.back();
}

uint8_t BasePlayer::GetMargin() const
{
    const float playTime = state_info_.playTime;

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

void BasePlayer::LoseGame(bool isWin)
{
    if (isWin == false)
    {
        if (game_board_)
        {
            game_board_->SetState(BoardState::Lose);
        }
    }
    
    if (result_view_)
    {
        float result_x = (player_id_ == GAME_APP.GetPlayerManager().GetMyPlayer()->GetId()) ?
            Constants::Board::POSITION_X + 20 : Constants::Board::PLAYER_POSITION_X + 20;
        float result_y = 100;
        result_view_->UpdateResult(result_x, result_y, isWin);
    }

    state_info_.currentPhase = GamePhase::GameOver;

    SetGameQuit();
}

void BasePlayer::AddInterruptBlock(int16_t count)
{
    score_info_.totalInterruptBlockCount += count;

    if (interrupt_view_)
    {
        interrupt_view_->UpdateInterruptBlock(score_info_.totalInterruptBlockCount);
    }

    if (game_board_ && game_board_->GetState() != BoardState::Lose)
    {
        game_board_->SetState(BoardState::Damaging);
    }
}

void BasePlayer::SetGameBoardState(BoardState bordState)
{
    if (game_board_ && game_board_->GetState() != BoardState::Lose)
    {
        game_board_->SetState(bordState);
    }
}

void BasePlayer::UpdateInterruptBlock(int16_t count)
{
    score_info_.totalInterruptBlockCount = count;

    if (interrupt_view_)
    {
        interrupt_view_->UpdateInterruptBlock(count);
    }
}

bool BasePlayer::IsPossibleMove(int xIdx)
{
    return
        board_blocks_[Constants::Board::BOARD_Y_COUNT - 2][xIdx] == nullptr ||
        board_blocks_[Constants::Board::BOARD_Y_COUNT - 1][xIdx] == nullptr ? true : false;
}


void BasePlayer::CreateBlocksFromFile()
{
    std::string currentPath = std::filesystem::current_path().string();
    std::cout << "���� �۾� ���丮: " << currentPath << std::endl;

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
            block->SetPlayerID(player_id_);

            board_blocks_[Constants::Board::BOARD_Y_COUNT - 1 - y][x] = block.get();
            block_list_.push_back(block);

            UpdateLinkState(block.get());
        }
    }

    block_list_.sort([](const auto& a, const auto& b) { return *a < *b; });
}


void BasePlayer::NotifyEvent(const std::shared_ptr<BasePlayerEvent>& event)
{
    for (auto* listener : event_listeners_) 
    {
        if (listener) 
        {
            listener->OnPlayerEvent(event);
        }
    }
}

void BasePlayer::AddEventListener(IPlayerEventListener* listener)
{
    if (listener) 
    {
        event_listeners_.push_back(listener);
    }
}

void BasePlayer::RemoveEventListener(IPlayerEventListener* listener)
{
    auto it = std::find(event_listeners_.begin(), event_listeners_.end(), listener);
    if (it != event_listeners_.end()) 
    {
        event_listeners_.erase(it);
    }
}