#include "tetrisUI.hpp"

static float frametime = 0.0;

TetrisUI::TetrisUI()
    : renderer(stats)
    , lineDropTimer(0)
    , lockDownTimer(0)
    , lockDownMove(0)
    , touchedDown(false)
    , tSpinDetected(false)
    , isNormalTspin(false)
{}

void TetrisUI::Update()
{
    HandleInput();

    if (gameOver) return;

    if (touchedDown)
    {
        if (CheckValidPos(0, 1)) lockDownTimer = 0;
        else
        {
            lockDownTimer += frametime;
            if (lockDownMove >= 15 || lockDownTimer > 0.5) LockBlock();
        }
    }

    auto now = chrono::steady_clock::now();
    stats.timeElapsed = now - stats.startTime;

    if (gameMode == BLITZ && static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(stats.timeElapsed).count() >= 120))
        gameOver = true;

    lineDropTimer += frametime;

    if (lineDropTimer >= 1 / gravity)
    {
        MoveVertical(1);
        lineDropTimer -= 1 / gravity;
    }
}

void TetrisUI::Draw()
{
    frametime = gameWindow.GetFrameTime();
    renderer.UpdateScreenSize();

    renderer.DrawHoldBox(holdBlock);
    renderer.DrawQueueColumn(currentBag);

    if (gameOver) renderer.DrawGameOver(board);
    else renderer.DrawBoard(currentBlock, GetHardDropPos(), board);
    
    renderer.DrawStats();
    renderer.DrawMessages();
}

void TetrisUI::SetConfig(float arr, float das, float sdf, float cfgGravity)
{
    gravity = cfgGravity * 60;
    cfgArr = arr / 60;
    cfgDas = das / 60;
    cfgSdf = sdf;
}

void TetrisUI::SetMode(GameMode mode)
{
    gameMode = mode;
    switch (mode)
    {
        case LINES:
            renderer.SetConfig(preset40Lines);
            break;
    
        case BLITZ:
            renderer.SetConfig(presetBlitz);
            break;

        case ZEN:
            renderer.SetConfig(presetZen);
            break;
    }
}

void TetrisUI::HandleInput()
{
    if (raylib::Keyboard::IsKeyPressed(KEY_R)) NewGame();

    if (gameOver) return;

    if (raylib::Keyboard::IsKeyDown(KEY_DOWN)) SoftDrop();

    if (raylib::Keyboard::IsKeyPressed(KEY_SPACE))
    {
        HardDrop();
        return;
    }

    if (raylib::Keyboard::IsKeyPressed(KEY_Z) || raylib::Keyboard::IsKeyPressed(KEY_LEFT_CONTROL))
        Rotate(LEFT);
    if (raylib::Keyboard::IsKeyPressed(KEY_X) || raylib::Keyboard::IsKeyPressed(KEY_UP)) Rotate(RIGHT);
    if (raylib::Keyboard::IsKeyPressed(KEY_A)) Rotate(DOWN);

    if (raylib::Keyboard::IsKeyPressed(KEY_LEFT_SHIFT)) HoldBlock();

    MoveLeftRight(raylib::Keyboard::IsKeyDown(KEY_LEFT), raylib::Keyboard::IsKeyDown(KEY_RIGHT));

    if (raylib::Keyboard::IsKeyPressed(KEY_LEFT)
        || raylib::Keyboard::IsKeyPressed(KEY_RIGHT)
        || raylib::Keyboard::IsKeyPressed(KEY_DOWN))
        stats.keyPressed++;
}

void TetrisUI::Rotate(RotateState direction)
{
    stats.keyPressed++;
    bool rotateCW = (direction == RIGHT);
    bool tBlock = (currentBlock == T);
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
    const auto& kickTable = currentBlock == I ? IsrsData : srsData;
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

        if (dasTimer >= cfgDas)
        {
            dasActive = true;
            int moveLength = floor((dasTimer - cfgDas) / cfgArr);
            MoveHorizontal(moveLeft, moveLength);
            dasTimer -= moveLength * cfgArr;
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
    lineDropTimer += frametime * (cfgSdf - 1);

    if (lineDropTimer >= 20 / gravity)
    {
        int droppedLine = GetHardDropPos();

        stats.score += 1 * droppedLine * stats.level;
        MoveVertical(droppedLine);
        lineDropTimer = 0;
    }
}

void TetrisUI::HoldBlock()
{
    TetrisCore::HoldBlock();
    stats.keyPressed++;
}

void TetrisUI::NewGame()
{
    TetrisCore::NewGame();

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
    if (tSpinDetected)
        ValidateTSpin();

    UpdateBoard();

    touchedDown = false;
    lockDownMove = 0;
    lockDownTimer = 0;

    stats.droppedBlockCount++;

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

    if (gameMode == LINES && stats.clearedLineCount >= 40) gameOver = true;

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
