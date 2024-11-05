#include "ui.hpp"
static float frametime = 0;

TetrisUI::TetrisUI() : renderer(stats)
{
    gravity = CFG_GRAVITY;

    lineDropTimer = 0;
    touchedDown = false;
    lockDownMove = 0;
    lockDownTimer = 0;
    tSpinDetected = false;
    isNormalTspin = false;
}

void TetrisUI::Update()
{
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
    lineDropTimer += frametime;

    if (lineDropTimer >= gravity)
    {
        int moveDown = floor(lineDropTimer / gravity);
        MoveVertical(moveDown);
        lineDropTimer -= moveDown * gravity;
    }

    HandleInput();
}

void TetrisUI::Draw()
{
    frametime = gameWindow.GetFrameTime();
    renderer.UpdateScreenSize();
    renderer.DrawHoldBox(holdBlock);
    renderer.DrawQueueColumn(currentBag);
    renderer.DrawBoard(currentBlock, GetHardDropPos(), board);
    renderer.DrawStats();
    renderer.DrawMessages();
}

void TetrisUI::HandleInput()
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
        Reset();
}

void TetrisUI::Rotate(RotateState direction)
{
    stats.keyPressed++;
    bool rotateCW = (direction == RIGHT);
    bool tBlock = (currentBlock.GetType() == T);
    RotateState currentState = currentBlock.GetRotation();
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
    const auto& kickTable = currentBlock.GetType() == I ? IsrsData : srsData;
    for (const Coord& offset : kickTable.at(currentState * 2 + rotateCW).offsets)
    {
        testIndex++;
        if (TryRotateBlock(currentState, newState, offset))
        {
            LockDownReset();
            if (tBlock && touchedDown)
            {
                tSpinDetected = true;
                isNormalTspin = (testIndex == 4);
            }
            return;
        }
    }
}

void TetrisUI::MoveLeftRight(bool leftPressed, bool rightPressed)
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

void TetrisUI::HardDrop()
{
    stats.keyPressed++;
    int droppedLine = GetHardDropPos();

    stats.score += 2 * droppedLine * stats.level;
    MoveVertical(droppedLine);
    LockBlock();
}

void TetrisUI::SoftDrop()
{
    lineDropTimer += frametime * (CFG_SDF - 1);

    if (lineDropTimer >= gravity)
        stats.score += floor(lineDropTimer / gravity);
}

void TetrisUI::HoldBlock()
{
    TetrisCore::HoldBlock();
    stats.keyPressed++;
}

void TetrisUI::Reset()
{
    NewGame();

    lineDropTimer = 0;
    touchedDown = false;
    lockDownMove = 0;
    lockDownTimer = 0;
    tSpinDetected = false;
    isNormalTspin = false;
    stats = GameStats();
}

void TetrisUI::LockDownReset()
{
    if (!touchedDown)
        return;

    lockDownMove++;
    lockDownTimer = 0;
}

void TetrisUI::LockBlock()
{
    UpdateBoard();

    touchedDown = false;
    lockDownMove = 0;
    lockDownTimer = 0;

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
        renderer.InvokeTSpinMsg(isNormalTspin);
        switch (clearedLine)
        {
            case 0: baseScore = 100 + 300 * isNormalTspin; break;
            case 1: baseScore = 200 + 600 * isNormalTspin; stats.b2bChain++; break;
            case 2: baseScore = 400 + 800 * isNormalTspin; stats.b2bChain++; break;
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
    isNormalTspin = false;
}

void TetrisUI::ValidateTSpin()
{
    static const int checkPairs[4][2] = {
        {0, 1},
        {0, 2},
        {2, 3},
        {1, 3}
    };

    Block cornerTester;
    int posX, posY;
    currentBlock.GetPosition(posX, posY);

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
        isNormalTspin = false;
        return;
    }

    if (cornersTouched == 4 || isNormalTspin)
        return;

    RotateState tState = currentBlock.GetRotation();

    const int* pair = checkPairs[tState];
    if (corners[pair[0]] && corners[pair[1]])
        isNormalTspin = true;
}

void TetrisUI::MoveVertical(int lines)
{
    if (!CheckValidPos(0, lines) || lines == 0)
        return;

    currentBlock.Move(0, lines);

    LockDownReset();
    tSpinDetected = false;
    isNormalTspin = false;
    if (!CheckValidPos(0, 1))
        touchedDown = true;
}

void TetrisUI::MoveHorizontal(bool left, int col)
{
    int steps = left ? -col : col;
    if (!CheckValidPos(steps, 0) || steps == 0)
        return;

    currentBlock.Move(steps, 0);
    LockDownReset();
    tSpinDetected = false;
    isNormalTspin = false;
}

bool TetrisUI::TryRotateBlock(RotateState currentState, RotateState newState, const Coord& offset)
{
    currentBlock.Rotate(newState);
    if (CheckValidPos(offset.x, offset.y))
    {
        currentBlock.Move(offset.x, offset.y);
        return true;
    }
    currentBlock.Rotate(currentState);
    return false;
}

template <size_t N>
bool TetrisUI::TryRotateOpposite(RotateState currentState, RotateState newState, 
                             const array<array<Coord, N>, 4>& srsData)
{
    for (const Coord& offset : srsData[currentState])
        if (TryRotateBlock(currentState, newState, offset))
            return true;
    return false;
}
