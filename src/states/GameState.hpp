#pragma once
/*
 *
 * ����: ���� ���� ����
 *
 */

#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <deque>
#include <set>
#include <list>
#include <array>
#include <string_view>
#include <span>
#include <functional>

#include "BaseState.hpp"
#include "../core/common/constants/Constants.hpp"
#include "../core/common/types/GameTypes.hpp"
#include "../game/block/Block.hpp"
#include "../network/packets/GamePackets.hpp"
#include "../network/NetworkController.hpp"


class NetworkController;

class Player;
class PlayerManager;
class RenderableObject;
class GameBackground;
class GameBoard;
class GroupBlock;
class GameGroupBlock;
class IceBlock;
class BulletEffect;
class InterruptBlockView;
class ComboView;
class EditBox;
class ResultView;
class Button;
class ImageTexture;
class GamePlayer;

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

// ���� ���� ������
enum class GamePhase 
{
    Standing,   // ��� ����
    Playing,    // ���� ���� ��
    Shattering, // ��� �ı� ��
    IceBlocking,// ���� ��� ���� ��
    GameOver    // ���� ����
};

// ��� ��ġ ��Ŀ ����ü
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

    // ����/�̵� ������ ����
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
    void Release() override;
    void HandleEvent(const SDL_Event& event) override;
    void HandleNetworkMessage(uint8_t connectionId, std::string_view message, uint32_t length) override;

    // ���� ���� ��ȸ
    [[nodiscard]] std::string_view GetStateName() const override { return "Game"; }
    [[nodiscard]] GameBoard* GetGameBoard() const { return gameboard_.get(); }
    [[nodiscard]] Block* (*GetGameBlocks())[Constants::Board::BOARD_X_COUNT] { return board_blocks_; }
    [[nodiscard]] GameBackground* GetBackGround() const { return background_.get(); }
    [[nodiscard]] InterruptBlockView* GetInterruptBlockView() const { return interrupt_view_.get(); }
    [[nodiscard]] const std::shared_ptr<GamePlayer>& GetPlayer() const { return game_player_; }    

    // ��� ���� ����
    bool IsPossibleMove(int xIdx);
    void UpdateTargetPosIdx();

    // ���� ���� ����
    bool GameRestart();
    bool GameExit();
    void GameQuit();

    [[nodiscard]] bool IsGameOver();
    void BulletUpdate(float deltaTime);
    void CreateBullet(Block* block);
    void CreateGamePlayer(const std::span<const uint8_t>& blocktype1, const std::span<const uint8_t>& blocktype2, uint8_t playerIdx, uint8_t characterIdx);
    void DestroyNextBlock();
    void CreateBlocksFromFile(); // ���Ϸ� ���� ��� ����

    // ��� ���� �˻� ����
    void CollectRemoveIceBlocks();
    void UpdateLinkState(Block* block);
    void UpdateInterruptBlockView();    

    //Setter
    void SetAttackComboState(bool enable) { stateInfo_.isComboAttack = enable; }
    void SetTotalInterruptEnemyBlockCount(bool enable) { stateInfo_.isComboAttack = enable; }
    void DefenseInterruptBlockCount(short cnt, float x, float y, unsigned char type);

    void AddInterruptBlockCount(short cnt, float x, float y, unsigned char type);
    

    [[nodiscard]] short GetTotalInterruptBlockCount() { return scoreInfo_.totalInterruptBlockCount; }
    [[nodiscard]] short GetTotalInterruptEnemyBlockCount() { return scoreInfo_.totalEnemyInterruptBlockCount; }

    [[nodiscard]] const std::unique_ptr<EditBox>& GetEditBox() { return chatbox_; }

private:

    // ���� ��� ����
    void CalculateIceBlockCount();
    void UpdateComboState();
    void ResetComboState();

    // �ʱ�ȭ ����
    bool InitializeComponents();
    bool InitializeGameBoard();
    bool CreateUI();
    bool LoadResources();

    // ���� ����
    void UpdateGameLogic(float deltaTime);
    void ProcessMatchedBlocks();
    void HandlePhaseTransition(GamePhase newPhase);
    bool CheckGameBlockState();
    void UpdateBlockPositions();
    void CheckGameOverCondition();
    void HandleGameOver();

    void HandleGameInitialize(uint8_t connectionId, const GameInitPacket* packet);
    void HandleAddNewBlock(uint8_t connectionId, const AddNewBlockPacket* packet);
    void HandleUpdateBlockMove(uint8_t connectionId, const MoveBlockPacket* packet);
    void HandleBlockRotate(uint8_t connectionId, const RotateBlockPacket* packet);
    void HandleStartGame();  
    void HandleCheckBlockState(uint8_t connectionId, const CheckBlockStatePacket* packet);
    //void HandleComboUpdate(uint8_t connectionId, const char* payload, uint32_t size);


    void UpdateIceBlockPhase(float deltaTime);
    void UpdateShatteringPhase(float deltaTime);
    void UpdateGameOverPhase(float deltaTime);

    void UpdateInterruptBlockState();

    // ��� ����
    void InitNextBlock();
    void CreateNextBlock();    
    void ProcessBlockMatching();
    void UpdateBlockLinks();
    bool FindMatchedBlocks(std::vector<std::vector<Block*>>& matchedGroups);
    short RecursionCheckBlock(short x, short y, Constants::Direction direction, std::vector<Block*>& matchedBlocks);

    // ���� ���
    void CalculateScore();
    [[nodiscard]] short GetComboConstant(uint8_t comboCount) const;
    [[nodiscard]] uint8_t GetLinkBonus(size_t linkCount) const;
    [[nodiscard]] uint8_t GetTypeBonus(size_t count) const;
    [[nodiscard]] uint8_t GetMargin() const;

    // ���� ��� ����
    void GenerateIceBlocks();
    void GenerateLargeIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID);
    void GenerateSmallIceBlockGroup(const std::shared_ptr<ImageTexture>& texture, uint8_t playerID);
    void InitializeIceBlock(IceBlock* block, const std::shared_ptr<ImageTexture>& texture, int x, int y, uint8_t playerID);

    // �Է� ó��
    void HandleKeyboardState();
    void HandleMouseInput(const SDL_Event& event);
    void HandleKeyboardInput(const SDL_Event& event);
    void HandleSystemEvent(const SDL_Event& event);

    // ��Ʈ��ũ ����
    void HandleNetworkCommand(const std::string_view& command);
    bool SendChatMsg();

    void InitializePacketProcessors();

    template<typename T>
    void ProcessTypedPacket(uint8_t connectionId, std::span<const char> data, void (GameState::* handler)(uint8_t, const T*));

    // ������ ����
    void RenderUI();
