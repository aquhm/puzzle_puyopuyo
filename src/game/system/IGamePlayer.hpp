#pragma once
/**
 *
 * 설명: 게임 플레이어 공통 인터페이스
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

// 게임 플레이어 공통 인터페이스
class IGamePlayer 
{
public:
    virtual ~IGamePlayer() = default;

    // 핵심 기능
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void Reset() = 0;
    virtual void Release() = 0;

    // 초기화 및 재시작
    virtual bool Initialize(const std::span<const uint8_t>& blocktype1,
        const std::span<const uint8_t>& blocktype2,
        uint8_t playerIdx,
        uint8_t characterIdx,
        const std::shared_ptr<GameBackground>& background) = 0;

    virtual bool Restart(const std::span<const uint8_t>& blockType1,
        const std::span<const uint8_t>& blockType2) = 0;

    // 블록 관리
    virtual void AddNewBlock(const std::span<const uint8_t, 2>& blockType) = 0;
    virtual void DestroyNextBlock() = 0;
    virtual bool CheckGameBlockState() = 0;

    // 블록 조작
    virtual void MoveBlock(uint8_t moveType, float position) = 0;
    virtual void RotateBlock(uint8_t rotateType, bool horizontalMoving) = 0;
    virtual void UpdateBlockPosition(float pos1, float pos2) = 0;
    virtual void UpdateFallingBlock(uint8_t fallingIdx, bool falling) = 0;
    virtual void ChangeBlockState(uint8_t state) = 0;

    // 방해 블록 관련
    virtual void AddInterruptBlock(int16_t count) = 0;
    virtual void AttackInterruptBlock(float x, float y, uint8_t type) = 0;
    virtual void DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type) = 0;
    virtual void UpdateInterruptBlock(int16_t count) = 0;

    // 게임 상태 제어
    virtual void LoseGame(bool isWin) = 0;
    virtual void SetGameQuit() = 0;

    // 게임 보드 접근
    virtual Block* (*GetGameBlocks())[Constants::Board::BOARD_X_COUNT] = 0;

    // 상태 조회
    virtual GamePhase GetGameState() const = 0;
    virtual uint8_t GetPlayerID() const = 0;
    virtual int16_t GetTotalInterruptBlockCount() const = 0;

    // 컴포넌트 접근
    virtual std::shared_ptr<GameBoard> GetGameBoard() const = 0;
};