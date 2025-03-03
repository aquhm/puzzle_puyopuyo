////#pragma once
/////*
//// *
//// * 설명: 게임 진행 상태
//// *
//// */
////
//#include <SDL3/SDL.h>
//#include <memory>
//#include <vector>
//#include <deque>
//#include <set>
//#include <list>
//#include <array>
//#include <string_view>
//#include <span>
//#include <functional>
//
//#include "BaseState.hpp"
//#include "../core/common/constants/Constants.hpp"
//#include "../core/common/types/GameTypes.hpp"
//#include "../game/block/Block.hpp"
//#include "../network/packets/GamePackets.hpp"
//#include "../network/NetworkController.hpp"
//#include "../network/PacketProcessor.hpp"
//#include "../game/system/LocalPlayer.hpp"
//#include <map>
//#include "../game/system/RemotePlayer.hpp"
//
//
//class NetworkController;
//
//class Player;
//class PlayerManager;
//class RenderableObject;
//class GameBackground;
//class GameBoard;
//class GroupBlock;
//class GameGroupBlock;
//class IceBlock;
//class BulletEffect;
//class InterruptBlockView;
//class ComboView;
//class EditBox;
//class ResultView;
//class Button;
//class ImageTexture;
//class GamePlayer;
//class PacketProcessor;
//
//namespace GameStateDetail 
//{
//    constexpr Constants::Direction GetOppositeDirection(Constants::Direction dir)
//    {
//        switch (dir) 
//        {
//        case Constants::Direction::Left:   
//            return Constants::Direction::Right;
//        case Constants::Direction::Right:  
//            return Constants::Direction::Left;
//        case Constants::Direction::Top:    
//            return Constants::Direction::Bottom;
//        case Constants::Direction::Bottom: 
//            return Constants::Direction::Top;
//        default:               
//            return Constants::Direction::None;
//        }
//    }
//}
//
//// 게임 상태 열거형
//enum class GamePhase 
//{
//    Standing,   // 대기 상태
//    Playing,    // 게임 진행 중
//    Shattering, // 블록 파괴 중
//    IceBlocking,// 방해 블록 생성 중
//    GameOver    // 게임 종료
//};
//
//// 블록 위치 마커 구조체
//struct BlockPositionMarker 
//{
//    float xPos;
//    float yPos;
//    BlockType type;
//};
//
//class GameState final : public BaseState 
//{
//public:
//    GameState();
//    ~GameState() override;
//
//    bool Init() override;
//    void Enter() override;
//    void Leave() override;
//    void Update(float deltaTime) override;
//    void Render() override;
//    void Release() override;
//    void HandleEvent(const SDL_Event& event) override;
//    void HandleNetworkMessage(uint8_t connectionId, std::span<const char> data, uint32_t length) override;
//
//    [[nodiscard]] std::string_view GetStateName() const override { return "Game"; }
//
//    bool GameRestart();
//    bool GameExit();
//    void GameQuit();
//
//    bool SendChatMsg();
//
//    [[nodiscard]] const std::shared_ptr<EditBox>& GetEditBox() { return chatbox_; }
//
//    void CreateRemotePlayer(const std::span<const uint8_t>& blocktype1,
//        const std::span<const uint8_t>& blocktype2,
//        uint8_t playerIdx,
//        uint8_t characterIdx);
//
//    [[nodiscard]] const std::shared_ptr<LocalPlayer>& GetLocalPlayer() { return local_player_; }
//    [[nodiscard]] const std::shared_ptr<RemotePlayer>& const GetRemotePlayer(uint8_t playerId);
//
//    [[nodiscard]] int16_t GetTotalInterruptBlockCount() const;
//    [[nodiscard]] int16_t GetTotalInterruptEnemyBlockCount() const;
//    void SetTotalInterruptEnemyBlockCount(int16_t count);
//
//private:
//
//    bool LoadResources();
//    bool CreateUI();
//    void InitializePacketHandlers();
//
//    void HandleMouseInput(const SDL_Event& event);
//    //void HandleKeyboardState();
//
//    void HandleGameInitialize(uint8_t connectionId, const GameInitPacket* packet);
//    void HandleAddNewBlock(uint8_t connectionId, const AddNewBlockPacket* packet);
//    void HandleUpdateBlockMove(uint8_t connectionId, const MoveBlockPacket* packet);
//    void HandleBlockRotate(uint8_t connectionId, const RotateBlockPacket* packet);
//    void HandleStartGame();
//    void HandleCheckBlockState(uint8_t connectionId, const CheckBlockStatePacket* packet);
//    void HandleGameOver();
//
//    void RenderUI();
//
//private:
//    
//    std::shared_ptr<LocalPlayer> local_player_;
//    std::map<uint8_t, std::shared_ptr<RemotePlayer>> remote_players_;
//
//    std::shared_ptr<GameBackground> background_;
//
//    std::shared_ptr<EditBox> chatbox_;
//    std::shared_ptr<Button> restart_button_;
//    std::shared_ptr<Button> exit_button_;
//
//    std::vector<RenderableObject*> draw_objects_;
//
//    uint64_t last_input_time_{ 0 };
//    bool initialized_{ false };
//
//    bool is_network_game_{ false };
//    uint8_t local_player_id_{ 0 };
//    PacketProcessor packet_processor_{};    
//};


