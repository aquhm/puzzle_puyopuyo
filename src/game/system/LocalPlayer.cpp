#include "LocalPlayer.hpp"

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

LocalPlayer::LocalPlayer() : BasePlayer(), lastInputTime_(0)
{
}

LocalPlayer::~LocalPlayer()
{
    Release();
}

bool LocalPlayer::Initialize(const std::span<const uint8_t>& blockType1, const std::span<const uint8_t>& blockType2,
    uint8_t playerIdx, uint16_t characterIdx, const std::shared_ptr<GameBackground>& background)
{
    Reset();

    try
    {
        player_id_ = playerIdx;
        character_id_ = characterIdx;
        background_ = background;

        InitializeNextBlocks();        

        if (!InitializeGameBoard(Constants::Board::POSITION_X, Constants::Board::POSITION_Y))
        {
            return false;
        }

        if (!InitializeControlBlock())
        {
            return false;
        }

        InitializeViews();

        if (interrupt_view_)
        {
            interrupt_view_->SetPosition(Constants::Board::POSITION_X, 0);
        }

#ifdef _APP_DEBUG_
        CreateBlocksFromFile();
#endif

        state_info_ = GameStateInfo{};
        score_info_ = ScoreInfo{};
        game_state_ = GamePhase::Playing;
        prev_game_state_ = GamePhase::Playing;

        state_info_.currentPhase = GamePhase::Playing;
        state_info_.previousPhase = GamePhase::Playing;


        std::array<uint8_t, 2> first_block_types{};
        std::array<uint8_t, 2> second_block_types{};

        auto& first_blocks = next_blocks_[0]->GetBlocks();
        first_block_types[0] = static_cast<uint8_t>(first_blocks[0]->GetBlockType());
        first_block_types[1] = static_cast<uint8_t>(first_blocks[1]->GetBlockType());

        auto& second_blocks = next_blocks_[1]->GetBlocks();
        second_block_types[0] = static_cast<uint8_t>(second_blocks[0]->GetBlockType());
        second_block_types[1] = static_cast<uint8_t>(second_blocks[1]->GetBlockType());

        NETWORK.GameInitialize(first_block_types, second_block_types);

        return true;
    }
    catch (const std::exception& e)
    {
        LOGGER.Error("LocalPlayer initialization failed: {}", e.what());
        return false;
    }
}

void LocalPlayer::InitializeNextBlocks()
{
    auto nextBlock1 = std::make_shared<GroupBlock>();
    auto nextBlock2 = std::make_shared<GroupBlock>();

    if (nextBlock1->Create() == false || nextBlock2->Create() == false)
    {
        throw std::runtime_error("Failed to create next blocks");
    }

    nextBlock1->SetPosXY(Constants::GroupBlock::NEXT_BLOCK_POS_X, Constants::GroupBlock::NEXT_BLOCK_POS_Y);
    nextBlock1->SetScale(Constants::Block::SIZE, Constants::Block::SIZE);
    nextBlock2->SetPosXY(Constants::GroupBlock::NEXT_BLOCK_POS_SMALL_X, Constants::GroupBlock::NEXT_BLOCK_POS_SMALL_Y);
    nextBlock2->SetScale(Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE, Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE);

    next_blocks_.emplace_back(nextBlock1);
    next_blocks_.emplace_back(nextBlock2);

    if (background_)
    {
        background_->Reset();
        background_->SetNextBlock(next_blocks_[0]);
        background_->SetNextBlock(next_blocks_[1]);
    }
}

void LocalPlayer::UpdateGameLogic(float deltaTime)
{
    switch (state_info_.currentPhase)
    {
    case GamePhase::GameOver:
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
    }
}

void LocalPlayer::UpdateIceBlockPhase(float deltaTime)
{
    if (block_list_.size() > 0)
    {
        bool allStationary = true;
        for (const auto& block : block_list_)
        {
            if (block->GetState() != BlockState::Stationary)
            {
                allStationary = false;
                break;
            }
        }

        if (allStationary)
        {
            if (ProcessGameOver() == false)
            {
                state_info_.currentPhase = GamePhase::Playing;
                game_state_ = GamePhase::Playing;
                CreateNextBlock();
            }            
        }
    }
}

