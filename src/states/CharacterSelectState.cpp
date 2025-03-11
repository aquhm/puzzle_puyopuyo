#include "CharacterSelectState.hpp"
#include "../network/NetworkController.hpp"
#include "../core/GameApp.hpp"
#include "../core/manager/ResourceManager.hpp"
#include "../core/manager/PlayerManager.hpp"
#include "../core/manager/StateManager.hpp"
#include "../texture/ImageTexture.hpp"
#include "../ui/Button.hpp"
#include "../network/packets/GamePackets.hpp"
#include "../network/player/Player.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_keyboard.h >
#include <format>
#include <stdexcept>
#include <random>
#include "GameState.hpp"
#include "../utils/Logger.hpp"


CharacterSelectState::CharacterSelectState()
{
    InitializePacketHandlers();
}

void CharacterSelectState::InitializePacketHandlers()
{
    packet_processor_.RegisterHandler<ChangeCharSelectPacket>(
        PacketType::ChangeCharSelect,
        [this](uint8_t connectionId, const ChangeCharSelectPacket* packet) {
            HandleChangeCharSelect(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<DecideCharacterPacket>(
        PacketType::DecideCharSelect,
        [this](uint8_t connectionId, const DecideCharacterPacket* packet) {
            HandleDecideCharSelect(connectionId, packet);
        }
    );

    packet_processor_.RegisterHandler<PacketBase>(
        PacketType::StartGame,
        [this](uint8_t connectionId, const PacketBase* packet) {
            HandleStartGame(connectionId, packet);
        }
    );
}

bool CharacterSelectState::Init()
{
    if (initialized)
    {
        return false;
    }

    try 
    {
        // 배경 텍스처 로드
        backgrounds_[0] = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_bg_b_2p_Cell_001.png");
        backgrounds_[1] = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_bg_b_2p_Cell_002.png");
        select_background_ = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_bg_on.png");
        decide_marker_ = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/0.png");

        if (!backgrounds_[0] || !backgrounds_[1] || !select_background_ || !decide_marker_)
        {
            throw std::runtime_error("Failed to load background textures");
        }

        // 캐릭터 이미지 로드
        const std::string smallPath = "CHARACTERSELECT/CHARSELE_MENU/charselect_charicon_";
        const std::string largePath = "CHARACTERSELECT/VS2P_CHAR/charselect_vs2p_char_";

        uint8_t fileIdx = 0;
        for (uint8_t y = 0; y < 4; ++y)
        {
            for (uint8_t x = 0; x < 7; ++x)
            {
                if ((y == 0 && x == 3) || (y == 3 && x == 3) || ((x == 0 || x == 6) && y >= 2))
                {
                    continue;
                }

                uint8_t idx = y * 7 + x;
                character_info_[idx] = std::make_unique<CharacterImageInfo>();
                auto& info = character_info_[idx];
                info->characterIndex = fileIdx;

                auto smallPortraitPath = std::format("{}{:02d}.png", smallPath, fileIdx);
                auto largePortraitPath = std::format("{}{:02d}.png", largePath, fileIdx);

                info->smallPortrait = ImageTexture::Create(smallPortraitPath);
                info->largePortrait = ImageTexture::Create(largePortraitPath);

                if (!info->smallPortrait || !info->largePortrait)
                {
                    throw std::runtime_error("Failed to load character textures");
                }

                ++fileIdx;
            }
        }

        character_info_[3] = std::make_unique<CharacterImageInfo>();
        character_info_[24] = std::make_unique<CharacterImageInfo>();

        character_info_[3]->smallPortrait = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_random.png");
        character_info_[24]->smallPortrait = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_null.png");

        if (!character_info_[3]->smallPortrait || !character_info_[24]->smallPortrait)
        {
            throw std::runtime_error("Failed to load special slot textures");
        }

        // UI 요소 로드
        selection_box_ = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_cursor_large.png");
        player_cursor_ = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_cursor_1p.png");
        enemy_cursor_ = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_cursor_2p.png");

        if (!selection_box_ || !player_cursor_ || !enemy_cursor_)
        {
            throw std::runtime_error("Failed to load UI textures");
        }

        // Start 버튼 초기화
        start_button_ = std::make_shared<Button>();
        if (auto buttonTex = ImageTexture::Create("UI/BUTTON/button.png"))
        {
            const SDL_FRect normalRect{ 0, 0, 136, 49 };
            const SDL_FRect hoverRect{ 0, 50, 136, 49 };

            start_button_->Init(buttonTex, (GAME_APP.GetWindowWidth() - 136) / 2.0f, 7.0f, 136.0f, 49.0f);
            start_button_->SetStateRect(Button::State::Normal, normalRect);
            start_button_->SetStateRect(Button::State::Hover, hoverRect);
            start_button_->SetEventCallback(Button::State::Down,
                [this]()
                {
                    return RequireGameStart();
                });
        }

        initialized = true;
        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "CharacterSelect initialization failed: %s", e.what());
        return false;
    }
}

void CharacterSelectState::Enter() 
{
    if (NETWORK.IsServer()) 
    {
        current_pos_ = { 0, 0 };
        enemy_pos_ = { 6, 0 };
    }
    else 
    {
        current_pos_ = { 6, 0 };
        enemy_pos_ = { 0, 0 };
    }

    is_selected_ = false;
    is_enemy_selected_ = false;

    if (start_button_) 
    {
        start_button_->SetVisible(false);
    }
}

void CharacterSelectState::Leave() 
{
    current_pos_ = { 0, 0 };
    if (start_button_) {
        start_button_->SetVisible(false);
    }
}

void CharacterSelectState::HandleEvent(const SDL_Event& event) 
{
    switch (event.type)
    {
        //TODO
   /* case SDL_EVENT_QUIT:
        GAME_APP.SetGameRun(false);
        break;*/

    case SDL_EVENT_MOUSE_MOTION:
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (start_button_) {
            start_button_->HandleEvent(event);
        }
        break;

    case SDL_EVENT_KEY_DOWN:
        if (!is_selected_) {
            HandleKeyInput(event.key.key);
        }
        break; 
    }
}

void CharacterSelectState::HandleKeyInput(SDL_Keycode key)
{
    SDL_Point prevPos = current_pos_;

    switch (key) 
    {
    case SDLK_UP:
        if (current_pos_.y > 0) 
        {
            --current_pos_.y;
        }
        break;

    case SDLK_DOWN:
        if (CanMoveDown()) {
            ++current_pos_.y;
        }
        break;

    case SDLK_LEFT:
        if (CanMoveLeft()) 
        {
            --current_pos_.x;
        }
        break;

    case SDLK_RIGHT:
        if (CanMoveRight()) 
        {
            ++current_pos_.x;
        }
        break;

    case SDLK_SPACE:
        HandleCharacterSelection();
        return;
    }

    if (current_pos_.x != prevPos.x || current_pos_.y != prevPos.y) 
    {
        auto characterId = static_cast<uint16_t>(current_pos_.y * 7 + current_pos_.x);
        
        GAME_APP.GetPlayerManager().GetMyPlayer()->SetCharacterId(characterId);
        NETWORK.ChangeCharSelect(current_pos_.x, current_pos_.y);
    }
}

void CharacterSelectState::HandleCharacterSelection() 
{
    if (is_selected_) 
    {
        return;
    }

    if (current_pos_.x == 3 && current_pos_.y == 0) 
    {  
        SelectRandomCharacter();
    }
    else 
    {
        auto characterId = static_cast<uint16_t>(current_pos_.y * 7 + current_pos_.x);
        GAME_APP.GetPlayerManager().GetMyPlayer()->SetCharacterId(characterId);
        NETWORK.DecideCharacter(current_pos_.x, current_pos_.y);
    }

    is_selected_ = true;
    UpdateStartButton();
}

void CharacterSelectState::SelectRandomCharacter() 
{
    std::random_device rd;
    std::mt19937 gen(rd());

    while (true) 
    {
        current_pos_.x = std::uniform_int_distribution<>(0, 6)(gen);
        current_pos_.y = std::uniform_int_distribution<>(0, 3)(gen);

        uint8_t idx = current_pos_.y * 7 + current_pos_.x;
        if (character_info_[idx] && character_info_[idx]->largePortrait) {
            break;
        }
    }

    auto characterId = static_cast<uint16_t>(current_pos_.y * 7 + current_pos_.x);

    GAME_APP.GetPlayerManager().GetMyPlayer()->SetCharacterId(characterId);
    NETWORK.DecideCharacter(current_pos_.x, current_pos_.y);
}

void CharacterSelectState::UpdateStartButton() 
{
    if (is_selected_ && is_enemy_selected_ && NETWORK.IsServer()) 
    {
        if (start_button_) {
            start_button_->SetVisible(true);
        }
    }
}

void CharacterSelectState::Render() 
{
    // 배경 렌더링
    if (backgrounds_[0]) {
        backgrounds_[0]->Render(0, 0);
    }
    if (backgrounds_[1]) {
        backgrounds_[1]->Render(512, 0);
    }

    // 캐릭터 프리뷰 렌더링
    RenderCharacterPreviews();

    // 선택 UI 렌더링
    if (select_background_) {
        select_background_->Render(CHAR_SELECT_BG_POS_X, CHAR_SELECT_BG_POS_Y);
    }

    // 캐릭터 그리드 렌더링
    RenderCharacterGrid();

    // 커서 및 선택 마커 렌더링
    RenderSelectionUI();

    // 시작 버튼 렌더링
    if (start_button_) {
        start_button_->Render();
    }
}

void CharacterSelectState::RenderCharacterPreviews() 
{
    uint8_t playerIdx = current_pos_.y * 7 + current_pos_.x;
    uint8_t enemyIdx = enemy_pos_.y * 7 + enemy_pos_.x;

    // 플레이어 캐릭터 프리뷰
    if (character_info_[playerIdx] && character_info_[playerIdx]->largePortrait) {
        character_info_[playerIdx]->largePortrait->Render(0, 192);
    }

    // 상대방 캐릭터 프리뷰
    if (character_info_[enemyIdx] && character_info_[enemyIdx]->largePortrait) {
        character_info_[enemyIdx]->largePortrait->Render(GAME_APP.GetWindowWidth() - 256.0f, 192.0f, nullptr, 0, nullptr, SDL_FLIP_HORIZONTAL);
    }
}

void CharacterSelectState::RenderCharacterGrid()
{
    for (int y = 0; y < 4; ++y) 
    {
        for (int x = 0; x < 7; ++x) 
        {
            uint8_t idx = y * 7 + x;
            if (character_info_[idx] && character_info_[idx]->smallPortrait) 
            {
                character_info_[idx]->smallPortrait->Render(CHAR_SELECT_BG_POS_X + 2.0f + x * 65.0f, CHAR_SELECT_BG_POS_Y + 2.0f + y * 51.0f);
            }
        }
    }
}

void CharacterSelectState::RenderSelectionUI() 
{
    // 플레이어 선택 UI
    if (!is_selected_) 
    {
        if (selection_box_) 
        {
            selection_box_->Render(CHAR_SELECT_BG_POS_X - 5.0f + current_pos_.x * 65.0f, CHAR_SELECT_BG_POS_Y - 5.0f + current_pos_.y * 51.0f);
        }
    }
    else if (decide_marker_) 
    {
        decide_marker_->Render(CHAR_SELECT_BG_POS_X + current_pos_.x * 66.0f, CHAR_SELECT_BG_POS_Y - 6.0f + current_pos_.y * 52.0f);
    }

    // 상대방 선택 UI
    if (!is_enemy_selected_) 
    {
        if (selection_box_) 
        {
            selection_box_->Render(CHAR_SELECT_BG_POS_X - 5 + enemy_pos_.x * 65.0f, CHAR_SELECT_BG_POS_Y - 5.0f + enemy_pos_.y * 51.0f);
        }
    }
    else if (decide_marker_) 
    {
        decide_marker_->Render(CHAR_SELECT_BG_POS_X + enemy_pos_.x * 66.0f, CHAR_SELECT_BG_POS_Y - 6.0f + enemy_pos_.y * 52.0f);
    }

    // 플레이어/적 커서
    if (player_cursor_) 
    {
        player_cursor_->Render(CHAR_SELECT_BG_POS_X + 25.0f + current_pos_.x * 65.0f, CHAR_SELECT_BG_POS_Y + 20.0f + current_pos_.y * 51.0f);
    }

    if (enemy_cursor_) 
    {
        enemy_cursor_->Render(CHAR_SELECT_BG_POS_X + 25.0f + enemy_pos_.x * 65.0f, CHAR_SELECT_BG_POS_Y + 20 + enemy_pos_.y * 51.0f);
    }
}

void CharacterSelectState::Release() 
{
    for (auto& info : character_info_) 
    {
        info.reset();
    }

    // UI 리소스 해제
    start_button_.reset();
    backgrounds_[0].reset();
    backgrounds_[1].reset();
    player_cursor_.reset();
    enemy_cursor_.reset();
    selection_box_.reset();
    decide_marker_.reset();
    select_background_.reset();
}

void CharacterSelectState::SetEnemySelectPos(uint8_t x, uint8_t y) 
{
    enemy_pos_.x = x;
    enemy_pos_.y = y;
}

void CharacterSelectState::SetEnemyDecide(uint8_t x, uint8_t y) 
{
    enemy_pos_.x = x;
    enemy_pos_.y = y;
    is_enemy_selected_ = true;

    if (is_selected_ && is_enemy_selected_ && NETWORK.IsServer()) 
    {
        if (start_button_) 
        {
            start_button_->SetVisible(true);
        }
    }
}

bool CharacterSelectState::RequireGameStart() 
{
    NETWORK.StartGame();
    
    GAME_APP.GetStateManager().ChangeState(StateManager::StateID::Game);
    return true;    
}

void CharacterSelectState::HandleNetworkMessage(uint8_t connectionId, std::span<const char> data, uint32_t length)
{
    packet_processor_.ProcessPacket(connectionId, data, length);
}

void CharacterSelectState::HandleChangeCharSelect(uint8_t connectionId, const ChangeCharSelectPacket* packet)
{
    if (auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id))
    {
        SetEnemySelectPos(packet->x_pos, packet->y_pos);
    }
}

void CharacterSelectState::HandleDecideCharSelect(uint8_t connectionId, const DecideCharacterPacket* packet)
{
    if (auto player = GAME_APP.GetPlayerManager().FindPlayer(packet->player_id))
    {
        SetEnemyDecide(packet->x_pos, packet->y_pos);
        player->SetCharacterId(packet->y_pos * 7 + packet->x_pos);
    }
}

void CharacterSelectState::HandleStartGame(uint8_t connectionId, const PacketBase* packet)
{
    GAME_APP.GetStateManager().ChangeState(StateManager::StateID::Game);
}

bool CharacterSelectState::CanMoveDown() 
{
    if ((current_pos_.x == 0 || current_pos_.x == 6) && current_pos_.y == 1) {
        return false;
    }
    if (current_pos_.x == 3 && current_pos_.y == 2) {
        return false;
    }
    return current_pos_.y < 3;
}


bool CharacterSelectState::CanMoveLeft()
{
    if (current_pos_.x == 1 && current_pos_.y > 1) 
    {
        return false;
    }
    if (current_pos_.x == 4 && current_pos_.y == 3) 
    {
        current_pos_.x -= 2;
        return true;
    }
    return current_pos_.x > 0;
}

bool CharacterSelectState::CanMoveRight()
{
    if (current_pos_.x == 5 && current_pos_.y > 1) 
    {
        return false;
    }
    if (current_pos_.x == 2 && current_pos_.y == 3) 
    {
        return true;
    }
    return current_pos_.x < 6;
}

void CharacterSelectState::Update(float deltaTime) 
{
}

bool CharacterSelectState::LoadBackgrounds()
{
    try
    {
        const std::array<std::string, 4> paths =
        {
            "CHARACTERSELECT/CHARSELE_MENU/charselect_bg_b_2p_Cell_001.png",
            "CHARACTERSELECT/CHARSELE_MENU/charselect_bg_b_2p_Cell_002.png",
            "CHARACTERSELECT/CHARSELE_MENU/charselect_bg_on.png",
            "CHARACTERSELECT/CHARSELE_MENU/0.png"
        };

        backgrounds_[0] = ImageTexture::Create(paths[0]);
        backgrounds_[1] = ImageTexture::Create(paths[1]);
        select_background_ = ImageTexture::Create(paths[2]);
        decide_marker_ = ImageTexture::Create(paths[3]);

        if (!backgrounds_[0] || !backgrounds_[1] || !select_background_ || !decide_marker_)
        {
            throw std::runtime_error("Failed to load background textures");
        }

        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to load backgrounds: %s", e.what());
        return false;
    }
}

bool CharacterSelectState::LoadCharacterResources()
{
    try
    {
        const std::string smallPath = "CHARACTERSELECT/CHARSELE_MENU/charselect_charicon_";
        const std::string largePath = "CHARACTERSELECT/VS2P_CHAR/charselect_vs2p_char_";

        uint8_t fileIdx = 0;
        for (uint8_t y = 0; y < Constants::Game::CharacterSelect::CHARACTER_GRID_HEIGHT; ++y)
        {
            for (uint8_t x = 0; x < Constants::Game::CharacterSelect::CHARACTER_GRID_WIDTH; ++x)
            {
                if ((y == 0 && x == 3) || (y == 3 && x == 3) ||
                    ((x == 0 || x == 6) && y >= 2)) 
                {
                    continue;
                }

                uint8_t idx = y * Constants::Game::CharacterSelect::CHARACTER_GRID_WIDTH + x;
                character_info_[idx] = std::make_unique<CharacterImageInfo>();
                auto& info = character_info_[idx];
                info->characterIndex = fileIdx;

                auto smallPortraitPath = std::format("{}_{:02d}.png", smallPath, fileIdx);
                auto largePortraitPath = std::format("{}_{:02d}.png", largePath, fileIdx);

                info->smallPortrait = ImageTexture::Create(smallPortraitPath);
                info->largePortrait = ImageTexture::Create(largePortraitPath);

                if (!info->smallPortrait || !info->largePortrait) {
                    throw std::runtime_error("Failed to load character textures");
                }

                ++fileIdx;
            }
        }

        // 특수 슬롯(Random, Null) 처리
        character_info_[3] = std::make_unique<CharacterImageInfo>();
        character_info_[24] = std::make_unique<CharacterImageInfo>();

        character_info_[3]->smallPortrait = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_random.png");
        character_info_[24]->smallPortrait = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_null.png");

        if (!character_info_[3]->smallPortrait || !character_info_[24]->smallPortrait)
        {
            throw std::runtime_error("Failed to load special slot textures");
        }

        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to load character resources: %s", e.what());
        return false;
    }
}

bool CharacterSelectState::LoadUIElements()
{
    try
    {
        // UI 텍스처 로드
        selection_box_ = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_cursor_large.png");
        player_cursor_ = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_cursor_1p.png");
        enemy_cursor_ = ImageTexture::Create("CHARACTERSELECT/CHARSELE_MENU/charselect_cursor_2p.png");

        if (!selection_box_ || !player_cursor_ || !enemy_cursor_)
        {
            throw std::runtime_error("Failed to load UI textures");
        }

        // 버튼 초기화
        start_button_ = std::make_shared<Button>();
        auto buttonTex = ImageTexture::Create("UI/BUTTON/button.png");

        if (!buttonTex)
        {
            throw std::runtime_error("Failed to load button texture");
        }

        start_button_->Init(buttonTex,
            (GAME_APP.GetWindowWidth() - 136) / 2.0f,
            7.0f,
            136.0f,
            49.0f);

        start_button_->SetStateRect(Button::State::Normal, SDL_FRect{ 0, 0, 136, 49 });
        start_button_->SetStateRect(Button::State::Hover, SDL_FRect{ 0, 50, 136, 49 });
        start_button_->SetEventCallback(Button::State::Down,
            [this]()
            {
                return RequireGameStart();
            });

        return true;
    }
    catch (const std::exception& e)
    {
        SDL_LOG_ERROR(SDL_LOG_CATEGORY_APPLICATION, "Failed to load UI elements: %s", e.what());
        return false;
    }
}