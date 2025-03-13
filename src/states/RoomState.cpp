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

#include "../utils/Logger.hpp"
#include "../utils/StringUtils.hpp"

#include <format>


RoomState::RoomState()
    : backgrounds_{}
    , ui_elements_{}
    , background_animation_{}
{
    InitializePacketHandlers();
}

void RoomState::InitializePacketHandlers()
{
    packet_processor_.RegisterHandler<ChatMessagePacket>(
        PacketType::ChatMessage,
        [this](uint8_t connectionId, const ChatMessagePacket* packet) {
            HandleChatMessage(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<AddPlayerPacket>(
        PacketType::AddPlayer,
        [this](uint8_t connectionId, const AddPlayerPacket* packet) {
            HandlePlayerJoined(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<RemovePlayerInRoomPacket>(
        PacketType::RemovePlayerInRoom,
        [this](uint8_t connectionId, const RemovePlayerInRoomPacket* packet) {
            HandlePlayerLeft(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<StartCharSelectPacket>(
        PacketType::StartCharSelect,
        [this](uint8_t connectionId, const StartCharSelectPacket* packet) {
            HandleGameStart(connectionId, packet);
        }
    );
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
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "RoomState initialization failed: %s", e.what());
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
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to load backgrounds: %s", e.what());
        return false;
    }
}

bool RoomState::InitializeUI()
{
    const auto screen_width = GAME_APP.GetWindowWidth();
    auto button_texture = ImageTexture::Create("UI/BUTTON/button.png");
    if (!button_texture) 
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to get button texture");
        return false;
    }

    // 채팅 레이블 초기화
    ui_elements_.chat_label = std::make_unique<Label>();
    if (!ui_elements_.chat_label->Init((screen_width - 300) / 2.0f - 70, 400.0f, 65.0f, 23.0f))
    {
        return false;
    }
    ui_elements_.chat_label->Configure("Chat:", TextAlignment::Right, SDL_Color{ 255, 255, 255, 255 });
    

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
    ui_elements_.chat_label->SetVisible(true);
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
    if (ui_elements_.chat_label)
        ui_elements_.chat_label->SetVisible(false);

    if (ui_elements_.start_button)
        ui_elements_.start_button->SetVisible(false);

    if (ui_elements_.exit_button)
        ui_elements_.exit_button->SetVisible(false);

    if (ui_elements_.chat_box)
        ui_elements_.chat_box->SetVisible(false);

    if (ui_elements_.chat_box)
        ui_elements_.chat_box->ClearContent();

    background_animation_.Reset();
    SDL_StopTextInput(GAME_APP.GetWindow());
}

bool RoomState::StartGame()
{
    NETWORK.StartCharacterSelect();
    GAME_APP.GetStateManager().RequestStateChange(StateManager::StateID::CharSelect);
    return true;
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
        std::string message = ui_elements_.chat_box->GetText(TextType::UTF8);

        NETWORK.ChatMessage(message);
        return true;
    }
    return false;
}

void RoomState::Update(float deltaTime)
{
    UpdateBackgroundAnimation(deltaTime);

    if (ui_elements_.chat_label) {
        ui_elements_.chat_label->Update(deltaTime);
    }    
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
    if (ui_elements_.chat_label) {
        ui_elements_.chat_label->Render();
    }
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


void RoomState::HandleNetworkMessage(uint8_t connectionId, std::span<const char> data, uint32_t length)
{
    packet_processor_.ProcessPacket(connectionId, data, length);
}

void RoomState::HandleChatMessage(uint8_t connectionId, const ChatMessagePacket* packet)
{
    if (GAME_APP.GetPlayerManager().GetMyPlayer()->GetId() != packet->player_id)
    {
        if (ui_elements_.chat_box)
        {
            std::string formatted_message = std::format("[Player({})]: {}",
                packet->player_id,
                packet->message.data());

            ui_elements_.chat_box->InputContent(formatted_message);
        }
    }
}

void RoomState::HandlePlayerJoined(uint8_t connectionId, const AddPlayerPacket* packet)
{
    if (const auto player = GAME_APP.GetPlayerManager().CreatePlayer(packet->player_id))
    {
        std::wstring wideMessage = std::format(L"[시스템]: {}번 플레이어가 입장했습니다.", packet->player_id);
        std::string message = StringUtils::WideToUtf8(wideMessage);

        if (ui_elements_.chat_box)
        {
            ui_elements_.chat_box->InputContent(message);
        }
    }
}

void RoomState::HandlePlayerLeft(uint8_t connectionId, const RemovePlayerInRoomPacket* packet)
{
    if (GAME_APP.GetPlayerManager().RemovePlayer(packet->id))
    {
        std::wstring wideMessage = std::format(L"[시스템]: {}번 플레이어가 퇴장했습니다.", packet->id);
        std::string message = StringUtils::WideToUtf8(wideMessage);

        if (ui_elements_.chat_box)
        {
            ui_elements_.chat_box->InputContent(message);
        }
    }
}

void RoomState::HandleGameStart(uint8_t connectionId, const PacketBase* packet)
{
    GAME_APP.GetStateManager().RequestStateChange(StateManager::StateID::CharSelect);
}