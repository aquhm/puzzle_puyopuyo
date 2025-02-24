#pragma once

#include <array>
#include <memory>
#include <list>
#include "../constants/Constants.hpp"


namespace GameTypes 
{
    namespace BoardConst = Constants::Board;

    class Block;
    using BlockPtr = std::shared_ptr<Block>;
    using BlockList = std::list<BlockPtr>;

    using BoardArray = std::array<std::array<BlockPtr, BoardConst::BOARD_X_COUNT>, BoardConst::BOARD_Y_COUNT> ;
}
