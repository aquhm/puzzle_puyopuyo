
// game/BlockMatcher.cpp
#include "BlockMatcher.hpp"

std::vector<BlockMatcher::MatchResult> BlockMatcher::findMatches(
    const std::vector<std::vector<std::shared_ptr<Block>>>& board) {

    std::vector<MatchResult> results;
    std::vector<std::vector<bool>> visited(board.size(),
        std::vector<bool>(board[0].size(), false));

    // 보드 전체를 순회하며 매칭 검사
    for (size_t y = 0; y < board.size(); ++y) {
        for (size_t x = 0; x < board[y].size(); ++x) {
            if (!visited[y][x] && board[y][x]) {
                auto result = checkMatch(board, x, y, visited);
                if (result) {
                    results.push_back(std::move(*result));
                }
            }
        }
    }

    return results;
}