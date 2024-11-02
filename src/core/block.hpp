#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <array>

using namespace std;

#define TETROMINO_SIZE 4
#define ROTATION_STATES 4
#define BLOCK_TYPES 7

enum BlockType {I, J, L, O, S, T, Z, EMPTY};
enum RotateState {INITIAL, LEFT, DOWN, RIGHT};

struct Coord
{
    int x, y;
};

/* Position of each block's mino relative to its origin {0, 0}
 * Indexed using enums (orders in enums and arrays are the same)
 */

static const array<array<array<Coord, TETROMINO_SIZE>, ROTATION_STATES>, BLOCK_TYPES> blockData = {{
    {{ // I
        {{ {0, 1}, {1, 1}, {2, 1}, {3, 1} }}, // INITIAL
        {{ {1, 0}, {1, 1}, {1, 2}, {1, 3} }}, // LEFT
        {{ {0, 2}, {1, 2}, {2, 2}, {3, 2} }}, // DOWN
        {{ {2, 0}, {2, 1}, {2, 2}, {2, 3} }}  // RIGHT
    }},
    {{ // J
        {{ {0, 0}, {0, 1}, {1, 1}, {2, 1} }}, // INITIAL
        {{ {1, 0}, {1, 1}, {0, 2}, {1, 2} }}, // LEFT
        {{ {0, 1}, {1, 1}, {2, 1}, {2, 2} }}, // DOWN
        {{ {1, 0}, {2, 0}, {1, 1}, {1, 2} }}  // RIGHT
    }},
    {{ // L
        {{ {2, 0}, {0, 1}, {1, 1}, {2, 1} }}, // INITIAL
        {{ {0, 0}, {1, 0}, {1, 1}, {1, 2} }}, // LEFT
        {{ {0, 1}, {1, 1}, {2, 1}, {0, 2} }}, // DOWN
        {{ {1, 0}, {1, 1}, {1, 2}, {2, 2} }}  // RIGHT
    }},
    {{ // O
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }}, // INITIAL
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }}, // LEFT
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }}, // DOWN
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }}  // RIGHT
    }},
    {{ // S
        {{ {1, 0}, {2, 0}, {0, 1}, {1, 1} }}, // INITIAL
        {{ {0, 0}, {0, 1}, {1, 1}, {1, 2} }}, // LEFT
        {{ {1, 1}, {2, 1}, {0, 2}, {1, 2} }}, // DOWN
        {{ {1, 0}, {1, 1}, {2, 1}, {2, 2} }}  // RIGHT
    }},
    {{ // T
        {{ {1, 0}, {0, 1}, {1, 1}, {2, 1} }}, // INITIAL
        {{ {1, 0}, {0, 1}, {1, 1}, {1, 2} }}, // LEFT
        {{ {0, 1}, {1, 1}, {2, 1}, {1, 2} }}, // DOWN
        {{ {1, 0}, {1, 1}, {2, 1}, {1, 2} }}  // RIGHT
    }},
    {{ // Z
        {{ {0, 0}, {1, 0}, {1, 1}, {2, 1} }}, // INITIAL
        {{ {1, 0}, {0, 1}, {1, 1}, {0, 2} }}, // LEFT
        {{ {0, 1}, {1, 1}, {1, 2}, {2, 2} }}, // DOWN
        {{ {2, 0}, {1, 1}, {2, 1}, {1, 2} }}  // RIGHT
    }}
}};

struct NinetyDegSrsData
{
    RotateState fromState, toState;
    array<Coord, 5> offsets;
};

/* The block will go through several test cases to determine
 * the final position after performing the rotation.
 */