#ifdef _DEBUG
    void RenderDebugInfo();
    void RenderDebugGrid();
#endif

private:
    // ���� ���� ���� ����ü
    struct GameStateInfo 
    {
        GamePhase currentPhase{ GamePhase::Standing };
        GamePhase previousPhase{ GamePhase::Standing };
        float playTime{ 0.0f };
        bool isRunning{ false };
        bool isDefending{ false };
        bool isAttacked{ false };
        bool isComboAttack{ false };
        bool shouldQuit{ false };
        uint8_t defenseCount{ 0 };
        bool hasIceBlock{ false };  // ���� ��� ���� ����
    } stateInfo_;

    // ���� ���� ����ü
    struct ScoreInfo 
    {
        uint32_t totalScore{ 0 };
        uint32_t restScore{ 0 };
        uint8_t comboCount{ 0 };
        int16_t totalInterruptBlockCount{ 0 };
        int16_t totalEnemyInterruptBlockCount{ 0 };
        int16_t addInterruptBlockCount{ 0 };

        void reset() 
        {
            totalScore = restScore = 0;
            comboCount = 0;
            totalInterruptBlockCount = 0;
            totalEnemyInterruptBlockCount = 0;
            addInterruptBlockCount = 0;
        }
    } scoreInfo_;

    // ���� ������Ʈ
    std::unique_ptr<GameBoard> gameboard_;
    std::shared_ptr<GameBackground> background_;
    std::unique_ptr<InterruptBlockView> interrupt_view_;
    std::unique_ptr<ComboView> combo_view_;
    std::unique_ptr<ResultView> result_view_;
    std::unique_ptr<GameGroupBlock> control_block_;
    std::shared_ptr<GamePlayer> game_player_;

    // UI ������Ʈ
    std::unique_ptr<EditBox> chatbox_;
    std::unique_ptr<Button> restart_button_;
    std::unique_ptr<Button> exit_button_;

    // ���� ���� ������
    Block* board_blocks_[Constants::Board::BOARD_Y_COUNT][Constants::Board::BOARD_X_COUNT]{ nullptr };
    std::vector<RenderableObject*> draw_objects_;
    std::deque<std::unique_ptr<GroupBlock>> next_blocks_;
    std::list<std::shared_ptr<Block>> block_list_;
    std::set<std::shared_ptr<IceBlock>> ice_blocks_;
    std::list<std::unique_ptr<BulletEffect>> bullets_;
    std::vector<BulletEffect*> bullets_to_delete_;
    std::vector<std::vector<Block*>> matched_blocks_;

    // �Է� ����
    uint64_t lastInputTime_{ 0 };
    bool initialized_{ false };

    // ��� ��Ī ���� �߰�
    struct MatchInfo 
    {
        std::vector<Block*> blocks;
        BlockType type;
        uint8_t count{ 0 };
    };

    // ���� ���� �߰�
    bool isNetworkGame_{ false };
    uint8_t localPlayerId_{ 0 };

    std::unordered_map<PacketType, std::function<void(uint8_t, std::span<const char>)>> packet_processors_;
};

template<typename T>
void GameState::ProcessTypedPacket(uint8_t connectionId, std::span<const char> data, void (GameState::* handler)(uint8_t, const T*))
{

    if (data.size() < sizeof(T))
    {
        return;
    }

    T packet;
    std::memcpy(&packet, data.data(), sizeof(T));
    (this->*handler)(connectionId, &packet);
}


class BlockComp 
{
public:
    bool operator()(const std::shared_ptr<Block>& lhs, const std::shared_ptr<Block>& rhs) const 
    {
        return *lhs < *rhs;
    }
};

// ��� ���� üũ�� ���� �Լ� ��ü
class CheckerBlockStateComp 
{
private:
    int checkCount_{ 0 };
    const BlockState stateType_;

public:
    explicit CheckerBlockStateComp(BlockState type)
        : stateType_(type) {
    }

    void operator()(const std::shared_ptr<Block>& block) 
    {
        if (block && block->GetState() == stateType_) 
        {
            ++checkCount_;
        }
    }

    [[nodiscard]] int GetCheckCount() const { return checkCount_; }
};