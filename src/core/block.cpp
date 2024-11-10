#include "block.hpp"

bool Block::operator==(const BlockType& type) const noexcept
{
    return blockType == type;
}

bool Block::operator==(const Block& block) const noexcept
{
    return blockType == block.blockType;
}

Block::operator bool() const noexcept
{
    return blockType != EMPTY;
}

void Block::Rotate(RotateState s)
{
    rotateState = s;
    UpdateCoord();
}

void Block::Move(int x, int y)
{
    posX += x;
    posY += y;
    UpdateCoord();
}

void Block::ResetPosition()
{
    posX = 0;
    posY = 0;
    rotateState = INITIAL;
    UpdateCoord();
}

BlockType Block::GetType() const
{
    return blockType;
}

RotateState Block::GetRotation() const
{
    return rotateState;
}

void Block::GetPosition(int& x, int& y) const
{
    x = posX;
    y = posY;
}

const array<Coord, TETROMINO_SIZE>& Block::GetCoords() const
{
    return coords;
}

void Block::UpdateCoord()
{
    const auto& relativeCoord = blockData[blockType][rotateState];
    for (size_t i = 0; i < TETROMINO_SIZE; ++i)
    {
        coords[i].x = relativeCoord[i].x + posX;
        coords[i].y = relativeCoord[i].y + posY;
    }
}
