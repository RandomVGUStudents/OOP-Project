#ifndef GAME_HPP
#define GAME_HPP

#include <algorithm>
#include <random>
#include "ui.hpp"

// Game config, may be moved somewhere else
#define CFG_ARR 0.005
#define CFG_DAS 0.102
#define CFG_SDF 50
#define BAG_SIZE 7

extern raylib::Window gameWindow;

// Kunstliche Intelligenz

class Game
{
public:
    Game();

    void Update();
    void Draw();

private:
    Board board;
    deque<Block> currentBag;
    optional<Block> currentBlock;
    optional<Block> holdBlock;
    bool usedHold;

    float gravity; // Milisec per row

    GameStats stats;
    GameRenderer renderer;

    //Internal
    float lineDropped;
    bool touchedDown;
    int lockDownMove;
    float lockDownTimer;
    bool tSpinDetected;
    bool normalTspin;

    random_device rd;
    mt19937 rng;

    //User input
    void HandleInput();
    void Rotate(RotateState direction);
    void MoveLeftRight(bool leftPressed, bool rightPressed);
    void HardDrop();
    void SoftDrop();
    void HoldBlock();

    //Internal
    void GenerateBag(deque<Block> &bag);
    Block NextBlock();
    void LockBlock();
    void LockDownReset();

    bool CheckValidPos(int offsetX, int offsetY);
    void ValidateTSpin();
    int GetHardDropPos();
    void MoveVertical(int lines);
    void MoveHorizontal(bool left, int col);
    bool TryRotateBlock(RotateState currentState, RotateState newState, const Coord& offset);

    template <size_t N>
    bool TryRotateOpposite(RotateState currentState, RotateState newState, 
                           const array<array<Coord, N>, 4>& srsData);
};

#endif /* GAME_HPP */
