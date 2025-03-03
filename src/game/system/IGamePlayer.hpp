#pragma once
/**
 *
 * ����: ���� �÷��̾� ���� �������̽�
 *
 */
#include <memory>
#include <span>
#include "../RenderableObject.hpp"
#include "../../core/common/constants/Constants.hpp"
#include "../../core/common/types/GameTypes.hpp"
#include "../../states/GameState.hpp"

class Block;
class GameBackground;
class GameBoard;
class GroupBlock;
class GameGroupBlock;
class InterruptBlockView;
class ComboView;
class ResultView;
class ImageTexture;

// ���� �÷��̾� ���� �������̽�
class IGamePlayer 
{
public:
    virtual ~IGamePlayer() = default;

    // �ٽ� ���
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void Reset() = 0;
    virtual void Release() = 0;

    // �ʱ�ȭ �� �����
    virtual bool Initialize(const std::span<const uint8_t>& blocktype1,
        const std::span<const uint8_t>& blocktype2,
        uint8_t playerIdx,
        uint8_t characterIdx,
        const std::shared_ptr<GameBackground>& background) = 0;

    virtual bool Restart(const std::span<const uint8_t>& blockType1,
        const std::span<const uint8_t>& blockType2) = 0;

    // ��� ����
    virtual void AddNewBlock(const std::span<const uint8_t, 2>& blockType) = 0;
    virtual void DestroyNextBlock() = 0;
    virtual bool CheckGameBlockState() = 0;

    // ��� ����
    virtual void MoveBlock(uint8_t moveType, float position) = 0;
    virtual void RotateBlock(uint8_t rotateType, bool horizontalMoving) = 0;
    virtual void UpdateBlockPosition(float pos1, float pos2) = 0;
    virtual void UpdateFallingBlock(uint8_t fallingIdx, bool falling) = 0;
    virtual void ChangeBlockState(uint8_t state) = 0;

    // ���� ��� ����
    virtual void AddInterruptBlock(int16_t count) = 0;
    virtual void AttackInterruptBlock(float x, float y, uint8_t type) = 0;
    virtual void DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type) = 0;
    virtual void UpdateInterruptBlock(int16_t count) = 0;

    // ���� ���� ����
    virtual void LoseGame(bool isWin) = 0;
    virtual void SetGameQuit() = 0;

    // ���� ���� ����
    virtual Block* (*GetGameBlocks())[Constants::Board::BOARD_X_COUNT] = 0;

    // ���� ��ȸ
    virtual GamePhase GetGameState() const = 0;
    virtual uint8_t GetPlayerID() const = 0;
    virtual int16_t GetTotalInterruptBlockCount() const = 0;

    // ������Ʈ ����
    virtual std::shared_ptr<GameBoard> GetGameBoard() const = 0;
};