void LocalPlayer::UpdateShatteringPhase(float deltaTime)
{
    if (matched_blocks_.empty() == false)
    {
        std::vector<SDL_FPoint> positions;
        std::list<SDL_Point> indexList;

        auto groupIter = matched_blocks_.begin();
        while (groupIter != matched_blocks_.end())
        {
            bool allPlayedOut = std::all_of(groupIter->begin(), groupIter->end(),
                [](const Block* block)
                {
                    return block->GetState() == BlockState::PlayOut;
                });

            if (allPlayedOut)
            {
                if (!groupIter->empty())
                {
                    auto* firstBlock = groupIter->front();

                    CreateBullet(firstBlock);                    
                }

                for (auto* block : *groupIter)
                {
                    SDL_FPoint pos{ block->GetX(), block->GetY() };
                    SDL_Point idx{ block->GetPosIdx_X(), block->GetPosIdx_Y() };

                    CreateBlockClearEffect(std::shared_ptr<Block>(block, [](Block*) {}));

                    board_blocks_[idx.y][idx.x] = nullptr;

                    auto it = std::find_if(block_list_.begin(), block_list_.end(),
                        [block](const std::shared_ptr<Block>& ptr)
                        {
                            return ptr.get() == block;
                        });

                    if (it != block_list_.end())
                    {
                        (*it)->Release();
                        block_list_.erase(it);
                    }

                    indexList.push_back(idx);
                }

                if (combo_view_ && score_info_.comboCount > 0 && !groupIter->empty())
                {
                    auto* firstBlock = groupIter->front();
                    combo_view_->UpdateComboCount(firstBlock->GetX(), firstBlock->GetY(), score_info_.comboCount);
                }

                if (!ice_blocks_.empty())
                {
                    for (const auto& iceBlock : ice_blocks_)
                    {
                        SDL_Point iceIdx{ iceBlock->GetPosIdx_X(), iceBlock->GetPosIdx_Y() };
                        LOGGER.Info("===========> iceblock position {} {}", iceIdx.x, iceIdx.y);
                        board_blocks_[iceIdx.y][iceIdx.x] = nullptr;

                        block_list_.remove(iceBlock);
                        indexList.push_back(iceIdx);
                    }
                    ice_blocks_.clear();
                }

                groupIter = matched_blocks_.erase(groupIter);
            }
            else
            {
                ++groupIter;
            }
        }

        if (matched_blocks_.empty())
        {
            UpdateFallingBlocks(indexList);
        }
    }
    else
    {
        block_list_.sort([](const auto& a, const auto& b) { return *a < *b; });

        if (!block_list_.empty())
        {
            bool allStationary = std::all_of(block_list_.begin(), block_list_.end(),
                [](const auto& block)
                {
                    return block->GetState() == BlockState::Stationary;
                });

            if (allStationary)
            {
                UpdateBlockLinks();

                if (CheckGameBlockState() == false)
                {
                    if (!state_info_.shouldQuit)
                    {
                        if (score_info_.totalInterruptBlockCount > 0 && !state_info_.isComboAttack && !state_info_.isDefending)
                        {
                            GenerateIceBlocks();
                        }
                        else
                        {
                            CreateNextBlock();
                        }
                    }
                }
            }
        }
        else
        {
            if (state_info_.shouldQuit)
            {
                state_info_.currentPhase = GamePhase::GameOver;
                game_state_ = GamePhase::GameOver;
            }
            else
            {
                if (NETWORK.IsRunning())
                {
                    NETWORK.StopComboAttack();
                }

                state_info_.currentPhase = GamePhase::Playing;
                state_info_.previousPhase = GamePhase::Playing;
                game_state_ = GamePhase::Playing;
                prev_game_state_ = GamePhase::Playing;

                if (score_info_.totalInterruptBlockCount > 0 && !state_info_.isComboAttack && !state_info_.isDefending)
                {
                    GenerateIceBlocks();
                }
                else
                {
                    CreateNextBlock();
                }
            }
        }
    }
}

