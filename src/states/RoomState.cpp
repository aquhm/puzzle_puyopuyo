#include "RoomState.hpp"

#include "../core/manager/ResourceManager.hpp"
#include "../core/GameApp.hpp"
#include "../core/manager/PlayerManager.hpp"
#include "../core/manager/StateManager.hpp"

#include "../network/NetworkController.hpp"
#include "../network/player/Player.hpp"
#include "../network/packets/GamePackets.hpp"

#include "../ui/EditBox.hpp"
#include "../ui/Button.hpp"
#include "../texture/ImageTexture.hpp"

#include <format>
#include <SDL3/SDL_log.h>


RoomState::RoomState()
    : backgrounds_{}
    , ui_elements_{}
    , background_animation_{}
{
}

bool RoomState::Init()
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
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "RoomState initialization failed: %s", e.what());
        return false;
    }
}

bool RoomState::LoadBackgrounds()
{
    try
    {
        for (size_t i = 0; i < BACKGROUND_COUNT; ++i)
        {
            auto path = std::format("MAINMENU/{:02d}.png", i);
            backgrounds_[i] = ImageTexture::Create(path);

            if (!backgrounds_[i])
            {
                throw std::runtime_error(std::format("Failed to load background texture: {}", path));
            }
        }
        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load backgrounds: %s", e.what());
        return false;
    }
}

bool RoomState::InitializeUI()
{
    const auto screen_width = GAME_APP.GetWindowWidth();
    auto button_texture = ImageTexture::Create("UI/BUTTON/button.png");
    if (!button_texture) 
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get button texture");
        return false;
    }

    // 채팅 박스 초기화
    ui_elements_.chat_box = std::make_unique<EditBox>();
    if (!ui_elements_.chat_box->Init((screen_width - 300) / 2.0f, 400.0f, 300.0f, 23.0f))
    {
        return false;
    }
    ui_elements_.chat_box->SetEventCallback([this]() { return SendChatMessage(); });

    // 시작 버튼 초기화
    ui_elements_.start_button = std::make_unique<Button>();
    ui_elements_.start_button->Init(button_texture, 30, 100, 136, 49);    
    ui_elements_.start_button->SetStateRect(Button::State::Normal, SDL_FRect{ 0, 0, 136, 49 });
    ui_elements_.start_button->SetStateRect(Button::State::Hover, SDL_FRect{ 0, 50, 136, 49 });
    ui_elements_.start_button->SetEventCallback(Button::State::Down,
        [this]() 
        { 
            return StartGame(); 
        });

    // 종료 버튼 초기화
    ui_elements_.exit_button = std::make_unique<Button>();
    ui_elements_.exit_button->Init(button_texture, 30, 150, 136, 49);
    ui_elements_.exit_button->SetStateRect(Button::State::Normal, SDL_FRect{ 0, 100, 136, 49 });
    ui_elements_.exit_button->SetStateRect(Button::State::Hover, SDL_FRect{ 0, 150, 136, 49 });
    ui_elements_.exit_button->SetEventCallback(Button::State::Down,
        [this]() 
        { 
            return ExitGame(); 
        });

    return true;
}

void RoomState::Enter()
{
    SDL_StartTextInput(GAME_APP.GetWindow());

    // UI 상태 설정
    ui_elements_.start_button->SetVisible(NETWORK.IsServer());
    ui_elements_.exit_button->SetVisible(true);
    ui_elements_.chat_box->SetVisible(true);
    ui_elements_.chat_box->ClearContent();

    // 배경 애니메이션 초기화
    background_animation_.Reset();

    // 클라이언트인 경우 로비 접속 요청
    if (!NETWORK.IsServer()) 
    {
        ConnectLobbyPacket packet;
        packet.id = GAME_APP.GetPlayerManager().GetMyPlayer()->GetId();
        NETWORK.SendData(packet);
    }
}

void RoomState::Leave()
{
    ui_elements_.start_button->SetVisible(false);
    ui_elements_.exit_button->SetVisible(false);
    ui_elements_.chat_box->SetVisible(false);
    ui_elements_.chat_box->ClearContent();

    background_animation_.Reset();
    SDL_StopTextInput(GAME_APP.GetWindow());
}

bool RoomState::StartGame()
{
    if (NETWORK.StartCharacterSelect()) 
    {
        GAME_APP.GetStateManager().RequestStateChange(StateManager::StateID::CharSelect);
        return true;
    }
    return false;
}

bool RoomState::ExitGame()
{
    NETWORK.Stop();
    GAME_APP.GetStateManager().RequestStateChange(StateManager::StateID::Login);
    return true;
}

bool RoomState::SendChatMessage()
{
    if (!ui_elements_.chat_box->IsEmpty()) 
    {
        NETWORK.ChatMessage(ui_elements_.chat_box->GetText(TextType::UTF8));
        return true;
    }
    return false;
}

