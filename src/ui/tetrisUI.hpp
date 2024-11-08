#ifndef TETRIS_UI_HPP
#define TETRIS_UI_HPP

#include "core/tetris.hpp"
#include "renderer.hpp"

enum GameMode { LINES, BLITZ, ZEN };


class TetrisUI : public TetrisCore
{
public:
    TetrisUI();

    void Update();
    void Draw();
    void SetConfig(float cfgArr, float cfgDas, float cfgSdf, float cfgGravity);
    void SetMode(GameMode mode);

protected:
    TetrisRenderer renderer;

    GameMode gameMode;
    float gravity;
    float cfgArr;
    float cfgDas;
    float cfgSdf;

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

#endif /* TETRIS_UI_HPP */