void LocalPlayer::CreateNextBlock()
{
    if (background_ && background_->IsChangingBlock())
    {
        return;
    }

    auto nextBlock = std::make_shared<GroupBlock>();
    if (!nextBlock->Create())
    {
        LOGGER.Error("Failed to create next block");
        return;
    }

    nextBlock->SetPosXY(Constants::GroupBlock::NEXT_BLOCK_POS_SMALL_X, 100);
    nextBlock->SetScale(Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE, Constants::GroupBlock::NEXT_BLOCK_SMALL_SIZE);

    if (background_)
    {
        next_blocks_.emplace_back(nextBlock);
        background_->SetNextBlock(nextBlock);
    }

    if (game_board_)
    {
        game_board_->SetRenderTargetMark(false);
    }

    if (NETWORK.IsRunning())
    {
        if (auto lastBlock = next_blocks_.back().get())
        {
            auto blocks = lastBlock->GetBlocks();
            std::array<uint8_t, 2> blockTypes{
                static_cast<uint8_t>(blocks[0]->GetBlockType()),
                static_cast<uint8_t>(blocks[1]->GetBlockType())
            };

            NETWORK.AddNewBlock(blockTypes);
        }
    }
}

void LocalPlayer::PlayNextBlock()
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
            control_block_->SetScale(Constants::Block::SIZE, Constants::Block::SIZE);

            if (game_board_)
            {
                game_board_->CreateNewBlockInGame(control_block_);
            }

            UpdateTargetPosIdx();
        }
    }
}

bool LocalPlayer::CheckGameBlockState()
{
    if (game_board_)
    {
        game_board_->SetRenderTargetMark(false);
    }

    if (state_info_.shouldQuit)
    {
        if (NETWORK.IsRunning())
        {
            NETWORK.StopComboAttack();
        }
        state_info_.currentPhase = GamePhase::GameOver;
        game_state_ = GamePhase::GameOver;
        return true;
    }

    if (block_list_.size() < Constants::Game::MIN_MATCH_COUNT)
    {
        state_info_.currentPhase = GamePhase::Playing;
        state_info_.previousPhase = GamePhase::Playing;
        game_state_ = GamePhase::Playing;
        prev_game_state_ = GamePhase::Playing;
        ResetComboState();
        return false;
    }

    matched_blocks_.clear();      

    if (FindMatchedBlocks(matched_blocks_))
    {
        state_info_.previousPhase = state_info_.currentPhase;
        prev_game_state_ = game_state_;
        HandlePhaseTransition(GamePhase::Shattering);

        CalculateScore();

        CollectRemoveIceBlocks();

        return true;
    }
    else
    {
        if (state_info_.defenseCount >= 1 && !state_info_.isComboAttack)
        {
            state_info_.isDefending = false;
            state_info_.defenseCount = 0;
        }

        if (score_info_.comboCount > 0)
        {
            score_info_.comboCount = 0;
            if (NETWORK.IsRunning())
            {
                NETWORK.StopComboAttack();
            }
        }

        if (score_info_.restScore > 0)
        {
            score_info_.restScore = 0;
        }
    }

    if (ProcessGameOver() == true)
    {
        return true;
    }

    if (state_info_.shouldQuit)
    {
        state_info_.currentPhase = GamePhase::Standing;
        game_state_ = GamePhase::Standing;
    }
    else
    {
        state_info_.currentPhase = GamePhase::Playing;
        state_info_.previousPhase = GamePhase::Playing;
        game_state_ = GamePhase::Playing;
        prev_game_state_ = GamePhase::Playing;

        if (score_info_.totalInterruptBlockCount > 0 &&
            !state_info_.isComboAttack && !state_info_.isDefending)
        {
            GenerateIceBlocks();
        }
        else
        {
            CreateNextBlock();
        }
    }

    return false;
}

