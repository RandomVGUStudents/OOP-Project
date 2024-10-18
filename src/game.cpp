#include "game.hpp"
#include <iostream>

static float frametime = 0;

Game::Game()
    : rng(rd())
    , renderer(stats)
{
    GenerateBag(currentBag); // Generate this bag
    GenerateBag(currentBag); // Generate next bag

    usedHold = false;

    gravity = 0.8;

    lineDropped = 0;
    touchedDown = false;
    lockDownMove = 0;
    lockDownTimer = 0;
    tSpinDetected = false;
    normalTspin = false;
}

void Game::Update()
{
    frametime = gameWindow.GetFrameTime();

    if (!currentBlock)
    {
        currentBlock = NextBlock();

        if (!CheckValidPos(0, 0))
        {
            cout << "Game Over" << endl;
            return;
        }
    }

    if (touchedDown)
    {
        if (CheckValidPos(0, 1))
            lockDownTimer = 0;
        else
        {
            lockDownTimer += frametime;

            if (lockDownMove >= 15 || lockDownTimer > 5)
                LockBlock();
        }
    }

    stats.timeElapsed += frametime;
    lineDropped += frametime;

    if (lineDropped >= gravity)
    {
        int moveDown = floor(lineDropped / gravity);
        MoveVertical(moveDown);
        lineDropped -= moveDown * gravity;
    }

    HandleInput();
}

void Game::Draw()
{
    renderer.UpdateScreenSize();
    renderer.DrawHoldBox(holdBlock);
    renderer.DrawQueueColumn(currentBag);
    renderer.DrawBoard(board, *currentBlock, GetHardDropPos());
    renderer.DrawStats();
    renderer.DrawMessages(frametime);
}

void Game::HandleInput()
{
    if (raylib::Keyboard::IsKeyDown(KEY_DOWN))
        SoftDrop();

    if (raylib::Keyboard::IsKeyPressed(KEY_SPACE))
    {
        HardDrop();
        return;
    }

    if (raylib::Keyboard::IsKeyPressed(KEY_Z) || raylib::Keyboard::IsKeyPressed(KEY_LEFT_CONTROL))
        Rotate(LEFT);
    if (raylib::Keyboard::IsKeyPressed(KEY_X) || raylib::Keyboard::IsKeyPressed(KEY_UP))
        Rotate(RIGHT);
    if (raylib::Keyboard::IsKeyPressed(KEY_A))
        Rotate(DOWN);

    if (raylib::Keyboard::IsKeyPressed(KEY_LEFT_SHIFT))
        HoldBlock();

    MoveLeftRight(raylib::Keyboard::IsKeyDown(KEY_LEFT), raylib::Keyboard::IsKeyDown(KEY_RIGHT));

    if (raylib::Keyboard::IsKeyPressed(KEY_LEFT)
        || raylib::Keyboard::IsKeyPressed(KEY_RIGHT)
        || raylib::Keyboard::IsKeyPressed(KEY_DOWN))
        stats.keyPressed++;

    if (raylib::Keyboard::IsKeyPressed(KEY_R))
    {
        this->~Game();
        new (this) Game();
    }
}

void Game::Rotate(RotateState direction)
{
    stats.keyPressed++;
    bool rotateCW = (direction == RIGHT);
    bool tBlock = (currentBlock->GetType() == T);
    RotateState currentState = currentBlock->GetRotation();
    RotateState newState = static_cast<RotateState>((currentState + direction) % 4);

    if (direction == DOWN)
    {
        bool success = tBlock
            ? TryRotateOpposite(currentState, newState, TOppositeSrsData)
            : TryRotateOpposite(currentState, newState, OppositeSrsData);

        if (success)
            LockDownReset();
        return;
    }

    size_t testIndex = -1;
    const auto& kickTable = currentBlock->GetType() == I ? IsrsData : srsData;
    for (const Coord& offset : kickTable.at(currentState * 2 + rotateCW).offsets)
    {
        testIndex++;
        if (TryRotateBlock(currentState, newState, offset))
        {
            LockDownReset();
            if (tBlock && touchedDown)
            {
                tSpinDetected = true;
                normalTspin = (testIndex == 4);
            }
            return;
        }
    }
}

