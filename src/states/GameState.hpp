#pragma once
/*
 *
 * 설명: 게임 진행 상태
 *
 */

#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <deque>
#include <set>
#include <list>
#include <array>
#include <string_view>
#include <span>
#include <functional>

#include "BaseState.hpp"
#include "../core/common/constants/Constants.hpp"
#include "../core/common/types/GameTypes.hpp"
#include "../game/block/Block.hpp"
#include "../network/packets/GamePackets.hpp"
#include "../network/NetworkController.hpp"


class NetworkController;

class Player;
class PlayerManager;
class RenderableObject;
class GameBackground;
class GameBoard;
class GroupBlock;
class GameGroupBlock;
class IceBlock;
class BulletEffect;
class InterruptBlockView;
class ComboView;
class EditBox;
class ResultView;
class Button;
class ImageTexture;
class GamePlayer;

namespace GameStateDetail 
{
    constexpr Constants::Direction GetOppositeDirection(Constants::Direction dir)
    {
        switch (dir) 
        {
        case Constants::Direction::Left:   
            return Constants::Direction::Right;
        case Constants::Direction::Right:  
            return Constants::Direction::Left;
        case Constants::Direction::Top:    
            return Constants::Direction::Bottom;
        case Constants::Direction::Bottom: 
            return Constants::Direction::Top;
        default:               
            return Constants::Direction::None;
        }
    }
}

// 게임 상태 열거형
enum class GamePhase 
{
    Standing,   // 대기 상태
    Playing,    // 게임 진행 중
    Shattering, // 블록 파괴 중
    IceBlocking,// 방해 블록 생성 중
    GameOver    // 게임 종료
};

// 블록 위치 마커 구조체
struct BlockPositionMarker 
{
    float xPos;
    float yPos;
    BlockType type;
};

class GameState final : public BaseState 
{
public:
    GameState();
    ~GameState() override;

    // 복사/이동 생성자 삭제
    GameState(const GameState&) = delete;
    GameState& operator=(const GameState&) = delete;
    GameState(GameState&&) = delete;
    GameState& operator=(GameState&&) = delete;

    // BaseState 인터페이스 구현
    bool Init() override;
    void Enter() override;
    void Leave() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Release() override;
    void HandleEvent(const SDL_Event& event) override;
    void HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length) override;

    // 게임 상태 조회
    [[nodiscard]] std::string_view GetStateName() const override { return "Game"; }
    [[nodiscard]] GameBoard* GetGameBoard() const { return gameboard_.get(); }
    [[nodiscard]] Block* (*GetGameBlocks())[Constants::Board::BOARD_X_COUNT] { return board_blocks_; }
    [[nodiscard]] GameBackground* GetBackGround() const { return background_.get(); }
    [[nodiscard]] InterruptBlockView* GetInterruptBlockView() const { return interrupt_view_.get(); }
    [[nodiscard]] const std::shared_ptr<GamePlayer>& GetPlayer() const { return game_player_; }    

    // 블록 조작 관련
    bool IsPossibleMove(int xIdx);
    void UpdateTargetPosIdx();

    // 게임 상태 제어
    bool GameRestart();
    bool GameExit();
    void GameQuit();

    [[nodiscard]] bool IsGameOver();
    void BulletUpdate(float deltaTime);
    void CreateBullet(Block* block);
    void CreateGamePlayer(const std::span<const uint8_t>& blocktype1, const std::span<const uint8_t>& blocktype2, uint8_t playerIdx, uint8_t characterIdx);
    void DestroyNextBlock();
    void CreateBlocksFromFile(); // 파일로 부터 블록 셋팅

    // 블록 연결 검사 관련
    void CollectRemoveIceBlocks();
    void UpdateLinkState(Block* block);
    void UpdateInterruptBlockView();    

    //Setter
    void SetAttackComboState(bool enable) { stateInfo_.isComboAttack = enable; }
    void SetTotalInterruptEnemyBlockCount(bool enable) { stateInfo_.isComboAttack = enable; }
    void DefenseInterruptBlockCount(short cnt, float x, float y, unsigned char type);

    void AddInterruptBlockCount(short cnt, float x, float y, unsigned char type);
    

    [[nodiscard]] short GetTotalInterruptBlockCount() { return scoreInfo_.totalInterruptBlockCount; }
    [[nodiscard]] short GetTotalInterruptEnemyBlockCount() { return scoreInfo_.totalEnemyInterruptBlockCount; }

    [[nodiscard]] const std::unique_ptr<EditBox>& GetEditBox() { return chatbox_; }