bool LocalPlayer::FindMatchedBlocks(std::vector<std::vector<Block*>>& matchedGroups)
{
    std::vector<Block*> currentGroup;
    int blockCount = 0;
    auto blockListSize = block_list_.size();

    for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; y++)
    {
        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++)
        {
            if (blockCount == blockListSize)
            {
                break;
            }

            Block* block = board_blocks_[y][x];
            if (block == nullptr)
            {
                continue;
            }
            else
            {
                blockCount++;
            }

            if (block->GetBlockType() == BlockType::Ice || block->IsRecursionCheck() == true)
            {
                continue;
            }

            block->SetRecursionCheck(true);
            currentGroup.clear();

            if (RecursionCheckBlock(x, y, Constants::Direction::None, currentGroup) >= Constants::Game::MIN_MATCH_COUNT - 1)
            {
                currentGroup.push_back(block);
                matchedGroups.push_back(currentGroup);
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

    return !matchedGroups.empty();
}

short LocalPlayer::RecursionCheckBlock(short x, short y, Constants::Direction direction, std::vector<Block*>& matchedBlocks)
{
    if (!board_blocks_[y][x])
    {
        return 0;
    }

    Block* block = board_blocks_[y][x];
    BlockType blockType = block->GetBlockType();
    short matchCount = 0;

    // �� ���� �˻�
    const std::array<std::pair<Constants::Direction, std::pair<short, short>>, 4> directions =
    { {
        {Constants::Direction::Left,   {x - 1, y}},
        {Constants::Direction::Right,  {x + 1, y}},
        {Constants::Direction::Top,    {x, y + 1}},
        {Constants::Direction::Bottom, {x, y - 1}}
    } };

    for (const auto& [dir, pos] : directions)
    {
        if (direction == dir)
        {
            continue;
        }

        const auto [checkX, checkY] = pos;
        if (checkX < 0 || checkX >= Constants::Board::BOARD_X_COUNT ||
            checkY < 0 || checkY >= Constants::Board::BOARD_Y_COUNT)
        {
            continue;
        }

        Block* checkBlock = board_blocks_[checkY][checkX];
        if (!checkBlock || checkBlock->IsRecursionCheck() ||  checkBlock->GetState() != BlockState::Stationary)
        {
            continue;
        }

        if (blockType == checkBlock->GetBlockType())
        {
            checkBlock->SetRecursionCheck(true);
            matchedBlocks.push_back(checkBlock);
            matchCount++;
            matchCount += RecursionCheckBlock(checkX, checkY, GameStateDetail::GetOppositeDirection(dir), matchedBlocks);
        }
    }

    return matchCount;
}

void LocalPlayer::CalculateScore()
{
    if (state_info_.previousPhase == GamePhase::Shattering)
    {
        score_info_.comboCount++;
    }
    else if (state_info_.previousPhase == GamePhase::Playing)
    {
        score_info_.comboCount = 1;
    }

    uint8_t linkBonus = 0;
    uint8_t blockCount = 0;
    uint8_t typeBonus = GetTypeBonus(matched_blocks_.size());
    short comboBonus = GetComboConstant(score_info_.comboCount);

    for (const auto& group : matched_blocks_)
    {
        for (auto* block : group)
        {
            block->SetState(BlockState::Destroying);
        }

        linkBonus += GetLinkBonus(group.size());
        blockCount += static_cast<uint8_t>(group.size());
    }

    int currentScore = ((blockCount * Constants::Game::Score::BASE_MATCH_SCORE) *
        (comboBonus + linkBonus + typeBonus + 1));

    score_info_.addInterruptBlockCount = (currentScore + score_info_.restScore) / GetMargin();
    score_info_.restScore = (currentScore + score_info_.restScore) % GetMargin();
    score_info_.totalScore += currentScore;

    UpdateInterruptBlockState();
}

void LocalPlayer::UpdateInterruptBlockState()
{
    if (score_info_.totalInterruptBlockCount > 0)
    {
        score_info_.totalInterruptBlockCount -= score_info_.addInterruptBlockCount;

        if (score_info_.totalInterruptBlockCount < 0)
        {
            score_info_.addInterruptBlockCount = std::abs(score_info_.totalInterruptBlockCount);
            state_info_.hasIceBlock = false;
            score_info_.totalInterruptBlockCount = 0;

            if (NETWORK.IsServer())
            {
                if (score_info_.addInterruptBlockCount > 0)
                {
                    score_info_.totalEnemyInterruptBlockCount += score_info_.addInterruptBlockCount;
                }
            }
        }
        else
        {
            state_info_.hasIceBlock = true;
        }
    }
    else
    {
        if (NETWORK.IsServer())
        {
            if (score_info_.addInterruptBlockCount > 0)
            {
                score_info_.totalEnemyInterruptBlockCount += score_info_.addInterruptBlockCount;
            }
        }
        state_info_.hasIceBlock = false;
        score_info_.totalInterruptBlockCount = 0;
    }
}

void LocalPlayer::GenerateIceBlocks()
{
    if (score_info_.totalInterruptBlockCount <= 0 || state_info_.currentPhase != GamePhase::Playing)
    {
        return;
    }

    auto texture = ImageTexture::Create("PUYO/puyo_beta.png");
    if (!texture)
    {
        LOGGER.Error("Failed to get ice block texture");
        return;
    }

    const auto playerID = player_id_;

    if (score_info_.totalInterruptBlockCount > 30)
    {
        GenerateLargeIceBlockGroup(texture, playerID);
    }
    else
    {
        GenerateSmallIceBlockGroup(texture, playerID);
    }

    if (interrupt_view_)
    {
        interrupt_view_->UpdateInterruptBlock(score_info_.totalInterruptBlockCount);
    }

    HandlePhaseTransition(GamePhase::IceBlocking);

    state_info_.defenseCount = 0;
}

void LocalPlayer::GenerateLargeIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID)
{
    score_info_.totalInterruptBlockCount -= 30;

    if (NETWORK.IsRunning())
    {
        NETWORK.AddInterruptBlock(5, 0, std::span<const uint8_t>());
    }

    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++)
        {
            auto iceBlock = std::make_shared<IceBlock>();
            InitializeIceBlock(iceBlock.get(), texture, x, y, playerID);
            block_list_.push_back(iceBlock);
        }
    }
}

