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

// �������� strongly typed�� ����
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

    // ����/�̵� ���� ����� ����
    GameState(const GameState&) = delete;
    GameState& operator=(const GameState&) = delete;
    GameState(GameState&&) = delete;
    GameState& operator=(GameState&&) = delete;

    // BaseState �������̽� ����
    bool Init() override;
    void Enter() override;
    void Leave() override;
    void Update(float deltaTime) override;
    void Render() override;
    void HandleEvent(const SDL_Event& event) override;
    void HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length) override;

    [[nodiscard]] std::string_view getStateName() const override { return "Game";}

private:
    // ���� ���� ����
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

    // ���� ����
    struct ScoreInfo {
        uint32_t totalScore{ 0 };
        uint32_t restScore{ 0 };
        uint8_t comboCount{ 0 };
        int16_t totalInterruptBlockCount{ 0 };
        int16_t totalEnemyInterruptBlockCount{ 0 };
        int16_t addInterruptBlockCount{ 0 };
    };
    ScoreInfo scoreInfo;

    // ���� ���� ����
    static constexpr int BOARD_HEIGHT = 13;
    static constexpr int BOARD_WIDTH = 6;
    using BoardArray = std::array<std::array<std::shared_ptr<Block>, BOARD_WIDTH>, BOARD_HEIGHT>;
    BoardArray gameBoard;

    // ���� ��ü��
    std::unique_ptr<GameBoard> board;
    std::unique_ptr<GameBackground> background;
    std::unique_ptr<InterruptBlockView> interruptView;
    std::unique_ptr<ComboView> comboView;
    std::unique_ptr<ResultView> resultView;
    std::unique_ptr<GameGroupBlock> controlBlock;
    std::shared_ptr<GamePlayer> player;

    // UI ��ҵ�
    std::unique_ptr<EditBox> chatBox;
    std::unique_ptr<Button> restartButton;
    std::unique_ptr<Button> exitButton;

    // ���� ������Ʈ �����̳ʵ�
    std::vector<std::unique_ptr<DrawObject>> drawObjects;
    std::deque<std::unique_ptr<GroupBlock>> nextBlocks;
    std::vector<std::shared_ptr<Block>> activeBlocks;
    std::set<std::shared_ptr<IceBlock>> iceBlocks;
    std::vector<std::unique_ptr<Bullet>> bullets;

private:
    // �ʱ�ȭ ���� �޼����
    bool initializeGameBoard();
    bool initializeUI();
    bool loadResources();
    void setupInitialBlocks();

    // ���� ���� �޼����
    void handleKeyboardInput(const SDL_Event& event);
    void processGameLogic(float deltaTime);
    void updateGameState();
    void checkCollisions();
    void manageBullets(float deltaTime);

    // ��� ���� �޼����
    void createNextBlock();
    void removeMatchedBlocks();
    bool checkMatchingBlocks();
    void updateBlockPositions();

    // ���� ��� �޼����
    void calculateScore();
    int calculateComboBonus(int comboCount) const;
    int calculateLinkBonus(int linkCount) const;

    // ��Ʈ��ũ ���� �޼����
    void sendGameState();
    void receiveGameState();
    void synchronizeBlocks();

private:
    // ��� ��Ī ���� ������Ʈ
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

    // ���ο� �޼����
    void processBlockMatching(float deltaTime);
    void handleMatchResults(const std::vector<BlockMatcher::MatchResult>& matches);
    void applyMatchEffects(const BlockMatcher::MatchResult& match);

private:
    std::unique_ptr<GameSynchronizer> synchronizer;

    void handleNetworkCommands();
    void sendGameSnapshot();
    void applyNetworkCommand(const GameSynchronizer::NetworkCommand& cmd);


private:
    // �Է� ó�� ����
    std::unique_ptr<InputHandler> inputHandler;

    // UI ����
    std::unique_ptr<GameUI> gameUI;

    // ��� ���� ���� �޼����
    void setupInputCommands();
    void handleBlockMovement(const SDL_Event& event);
    void handleBlockRotation();
    void handleBlockDrop();

    // UI �̺�Ʈ �ڵ鷯��
    void handlePauseMenu();
    void handleGameOver();
    void updateScoreDisplay();


private:
    // ���� ����/�����Ǵ� ��ü���� ���� Ǯ
    ObjectPool<Block> blockPool;
    ObjectPool<Particle> particlePool;
    ObjectPool<Bullet> bulletPool;
};