private:

    // 점수 계산 관련
    void CalculateIceBlockCount();
    void UpdateComboState();
    void ResetComboState();

    // 초기화 헬퍼
    bool InitializeComponents();
    bool InitializeGameBoard();
    bool CreateUI();
    bool LoadResources();

    // 게임 로직
    void UpdateGameLogic(float deltaTime);
    void ProcessMatchedBlocks();
    void HandlePhaseTransition(GamePhase newPhase);
    bool CheckGameBlockState();
    void UpdateBlockPositions();
    void CheckGameOverCondition();
    void HandleGameOver();

    void HandleGameInitialize(uint8_t connectionId, const GameInitPacket* packet);
    void HandleAddNewBlock(uint8_t connectionId, const AddNewBlockPacket* packet);
    void HandleUpdateBlockMove(uint8_t connectionId, const MoveBlockPacket* packet);
    void HandleBlockRotate(uint8_t connectionId, const RotateBlockPacket* packet);
    void HandleStartGame();  
    void HandleCheckBlockState(uint8_t connectionId, const CheckBlockStatePacket* packet);
    //void HandleComboUpdate(uint8_t connectionId, const char* payload, uint32_t size);


    void UpdateIceBlockPhase(float deltaTime);
    void UpdateShatteringPhase(float deltaTime);
    void UpdateGameOverPhase(float deltaTime);

    void UpdateInterruptBlockState();

    // 블록 관리
    void InitNextBlock();
    void CreateNextBlock();    
    void ProcessBlockMatching();
    void UpdateBlockLinks();
    bool FindMatchedBlocks(std::vector<std::vector<Block*>>& matchedGroups);
    short RecursionCheckBlock(short x, short y, Constants::Direction direction, std::vector<Block*>& matchedBlocks);

    // 점수 계산
    void CalculateScore();
    [[nodiscard]] short GetComboConstant(uint8_t comboCount) const;
    [[nodiscard]] uint8_t GetLinkBonus(size_t linkCount) const;
    [[nodiscard]] uint8_t GetTypeBonus(size_t count) const;
    [[nodiscard]] uint8_t GetMargin() const;

    // 방해 블록 관리
    void GenerateIceBlocks();
    void GenerateLargeIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID);
    void GenerateSmallIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID);
    void InitializeIceBlock(IceBlock* block, const std::shared_ptr<ImageTexture>& texture, int x, int y, uint8_t playerID);

    // 입력 처리
    void HandleKeyboardState();
    void HandleMouseInput(const SDL_Event& event);
    void HandleKeyboardInput(const SDL_Event& event);
    void HandleSystemEvent(const SDL_Event& event);

    // 네트워크 관련
    void HandleNetworkCommand(const std::string_view& command);
    bool SendChatMsg();

    void InitializePacketProcessors();

    template<typename T>
    void ProcessTypedPacket(uint8_t connectionId, std::span<const char> data, void (GameState::* handler)(uint8_t, const T*));

    // 렌더링 관련
    void RenderUI();
#ifdef _DEBUG
    void RenderDebugInfo();
    void RenderDebugGrid();
#endif

private:
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
    } stateInfo_;

    // 점수 관리 구조체
    struct ScoreInfo 
    {
        uint32_t totalScore{ 0 };
        uint32_t restScore{ 0 };
        uint8_t comboCount{ 0 };
        int16_t totalInterruptBlockCount{ 0 };
        int16_t totalEnemyInterruptBlockCount{ 0 };
        int16_t addInterruptBlockCount{ 0 };

        void reset() 
        {
            totalScore = restScore = 0;
            comboCount = 0;
            totalInterruptBlockCount = 0;
            totalEnemyInterruptBlockCount = 0;
            addInterruptBlockCount = 0;
        }
    } scoreInfo_;

    // 게임 컴포넌트
    std::unique_ptr<GameBoard> gameboard_;
    std::shared_ptr<GameBackground> background_;
    std::unique_ptr<InterruptBlockView> interrupt_view_;
    std::unique_ptr<ComboView> combo_view_;
    std::unique_ptr<ResultView> result_view_;
    std::unique_ptr<GameGroupBlock> control_block_;
    std::shared_ptr<GamePlayer> game_player_;

    // UI 컴포넌트
    std::unique_ptr<EditBox> chatbox_;
    std::unique_ptr<Button> restart_button_;
    std::unique_ptr<Button> exit_button_;

    // 게임 보드 데이터
    Block* board_blocks_[Constants::Board::BOARD_Y_COUNT][Constants::Board::BOARD_X_COUNT]{ nullptr };
    std::vector<RenderableObject*> draw_objects_;
    std::deque<std::unique_ptr<GroupBlock>> next_blocks_;
    std::list<std::shared_ptr<Block>> block_list_;
    std::set<std::shared_ptr<IceBlock>> ice_blocks_;
    std::list<std::unique_ptr<BulletEffect>> bullets_;
    std::vector<BulletEffect*> bullets_to_delete_;
    std::vector<std::vector<Block*>> matched_blocks_;

    // 입력 제어
    uint64_t lastInputTime_{ 0 };
    bool initialized_{ false };

    // 블록 매칭 관련 추가
    struct MatchInfo 
    {
        std::vector<Block*> blocks;
        BlockType type;
        uint8_t count{ 0 };
    };

    // 상태 변수 추가
    bool isNetworkGame_{ false };
    uint8_t localPlayerId_{ 0 };

    std::unordered_map<PacketType, std::function<void(uint8_t, std::span<const char>)>> packet_processors_;
};

template<typename T>
void GameState::ProcessTypedPacket(uint8_t connectionId, std::span<const char> data, void (GameState::* handler)(uint8_t, const T*))
{

    if (data.size() < sizeof(T))
    {
        return;
    }

    T packet;
    std::memcpy(&packet, data.data(), sizeof(T));
    (this->*handler)(connectionId, &packet);
}


class BlockComp 
{
public:
    bool operator()(const std::shared_ptr<Block>& lhs, const std::shared_ptr<Block>& rhs) const 
    {
        return *lhs < *rhs;
    }
};

// 블록 상태 체크를 위한 함수 객체
class CheckerBlockStateComp 
{
private:
    int checkCount_{ 0 };
    const BlockState stateType_;

public:
    explicit CheckerBlockStateComp(BlockState type)
        : stateType_(type) {
    }

    void operator()(const std::shared_ptr<Block>& block) 
    {
        if (block && block->GetState() == stateType_) 
        {
            ++checkCount_;
        }
    }

    [[nodiscard]] int GetCheckCount() const { return checkCount_; }
};