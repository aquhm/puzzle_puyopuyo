#pragma once
/**
 *
 * 설명: 원격 플레이어 구현 (네트워크를 통해 다른 플레이어의 게임을 표시)
 *
 */
#include "BasePlayer.hpp"
#include <vector>
#include <set>
#include <deque>
#include <list>

class Block;
class GroupBlock;
class IceBlock;
class BulletEffect;
class ImageTexture;

class RemotePlayer : public BasePlayer 
{
public:

    RemotePlayer();
    ~RemotePlayer() override;

    bool Initialize(const std::span<const uint8_t>& blocktype1,
        const std::span<const uint8_t>& blocktype2,
        uint8_t playerIdx,
        uint16_t characterIdx,
        const std::shared_ptr<GameBackground>& background) override;

    void Release() override;
    void Reset() override;
    bool Restart(const std::span<const uint8_t>& blockType1 = {}, const std::span<const uint8_t>& blockType2 = {}) override;
    void CreateNextBlock() override;
    void PlayNextBlock() override;
    bool CheckGameBlockState() override;
    void CreateBullet(Block* block) override;

    void MoveBlock(uint8_t moveType, float position) override;
    void RotateBlock(uint8_t rotateType, bool horizontalMoving) override;
    void UpdateBlockPosition(float pos1, float pos2) override;
    void UpdateFallingBlock(uint8_t fallingIdx, bool falling) override;
    void ChangeBlockState(uint8_t state) override;
    bool PushBlockInGame(const std::span<const float>& pos1, const std::span<const float>& pos2);
    void AddNewBlock(const std::span<const uint8_t, 2>& block_type);
    void AttackInterruptBlock(float x, float y, uint8_t type) override;
    void DefenseInterruptBlockCount(int16_t count, float x, float y, uint8_t type) override;

    // 방해 블록 관련 추가 메서드
    using BasePlayer::AddInterruptBlock;
    void AddInterruptBlock(uint8_t y_row_cnt, const std::span<const uint8_t>& x_idx);
    void AddInterruptBlockCnt(short cnt, float x, float y, unsigned char type);

    // 캐릭터 관련
    void SetCharacterID(int16_t charID) { character_id_ = charID; }
    [[nodiscard]] int16_t GetCharacterID() const { return character_id_; }

    // 상태 업데이트 메서드
    void UpdateGameState(float deltaTime);
    
    // 블록 위치
    void SyncPositionY(float targetY, float velocity);

private:
    // 초기화 메서드
    void InitializeNextBlocks(const std::span<const uint8_t>& blockType1, const std::span<const uint8_t>& blockType2);

    // 방해 블록 관련 메서드
    void CreateFullRowInterruptBlocks(std::shared_ptr<ImageTexture>& texture);
    void CreatePartialRowInterruptBlocks(uint8_t y_row_cnt, const std::span<const uint8_t>& x_idx, std::shared_ptr<ImageTexture>& texture);
    void CreateSingleIceBlock(int x, int y, std::shared_ptr<ImageTexture>& texture);

    // 게임 상태 업데이트 메서드
    void UpdateGameOverState(float deltaTime);
    void UpdatePlayingState(float deltaTime);
    void UpdateIceBlockDowningState();
    void UpdateShatteringState();
    void UpdateMatchedBlocks();
    void HandleClearedBlockGroup(std::list<BlockVector>::iterator& group_it, SDL_FPoint& pos, SDL_Point& pos_idx, std::list<SDL_Point>& x_index_list);
    void UpdateAfterBlocksCleared();
    void UpdateComboDisplay(const SDL_FPoint& pos);

private:
    // 동기화용 변수들
    float target_y_position_{ 0.0f };
    float current_sync_velocity_{ 0.0f };
    bool is_syncing_position_{ false };
    float sync_lerp_factor_{ 0.15f };
};