void RoomState::Update(float deltaTime)
{
    UpdateBackgroundAnimation(deltaTime);

    // UI 업데이트
    if (ui_elements_.chat_box) {
        ui_elements_.chat_box->Update(deltaTime);
    }
    if (ui_elements_.start_button) {
        ui_elements_.start_button->Update(deltaTime);
    }
    if (ui_elements_.exit_button) {
        ui_elements_.exit_button->Update(deltaTime);
    }
}

void RoomState::UpdateBackgroundAnimation(float deltaTime)
{
    background_animation_.scroll_offset -= BACKGROUND_SCROLL_SPEED * deltaTime;

    const auto screen_width = static_cast<float>(GAME_APP.GetWindowWidth());
    if (background_animation_.scroll_offset <= -screen_width) {
        background_animation_.scroll_offset = 0.0f;
        background_animation_.render_index =
            (background_animation_.render_index + 2) % 8;
    }
}

void RoomState::Release()
{
    SDL_StopTextInput(GAME_APP.GetWindow());
}

void RoomState::Render()
{
    RenderBackground();
    RenderUI();
}

void RoomState::RenderBackground() const
{
    const auto screen_width = static_cast<float>(GAME_APP.GetWindowWidth());
    const auto screen_height = static_cast<float>(GAME_APP.GetWindowHeight());

    // 스크롤링 배경 4개 렌더링
    float x_positions[] = {
        background_animation_.scroll_offset,
        background_animation_.scroll_offset + 512,
        screen_width + background_animation_.scroll_offset,
        screen_width + background_animation_.scroll_offset + 512
    };

    for (size_t i = 0; i < 4; ++i) {
        auto bg_index = (background_animation_.render_index + i) % 8;
        if (backgrounds_[bg_index]) {
            backgrounds_[bg_index]->Render(x_positions[i], 0);
        }
    }

    // 하단 고정 배경 2개 렌더링
    if (backgrounds_[8]) {
        backgrounds_[8]->Render(0, screen_height - 219);
    }
    if (backgrounds_[9]) {
        backgrounds_[9]->Render(512, screen_height - 219);
    }
}

void RoomState::RenderUI() const
{
    if (ui_elements_.chat_box) {
        ui_elements_.chat_box->Render();
    }
    if (ui_elements_.start_button) {
        ui_elements_.start_button->Render();
    }
    if (ui_elements_.exit_button) {
        ui_elements_.exit_button->Render();
    }
}

void RoomState::HandleEvent(const SDL_Event& event)
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

void RoomState::HandleMouseEvent(const SDL_Event& event)
{
    if (ui_elements_.start_button) {
        ui_elements_.start_button->HandleEvent(event);
    }
    if (ui_elements_.exit_button) {
        ui_elements_.exit_button->HandleEvent(event);
    }
}

void RoomState::HandleKeyboardEvent(const SDL_Event& event)
{
    if (!ui_elements_.chat_box) {
        return;
    }

    ui_elements_.chat_box->HandleEvent(event);
}


void RoomState::HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length)
{
    const auto* packet = reinterpret_cast<const PacketBase*>(message.data());
    if (!packet || length < sizeof(PacketBase)) {
        return;
    }

    switch (packet->GetType()) {
    case PacketType::ChatMessage:
        if (length >= sizeof(ChatMessagePacket)) {
            const auto* chat_packet = reinterpret_cast<const ChatMessagePacket*>(packet);
            HandleChatMessage(chat_packet->message.data());
        }
        break;

    case PacketType::AddPlayer:
        if (length >= sizeof(AddPlayerPacket)) {
            const auto* player_packet = reinterpret_cast<const AddPlayerPacket*>(packet);
            HandlePlayerJoined(player_packet->player_id);
        }
        break;

    case PacketType::RemovePlayerInRoom:
        if (length >= sizeof(RemovePlayerInRoomPacket)) {
            const auto* remove_packet = reinterpret_cast<const RemovePlayerInRoomPacket*>(packet);
            HandlePlayerLeft(remove_packet->id);
        }
        break;

    case PacketType::StartCharSelect:
        HandleGameStart();
        break;
    }
}

void RoomState::HandleChatMessage(std::string_view message)
{
    if (ui_elements_.chat_box) {
        ui_elements_.chat_box->InputContent(message);
    }
}

void RoomState::HandlePlayerJoined(uint8_t playerId)
{
    if (const auto player = GAME_APP.GetPlayerManager().CreatePlayer(playerId)) 
    {
        auto message = std::format(" {}님이 입장하셨습니다.", playerId);
        HandleChatMessage(message);
    }
}

void RoomState::HandlePlayerLeft(uint8_t playerId)
{
    if (GAME_APP.GetPlayerManager().RemovePlayer(playerId)) 
    {
        auto message = std::format(" {}님이 퇴장하셨습니다.", playerId);
        HandleChatMessage(message);
    }
}

void RoomState::HandleGameStart()
{
    GAME_APP.GetStateManager().RequestStateChange(StateManager::StateID::CharSelect);
}