void LocalPlayer::GenerateSmallIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID)
{
    const auto yCnt = score_info_.totalInterruptBlockCount / Constants::Board::BOARD_X_COUNT;
    const auto xCnt = score_info_.totalInterruptBlockCount % Constants::Board::BOARD_X_COUNT;

    for (int y = 0; y < yCnt; y++)
    {
        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++)
        {
            auto iceBlock = std::make_shared<IceBlock>();
            InitializeIceBlock(iceBlock.get(), texture, x, y, playerID);
            block_list_.push_back(iceBlock);
        }
    }

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
            xIdxList[idx++] = static_cast<uint8_t>(pos);
        }

        if (NETWORK.IsRunning())
        {
            NETWORK.AddInterruptBlock(static_cast<uint8_t>(yCnt), static_cast<uint8_t>(xCnt), std::span<const uint8_t>(xIdxList));
        }
    }

    score_info_.totalInterruptBlockCount = 0;
}

void LocalPlayer::InitializeIceBlock(IceBlock* block, const std::shared_ptr<ImageTexture>& texture, int x, int y, uint8_t playerID)
{
    if (!block)
    {
        return;
    }

    block->SetBlockType(BlockType::Ice);
    block->SetLinkState(LinkState::Max);
    block->SetState(BlockState::DownMoving);
    block->SetBlockTex(texture);
    block->SetPosIdx(x, y);

    float renderX = Constants::Board::WIDTH_MARGIN + Constants::Block::SIZE * x;
    float renderY = -Constants::Block::SIZE * (y + 1);
    
    block->SetPosition(renderX, renderY);
    block->SetScale(Constants::Block::SIZE, Constants::Block::SIZE);
    block->SetPlayerID(playerID);
}

