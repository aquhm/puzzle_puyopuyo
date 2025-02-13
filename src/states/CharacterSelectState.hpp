#pragma once

#include "BaseState.hpp"
//#include "core/ResourceManager.hpp"
//#include "ui/Button.hpp"
#include <array>
#include <vector>
#include <optional>

class CharacterSelectState final : public BaseState {
public:
    CharacterSelectState();
    ~CharacterSelectState() override;

    // BaseState 인터페이스 구현
    bool Init() override;
    void Enter() override;
    void Leave() override;
    void Update(float deltaTime) override;
    void Render() override;
    virtual void Release() = 0;
    /*void HandleEvent(const SDL_Event& event) override;
    void HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length) override;*/

    [[nodiscard]] std::string_view getStateName() const override {
        return "CharacterSelect";
    }

private:
    struct CharacterInfo {
        uint8_t characterIndex{ 0 };
        std::unique_ptr<Texture> smallPortrait;
        std::unique_ptr<Texture> largePortrait;
    };

    struct Position {
        int x{ 0 };
        int y{ 0 };
    };

    // 리소스 관리
    std::array<std::unique_ptr<Texture>, 2> backgrounds;
    std::unique_ptr<Texture> selectBackground;
    std::unique_ptr<Texture> selectionBox;
    std::unique_ptr<Texture> playerCursor;
    std::unique_ptr<Texture> enemyCursor;
    std::unique_ptr<Texture> decideMarker;
    std::unique_ptr<Button> startButton;

    // 캐릭터 정보
    std::vector<std::optional<CharacterInfo>> characters;

    // 선택 상태
    Position currentPos;
    Position enemyPos;
    bool isSelected{ false };
    bool isEnemySelected{ false };

    // 내부 구현 메서드들
    void initializeCharacters();
    void updateSelection(const SDL_Event& event);
    void handleCharacterSelection();
    bool requestGameStart();
    void setEnemySelection(uint8_t x, uint8_t y);
    void updateUIState();

    // 리소스 로딩 헬퍼 함수들
    bool loadBackgrounds();
    bool loadCharacterResources();
    bool loadUIElements();
};