#pragma once
/*
 *
 * 설명: 게임 진행 상태 관리자
 *
 */

#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <span>
#include <string_view>
#include <array>

#include "BaseState.hpp"
#include "../core/common/types/GameTypes.hpp"
#include "../network/PacketProcessor.hpp"
#include "../network/NetworkController.hpp"
#include "../game/block/Block.hpp"
#include "../core/common/constants/Constants.hpp"

class EditBox;
class Button;
class GameBackground;
class InterruptBlockView;
class ComboView;
class ResultView;
class IGamePlayer;
class LocalPlayer;
class RemotePlayer;
class Player;
class NetworkController;
class ClientInfo;

namespace GameStateDetail
{
    constexpr Constants::Direction GetOppositeDirection(Constants::Direction dir)
    {
        switch (dir)
        {
        case Constants::Direction::Left:
            return Constants::Direction::Right;
        case Constants::Direction::Right:
            return Constants::Direction::Left;
        case Constants::Direction::Top:
            return Constants::Direction::Bottom;
        case Constants::Direction::Bottom:
            return Constants::Direction::Top;
        default:
            return Constants::Direction::None;
        }
    }
}

// 게임 상태 열거형
enum class GamePhase
{
    Standing,   // 대기 상태
    Playing,    // 게임 진행 중
    Shattering, // 블록 파괴 중
    IceBlocking,// 방해 블록 생성 중
    GameOver    // 게임 종료
};

// 블록 위치 마커 구조체
struct BlockPositionMarker
{
    float xPos;
    float yPos;
    BlockType type;
};



class GameState final : public BaseState
{
public:
    GameState();
    ~GameState() override;

    // 복사/이동 생성자 삭제
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
    void Release() override;
    void HandleEvent(const SDL_Event& event) override;
    void HandleNetworkMessage(uint8_t connectionId, std::span<const char> data, uint32_t length) override;

    // 게임 상태 관리
    [[nodiscard]] std::string_view GetStateName() const override { return "Game"; }
    [[nodiscard]] const std::shared_ptr<LocalPlayer>& GetLocalPlayer() const { return local_player_; }
    [[nodiscard]] const std::shared_ptr<RemotePlayer>& GetRemotePlayer() const { return remote_player_; }
    [[nodiscard]] GameBackground* GetBackGround() const { return background_.get(); }
    [[nodiscard]] Block* (*GetGameBlocks(uint8_t playerId))[Constants::Board::BOARD_X_COUNT];

    // 게임 Flow 제어
    bool GameRestart();
    bool GameExit();
    void GameQuit();
    void CreateGamePlayer(const std::span<const uint8_t>& blocktype1, const std::span<const uint8_t>& blocktype2,
        uint8_t playerIdx, uint8_t characterIdx);

    void ScheduleGameStart();


    // UI 관련
    [[nodiscard]] const std::shared_ptr<EditBox>& GetEditBox() { return chatbox_; }
    bool SendChatMsg();

private:
    // 초기화 관련
    bool LoadResources();
    bool CreateUI();
    bool InitializeComponents();

    // 이벤트 핸들링
    void HandleMouseInput(const SDL_Event& event);
    void HandleKeyboardInput(const SDL_Event& event);
    void HandleKeyboardState();
    void HandleSystemEvent(const SDL_Event& event);

    // 렌더링 관련
    void RenderUI();
#ifdef _DEBUG
    void RenderDebugInfo();
    void RenderDebugGrid();
#endif

    // 패킷 프로세서 관련
    void InitializePacketHandlers();

    // 패킷 핸들러
    void HandleGameInitialize(uint8_t connectionId, const GameInitPacket* packet);
    void HandleAddNewBlock(uint8_t connectionId, const AddNewBlockPacket* packet);
    void HandleUpdateBlockMove(uint8_t connectionId, const MoveBlockPacket* packet);
    void HandleBlockRotate(uint8_t connectionId, const RotateBlockPacket* packet);
    void HandleStartGame();
    void HandleCheckBlockState(uint8_t connectionId, const CheckBlockStatePacket* packet);
    void HandleGameOver();

private:
    // 플레이어 구성 요소
    std::shared_ptr<LocalPlayer> local_player_;
    std::shared_ptr<RemotePlayer> remote_player_;
    std::shared_ptr<GameBackground> background_;

    // UI 컴포넌트
    std::shared_ptr<EditBox> chatbox_;
    std::shared_ptr<Button> restart_button_;
    std::shared_ptr<Button> exit_button_;

    std::shared_ptr<InterruptBlockView> interrupt_view_;
    std::shared_ptr<ComboView> combo_view_;
    std::shared_ptr<ResultView> result_view_;

    // 상태 변수
    uint64_t lastInputTime_{ 0 };
    bool initialized_{ false };
    bool isNetworkGame_{ false };
    uint8_t localPlayerId_{ 0 };
    bool shouldQuit_{ false };

    // 패킷 프로세서
    PacketProcessor packet_processor_{};
};