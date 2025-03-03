//// GameCore.hpp
//#pragma once
//
//#include <memory>
//#include <vector>
//#include <list>
//#include <array>
//#include <functional>
//
//#include "../../core/common/constants/Constants.hpp"
//#include "../../core/common/types/GameTypes.hpp"
//#include "../block/Block.hpp"
//#include "../../states/GameState.hpp"
//#include "../block/GameGroupBlock.hpp"
//
//class ImageTexture;
//class IceBlock;
//
//// 게임 로직을 위한 주요 이벤트 타입
//enum class GameCoreEvent {
//    MatchFound,          // 블록 매치 발견
//    ScoreChanged,        // 점수 변경
//    ComboChanged,        // 콤보 상태 변경
//    InterruptCountChanged, // 인터럽트 블록 카운트 변경
//    PhaseChanged,        // 게임 페이즈 변경
//    GameOver             // 게임 종료
//};
//
//// 게임 상태 정보 구조체
//struct GameStateInfo 
//{
//    GamePhase currentPhase{ GamePhase::Standing };
//    GamePhase previousPhase{ GamePhase::Standing };
//    float playTime{ 0.0f };
//    bool isRunning{ false };
//    bool isDefending{ false };
//    bool isAttacked{ false };
//    bool isComboAttack{ false };
//    bool shouldQuit{ false };
//    uint8_t defenseCount{ 0 };
//    bool hasIceBlock{ false };  // 방해 블록 보유 여부
//};
//
//// 점수 정보 구조체
//struct ScoreInfo {
//    uint32_t totalScore{ 0 };
//    uint32_t restScore{ 0 };
//    uint8_t comboCount{ 0 };
//    int16_t totalInterruptBlockCount{ 0 };
//    int16_t totalEnemyInterruptBlockCount{ 0 };
//    int16_t addInterruptBlockCount{ 0 };
//
//    void reset() {
//        totalScore = restScore = 0;
//        comboCount = 0;
//        totalInterruptBlockCount = 0;
//        totalEnemyInterruptBlockCount = 0;
//        addInterruptBlockCount = 0;
//    }
//};
//
//class GameCore 
//{
//public:
//    GameCore();
//    ~GameCore();
//
//    // 초기화 및 리셋
//    bool Initialize(float boardX, float boardY);
//    void Reset();
//
//    // 업데이트 메서드
//    void Update(float deltaTime);
//
//    // 게임 상태 관리
//    void SetGamePhase(GamePhase newPhase);
//    GamePhase GetCurrentPhase() { return stateInfo_.currentPhase; }
//    GamePhase GetPreviousPhase() { return stateInfo_.previousPhase; }
//
//    // 블록 매칭 관련 메서드
//    bool FindMatchedBlocks(std::vector<std::vector<Block*>>& matchedGroups);
//    short RecursionCheckBlock(short x, short y, Constants::Direction direction, std::vector<Block*>& matchedBlocks);
//    void UpdateLinkState(Block* block);
//    bool CheckGameBlockState(std::list<std::shared_ptr<Block>>& blockList);
//    bool IsGameOver();
//
//    // 콤보 및 점수 계산
//    void CalculateScore(const std::vector<std::vector<Block*>>& matchedGroups);
//    void UpdateComboState();
//    void ResetComboState();
//    short GetComboConstant(uint8_t comboCount) const;
//    uint8_t GetLinkBonus(size_t linkCount) const;
//    uint8_t GetTypeBonus(size_t count) const;
//    uint8_t GetMargin() const;
//
//    // 인터럽트 블록(방해 블록) 관리
//    void CalculateIceBlockCount(const std::vector<std::vector<Block*>>& matchedGroups);
//    void UpdateInterruptBlockState();
//    void GenerateIceBlocks(std::list<std::shared_ptr<Block>>& blockList,
//        std::set<std::shared_ptr<IceBlock>>& iceBlocks,
//        uint8_t playerID);
//    void CollectRemoveIceBlocks(const std::vector<std::vector<Block*>>& matchedGroups,
//        std::set<std::shared_ptr<IceBlock>>& iceBlocks);
//
//    // 블록 움직임 검증
//    bool CanMoveLeft(int posX, int posY) const noexcept;
//    bool CanMoveRight(int posX, int posY) const noexcept;
//    bool CanRotate(int posX, int posY, RotateState currentState) const noexcept;
//
//    // 유틸리티 메서드
//    void UpdateBlockPositions();
//    void ProcessMatchedBlocks(std::vector<std::vector<Block*>>& matchedGroups,
//        std::list<std::shared_ptr<Block>>& blockList);
//
//    // 이벤트 리스너 등록
//    using EventCallback = std::function<void(GameCoreEvent, const void*)>;
//    void SetEventListener(EventCallback callback);
//
//    // 게임 상태 접근자
//    const GameStateInfo& GetStateInfo() const { return stateInfo_; }
//    const ScoreInfo& GetScoreInfo() const { return scoreInfo_; }
//
//    void UpdatePlayTime(float deltaTime) { stateInfo_.playTime += deltaTime; }
//    void SetHasIceBlock(bool hasIce) { stateInfo_.hasIceBlock = hasIce; }
//    bool HasIceBlock() const { return stateInfo_.hasIceBlock; }
//    void SetShouldQuit(bool quit) { stateInfo_.shouldQuit = quit; }
//
//    void AddInterruptBlockCount(int16_t count);
//    int16_t GetTotalInterruptBlockCount() const { return scoreInfo_.totalInterruptBlockCount; }
//    int16_t GetComboCount() const { return scoreInfo_.comboCount; }
//
//    // 보드 블록 배열 접근 메서드
//    Block* GetBlockAt(int x, int y);
//    void SetBlockAt(int x, int y, Block* block);
//    void ClearBoard();
//    void SetBoardBlocks(Block* blocks[Constants::Board::BOARD_Y_COUNT][Constants::Board::BOARD_X_COUNT]);
//    Block* (*GetBoardBlocks())[Constants::Board::BOARD_X_COUNT] { return boardBlocks_; }
//
//private:
//    
//    void InitializeIceBlock(IceBlock* block, const std::shared_ptr<ImageTexture>& texture,
//        int x, int y, uint8_t playerID);
//    void GenerateLargeIceBlockGroup(const std::shared_ptr<ImageTexture>& texture,
//        std::list<std::shared_ptr<Block>>& blockList,
//        std::set<std::shared_ptr<IceBlock>>& iceBlocks,
//        uint8_t playerID);
//    void GenerateSmallIceBlockGroup(const std::shared_ptr<ImageTexture>& texture,
//        std::list<std::shared_ptr<Block>>& blockList,
//        std::set<std::shared_ptr<IceBlock>>& iceBlocks,
//        uint8_t playerID);
//    void NotifyEvent(GameCoreEvent event, const void* data = nullptr);
//
//private:
//    
//    GameStateInfo stateInfo_;
//    ScoreInfo scoreInfo_;
//
//    // 보드 위치 정보
//    float boardX_;
//    float boardY_;
//
//    // 게임 보드 블록 배열
//    Block* boardBlocks_[Constants::Board::BOARD_Y_COUNT][Constants::Board::BOARD_X_COUNT]{ nullptr };
//
//    EventCallback eventCallback_;
//};