// states/GameState.cpp
#include "GameState.hpp"
#include "managers/ResourceManager.hpp"
#include "network/NetworkManager.hpp"
#include "utils/Logger.hpp"

GameState::GameState() {
    drawObjects.reserve(100);
}

bool GameState::init() {
    if (isInitialized()) {
        return false;
    }

    try {
        if (!loadResources() ||
            !initializeGameBoard() ||
            !initializeUI()) {
            return false;
        }

        setupInitialBlocks();
        initialized = true;
        return true;
    }
    catch (const std::exception& e) {
        Logger::error("GameState initialization failed: {}", e.what());
        return false;
    }
}

bool GameState::loadResources() {
    const std::vector<std::string> requiredTextures = {
        "./Image/PUYO/puyo_beta.png",
        "./Image/PUYO/Effect/effect.png",
        "./Image/PUYO/Effect/attack_eff_mix_01.png",
        "./Image/PUYO/rensa_font.png",
        "./Image/PUYO/result.png"
    };

    auto& resourceManager = ResourceManager::getInstance();

    for (const auto& texturePath : requiredTextures) {
        try {
            resourceManager.loadTexture(texturePath);
        }
        catch (const std::runtime_error& e) {
            Logger::error("Failed to load texture {}: {}", texturePath, e.what());
            return false;
        }
    }

    return true;
}

void GameState::update(float deltaTime) {
    stateInfo.playTime += deltaTime;

    // 게임 시작 체크
    if (stateInfo.playTime >= 2.0f && !stateInfo.isRunning &&
        stateInfo.currentState == GameStatus::Playing &&
        background->isReadyForGame()) {

        stateInfo.isRunning = true;
        if (auto& netManager = NetworkManager::getInstance();
            netManager.isRunning() && netManager.isServer()) {
            netManager.startPlayGame();
            createNextBlock();
        }
    }

    // 각 컴포넌트 업데이트
    for (const auto& obj : drawObjects) {
        obj->update(deltaTime);
    }

    // 게임 로직 처리
    processGameLogic(deltaTime);

    // 블록 관리
    for (const auto& block : activeBlocks) {
        block->update(deltaTime);
    }

    // 특수 상태 처리
    updateGameState();

    // 총알 관리
    manageBullets(deltaTime);

    // 네트워크 동기화
    if (NetworkManager::getInstance().isRunning()) {
        synchronizeBlocks();
    }

    // UI 업데이트
    if (chatBox) chatBox->update(deltaTime);
}

// states/GameState.cpp에 추가할 부분

void GameState::processBlockMatching(float deltaTime) {
    if (stateInfo.currentState != GameStatus::Playing) {
        return;
    }

    auto& matching = matchingSystem;

    // 현재 매칭 처리 중이면 애니메이션 진행
    if (matching.isProcessingMatches) {
        matching.matchAnimationTimer += deltaTime;
        if (matching.matchAnimationTimer >= MATCH_ANIMATION_DURATION) {
            handleMatchResults(matching.currentMatches);
            matching.reset();
        }
        return;
    }

    // 새로운 매칭 검사
    auto matches = BlockMatcher::findMatches(boardToVector());
    if (!matches.empty()) {
        matching.currentMatches = std::move(matches);
        matching.isProcessingMatches = true;
        matching.matchAnimationTimer = 0.0f;

        // 상태 변경
        stateInfo.previousState = stateInfo.currentState;
        stateInfo.currentState = GameStatus::Shattering;
    }
}

void GameState::handleMatchResults(const std::vector<BlockMatcher::MatchResult>& matches) {
    bool hasSpecialMatches = false;
    int totalMatchedBlocks = 0;

    for (const auto& match : matches) {
        totalMatchedBlocks += match.matchedBlocks.size();
        applyMatchEffects(match);

        if (match.includesSpecialBlock) {
            hasSpecialMatches = true;
        }
    }

    // 콤보 시스템 업데이트
    if (totalMatchedBlocks > 0) {
        scoreInfo.comboCount++;
        calculateScore(totalMatchedBlocks, scoreInfo.comboCount, hasSpecialMatches);
    }

    // 매칭된 블록 제거 및 새로운 블록 생성
    removeMatchedBlocks();
    createNewBlocks();

    // 네트워크 동기화
    if (NetworkManager::getInstance().isRunning()) {
        sendMatchResults(matches);
    }
}

void GameState::applyMatchEffects(const BlockMatcher::MatchResult& match) {
    // 파티클 효과 생성
    for (const auto& block : match.matchedBlocks) {
        auto effect = std::make_unique<MatchEffect>(
            block->getPosition(),
            block->getType()
        );
        effects.push_back(std::move(effect));
    }

    // 사운드 효과 재생
    if (match.chainCount > 1) {
        AudioManager::getInstance().playSound(
            "combo_" + std::to_string(std::min(match.chainCount, 5))
        );
    }

    // 특수 효과 처리
    if (match.includesSpecialBlock) {
        handleSpecialBlockEffect(match);
    }
}


// states/GameState.cpp의 상태 전이 관련 구현
void GameState::initializeStateTransitions() 
{
    stateTransition.addTransition(
        GameStatus::Playing,
        GameStatus::Shattering,
        [this]() {
            startBlockShatteringAnimation();
            updateScoreAndCombo();
        }
    );

    stateTransition.addTransition(
        GameStatus::Shattering,
        GameStatus::Playing,
        [this]() {
            cleanupShatteredBlocks();
            checkForNewMatches();
        }
    );

    stateTransition.addTransition(
        GameStatus::Playing,
        GameStatus::IceBlockDowning,
        [this]() {
            startIceBlockAnimation();
        }
    );

    // 게임 종료 관련 전이는 확인이 필요
    stateTransition.addTransition(
        GameStatus::Playing,
        GameStatus::Standing,
        [this]() {
            handleGameOver();
        },
        true
    );
}

