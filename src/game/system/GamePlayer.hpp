#pragma once
/**
 *
 * 설명: 네트워크 플레이어 관련
 *
 */
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <memory>
#include <span>
#include "../RenderableObject.hpp"
#include "../../core/common/constants/Constants.hpp"
#include "../../states/GameState.hpp"

class Block;
class GameBackGround;
class ImageTexture;
class GroupBlock;
class GameBoard;
class GameGroupBlock;
class IceBlock;
class BulletEffect;
class InterruptBlockView;
class ComboView;
class ResultView;

class GamePlayer : public RenderableObject
{
    using BlockVector = std::vector<std::shared_ptr<Block>>;

public:

    GamePlayer();
    ~GamePlayer() override;

    GamePlayer(const GamePlayer&) = delete;
    GamePlayer& operator=(const GamePlayer&) = delete;
    GamePlayer(GamePlayer&&) noexcept = delete;
    GamePlayer& operator=(GamePlayer&&) noexcept = delete;

    void Update(float delta_time) override;
    void Render() override;
    void Release() override;
    bool Initialize(const std::span<const uint8_t>& blocktype1, const std::span<const uint8_t>& blocktype2, uint8_t playerIdx, uint8_t characterIdx, 
        const std::shared_ptr<GameBackground>& background);
    bool Restart(const std::span<const uint8_t>& block_type1, const std::span<const uint8_t>& block_type2);

    void AddNewBlock(const std::span<const uint8_t, 2>& block_type);
    void DestroyNextBlock();
    bool PushBlockInGame(std::span<float> pos1, std::span<float> pos2);
    void MoveBlock(uint8_t move_type, float position);
    void RotateBlock(uint8_t rotate_type, bool horizontal_moving);
    void UpdateBlockPosition(float pos1, float pos2);    
    void UpdateFallingBlock(uint8_t falling_idx, bool falling);
    void ChangeBlockState(uint8_t state);
    bool CheckGameBlockState();

    void AddInterruptBlock(int16_t count);
    void AddInterruptBlock(uint8_t y_row_cnt, std::span<uint8_t> x_idx);
    void AttackInterruptBlock(float x, float y, uint8_t type);
    void DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type);
    void UpdateInterruptBlock(int16_t count);
    void CollectRemoveIceBlocks();
    void CalculateIceBlockCount();

    void LoseGame(bool is_win);
    void SetGameQuit() { is_game_quit_ = true; }

    [[nodiscard]] int16_t GetComboConstant(uint8_t combo_count) const;
    [[nodiscard]] uint8_t GetLinkBonus(uint8_t link_count) const;
    [[nodiscard]] uint8_t GetTypeCntBonus(uint8_t count) const;
    [[nodiscard]] uint8_t GetMargin() const;

    void SetCharacterID(int16_t charID) { _characterID = charID; }
    [[nodiscard]] int16_t GetCharacterID() { return _characterID; }

    Block* (*GetGameBlocks())[Constants::Board::BOARD_X_COUNT] { return board_blocks_; }

private:

    void Reset();
    void UpdateLinkState(Block* block);
    void CreateBullet(Block* block);
    void UpdateBullets(float delta_time);
    void CreateBlocksFromFile();
    void CreateBlockClearEffect(const std::shared_ptr<Block>& block);
    int16_t RecursionCheckBlock(int16_t x, int16_t y, int16_t direction, std::vector<Block*>& block_list);

    void InitializeNextBlocks(const std::span<const uint8_t>& blocktype1, const std::span<const uint8_t>& blocktype2);
    void InitializeGameBoard();
    void InitializeViews();
    void InitializeControlBlock();

    void HandleMatchedBlocks();
    void HandleClearedBlockGroup(std::list<BlockVector>::iterator& group_it, SDL_FPoint& pos, SDL_Point& pos_idx, std::list<SDL_Point>& x_index_list);
    void ResetMatchState();
    void CreateFullRowInterruptBlocks(std::shared_ptr<ImageTexture>& texture);
    void CreatePartialRowInterruptBlocks(uint8_t y_row_cnt, std::span<uint8_t> x_idx, std::shared_ptr<ImageTexture>& texture);
    void CreateSingleIceBlock(int x, int y, std::shared_ptr<ImageTexture>& texture);
    void CollectAdjacentIceBlocks(const std::shared_ptr<Block>& block);

    void UpdateGameState(float delta_time);
    void UpdateStandingState(float delta_time);
    void UpdatePlayingState(float delta_time);
    void UpdateIceBlockDowningState();
    void UpdateFallingBlocks(const std::list<SDL_Point>& x_index_list);
    void UpdateShatteringState();
    void UpdateAfterBlocksCleared();
    void UpdateBlockLinks();
    void UpdateMatchedBlocks();
    void UpdateComboDisplay(const SDL_FPoint& pos);

    void RemoveBlock(const std::shared_ptr<Block>& block, const SDL_Point& pos_idx);
    void RemoveIceBlocks(std::list<SDL_Point>& x_index_list);

private:
    
    bool is_running_{ false };
    bool has_ice_block_{ false };
    bool is_game_quit_{ false };
    GamePhase game_state_{ GamePhase::Standing };
    GamePhase prev_game_state_{ GamePhase::Standing };
    float play_time_{ 0.0f };

    uint32_t total_score_{ 0 };
    uint32_t rest_score_{ 0 };
    uint8_t combo_count_{ 0 };
    int16_t total_interrupt_block_count_{ 0 };
    int16_t _characterID{ 0 };

    std::shared_ptr<GameBoard> game_board_;
    std::shared_ptr<InterruptBlockView> interrupt_block_view_;
    std::shared_ptr<ComboView> combo_view_;
    std::shared_ptr<ResultView> result_view_;
    std::shared_ptr<GameGroupBlock> control_block_;
    std::shared_ptr<GameBackground> background_;

    std::vector<RenderableObject*> draw_objects_;
    std::deque<std::shared_ptr<GroupBlock>> new_blocks_;
    std::list<std::shared_ptr<Block>> block_list_;
    std::set<std::shared_ptr<IceBlock>> ice_block_set_;
    std::list<std::shared_ptr<BulletEffect>> bullet_list_;
    std::vector<std::shared_ptr<BulletEffect>> del_bullet_array_;
    
    
    std::list<BlockVector> equal_block_list_;
    Block* board_blocks_[Constants::Board::BOARD_Y_COUNT][Constants::Board::BOARD_X_COUNT]{ nullptr };
};