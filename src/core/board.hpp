#ifndef BOARD_HPP
#define BOARD_HPP

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 22

#include "block.hpp"

class Board
{
public:
    Board();

    void LockBlock(const Block& block);

    int CheckFullRow();
    bool CheckFullClear()                                       const;
    bool CheckFit(int offsetX, int offsetY, const Block& block) const;
    BlockType GetCell(int posX, int posY)                       const;

    array<array<BlockType, BOARD_HEIGHT>, BOARD_WIDTH>& GetBoard();

private:
    array<array<BlockType, BOARD_HEIGHT>, BOARD_WIDTH> board;

    void ClearRow(int startRow);
    void MoveRowDown(int row);
};

#endif /* BOARD_HPP */
