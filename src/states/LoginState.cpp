#include "LoginState.hpp"

#include "../core/GameApp.hpp"
#include "../core/manager/ResourceManager.hpp"
#include "../core/manager/PlayerManager.hpp"
#include "../core/manager/StateManager.hpp"
#include "../core/common/constants/Constants.hpp"

#include "../ui/TextBox.hpp"
#include "../ui/Button.hpp"
#include "../ui/Label.hpp"

#include "../texture/ImageTexture.hpp"
#include "../utils/Logger.hpp"

#include "../network/packets/GamePackets.hpp"
#include "../network/NetworkController.hpp"

#include <format>
#include <exception>
#include <SDL3/SDL_system.h>



LoginState::LoginState()
    : backgrounds_{}
    , ui_elements_{}
{
    InitializePacketHandlers();
}

void LoginState::InitializePacketHandlers()
{
    packet_processor_.RegisterHandler<GiveIdPacket>(
        PacketType::GiveId,
        
        [this](uint8_t connectionId, const GiveIdPacket* packet) 
        {
            HandleGiveId(packet->player_id);
        }
    );
}

bool LoginState::Init()
{
    if (initialized) 
    {
        return false;
    }

    try 
    {
        if (!LoadBackgrounds() || !InitializeUI()) 
        {
            return false;
        }

        initialized = true;
        return true;
    }
    catch (const std::exception& e) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "LoginState initialization failed: %s", e.what());
        return false;
    }
}


bool LoginState::LoadBackgrounds()
{
    const std::array<std::string, BACKGROUND_COUNT> bg_paths = 
    {
        "MAINMENU/20.png",
        "MAINMENU/21.png"
    };

    for (size_t i = 0; i < BACKGROUND_COUNT; ++i) 
    {
        auto texture = ImageTexture::Create(bg_paths[i]);
        if (texture == nullptr)
        {
            SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to load background texture: %s", bg_paths[i].c_str());
            return false;
        }
        backgrounds_[i] = std::move(texture);
    }
    return true;
}

bool LoginState::InitializeUI()
{
    const auto screen_width = GAME_APP.GetWindowWidth();
    auto buttonTexture = ImageTexture::Create("UI/BUTTON/button.png");

    if (!buttonTexture) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to get button texture");
        return false;
    }

    ui_elements_.ip_label = std::make_unique<Label>();
    ui_elements_.ip_label->Init((screen_width - 150) / 2.0f, 175.0f, 150.0f, 20.0f);
    ui_elements_.ip_label->Configure("Server IP Address:", TextAlignment::Center, SDL_Color{ 255, 255, 255, 255 });

    // IP 입력 TextBox 초기화
    ui_elements_.ip_input = std::make_unique<TextBox>();
    if (!ui_elements_.ip_input->Init( (screen_width - 150) / 2.0f, 200.0f, 150.0f, 23.0f))
    {
        return false;
    }

    ui_elements_.ip_input->SetEventCallback([this]() { return RequireConnect(); });

    // 로그인 버튼 초기화
    ui_elements_.login_button = std::make_unique<Button>();
    ui_elements_.login_button->Init(buttonTexture, (screen_width - 136.0f) / 2.0f, 230.0f, 136.0f, 49.0f);
    ui_elements_.login_button->SetStateRect(Button::State::Normal, SDL_FRect{ 137, 0, 136, 49 });
    ui_elements_.login_button->SetStateRect(Button::State::Hover, SDL_FRect{ 137, 50, 136, 49 });
    ui_elements_.login_button->SetEventCallback(Button::State::Down, 
        [this]() 
        { 
            return RequireConnect(); 
        });

    // 서버 생성 버튼 초기화
    ui_elements_.create_server_button = std::make_unique<Button>();
    ui_elements_.create_server_button->Init(buttonTexture, (screen_width - 136.0f) / 2.0f, 300.0f, 136.0f, 49.0f);
    ui_elements_.create_server_button->SetStateRect(Button::State::Normal, SDL_FRect{ 137, 100, 136, 49 });
    ui_elements_.create_server_button->SetStateRect(Button::State::Hover, SDL_FRect{ 137, 150, 136, 49 });
    ui_elements_.create_server_button->SetEventCallback(Button::State::Down,
        [this]() 
        { 
            return RequireInitGameSrv(); 
        });

    return true;
}

