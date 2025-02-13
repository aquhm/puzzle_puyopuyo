#include "LoginState.hpp"
#include "managers/ResourceManager.hpp"
#include "managers/NetworkManager.hpp"
#include "managers/StateManager.hpp"
#include "ui/Button.hpp"
#include "ui/TextBox.hpp"
#include "utils/Logger.hpp"
#include <stdexcept>

LoginState::LoginState()
{
 
}

bool LoginState::Init() 
{
    if (isInitialized()) 
    {
        return false;
    }

    try 
    {
        if (!loadResources() ||
            !initializeBackgrounds() ||
            !initializeUI()) 
        {
            return false;
        }

        initialized = true;
        return true;
    }
    catch (const std::exception& e) 
    {
        //Logger::error("LoginState initialization failed: {}", e.what());
        return false;
    }
}

bool LoginState::loadResources() 
{
    try 
    {
        auto& resourceManager = ResourceManager::getInstance();
        resourceManager.loadTexture("./Image/UI/BUTTON/button.png");
        return true;
    }
    catch (const std::runtime_error& e) 
    {
        Logger::error("Failed to load resources: {}", e.what());
        return false;
    }
}

bool LoginState::initializeBackgrounds() {
    for (size_t i = 0; i < BACKGROUND_COUNT; ++i) {
        backgrounds[i] = std::make_unique<Texture>();

        std::string path = fmt::format("./image/MAINMENU/{}.png", 20 + i);
        if (!backgrounds[i]->loadFromFile(path)) {
            Logger::error("Failed to load background texture: {}", path);
            return false;
        }
    }
    return true;
}

bool LoginState::initializeUI() {
    auto screenWidth = Application::getInstance().getScreenWidth();

    // IP �Է� �ؽ�Ʈ�ڽ� �ʱ�ȭ
    ipAddressInput = std::make_unique<TextBox>();
    centerUIElements();

    if (!ipAddressInput->init(
        uiConfig.textBox.posX,
        uiConfig.textBox.posY,
        uiConfig.textBox.width,
        uiConfig.textBox.height)) {
        return false;
    }

    // ��ư �ʱ�ȭ
    auto buttonTexture = ResourceManager::getInstance().getTexture("./Image/UI/BUTTON/button.png");
    if (!buttonTexture) {
        return false;
    }

    loginButton = std::make_unique<Button>();
    createServerButton = std::make_unique<Button>();

    // �α��� ��ư ����
    if (!initializeButton(loginButton.get(), buttonTexture,
        { .x = (screenWidth - DEFAULT_BUTTON_WIDTH) / 2,
         .y = uiConfig.buttons.loginPosY })) {
        return false;
    }

    // ���� ���� ��ư ����
    if (!initializeButton(createServerButton.get(), buttonTexture,
        { .x = (screenWidth - DEFAULT_BUTTON_WIDTH) / 2,
         .y = uiConfig.buttons.createServerPosY })) {
        return false;
    }

    // ��ư �ݹ� ����
    loginButton->setCallback(Button::State::Down,
        [this]() { return requestConnection(); });

    createServerButton->setCallback(Button::State::Down,
        [this]() { return requestGameServerCreation(); });

    return true;
}

void LoginState::Enter() {
    SDL_StartTextInput();

    loginButton->setVisible(true);
    createServerButton->setVisible(true);
    ipAddressInput->setVisible(true);
    ipAddressInput->clear();
}

void LoginState::Leave() {
    loginButton->setVisible(false);
    createServerButton->setVisible(false);
    ipAddressInput->setVisible(false);
    ipAddressInput->clear();

    SDL_StopTextInput();
}

void LoginState::Update(float deltaTime) {
    ipAddressInput->update(deltaTime);
}

void LoginState::Render() {
    // ��� ������
    for (size_t i = 0; i < BACKGROUND_COUNT; ++i) {
        if (backgrounds[i]) {
            backgrounds[i]->render(i * 512, 0);
        }
    }

    // UI ������
    ipAddressInput->render();
    loginButton->render();
    createServerButton->render();
}

bool LoginState::requestConnection() {
#ifdef _APP_DEBUG_
    NetworkManager::getInstance().create(false);
    NetworkManager::getInstance().setIP("127.0.0.1");
    return NetworkManager::getInstance().start();
#else
    if (!ipAddressInput->isEmpty()) {
        NetworkManager::getInstance().create(false);
        NetworkManager::getInstance().setIP(ipAddressInput->getText());
        return NetworkManager::getInstance().start();
    }
    return false;
#endif
}

bool LoginState::requestGameServerCreation() {
    NetworkManager::getInstance().create(true);

    if (NetworkManager::getInstance().start()) {
        StateManager::getInstance().requestStateChange(StateID::Room);
        return true;
    }
    return false;
}