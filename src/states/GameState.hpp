// states/GameState.hpp
#pragma once

#include "BaseState.hpp"
#include "game/GameBoard.hpp"
#include "game/Block.hpp"
#include <array>
#include <vector>
#include <deque>
#include <set>
#include <memory>

// 열거형을 strongly typed로 변경
enum class GameStatus {
    Standing,
    Playing,
    Shattering,
    IceBlockDowning,
    Exiting
};

class GameState final : public BaseState {
public:
    GameState();
    ~GameState() override = default;

    // 복사/이동 연산 명시적 제어
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
    void HandleEvent(const SDL_Event& event) override;
    void HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length) override;

    [[nodiscard]] std::string_view getStateName() const override { return "Game";}

private:
    // 게임 상태 관리
    struct GameStateInfo {
        GameStatus currentState{ GameStatus::Standing };
        GameStatus previousState{ GameStatus::Standing };
        float playTime{ 0.0f };
        bool isRunning{ false };
        bool isDefending{ false };
        bool isAttacked{ false };
        bool isComboAttack{ false };
        bool shouldQuit{ false };
        uint8_t defenseCount{ 0 };
    };
    GameStateInfo stateInfo;

    // 점수 관리
    struct ScoreInfo {
        uint32_t totalScore{ 0 };
        uint32_t restScore{ 0 };
        uint8_t comboCount{ 0 };
        int16_t totalInterruptBlockCount{ 0 };
        int16_t totalEnemyInterruptBlockCount{ 0 };
        int16_t addInterruptBlockCount{ 0 };
    };
    ScoreInfo scoreInfo;

    // 게임 보드 관리
    static constexpr int BOARD_HEIGHT = 13;
    static constexpr int BOARD_WIDTH = 6;
    using BoardArray = std::array<std::array<std::shared_ptr<Block>, BOARD_WIDTH>, BOARD_HEIGHT>;
    BoardArray gameBoard;

    // 게임 객체들
    std::unique_ptr<GameBoard> board;
    std::unique_ptr<GameBackground> background;
    std::unique_ptr<InterruptBlockView> interruptView;
    std::unique_ptr<ComboView> comboView;
    std::unique_ptr<ResultView> resultView;
    std::unique_ptr<GameGroupBlock> controlBlock;
    std::shared_ptr<GamePlayer> player;

    // UI 요소들
    std::unique_ptr<EditBox> chatBox;
    std::unique_ptr<Button> restartButton;
    std::unique_ptr<Button> exitButton;

    // 게임 오브젝트 컨테이너들
    std::vector<std::unique_ptr<DrawObject>> drawObjects;
    std::deque<std::unique_ptr<GroupBlock>> nextBlocks;
    std::vector<std::shared_ptr<Block>> activeBlocks;
    std::set<std::shared_ptr<IceBlock>> iceBlocks;
    std::vector<std::unique_ptr<Bullet>> bullets;

private:
    // 초기화 관련 메서드들
    bool initializeGameBoard();
    bool initializeUI();
    bool loadResources();
    void setupInitialBlocks();

    // 게임 로직 메서드들
    void handleKeyboardInput(const SDL_Event& event);
    void processGameLogic(float deltaTime);
    void updateGameState();
    void checkCollisions();
    void manageBullets(float deltaTime);

    // 블록 관리 메서드들
    void createNextBlock();
    void removeMatchedBlocks();
    bool checkMatchingBlocks();
    void updateBlockPositions();

    // 점수 계산 메서드들
    void calculateScore();
    int calculateComboBonus(int comboCount) const;
    int calculateLinkBonus(int linkCount) const;

    // 네트워크 관련 메서드들
    void sendGameState();
    void receiveGameState();
    void synchronizeBlocks();

private:
    // 블록 매칭 관련 컴포넌트
    struct MatchingSystem {
        std::vector<BlockMatcher::MatchResult> currentMatches;
        float matchAnimationTimer{ 0.0f };
        bool isProcessingMatches{ false };

        void reset() {
            currentMatches.clear();
            matchAnimationTimer = 0.0f;
            isProcessingMatches = false;
        }
    };
    
    MatchingSystem matchingSystem;

    // 새로운 메서드들
    void processBlockMatching(float deltaTime);
    void handleMatchResults(const std::vector<BlockMatcher::MatchResult>& matches);
    void applyMatchEffects(const BlockMatcher::MatchResult& match);

private:
    std::unique_ptr<GameSynchronizer> synchronizer;

    void handleNetworkCommands();
    void sendGameSnapshot();
    void applyNetworkCommand(const GameSynchronizer::NetworkCommand& cmd);


private:
    // 입력 처리 관련
    std::unique_ptr<InputHandler> inputHandler;

    // UI 관련
    std::unique_ptr<GameUI> gameUI;

    // 블록 조작 관련 메서드들
    void setupInputCommands();
    void handleBlockMovement(const SDL_Event& event);
    void handleBlockRotation();
    void handleBlockDrop();

    // UI 이벤트 핸들러들
    void handlePauseMenu();
    void handleGameOver();
    void updateScoreDisplay();


private:
    // 자주 생성/삭제되는 객체들을 위한 풀
    ObjectPool<Block> blockPool;
    ObjectPool<Particle> particlePool;
    ObjectPool<Bullet> bulletPool;
};