void Game::MoveLeftRight(bool leftPressed, bool rightPressed)
{
    static int leftPressedFrames = 0;
    static int rightPressedFrames = 0;
    static float dasTimer = 0;
    static bool dasActive = false;
    static bool moveLeft = false;

    leftPressed ? leftPressedFrames++ : leftPressedFrames = 0;
    rightPressed ? rightPressedFrames++ : rightPressedFrames = 0;

    if (leftPressed || rightPressed)
    {
        dasTimer += frametime;

        moveLeft = !((leftPressedFrames < rightPressedFrames) ^ (leftPressed && rightPressed));

        if (dasTimer >= CFG_DAS)
        {
            dasActive = true;
            int moveLength = floor((dasTimer - CFG_DAS) / CFG_ARR);
            MoveHorizontal(moveLeft, moveLength);
            dasTimer -= moveLength * CFG_ARR;
        }
    }
    else
    {
        dasTimer = 0;
        dasActive = false;
    }

    if (!dasActive)
    {
        if (leftPressed && leftPressedFrames == 1)
            MoveHorizontal(true, 1);
        else if (rightPressed && rightPressedFrames == 1)
            MoveHorizontal(false, 1);
    }
}

void Game::HardDrop()
{
    stats.keyPressed++;
    int droppedLine = GetHardDropPos() - BOARD_HEIGHT + 20;

    stats.score += 2 * droppedLine * stats.level;
    MoveVertical(droppedLine);
    LockBlock();
}

void Game::SoftDrop()
{
    lineDropped += frametime * (CFG_SDF - 1);

    if (lineDropped >= gravity)
        stats.score += floor(lineDropped / gravity);
}

void Game::HoldBlock()
{
    if (usedHold)
        return;

    if (holdBlock)
        currentBag.push_front(*holdBlock);

    holdBlock.emplace(currentBlock->GetType());
    currentBlock.reset();
    usedHold = true;
    stats.keyPressed++;
}

void Game::GenerateBag(deque<Block> &bag)
{
    array<Block, BAG_SIZE> newBag = {
        Block(I), Block(J), Block(L), Block(O), Block(S), Block(T), Block(Z)
    };

    shuffle(newBag.begin(), newBag.end(), rng);
    bag.insert(bag.end(), make_move_iterator(newBag.begin()), make_move_iterator(newBag.end()));
}

Block Game::NextBlock()
{
    Block nextBlock = std::move(currentBag.front());
    currentBag.pop_front();

    if (currentBag.size() == BAG_SIZE)
        GenerateBag(currentBag);

    nextBlock.Move(nextBlock.GetType() == O ? 4 : 3, 0);
    return std::move(nextBlock);
}

