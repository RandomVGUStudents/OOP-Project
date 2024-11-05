#ifndef UI_HPP
#define UI_HPP

#include "core/tetris.hpp"
#include "renderer.hpp"

// Game config, may be moved somewhere else
constexpr float CFG_ARR = 0.005;
constexpr float CFG_DAS = 0.102;
constexpr float CFG_GRAVITY = 0.8;
constexpr int CFG_SDF = 50;

class TetrisUI : public TetrisCore
{
public:
    TetrisUI();

    void Update();
    void Draw();

protected:
    TetrisRenderer renderer;

    float gravity;

    // Internal stats
    float lineDropTimer;
    float lockDownTimer;
    int lockDownMove;
    bool touchedDown;
    bool tSpinDetected;
    bool isNormalTspin;

    // User input
    void HandleInput();
    void Rotate(RotateState direction);
    void MoveLeftRight(bool leftPressed, bool rightPressed);
    void HardDrop();
    void SoftDrop();
    void HoldBlock() override;
    void Reset();

    void LockDownReset();
    virtual void LockBlock();

    void ValidateTSpin();
    void MoveVertical(int lines);
    void MoveHorizontal(bool left, int col);
    bool TryRotateBlock(RotateState currentState, RotateState newState, const Coord& offset);

    template <size_t N>
    bool TryRotateOpposite(RotateState currentState, RotateState newState, 
                           const array<array<Coord, N>, 4>& srsData);
};

#endif /* UI_HPP */
