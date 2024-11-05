#ifndef BOARD_HPP
#define BOARD_HPP

#include "block.hpp"

class Board
{
public:
    Board() {};
    void Init();

    void LockBlock(const Block& block);

    int CheckFullRow();
    bool CheckFullClear()                                       const;
    bool CheckFit(int offsetX, int offsetY, const Block& block) const;
    BlockType GetCell(int posX, int posY)                       const;

    array<array<BlockType, BOARD_HEIGHT>, BOARD_WIDTH>& GetBoard();

private:
    array<array<BlockType, BOARD_HEIGHT>, BOARD_WIDTH> board;

    void ClearRow(int row);
    void MoveRowDown(int row);
};

#endif /* BOARD_HPP */