void Game::LockBlock()
{
    board.LockBlock(*currentBlock);

    currentBlock.reset();
    touchedDown = false;
    lockDownMove = 0;
    lockDownTimer = 0;
    usedHold = false;

    stats.droppedBlockCount++;

    if (tSpinDetected)
        ValidateTSpin();

    int clearedLine = board.CheckFullRow();
    if (clearedLine == 0 && !tSpinDetected)
    {
        stats.comboCount = -1;
        return;
    }
    else if (clearedLine)
    {
        stats.comboCount++;
        stats.clearedLineCount += clearedLine;
        stats.score += stats.comboCount * 50 * stats.level;

        if (stats.comboCount > 0)
            renderer.InvokeComboMsg(stats.comboCount);
    }

    int baseScore = 0;

    if (tSpinDetected)
    {
        stats.tSpinCount++;
        renderer.InvokeTSpinMsg(normalTspin);
        switch (clearedLine)
        {
            case 0: baseScore = 100 + 300 * normalTspin; break;
            case 1: baseScore = 200 + 600 * normalTspin; stats.b2bChain++; break;
            case 2: baseScore = 400 + 800 * normalTspin; stats.b2bChain++; break;
            case 3: baseScore = 1600; stats.b2bChain++; break;
        }
    } else {
        switch (clearedLine)
        {
            case 1: baseScore = 100; stats.b2bChain = -1; break;
            case 2: baseScore = 300; stats.b2bChain = -1; break;
            case 3: baseScore = 500; stats.b2bChain = -1; break;
            case 4: baseScore = 800; stats.tetrisCount++; stats.b2bChain++; break;
        }
    }

    renderer.InvokeClearMsg(clearedLine);
    
    if (stats.b2bChain > 0)
    {
        baseScore *= 1.5;
        renderer.InvokeB2BMsg(stats.b2bChain);
    }

    if (board.CheckFullClear())
    {
        stats.fullClearCount++;
        renderer.InvokeFullClearMsg();
        switch (clearedLine) {
            case 1: baseScore += 800; break;
            case 2: baseScore += 1200; break;
            case 3: baseScore += 1800; break;
            case 4: baseScore += (stats.b2bChain > 0) ? 3200 : 2000; break;
        }
    }

    stats.score += baseScore * stats.level;

    tSpinDetected = false;
    normalTspin = false;
}

void Game::LockDownReset()
{
    if (!touchedDown)
        return;

    lockDownMove++;
    lockDownTimer = 0;
}

bool Game::CheckValidPos(int offsetX, int offsetY)
{
    return board.CheckFit(offsetX, offsetY, *currentBlock);
}

void Game::ValidateTSpin()
{
    static const int checkPairs[4][2] = {
        {0, 1},
        {0, 2},
        {2, 3},
        {1, 3}
    };

    Block cornerTester;
    int posX, posY;
    currentBlock->GetPosition(posX, posY);

    bool corners[4] = {
        !board.CheckFit(posX, posY, cornerTester),
        !board.CheckFit(posX + 2, posY, cornerTester),
        !board.CheckFit(posX, posY + 2, cornerTester),
        !board.CheckFit(posX + 2, posY + 2, cornerTester)
    };

    int cornersTouched = corners[0] + corners[1] + corners[2] + corners[3];

    if (cornersTouched <= 2)
    {
        tSpinDetected = false;
        normalTspin = false;
        return;
    }

    if (cornersTouched == 4 || normalTspin)
        return;

    RotateState tState = currentBlock->GetRotation();

    const int* pair = checkPairs[tState];
    if (corners[pair[0]] && corners[pair[1]])
        normalTspin = true;
}

int Game::GetHardDropPos()
{
    int i = 0;
    while (CheckValidPos(0, i++));
    return i;
}

void Game::MoveVertical(int lines)
{
    if (!CheckValidPos(0, lines) || lines == 0)
        return;

    currentBlock->Move(0, lines);

    LockDownReset();
    tSpinDetected = false;
    normalTspin = false;
    if (!CheckValidPos(0, 1))
        touchedDown = true;
}

void Game::MoveHorizontal(bool left, int col)
{
    int steps = left ? -col : col;
    if (!CheckValidPos(steps, 0) || steps == 0)
        return;

    currentBlock->Move(steps, 0);
    LockDownReset();
    tSpinDetected = false;
    normalTspin = false;
}

bool Game::TryRotateBlock(RotateState currentState, RotateState newState, const Coord& offset)
{
    currentBlock->Rotate(newState);
    if (CheckValidPos(offset.x, offset.y))
    {
        currentBlock->Move(offset.x, offset.y);
        return true;
    }
    currentBlock->Rotate(currentState);
    return false;
}

template <size_t N>
bool Game::TryRotateOpposite(RotateState currentState, RotateState newState, 
                             const array<array<Coord, N>, 4>& srsData)
{
    for (const Coord& offset : srsData[currentState])
        if (TryRotateBlock(currentState, newState, offset))
            return true;
    return false;
}
