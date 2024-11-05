#include "block.hpp"

Block::Block(Block& block)
{
    blockType = block.blockType;
    rotateState = block.rotateState;
    posX = block.posX;
    posY = block.posY;
    coords = block.coords;
}

Block::Block(Block&& block) noexcept
{
    blockType = std::move(block.blockType);
    rotateState = std::move(block.rotateState);
    posX = std::move(block.posX);
    posY = std::move(block.posY);
    coords = std::move(block.coords);
}

Block& Block::operator=(const Block& block)
{
    if (this != &block)
    {
        blockType = block.blockType;
        posX = block.posX;
        posY = block.posY;
        rotateState = block.rotateState;
        coords = block.coords;
    }
    return *this;
}

Block& Block::operator=(const Block&& block) noexcept
{
    if (this != &block)
    {
        blockType = std::move(block.blockType);
        rotateState = std::move(block.rotateState);
        posX = std::move(block.posX);
        posY = std::move(block.posY);
        coords = std::move(block.coords);
    }
    return *this;
}

bool Block::operator==(const BlockType& type) const noexcept
{
    return blockType == type;
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
