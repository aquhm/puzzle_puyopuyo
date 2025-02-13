#include "RoomState.hpp"
#include "managers/ResourceManager.hpp"
#include "managers/NetworkManager.hpp"
#include "managers/PlayerManager.hpp"
#include "managers/StateManager.hpp"
#include "ui/EditBox.hpp"
#include "ui/Button.hpp"
#include "utils/Logger.hpp"
#include <format>

bool RoomState::init() {
    if (isInitialized()) {
        return false;
    }

    try {
        if (!loadResources() ||
            !initializeBackgrounds() ||
            !initializeUI()) {
            return false;
        }

        initialized = true;
        return true;
    }
    catch (const std::exception& e) {
        Logger::error("RoomState initialization failed: {}", e.what());
        return false;
    }
}

bool RoomState::initializeBackgrounds() {
    for (size_t i = 0; i < BACKGROUND_COUNT; ++i) {
        backgrounds[i] = std::make_unique<Texture>();

        auto path = std::format("./Image/MAINMENU/{:02d}.png", i);
        if (!backgrounds[i]->loadFromFile(path)) {
            Logger::error("Failed to load background texture: {}", path);
            return false;
        }
    }
    return true;
}

bool RoomState::initializeUI() {
    auto& resourceManager = ResourceManager::getInstance();
    auto buttonTexture = resourceManager.getTexture("./Image/UI/BUTTON/button.png");
    if (!buttonTexture) {
        return false;
    }

    // 채팅박스 초기화
    ui.chatBox = std::make_unique<EditBox>();
    if (!ui.chatBox->init(
        (Application::getInstance().getScreenWidth() - 300) / 2,
        400,
        300,
        23)) {
        return false;
    }
    ui.chatBox->setReturnCallback([this]() { return sendChatMessage(); });

    // 버튼 초기화
    ui.startButton = std::make_unique<Button>();
    ui.exitButton = std::make_unique<Button>();

    if (!ui.startButton->init(buttonTexture, { 30, 100, 136, 49 }) ||
        !ui.exitButton->init(buttonTexture, { 30, 150, 136, 49 })) {
        return false;
    }

    // 버튼 콜백 설정
    ui.startButton->setCallback(Button::State::Down,
        [this]() { return startGame(); });
    ui.exitButton->setCallback(Button::State::Down,
        [this]() { return exitGame(); });

    return true;
}

void RoomState::enter() {
    SDL_StartTextInput();
    bgAnimation.reset();

    // 서버/클라이언트에 따른 UI 설정
    ui.startButton->setVisible(NetworkManager::getInstance().isServer());
    ui.exitButton->setVisible(true);
    ui.chatBox->setVisible(true);

    // 클라이언트인 경우 로비 접속 요청
    if (!NetworkManager::getInstance().isServer()) {
        auto playerId = PlayerManager::getInstance().getLocalPlayer()->getId();
        NetworkManager::getInstance().sendLobbyJoinRequest(playerId);
    }
}

void RoomState::update(float deltaTime) {
    updateBackgroundAnimation(deltaTime);
    ui.chatBox->update(deltaTime);
}

void RoomState::updateBackgroundAnimation(float deltaTime) {
    bgAnimation.scrollOffset -= BACKGROUND_SCROLL_SPEED * deltaTime;

    if (bgAnimation.scrollOffset <= -Application::getInstance().getScreenWidth()) {
        bgAnimation.scrollOffset = 0.0f;
        bgAnimation.renderIndex = (bgAnimation.renderIndex + 2) % 8;
    }
}

void RoomState::render() {
    renderBackground();

    // UI 렌더링
    ui.chatBox->render();
    ui.startButton->render();
    ui.exitButton->render();
}

void RoomState::renderBackground() const {
    auto screenWidth = Application::getInstance().getScreenWidth();
    auto screenHeight = Application::getInstance().getScreenHeight();

    // 스크롤링 배경
    for (int i = 0; i < 4; ++i) {
        float xPos = bgAnimation.scrollOffset;
        switch (i) {
        case 0: xPos += 0; break;
        case 1: xPos += 512; break;
        case 2: xPos += screenWidth; break;
        case 3: xPos += screenWidth + 512; break;
        }

        auto bgIndex = (bgAnimation.renderIndex + i) % 8;
        if (backgrounds[bgIndex]) {
            backgrounds[bgIndex]->render(xPos, 0);
        }
    }

    // 하단 고정 배경
    if (backgrounds[8]) {
        backgrounds[8]->render(0, screenHeight - 219);
    }
    if (backgrounds[9]) {
        backgrounds[9]->render(512, screenHeight - 219);
    }
}

bool RoomState::startGame() {
    if (NetworkManager::getInstance().startCharacterSelect()) {
        StateManager::getInstance().requestStateChange(StateID::CharacterSelect);
        return true;
    }
    return false;
}

bool RoomState::exitGame() {
    NetworkManager::getInstance().exit();
    StateManager::getInstance().requestStateChange(StateID::Login);
    return true;
}