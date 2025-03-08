#pragma once
/**
 *
 * 설명: 게임 플레이어 기본 클래스 (공통 기능 구현)
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

    // 기본 인터페이스
    void Update(float deltaTime) override;
    void Render() override;
    virtual void Reset();
    virtual void Release();

    template<typename Container>
    void ReleaseContainer(Container& container);

    // 초기화 및 재시작
    virtual bool Initialize(const std::span<const uint8_t>& blocktype1,
        const std::span<const uint8_t>& blocktype2,
        uint8_t playerIdx,
        uint16_t characterIdx,
        const std::shared_ptr<GameBackground>& background) = 0;

    virtual bool Restart(const std::span<const uint8_t>& blockType1 = {}, const std::span<const uint8_t>& blockType2 = {}) = 0;

    // 블록 관리
    virtual void CreateNextBlock() = 0;
    virtual void PlayNextBlock() = 0;
    virtual bool CheckGameBlockState() = 0;

    // 블록 조작
    virtual void MoveBlock(uint8_t moveType, float position) = 0;
    virtual void RotateBlock(uint8_t rotateType, bool horizontalMoving) = 0;
    virtual void UpdateBlockPosition(float pos1, float pos2) = 0;
    virtual void UpdateFallingBlock(uint8_t fallingIdx, bool falling) = 0;
    virtual void ChangeBlockState(uint8_t state) = 0;

    // 방해 블록 관련
    virtual void AddInterruptBlock(int16_t count);
    virtual void AttackInterruptBlock(float x, float y, uint8_t type) = 0;
    virtual void DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type) = 0;
    virtual void UpdateInterruptBlock(int16_t count);
    virtual void CollectRemoveIceBlocks() = 0;

    // 게임 상태 제어
    virtual void LoseGame(bool isWin);
    virtual void SetGameQuit() { is_game_quit_ = true; }

    // 상태 조회
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
    // 컴포넌트 설정
    void SetInterruptView(const std::shared_ptr<InterruptBlockView>& view) { interrupt_view_ = view; }
    void SetComboView(const std::shared_ptr<ComboView>& view) { combo_view_ = view; }
    void SetResultView(const std::shared_ptr<ResultView>& view) { result_view_ = view; }
    void SetGameBoard(const std::shared_ptr<GameBoard>& board) { game_board_ = board; }
    void SetBackGround(const std::shared_ptr<GameBackground>& backGround) { background_ = backGround; }

    void SetComboAttackState(bool enable) { state_info_.isComboAttack = enable; }
    void SetTotalInterruptBlockCount(uint16_t count) { score_info_.totalInterruptBlockCount += count; }


    // 리스너 등록/해제 메서드
    void AddEventListener(IPlayerEventListener* listener);
    void RemoveEventListener(IPlayerEventListener* listener);

protected:
    // 초기화 관련 메서드
    virtual bool InitializeViews();
    virtual bool InitializeGameBoard(float posX, float posY);
    virtual bool InitializeControlBlock();

    // 블록 상태 관련 메서드
    void UpdateLinkState(Block* block);
    void CreateBlockClearEffect(const std::shared_ptr<Block>& block);

    // 블록 관리 관련 메서드
    void RemoveBlock(Block* block, const SDL_Point& pos_idx);
    void UpdateFallingBlocks(const std::list<SDL_Point>& x_index_list);
    void UpdateBlockLinks();

    // 점수 계산 관련 메서드
    int16_t GetComboConstant(uint8_t combo_count) const;
    uint8_t GetLinkBonus(size_t link_count) const;
    uint8_t GetTypeBonus(size_t count) const;
    uint8_t GetMargin() const;

    // 총알 및 이펙트 관련 메서드
    virtual void CreateBullet(Block* block);
    void UpdateBullets(float delta_time);

    void NotifyEvent(const std::shared_ptr<BasePlayerEvent>& event);


    // 블록 파일로 부터 생성
    void CreateBlocksFromFile();

    // 게임 상태 관리 구조체
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
        bool hasIceBlock{ false };  // 방해 블록 보유 여부
    };

    // 점수 관리 구조체
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
    // 게임 상태 변수
    uint8_t player_id_{ 0 };
    int16_t character_id_{ 0 };
    bool is_game_quit_{ false };
    GamePhase game_state_{ GamePhase::Standing };
    GamePhase prev_game_state_{ GamePhase::Standing };
    float play_time_{ 0.0f };

    // 점수 관련 변수
    uint32_t total_score_{ 0 };
    uint32_t rest_score_{ 0 };
    uint8_t combo_count_{ 0 };
    int16_t total_interrupt_block_count_{ 0 };
    int16_t total_enemy_interrupt_block_count_{ 0 };
    int16_t add_interrupt_block_count_{ 0 };

    // 상태 정보
    GameStateInfo state_info_;
    ScoreInfo score_info_;

    // 컴포넌트
    std::shared_ptr<GameBoard> game_board_;
    std::shared_ptr<InterruptBlockView> interrupt_view_;
    std::shared_ptr<ComboView> combo_view_;
    std::shared_ptr<ResultView> result_view_;
    std::shared_ptr<GameGroupBlock> control_block_;
    std::shared_ptr<GameBackground> background_;

    // 게임 데이터
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