void GameState::updateGameState() {
    GameStatus nextState = determineNextState();

    if (nextState != stateInfo.currentState &&
        stateTransition.canTransition(stateInfo.currentState, nextState)) {

        // 네트워크 게임에서는 서버의 확인이 필요할 수 있음
        if (NetworkManager::getInstance().isRunning() &&
            NetworkManager::getInstance().isClient()) {
            requestStateTransition(nextState);
            return;
        }

        stateTransition.executeTransition(stateInfo.currentState, nextState);
    }
}

// states/GameState.cpp의 네트워크 관련 구현
void GameState::handleNetworkCommands() {
    while (synchronizer->hasCommandsToProcess()) {
        auto cmd = synchronizer->getNextCommand();
        applyNetworkCommand(cmd);
    }
}

void GameState::applyNetworkCommand(const GameSynchronizer::NetworkCommand& cmd) {
    switch (cmd.type) {
    case GameSynchronizer::NetworkCommand::Type::BlockMove:
        handleBlockMoveCommand(cmd.data);
        break;

    case GameSynchronizer::NetworkCommand::Type::BlockMatch:
        handleBlockMatchCommand(cmd.data);
        break;

    case GameSynchronizer::NetworkCommand::Type::ScoreUpdate:
        handleScoreUpdateCommand(cmd.data);
        break;

    case GameSynchronizer::NetworkCommand::Type::StateChange:
        handleStateChangeCommand(cmd.data);
        break;

    case GameSynchronizer::NetworkCommand::Type::GameSync:
        handleFullGameSyncCommand(cmd.data);
        break;
    }
}

void GameState::update(float deltaTime) {
    // 네트워크 명령 처리
    if (NetworkManager::getInstance().isRunning()) {
        handleNetworkCommands();
    }

    // 일반 업데이트 로직
    processBlockMatching(deltaTime);
    updateGameState();
    updateEntities(deltaTime);

    // 주기적인 게임 상태 동기화
    if (NetworkManager::getInstance().isServer()) {
        synchronizer->update(deltaTime);
        if (synchronizer->shouldSendSnapshot()) {
            sendGameSnapshot();
        }
    }
}

// states/GameState.cpp의 구현
void GameState::setupInputCommands() {
    InputCommand moveLeftCommand{
        .pressCallback = nullptr,
        .releaseCallback = nullptr,
        .holdCallback = [this](float dt) { moveBlockLeft(); },
        .holdDelay = 0.1f,
        .holdInterval = 0.05f
    };
    inputHandler->registerCommand(SDLK_LEFT, moveLeftCommand);

    InputCommand moveRightCommand{
        .pressCallback = nullptr,
        .releaseCallback = nullptr,
        .holdCallback = [this](float dt) { moveBlockRight(); },
        .holdDelay = 0.1f,
        .holdInterval = 0.05f
    };
    inputHandler->registerCommand(SDLK_RIGHT, moveRightCommand);

    InputCommand rotateCommand{
        .pressCallback = [this]() { handleBlockRotation(); },
        .releaseCallback = nullptr,
        .holdCallback = nullptr
    };
    inputHandler->registerCommand(SDLK_UP, rotateCommand);

    InputCommand dropCommand{
        .pressCallback = nullptr,
        .releaseCallback = nullptr,
        .holdCallback = [this](float dt) { handleBlockDrop(); },
        .holdDelay = 0.0f,
        .holdInterval = 0.016f
    };
    inputHandler->registerCommand(SDLK_DOWN, dropCommand);
}

void GameState::init() {
    if (!BaseState::init()) return false;

    inputHandler = std::make_unique<InputHandler>();
    gameUI = std::make_unique<GameUI>();

    setupInputCommands();
    setupUI();

    return true;
}

void GameState::setupUI() {
    // 게임 보드 UI
    auto boardView = std::make_unique<BoardView>(gameBoard);
    gameUI->addElement(std::move(boardView), GameUI::Layer::Game);

    // 다음 블록 표시
    auto nextBlockView = std::make_unique<NextBlockView>();
    gameUI->addElement(std::move(nextBlockView), GameUI::Layer::HUD);

    // 스코어 표시
    auto scoreView = std::make_unique<ScoreView>();
    gameUI->addElement(std::move(scoreView), GameUI::Layer::HUD);

    // 콤보 표시
    auto comboView = std::make_unique<ComboView>();
    gameUI->addElement(std::move(comboView), GameUI::Layer::Overlay);

    // 채팅 UI
    auto chatBox = std::make_unique<ChatBox>();
    gameUI->addElement(std::move(chatBox), GameUI::Layer::Overlay);
}

void GameState::update(float deltaTime) {
    if (stateInfo.currentState == GameStatus::Playing) {
        inputHandler->update(deltaTime);
    }

    gameUI->update(deltaTime);
    updateGameLogic(deltaTime);
}

void GameState::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        inputHandler->handleEvent(event);
    }

    gameUI->handleEvent(event);

    // 네트워크 이벤트 처리
    if (event.type == SDL_SYSWMEVENT) {
        handleNetworkEvent(event.syswm);
    }
}

void GameState::render() {
    gameUI->render();
}

void GameState::updateScoreDisplay() {
    if (auto* scoreView = gameUI->getComponent<ScoreView>("score")) {
        scoreView->updateScore(scoreInfo.totalScore);
        scoreView->updateCombo(scoreInfo.comboCount);
    }
}