void LoginState::Enter()
{
    SDL_StartTextInput(GAME_APP.GetWindow());

    // UI 상태 초기화
    ui_elements_.login_button->SetVisible(true);
    ui_elements_.create_server_button->SetVisible(true);
    ui_elements_.ip_input->SetVisible(true);
    ui_elements_.ip_input->ClearContent();
}

void LoginState::Leave()
{
    // UI 상태 정리
    ui_elements_.login_button->SetVisible(false);
    ui_elements_.create_server_button->SetVisible(false);
    ui_elements_.ip_input->SetVisible(false);
    ui_elements_.ip_input->ClearContent();

    SDL_StopTextInput(GAME_APP.GetWindow());
}

void LoginState::Update(float deltaTime)
{
    if (ui_elements_.ip_input) 
    {
        ui_elements_.ip_input->Update(deltaTime);
    }
}

void LoginState::Render()
{
    RenderBackground();
    RenderUI();
}

void LoginState::RenderBackground() const
{
    if (backgrounds_[0]) {
        backgrounds_[0]->Render(0, 0);
    }
    if (backgrounds_[1]) {
        backgrounds_[1]->Render(512, 0);
    }
}

void LoginState::RenderUI() const
{
    if (ui_elements_.ip_input) {
        ui_elements_.ip_input->Render();
    }
    if (ui_elements_.login_button) {
        ui_elements_.login_button->Render();
    }
    if (ui_elements_.create_server_button) {
        ui_elements_.create_server_button->Render();
    }
    if (ui_elements_.ip_label) {
        ui_elements_.ip_label->Render();
    }
}

void LoginState::HandleEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_EVENT_QUIT:
        //GAME_APP.RequestQuit();
        break;

    case SDL_EVENT_MOUSE_MOTION:
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        HandleMouseEvent(event);
        break;

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_TEXT_INPUT:
    case SDL_EVENT_TEXT_EDITING:
        HandleKeyboardEvent(event);
        break;
    }
}

void LoginState::HandleMouseEvent(const SDL_Event& event)
{
    if (ui_elements_.login_button) {
        ui_elements_.login_button->HandleEvent(event);
    }
    if (ui_elements_.create_server_button) {
        ui_elements_.create_server_button->HandleEvent(event);
    }
}

void LoginState::HandleKeyboardEvent(const SDL_Event& event)
{
    if (!ui_elements_.ip_input) {
        return;
    }

    switch (event.type) 
    {
    case SDL_EVENT_KEY_DOWN:
        switch (event.key.key) 
        {
        case SDLK_RETURN:
        case SDLK_BACKSPACE:
            ui_elements_.ip_input->HandleEvent(event);
            break;
        }
        break;

    case SDL_EVENT_TEXT_INPUT:
    case SDL_EVENT_TEXT_EDITING:
        ui_elements_.ip_input->HandleEvent(event);
        break;
    }
}

void LoginState::HandleNetworkMessage(uint8_t connectionId, std::span<const char> data, uint32_t length)
{
    packet_processor_.ProcessPacket(connectionId, data, length);
}

void LoginState::HandleGiveId(uint8_t playerId)
{
    auto& playerManager = GAME_APP.GetPlayerManager();
    if (auto player = playerManager.CreatePlayer(playerId))
    {
        playerManager.SetMyPlayer(player);        
        GAME_APP.GetStateManager().RequestStateChange(StateManager::StateID::Room);
    }
}

void LoginState::Release()
{
}

bool LoginState::RequireConnect()
{
#ifdef _APP_DEBUG_
    NETWORK.Initialize(NetworkRole::Client);
    NETWORK.SetAddress("127.0.0.1");
    return NETWORK.Start();
#else
    if (ui_elements_.ip_input && !ui_elements_.ip_input->IsEmpty()) 
    {
        NETWORK.Initialize(NetworkRole::Client);
        NETWORK.SetAddress(ui_elements_.ip_input->GetText(TextType::ANSI));
        return NETWORK.Start();
    }
    return false;
#endif
}

bool LoginState::RequireInitGameSrv()
{
    NETWORK.Initialize(NetworkRole::Server);
    if (NETWORK.Start()) 
    {
        GAME_APP.GetStateManager().RequestStateChange(StateManager::StateID::Room);
        return true;
    }
    return false;
}