static const array<NinetyDegSrsData, ROTATION_STATES * 2> srsData = {{
    { INITIAL, LEFT,    {{ {0, 0}, { 1, 0}, { 1, -1}, {0,  2}, {-1,  2} }} },
    { INITIAL, RIGHT,   {{ {0, 0}, {-1, 0}, {-1, -1}, {0,  2}, {-1,  2} }} },
    { LEFT,    DOWN,    {{ {0, 0}, { 1, 0}, {-1,  1}, {0, -2}, {-1, -2} }} },
    { LEFT,    INITIAL, {{ {0, 0}, {-1, 0}, {-1,  1}, {0, -2}, {-1, -2} }} },
    { DOWN,    RIGHT,   {{ {0, 0}, {-1, 0}, {-1, -1}, {0,  2}, {-1,  2} }} },
    { DOWN,    LEFT,    {{ {0, 0}, { 1, 0}, { 1,  1}, {0,  2}, { 1,  2} }} },
    { RIGHT,   INITIAL, {{ {0, 0}, { 1, 0}, { 1,  1}, {0, -2}, { 1, -2} }} },
    { RIGHT,   DOWN,    {{ {0, 0}, { 1, 0}, { 1,  1}, {0, -2}, { 1, -2} }} },
}};

static const array<NinetyDegSrsData, ROTATION_STATES * 2> IsrsData = {{
    { INITIAL, LEFT,    {{ {0, 0}, {-1, 0}, { 2, 0}, {-1, -2}, { 2,  1} }} },
    { INITIAL, RIGHT,   {{ {0, 0}, {-2, 0}, { 1, 0}, {-2,  1}, { 1, -2} }} },
    { LEFT,    DOWN,    {{ {0, 0}, {-2, 0}, { 1, 0}, {-2,  1}, { 1, -2} }} },
    { LEFT,    INITIAL, {{ {0, 0}, { 1, 0}, {-2, 0}, { 1,  2}, {-2, -1} }} },
    { DOWN,    RIGHT,   {{ {0, 0}, { 1, 0}, {-2, 0}, { 1,  2}, {-2, -1} }} },
    { DOWN,    LEFT,    {{ {0, 0}, { 2, 0}, {-1, 0}, { 2, -1}, {-1,  2} }} },
    { RIGHT,   INITIAL, {{ {0, 0}, { 2, 0}, {-1, 0}, { 2, -1}, {-1,  2} }} },
    { RIGHT,   DOWN,    {{ {0, 0}, {-1, 0}, { 2, 0}, {-1, -2}, { 2,  1} }} }
}};

// Rotating 180deg

static const array<array<Coord, 6>, ROTATION_STATES> TOppositeSrsData = {{
    {{ {0, 0}, { 0, -1}, { 1, -1}, {-1, -1}, { 1,  0}, {-1,  0} }}, // INITIAL -> DOWN
    {{ {0, 0}, {-1,  0}, {-1, -2}, {-1, -1}, { 0, -2}, { 0, -1} }}, // LEFT    -> RIGHT
    {{ {0, 0}, { 0,  1}, {-1,  1}, { 1,  1}, {-1,  0}, { 1,  0} }}, // DOWN    -> INITIAL
    {{ {0, 0}, { 1,  0}, { 1, -2}, { 1, -1}, { 0, -2}, { 0, -1} }}  // RIGHT   -> LEFT
}};

static const array<array<Coord, 2>, ROTATION_STATES> OppositeSrsData = {{
    {{ {0, 0}, { 0, -1} }}, // INITIAL -> DOWN
    {{ {0, 0}, {-1,  0} }}, // LEFT    -> RIGHT
    {{ {0, 0}, { 0,  1} }}, // DOWN    -> INITIAL
    {{ {0, 0}, { 1,  0} }}  // RIGHT   -> LEFT
}};

class Block
{
public:
    Block()
        : blockType(EMPTY)
        , coords({{ {0, 0}, {0, 0}, {0, 0}, {0, 0} }}) {};
    Block(BlockType type)
        : blockType(type)
        , posX(0)
        , posY(0)
        , rotateState(INITIAL)
        , coords(blockData.at(type).at(INITIAL)) {};

    void Rotate(RotateState s);
    void Move(int x, int y);

    void HoldReset();
    BlockType GetType()                             const;
    RotateState GetRotation()                       const;
    void GetPosition(int& posX, int& posY)          const;
    const array<Coord, TETROMINO_SIZE>& GetCoords() const;

private:
    BlockType blockType;
    int posX;
    int posY;
    RotateState rotateState;
    array<Coord, TETROMINO_SIZE> coords;

    void UpdateCoord();
};

#endif /* BLOCK_HPP */
