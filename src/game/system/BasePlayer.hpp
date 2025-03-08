#pragma once
/**
 *
 * ����: ���� �÷��̾� �⺻ Ŭ���� (���� ��� ����)
 *
 */
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <memory>
#include <array>
#include <span>
#include "../RenderableObject.hpp"
#include "../../core/common/constants/Constants.hpp"
#include "../../core/common/types/GameTypes.hpp"
#include "../../states/GameState.hpp"
#include "../event/PlayerEvent.hpp"

class Block;
class GameBackground;
class GroupBlock;
class GameGroupBlock;
class IceBlock;
class BulletEffect;
class GameBoard;
class InterruptBlockView;
class ComboView;
class ResultView;
class ImageTexture;
class IPlayerEventListener;

class BasePlayer : public RenderableObject {
public:
    BasePlayer();
    virtual ~BasePlayer();

    // �⺻ �������̽�
    void Update(float deltaTime) override;
    void Render() override;
    virtual void Reset();
    virtual void Release();

    template<typename Container>
    void ReleaseContainer(Container& container);

    // �ʱ�ȭ �� �����
    virtual bool Initialize(const std::span<const uint8_t>& blocktype1,
        const std::span<const uint8_t>& blocktype2,
        uint8_t playerIdx,
        uint16_t characterIdx,
        const std::shared_ptr<GameBackground>& background) = 0;

    virtual bool Restart(const std::span<const uint8_t>& blockType1 = {}, const std::span<const uint8_t>& blockType2 = {}) = 0;

    // ��� ����
    virtual void CreateNextBlock() = 0;
    virtual void PlayNextBlock() = 0;
    virtual bool CheckGameBlockState() = 0;

    // ��� ����
    virtual void MoveBlock(uint8_t moveType, float position) = 0;
    virtual void RotateBlock(uint8_t rotateType, bool horizontalMoving) = 0;
    virtual void UpdateBlockPosition(float pos1, float pos2) = 0;
    virtual void UpdateFallingBlock(uint8_t fallingIdx, bool falling) = 0;
    virtual void ChangeBlockState(uint8_t state) = 0;

    // ���� ��� ����
    virtual void AddInterruptBlock(int16_t count);
    virtual void AttackInterruptBlock(float x, float y, uint8_t type) = 0;
    virtual void DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type) = 0;
    virtual void UpdateInterruptBlock(int16_t count);
    virtual void CollectRemoveIceBlocks() = 0;

    // ���� ���� ����
    virtual void LoseGame(bool isWin);
    virtual void SetGameQuit() { is_game_quit_ = true; }

    // ���� ��ȸ
    GamePhase GetGameState() const { return game_state_; }
    uint8_t GetPlayerID() const { return player_id_; }
    int16_t GetTotalInterruptBlockCount() const { return total_interrupt_block_count_; }
    int16_t GetTotalEnemyInterruptBlockCount() const { return total_enemy_interrupt_block_count_; }
    std::shared_ptr<GameBoard> GetGameBoard() const { return game_board_; }
    Block* (*GetGameBlocks())[Constants::Board::BOARD_X_COUNT] { return board_blocks_; }

    bool IsRunning() const { return state_info_.isRunning; }
    void SetRunning(bool running) { state_info_.isRunning = running; }

    [[nodiscard]] bool IsPossibleMove(int xIdx);


    const std::shared_ptr<InterruptBlockView>& GetInterruptView() { return interrupt_view_; }
    // ������Ʈ ����
    void SetInterruptView(const std::shared_ptr<InterruptBlockView>& view) { interrupt_view_ = view; }
    void SetComboView(const std::shared_ptr<ComboView>& view) { combo_view_ = view; }
    void SetResultView(const std::shared_ptr<ResultView>& view) { result_view_ = view; }
    void SetGameBoard(const std::shared_ptr<GameBoard>& board) { game_board_ = board; }
    void SetBackGround(const std::shared_ptr<GameBackground>& backGround) { background_ = backGround; }

    void SetComboAttackState(bool enable) { state_info_.isComboAttack = enable; }
    void SetTotalInterruptBlockCount(uint16_t count) { score_info_.totalInterruptBlockCount += count; }


    // ������ ���/���� �޼���
    void AddEventListener(IPlayerEventListener* listener);
    void RemoveEventListener(IPlayerEventListener* listener);

