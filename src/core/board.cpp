#include "board.hpp"

void Board::LockBlock(const Block& block)
{
    const BlockType type = block.GetType();

    for (const Coord& coord : block.GetCoords())
        board[coord.x][coord.y] = type;
}

int Board::CheckFullRow()
{
    size_t count = 0;

    for (size_t i = 0; i < BOARD_HEIGHT; ++i)
    {
        bool fullRow = true;

        for (size_t j = 0; j < BOARD_WIDTH; ++j)
            if (board[j][i] == EMPTY)
            {
                fullRow = false;
                break;
            }

        if (fullRow)
        {
            count++;
            ClearRow(i);
            MoveRowDown(i);
        }
    }

    return count;
}

bool Board::CheckFullClear() const
{
    for (size_t i = 0; i < BOARD_WIDTH; ++i)
        for (size_t j = 0; j < BOARD_HEIGHT; ++j)
            if (board[i][j] != EMPTY)
                return false;

    return true;
}

bool Board::CheckFit(int offsetX, int offsetY, const Block& block) const
{
    for (const Coord& coord : block.GetCoords())
    {
        if (offsetX + coord.x >= BOARD_WIDTH
            || offsetY + coord.y >= BOARD_HEIGHT
            || offsetX + coord.x < 0
            || offsetY + coord.y < 0)
            return false;
        if (board[offsetX + coord.x][offsetY + coord.y] != EMPTY)
            return false;
    }
    return true;
}

BlockType Board::GetCell(int posX, int posY) const
{
    return board[posX][posY];
}

void Board::Init()
{
    for (size_t i = 0; i < BOARD_WIDTH; ++i)
        for (size_t j = 0; j < BOARD_HEIGHT; ++j)
            board[i][j] = EMPTY;
}

array<array<BlockType, BOARD_HEIGHT>, BOARD_WIDTH>& Board::GetBoard()
{
    return board;
}

void Board::ClearRow(int row)
{
    for (size_t i = 0; i < BOARD_WIDTH; ++i)
        board[i][row] = EMPTY;
}

void Board::MoveRowDown(int row)
{
    for (size_t i = 0; i < BOARD_WIDTH; ++i)
        for (size_t j = row - 1; j >= 2; --j)
        {
            board[i][j + 1] = board[i][j];
            board[i][j] = EMPTY;
        }
}
