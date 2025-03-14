#include "GroupBlock.hpp"
#include "../../core/manager/ResourceManager.hpp"
#include "../../core/manager/PlayerManager.hpp"
#include "../../states/GameState.hpp"
#include "../../core/common/constants/Constants.hpp"
#include "../../core/GameApp.hpp"
#include "../../network/player/Player.hpp"
#include <random>
#include <functional>
#include <cassert>

GroupBlock::GroupBlock()
    : RenderableObject()
{    
    InitializeBlocks();
    SetScale(Constants::Block::SIZE, Constants::Block::SIZE);
}

void GroupBlock::InitializeBlocks() 
{
    for (auto& block : blocks_) 
    {
        block = std::make_shared<Block>();
    }
}

bool GroupBlock::Create() 
{
    try 
    {
        InitializeBlocks();

        auto texture = ImageTexture::Create("PUYO/puyo_beta.png");
        if (texture == nullptr) 
        {
            throw std::runtime_error("Failed to load block texture");
        }

        std::random_device rd;
        std::mt19937 randEngine(rd());
        std::uniform_int_distribution<int> distribution(static_cast<int>(BlockType::Red), static_cast<int>(BlockType::Purple));

        for (auto& block : blocks_) 
        {
            if (!block) 
            {
                throw std::runtime_error("Block initialization failed");
            }

#ifdef _APP_DEBUG_
            block->SetBlockType(BlockType::Purple);
#else
            block->SetBlockType(static_cast<BlockType>(distribution(randEngine)));
#endif

            block->SetBlockTex(texture);
            block->SetPlayerID(GAME_APP.GetPlayerManager().GetMyPlayer()->GetId());
        }

        return true;
    }
    catch (const std::exception&) 
    {    
        blocks_.fill(nullptr);
        return false;
    }
}

bool GroupBlock::Create(BlockType type1, BlockType type2) {
    try 
    {
        InitializeBlocks();

        auto texture = ImageTexture::Create("PUYO/puyo_beta.png");
        if (texture == nullptr) 
        {
            throw std::runtime_error("Failed to load block texture");
        }

        blocks_[0]->SetBlockType(type1);
        blocks_[1]->SetBlockType(type2);

        for (auto& block : blocks_) 
        {
            block->SetBlockTex(texture);
        }

        return true;
    }
    catch (const std::exception&) 
    {
        // Log error and cleanup
        blocks_.fill(nullptr);
        return false;
    }
}

void GroupBlock::Update(float deltaTime)
{
    for (auto& block : blocks_) 
    {
        if (block) 
        {
            block->Update(deltaTime);
        }
    }
}

void GroupBlock::Render() 
{
    if (!is_visible_) return;

    for (auto& block : blocks_) 
    {
        if (block) 
        {
            block->Render();
        }
    }
}

void GroupBlock::Release() 
{
    for (auto& block : blocks_) 
    {
        if (block) 
        {
            block->Release();
        }
    }
    blocks_.fill(nullptr);
}

void GroupBlock::SetState(BlockState state) 
{
    state_ = state;

    if (blocks_[0] && blocks_[1]) 
    {
        blocks_[0]->SetState(state);
        blocks_[1]->SetState(state);

        if (state_ == BlockState::Playing) 
        {
            blocks_[0]->SetStandard(true);
        }
    }
}

Block* GroupBlock::GetBlock(int index) 
{
    if (!ValidateBlockIndex(index)) 
    {
        return nullptr;
    }
    return blocks_[index].get();
}

bool GroupBlock::ValidateBlockIndex(int index) const 
{
    return index >= 0 && index < static_cast<int>(Constants::GroupBlock::COUNT);
}


void GroupBlock::SetPosXY(float x, float y)
{
    position_.x = x;
    position_.y = y;

    if (blocks_[0]) 
    {
        blocks_[0]->SetPosition(position_.x, position_.y);
    }
    if (blocks_[1]) 
    {
        blocks_[1]->SetPosition(position_.x, position_.y + size_.y);
    }

    UpdateDestRect();
}

void GroupBlock::SetPosX(float x) 
{
    position_.x = x;

    if (blocks_[0]) 
    {
        blocks_[0]->SetPosition(position_.x, position_.y);
    }
    
    if (blocks_[1]) 
    {
        blocks_[1]->SetPosition(position_.x, position_.y + size_.y);
    }

    UpdateDestRect();
}

void GroupBlock::SetPosY(float y) 
{
    position_.y = y;

    if (blocks_[0]) 
    {
        blocks_[0]->SetPosition(position_.x, position_.y);
    }
    
    if (blocks_[1]) 
    {
        blocks_[1]->SetPosition(position_.x, position_.y + size_.y);
    }

    UpdateDestRect();
}

void GroupBlock::SetScale(float width, float height)
{
    size_.x = width;
    size_.y = height;

    for (auto& block : blocks_) 
    {
        if (block) 
        {
            block->SetScale(width, height);
        }
    }

    if (blocks_[0]) blocks_[0]->SetPosition(position_.x, position_.y);
    if (blocks_[1]) blocks_[1]->SetPosition(position_.x, position_.y + size_.y);

    UpdateDestRect();
}

void GroupBlock::UpdateDestRect() 
{
    if (!blocks_[0] || !blocks_[1])
    {
        return;
    }

    float minX = std::min<float>(blocks_[0]->GetX(), blocks_[1]->GetX());
    float minY = std::min<float>(blocks_[0]->GetY(), blocks_[1]->GetY());

    destination_rect_.x = minX;
    destination_rect_.y = minY;

    bool sameX = std::abs(blocks_[0]->GetX() - blocks_[1]->GetX()) < std::numeric_limits<float>::epsilon();
    bool sameY = std::abs(blocks_[0]->GetY() - blocks_[1]->GetY()) < std::numeric_limits<float>::epsilon();

    if (sameX) 
    {
        destination_rect_.w = size_.x;
        destination_rect_.h = size_.y * 2;
    }
    else if (sameY) 
    {
        destination_rect_.w = size_.x * 2;
        destination_rect_.h = size_.y;
    }
    else 
    {
        // 대각선 배치의 경우
        destination_rect_.w = size_.x * 2;
        destination_rect_.h = size_.y * 2;
    }
}