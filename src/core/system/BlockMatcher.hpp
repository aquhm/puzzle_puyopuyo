// game/BlockMatcher.hpp
#pragma once

#include "Block.hpp"
#include <vector>
#include <memory>
#include <optional>

class BlockMatcher {
public:
    struct MatchResult {
        std::vector<std::shared_ptr<Block>> matchedBlocks;
        int chainCount{ 0 };
        bool includesSpecialBlock{ false };
    };

    struct MatchInfo {
        int x;
        int y;
        BlockType type;
        int count;
    };

    // 블록 매칭 검색 방향
    enum class Direction : uint8_t {
        None = 0,
        Left = 1 << 0,
        Right = 1 << 1,
        Up = 1 << 2,
        Down = 1 << 3
    };

    static std::vector<MatchResult> findMatches(const std::vector<std::vector<std::shared_ptr<Block>>>& board);

private:
    static std::optional<MatchResult> checkMatch(const std::vector<std::vector<std::shared_ptr<Block>>>& board,
        int startX, int startY,
        std::vector<std::vector<bool>>& visited);

    static void floodFill(const std::vector<std::vector<std::shared_ptr<Block>>>& board,
        int x, int y, BlockType targetType,
        std::vector<std::vector<bool>>& visited,
        std::vector<std::shared_ptr<Block>>& matches);
};