protected:
    // �ʱ�ȭ ���� �޼���
    virtual bool InitializeViews();
    virtual bool InitializeGameBoard(float posX, float posY);
    virtual bool InitializeControlBlock();

    // ��� ���� ���� �޼���
    void UpdateLinkState(Block* block);
    void CreateBlockClearEffect(const std::shared_ptr<Block>& block);

    // ��� ���� ���� �޼���
    void RemoveBlock(Block* block, const SDL_Point& pos_idx);
    void UpdateFallingBlocks(const std::list<SDL_Point>& x_index_list);
    void UpdateBlockLinks();

    // ���� ��� ���� �޼���
    int16_t GetComboConstant(uint8_t combo_count) const;
    uint8_t GetLinkBonus(size_t link_count) const;
    uint8_t GetTypeBonus(size_t count) const;
    uint8_t GetMargin() const;

    // �Ѿ� �� ����Ʈ ���� �޼���
    virtual void CreateBullet(Block* block);
    void UpdateBullets(float delta_time);

    void NotifyEvent(const std::shared_ptr<BasePlayerEvent>& event);


    // ��� ���Ϸ� ���� ����
    void CreateBlocksFromFile();

    // ���� ���� ���� ����ü
    struct GameStateInfo 
{
        GamePhase currentPhase{ GamePhase::Standing };
        GamePhase previousPhase{ GamePhase::Standing };
        float playTime{ 0.0f };
        bool isRunning{ false };
        bool isDefending{ false };
        bool isAttacked{ false };
        bool isComboAttack{ false };
        bool shouldQuit{ false };
        uint8_t defenseCount{ 0 };
        bool hasIceBlock{ false };  // ���� ��� ���� ����
    };

    // ���� ���� ����ü
    struct ScoreInfo 
    {
        uint32_t totalScore{ 0 };
        uint32_t restScore{ 0 };
        uint8_t comboCount{ 0 };
        int16_t totalInterruptBlockCount{ 0 };
        int16_t totalEnemyInterruptBlockCount{ 0 };
        int16_t addInterruptBlockCount{ 0 };

        void reset() {
            totalScore = restScore = 0;
            comboCount = 0;
            totalInterruptBlockCount = 0;
            totalEnemyInterruptBlockCount = 0;
            addInterruptBlockCount = 0;
        }
    };

protected:
    // ���� ���� ����
    uint8_t player_id_{ 0 };
    int16_t character_id_{ 0 };
    bool is_game_quit_{ false };
    GamePhase game_state_{ GamePhase::Standing };
    GamePhase prev_game_state_{ GamePhase::Standing };
    float play_time_{ 0.0f };

    // ���� ���� ����
    uint32_t total_score_{ 0 };
    uint32_t rest_score_{ 0 };
    uint8_t combo_count_{ 0 };
    int16_t total_interrupt_block_count_{ 0 };
    int16_t total_enemy_interrupt_block_count_{ 0 };
    int16_t add_interrupt_block_count_{ 0 };

    // ���� ����
    GameStateInfo state_info_;
    ScoreInfo score_info_;

    // ������Ʈ
    std::shared_ptr<GameBoard> game_board_;
    std::shared_ptr<InterruptBlockView> interrupt_view_;
    std::shared_ptr<ComboView> combo_view_;
    std::shared_ptr<ResultView> result_view_;
    std::shared_ptr<GameGroupBlock> control_block_;
    std::shared_ptr<GameBackground> background_;

    // ���� ������
    Block* board_blocks_[Constants::Board::BOARD_Y_COUNT][Constants::Board::BOARD_X_COUNT]{ nullptr };
    std::vector<RenderableObject*> draw_objects_;
    std::list<std::shared_ptr<Block>> block_list_;
    std::list<std::shared_ptr<BulletEffect>> bullet_list_;

    std::vector<IPlayerEventListener*> event_listeners_;
};

template<typename Container>
void BasePlayer::ReleaseContainer(Container& container) 
{
    for (auto& item : container) 
    {
        if (item) {
            item->Release();
        }
    }
    container.clear();
}