void LocalPlayer::CollectRemoveIceBlocks()
{
    if (block_list_.empty() || matched_blocks_.empty() || state_info_.currentPhase != GamePhase::Shattering)
    {
        return;
    }

    for (const auto& group : matched_blocks_)
    {
        for (auto* block : group)
        {
            if (block == nullptr)
            {
                continue;
            }

            const int x = block->GetPosIdx_X();
            const int y = block->GetPosIdx_Y();

            const std::array<std::pair<int, int>, 4> directions =
            { {
                {x - 1, y}, {x + 1, y}, {x, y - 1}, {x, y + 1}
            } };

            for (const auto& [checkX, checkY] : directions)
            {
                if (checkX >= 0 && checkX < Constants::Board::BOARD_X_COUNT &&
                    checkY >= 0 && checkY < Constants::Board::BOARD_Y_COUNT)
                {
                    if (auto checkBlock = board_blocks_[checkY][checkX])
                    {
                        if (checkBlock->GetBlockType() == BlockType::Ice &&
                            checkBlock->GetState() == BlockState::Stationary)
                        {
                            if (auto iceBlock = dynamic_cast<IceBlock*>(checkBlock))
                            {
                                iceBlock->SetState(BlockState::Destroying);

                                auto found = std::find_if(block_list_.begin(), block_list_.end(),
                                    [iceBlock](const auto& block) {
                                        return block.get() == iceBlock;
                                    });

                                if (found != block_list_.end()) 
                                {
                                    ice_blocks_.insert(std::static_pointer_cast<IceBlock>(*found));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void LocalPlayer::MoveBlock(uint8_t moveType, float position)
{
    if (!control_block_ || control_block_->GetState() != BlockState::Playing)
    {
        return;
    }

    switch (static_cast<Constants::Direction>(moveType))
    {
    case Constants::Direction::Left:
        control_block_->MoveLeft();
        break;

    case Constants::Direction::Right:
        control_block_->MoveRight();
        break;

    case Constants::Direction::Bottom:
        if (control_block_->GetAddForceVelocityY() <= 70.0f)
        {
            control_block_->ForceAddVelocityY(10.0f);
        }
        break;

    default:
        break;
    }
}

void LocalPlayer::RotateBlock(uint8_t rotateType, bool horizontalMoving)
{
    if (control_block_ && control_block_->GetState() == BlockState::Playing)
    {
        control_block_->Rotate();
    }
}

void LocalPlayer::UpdateBlockPosition(float pos1, float pos2)
{
}

void LocalPlayer::UpdateFallingBlock(uint8_t fallingIdx, bool falling)
{
    if (control_block_ && control_block_->GetState() == BlockState::Playing)
    {
    }
}

void LocalPlayer::ChangeBlockState(uint8_t state)
{
    if (control_block_)
    {
        control_block_->SetState(static_cast<BlockState>(state));
    }
}

bool LocalPlayer::PushBlockInGame(GameGroupBlock* groupBlock)
{
    if (!groupBlock)
    {
        LOGGER.Error("PushBlockInGame called with null block");
        return false;
    }

    const auto& blocks = groupBlock->GetBlocks();

    for (const auto& currentBlock : blocks)
    {
        if (currentBlock)
        {
            block_list_.push_back(currentBlock);

            int xIdx = currentBlock->GetPosIdx_X();
            int yIdx = currentBlock->GetPosIdx_Y();
            board_blocks_[yIdx][xIdx] = currentBlock.get();

            UpdateLinkState(currentBlock.get());
        }
    }

    block_list_.sort([](const auto& a, const auto& b) { return *a < *b; });

    if (game_board_)
    {
        game_board_->ClearActiveGroupBlock();
    }

    if (CheckGameBlockState() == false && state_info_.currentPhase == GamePhase::Playing)
    {
        if (state_info_.isDefending)
        {
            state_info_.isDefending = false;
            state_info_.defenseCount = 0;
        }

        if (score_info_.totalInterruptBlockCount > 0 &&
            !state_info_.isComboAttack && !state_info_.isDefending)
        {
            GenerateIceBlocks();
        }
        else
        {
            CreateNextBlock();
        }
    }
    else
    {
        if (state_info_.hasIceBlock)
        {
            if (state_info_.defenseCount >= 1 && !state_info_.isComboAttack)
            {
                state_info_.isDefending = false;
                state_info_.defenseCount = 0;
                return true;
            }

            state_info_.defenseCount++;
            state_info_.isDefending = true;
        }
    }

    groupBlock->ResetBlock();
    return true;
}

void LocalPlayer::UpdateComboState()
{
    if (state_info_.previousPhase == GamePhase::Shattering)
    {
        score_info_.comboCount++;
    }
    else if (state_info_.previousPhase == GamePhase::Playing)
    {
        score_info_.comboCount = 1;
    }
}

void LocalPlayer::ResetComboState()
{
    if (score_info_.comboCount > 0)
    {
        score_info_.comboCount = 0;
        if (NETWORK.IsRunning())
        {
            NETWORK.StopComboAttack();
        }
    }

    if (score_info_.restScore > 0)
    {
        score_info_.restScore = 0;
    }
}

void LocalPlayer::AttackInterruptBlock(float x, float y, uint8_t type)
{
    NETWORK.AttackInterruptBlock(score_info_.addInterruptBlockCount, x, y, type);
}

void LocalPlayer::DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type)
{
    score_info_.totalEnemyInterruptBlockCount -= count;

    if (score_info_.totalEnemyInterruptBlockCount < 0)
    {
        score_info_.totalEnemyInterruptBlockCount = 0;
    }
       
    NotifyEvent(std::make_shared<DefenseBlockEvent>(count, x, y, type));
}

void LocalPlayer::HandlePhaseTransition(GamePhase newPhase)
{
    if (state_info_.currentPhase == newPhase)
    {
        return;
    }

    state_info_.previousPhase = state_info_.currentPhase;
    state_info_.currentPhase = newPhase;

    prev_game_state_ = game_state_;
    game_state_ = newPhase;
}

bool LocalPlayer::Restart(const std::span<const uint8_t>& blockType1, const std::span<const uint8_t>& blockType2)
{
    Reset();

    try {
        InitializeNextBlocks();

        if (!InitializeGameBoard(Constants::Board::POSITION_X, Constants::Board::POSITION_Y))
        {
            LOGGER.Error("Failed to initialize game board during restart");
            return false;
        }

        if (!InitializeControlBlock())
        {
            LOGGER.Error("Failed to initialize control block during restart");
            return false;
        }

        if (interrupt_view_) interrupt_view_->Initialize();
        if (combo_view_) combo_view_->Initialize();
        if (result_view_) result_view_->Initialize();

        state_info_ = GameStateInfo{};
        score_info_ = ScoreInfo{};
        state_info_.currentPhase = GamePhase::Playing;
        state_info_.previousPhase = GamePhase::Playing;
        game_state_ = GamePhase::Playing;
        prev_game_state_ = GamePhase::Playing;

        NotifyEvent(std::make_shared<GameRestartEvent>(player_id_));

        return true;
    }
    catch (const std::exception& e) {
        LOGGER.Error("Error restarting local player: {}", e.what());
        return false;
    }
}

void LocalPlayer::UpdateTargetPosIdx()
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

    float renderPos = (Constants::Block::SIZE - Constants::Board::BLOCK_MARK_SIZE) / 2.0f;

    for (int i = 0; i < 2; ++i)
    {
        if (const auto& block = blocks[currentIndex])
        {
            int xIdx = block->GetPosIdx_X();
            BlockType blockType = block->GetBlockType();

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

            currentIndex = (rotateState == RotateState::Default) ? 0 : 1;
        }
    }

    if (game_board_)
    {
        game_board_->UpdateTargetBlockMark(markPositions);
    }
}

void LocalPlayer::CreateBullet(Block* block)
{
    if (!block)
    {
        LOGGER.Error("LocalPlayer::CreateBullet - block is NULL");
        return;
    }

    SDL_FPoint startPos
    {
        Constants::Board::POSITION_X + Constants::Board::WIDTH_MARGIN + block->GetX() + Constants::Block::SIZE / 2,
        Constants::Board::POSITION_Y + block->GetY() + Constants::Block::SIZE / 2
    };

    SDL_FPoint endPos;

    if (state_info_.hasIceBlock)
    {
        endPos =
        {
            Constants::Board::POSITION_X + (Constants::Board::WIDTH / 2),
            Constants::Board::POSITION_Y
        };

        NETWORK.DefenseInterruptBlock(
            score_info_.addInterruptBlockCount,
            block->GetX(),
            block->GetY(),
            static_cast<uint8_t>(block->GetBlockType())
        );

        // 플레이어 방해블록개수 갱신
        if (interrupt_view_)
        {
            interrupt_view_->UpdateInterruptBlock(score_info_.totalInterruptBlockCount);
        }
    }
    else
    {  
        endPos =
        {
            GAME_APP.GetWindowWidth() - (Constants::Board::POSITION_X + (Constants::Board::WIDTH / 2)),
            Constants::Board::POSITION_Y
        };

        // 플레이어 방해블록개수 갱신
        NETWORK.AttackInterruptBlock(
            score_info_.addInterruptBlockCount,
            block->GetX(),
            block->GetY(),
            static_cast<uint8_t>(block->GetBlockType())
        );


        if (NETWORK.IsServer())
        {
            NotifyEvent(std::make_shared<AddInterruptBlockEvent>(player_id_, score_info_.addInterruptBlockCount));
        }        

        if (interrupt_view_)
        {
            interrupt_view_->UpdateInterruptBlock(score_info_.totalInterruptBlockCount);
        }
    }

    score_info_.addInterruptBlockCount = 0;

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

void LocalPlayer::Release()
{
    bullets_to_delete_.clear();
    matched_blocks_.clear();

    ReleaseContainer(ice_blocks_);    
    ReleaseContainer(next_blocks_);    

    BasePlayer::Release();
}

void LocalPlayer::Reset()
{
    bullets_to_delete_.clear();
    matched_blocks_.clear();

    ReleaseContainer(ice_blocks_);
    ReleaseContainer(next_blocks_);    

    BasePlayer::Reset();
}

bool LocalPlayer::IsGameOver() const
{
    bool isGameOverState = 
        board_blocks_[Constants::Board::BOARD_Y_COUNT - 1][2] != nullptr ||
        board_blocks_[Constants::Board::BOARD_Y_COUNT - 1][3] != nullptr ||
        board_blocks_[Constants::Board::BOARD_Y_COUNT - 2][2] != nullptr ||
        board_blocks_[Constants::Board::BOARD_Y_COUNT - 2][3] != nullptr;

    return isGameOverState;
}

bool LocalPlayer::ProcessGameOver()
{
    if (IsGameOver() == true)
    {
        if (NETWORK.IsRunning())
        {
            NETWORK.LoseGame();
        }

        LoseGame(false);

        NotifyEvent(std::make_shared<GameOverEvent>(player_id_, true));

		state_info_.currentPhase = GamePhase::GameOver;
		game_state_ = GamePhase::GameOver;

		return true;
    }

    return false;
}

void LocalPlayer::AddInterruptBlockCount(int16_t count, float x, float y, uint8_t type)
{
    if (NETWORK.IsServer())
    {
		state_info_.isComboAttack = true;
        AddInterruptBlock(count);		

        score_info_.totalEnemyInterruptBlockCount = 0;
		
        NotifyEvent(std::make_shared<AttackInterruptBlockEvent>(player_id_, x, y, type));        
    }
}