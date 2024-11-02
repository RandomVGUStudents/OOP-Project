#include "ui.hpp"
#include "animation.hpp"

// Animation helper function
float getBakedValue(float currentTime)
{
    int index = static_cast<int>(currentTime / TIME_STEP);
    return ANIMATION_DATA[index];
}

TetrisUI::TetrisUI()
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
    if (!currentBlock)
    {
        NextBlock();

        if (!CheckValidPos(0, 0))
        {
            gameOver = true;
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
    lineDropTimer += frametime;

    if (lineDropTimer >= gravity)
    {
        int moveDown = floor(lineDropTimer / gravity);
        MoveVertical(moveDown);
        lineDropTimer -= moveDown * gravity;
    }

    HandleInput();
}

void TetrisUI::InvokeClearMsg(int clearedLine)
{
    string msg;
    switch (clearedLine)
    {
        case 1: msg = "SINGLE"; break;
        case 2: msg = "DOUBLE"; break;
        case 3: msg = "TRIPLE"; break;
        case 4: msg = "QUAD"; break;
        default: return;
    }

    messagesTimer[CLEAR_MSG] = ANIMATION_DURATION;
    messagesData[CLEAR_MSG] = msg;
}

void TetrisUI::InvokeTSpinMsg()
{
    string msg = "t-spin";
    if (!isNormalTspin)
        msg = "mini " + msg;

    messagesTimer[T_SPIN_MSG] = ANIMATION_DURATION;
    messagesData[T_SPIN_MSG] = msg;
}

void TetrisUI::InvokeB2BMsg()
{
    messagesTimer[B2B_MSG] = ANIMATION_DURATION;
    messagesData[B2B_MSG] = format("back-to-back x{}", stats.b2bChain);
}

void TetrisUI::InvokeComboMsg()
{
    messagesTimer[COMBO_MSG] = ANIMATION_DURATION;
    messagesData[COMBO_MSG] = format("{} COMBO", stats.comboCount);
}

void TetrisUI::InvokeFullClearMsg()
{
    messagesTimer[FC_MSG] = ANIMATION_DURATION;
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
    {
        this->~TetrisUI();
        new (this) TetrisUI();
    }
}

void TetrisUI::Rotate(RotateState direction)
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
    int droppedLine = GetHardDropPos() - BOARD_HEIGHT + 20;

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

void TetrisUI::Hold()
{
    HoldBlock();
    stats.keyPressed++;
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
            InvokeComboMsg();
    }

    int baseScore = 0;

    if (tSpinDetected)
    {
        stats.tSpinCount++;
        InvokeTSpinMsg();
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

    InvokeClearMsg(clearedLine);
    
    if (stats.b2bChain > 0)
    {
        baseScore *= 1.5;
        InvokeB2BMsg();
    }

    if (board.CheckFullClear())
    {
        stats.fullClearCount++;
        InvokeFullClearMsg();
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
        isNormalTspin = false;
        return;
    }

    if (cornersTouched == 4 || isNormalTspin)
        return;

    RotateState tState = currentBlock->GetRotation();

    const int* pair = checkPairs[tState];
    if (corners[pair[0]] && corners[pair[1]])
        isNormalTspin = true;
}

void TetrisUI::MoveVertical(int lines)
{
    if (!CheckValidPos(0, lines) || lines == 0)
        return;

    currentBlock->Move(0, lines);

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

    currentBlock->Move(steps, 0);
    LockDownReset();
    tSpinDetected = false;
    isNormalTspin = false;
}

bool TetrisUI::TryRotateBlock(RotateState currentState, RotateState newState, const Coord& offset)
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
bool TetrisUI::TryRotateOpposite(RotateState currentState, RotateState newState, 
                             const array<array<Coord, N>, 4>& srsData)
{
    for (const Coord& offset : srsData[currentState])
        if (TryRotateBlock(currentState, newState, offset))
            return true;
    return false;
}


TetrisRenderer::TetrisRenderer()
    : screenWidth(0)
    , screenHeight(0)
    , aspectRatio(0)
    , scaleX(0)
    , scaleY(0)
    , fontSize(0)
    , slotConfig(preset40Lines) // Temporarily
{
    messagesData.fill("");
    messagesTimer.fill(0);

    DrawStatsFunctions = {{
        nullptr,
        &TetrisRenderer::DrawLevel,
        &TetrisRenderer::DrawScore,
        &TetrisRenderer::DrawTime,
        &TetrisRenderer::DrawLineCount,
        &TetrisRenderer::DrawPieces,
        &TetrisRenderer::DrawInputs,
        &TetrisRenderer::DrawFullClear
    }};

    UpdateScreenSize();
}

void TetrisRenderer::Draw()
{
    frametime = gameWindow.GetFrameTime();
    UpdateScreenSize();
    DrawHoldBox();
    DrawQueueColumn();
    DrawBoard();
    DrawStats();
    DrawMessages();
}

void TetrisRenderer::UpdateScreenSize()
{
    if (screenWidth == gameWindow.GetRenderWidth() && screenHeight == gameWindow.GetRenderHeight())
        return;

    screenWidth = gameWindow.GetRenderWidth();
    screenHeight = gameWindow.GetRenderHeight();

    aspectRatio = 1.0 * screenWidth / screenHeight;
    scaleX = 1.0;
    scaleY = 1.0;

    if (aspectRatio > TARGET_ASPECT_RATIO) {
        scaleX = aspectRatio / TARGET_ASPECT_RATIO;
    } else if (aspectRatio < TARGET_ASPECT_RATIO) {
        scaleY = TARGET_ASPECT_RATIO / aspectRatio;
    }

    CalculateElements();
}

void TetrisRenderer::CalculateElements()
{
    canvas.SetSize(
        (CANVAS_WIDTH * screenWidth) / scaleX,
        (CANVAS_HEIGHT * screenHeight) / scaleY
    );
    canvas.SetPosition(
        (screenWidth - canvas.GetWidth()) / 2,
        (screenHeight - canvas.GetHeight()) / 2
    );

    fontSize = FONT_SIZE * canvas.GetWidth();
    bigFontSize = fontSize * 1.75;

    holdBox.SetSize(
        HOLDBOX_WIDTH * canvas.GetWidth(),
        HOLDBOX_HEIGHT * canvas.GetHeight()
    );
    holdBox.SetPosition(canvas.GetPosition());

    queueColumn.SetSize(
        QUEUE_COLUMN_WIDTH * canvas.GetWidth(),
        QUEUE_COLUMN_HEIGHT * canvas.GetHeight()
    );
    queueColumn.SetPosition(
        canvas.GetX() + canvas.GetWidth() - queueColumn.GetWidth(),
        canvas.GetY()
    );

    bottomLeft.SetX(canvas.GetX() + holdBox.GetWidth());
    bottomLeft.SetY(canvas.GetY() + canvas.GetHeight());

    bottomRight.SetX(queueColumn.GetX());
    bottomRight.SetY(bottomLeft.GetY());

    boardMiddle.SetX((bottomLeft.GetX() + bottomRight.GetX()) / 2);
    boardMiddle.SetY(canvas.GetY() + canvas.GetHeight() / 2);

    mino.SetSize(canvas.GetSize().Scale(CELL_SIZE));

    for (size_t i = 0; i < BLOCK_TYPES; ++i)
    {
        switch (i) {
            case I:
                blockSizes[i].SetX(4);
                blockSizes[i].SetY(3);
                break;
                
            case O:
                blockSizes[i].SetX(2);
                blockSizes[i].SetY(2);
                break;

            default:
                blockSizes[i].SetX(3);
                blockSizes[i].SetY(2);
                break;
        }
        blockSizes[i] *= mino.GetSize();

        textureCoords[i].SetSize(96, 96);
        textureCoords[i].SetPosition(96 * i, 0);
    }
    textureCoords[BLOCK_TYPES].SetSize(96, 96);
    textureCoords[BLOCK_TYPES].SetPosition(96 * BLOCK_TYPES, 0);

    float textPadding = canvas.GetWidth() * TEXT_PADDING; 
    float leftXPos = holdBox.GetX() + holdBox.GetWidth() - textPadding;

    // Right slot
    statSlotCoords[4][0].SetX(queueColumn.GetX() + textPadding);
    statSlotCoords[4][0].SetY(queueColumn.GetY() + queueColumn.GetHeight() + textPadding);
    statSlotCoords[4][1].SetX(queueColumn.GetX() + textPadding);
    statSlotCoords[4][1].SetY(statSlotCoords[4][0].GetY() + fontSize + textPadding);

    // Left slots
    statSlotCoords[0][1].SetX(leftXPos);
    statSlotCoords[0][1].SetY(bottomLeft.GetY());
    statSlotCoords[0][0].SetX(leftXPos);
    statSlotCoords[0][0].SetY(statSlotCoords[0][1].GetY() - bigFontSize - textPadding);
    for (size_t i = 1; i < 4; ++i)
    {
        statSlotCoords[i][1].SetX(leftXPos);
        statSlotCoords[i][1].SetY(statSlotCoords[i - 1][0].GetY() - fontSize - 2 * textPadding);
        statSlotCoords[i][0].SetX(leftXPos);
        statSlotCoords[i][0].SetY(statSlotCoords[i][1].GetY() - bigFontSize - textPadding);
    }

    messagesCoords[0].SetX(leftXPos);
    messagesCoords[0].SetY(canvas.GetY() + holdBox.GetHeight() + bigFontSize + 2 * textPadding);
    for (size_t i = 1; i < 4; ++i)
    {
        messagesCoords[i].SetX(leftXPos);
        messagesCoords[i].SetY(messagesCoords[i - 1].GetY() + bigFontSize + textPadding);
    }
}

void TetrisRenderer::DrawHoldBox()
{
    holdBox.Draw(BLACK);
    holdBox.DrawLines(RAYWHITE, 2.0);

    if (!holdBlock)
        return;

    const BlockType type = holdBlock->GetType();

    for (const Coord& coord : holdBlock->GetCoords())
    {
        mino.SetPosition(
            holdBox.GetPosition().Add((holdBox.GetSize() - blockSizes[type]).Scale(0.5))
        );

        mino.SetPosition(
            mino.GetPosition().Add(mino.GetSize().Multiply(raylib::Vector2(coord.x, coord.y)))
        );

        minoTexture.Draw(textureCoords[type], mino);
    }
}

void TetrisRenderer::DrawQueueColumn()
{
    queueColumn.Draw(BLACK);
    queueColumn.DrawLines(RAYWHITE, 2.0);

    for (int i = 0; i < 5; ++i)
    {
        const BlockType type = currentBag.at(i).GetType();
        for (const Coord& coord : currentBag.at(i).GetCoords())
        {
            mino.SetX(
                queueColumn.GetX()
                + (queueColumn.GetWidth() - blockSizes[type].GetX()) * 0.5
                + coord.x * mino.GetWidth()
            );

            mino.SetY(
                queueColumn.GetY()
                + mino.GetHeight() * (coord.y + i * 3 + 1)
            );

            minoTexture.Draw(textureCoords[type], mino);
        }
    }
}

void TetrisRenderer::DrawBoard()
{
    int hardDropPos = GetHardDropPos();
    BlockType type = currentBlock->GetType();

    for (const Coord& coord : currentBlock->GetCoords())
    {
        mino.SetPosition(
            holdBox.GetWidth() + canvas.GetX() + coord.x * mino.GetWidth(),
            canvas.GetY() + (coord.y - 2 * LINE_OFFSET + hardDropPos) * mino.GetHeight()
        );

        minoTexture.Draw(textureCoords[EMPTY], mino);

        mino.SetPosition(
            holdBox.GetWidth() + canvas.GetX() + coord.x * mino.GetWidth(),
            canvas.GetY() + (coord.y - LINE_OFFSET) * mino.GetHeight()
        );

        minoTexture.Draw(textureCoords[type], mino);
    }

    for (int i = 0; i < BOARD_WIDTH; ++i)
        for (int j = LINE_OFFSET; j < BOARD_HEIGHT; ++j)
        {
            j -= LINE_OFFSET;

            mino.SetPosition(
                holdBox.GetWidth() + canvas.GetX() + i * mino.GetWidth(),
                canvas.GetY() + j * mino.GetHeight()
            );

            j += LINE_OFFSET;

            mino.DrawLines(DARKGRAY, 1.0);
            type = board.GetCell(i, j);
            if (type != EMPTY)
                minoTexture.Draw(textureCoords[type], mino);
        }

    canvas.GetPosition().Add(holdBox.GetSize()).DrawLine(bottomLeft, 2.0, RAYWHITE);
    bottomLeft.DrawLine(bottomRight, 2.0, RAYWHITE);
    bottomRight.DrawLine(queueColumn.GetPosition(), 2.0, RAYWHITE);
}

void TetrisRenderer::DrawStats()
{
    for (size_t i = 0; i < STAT_SLOT_COUNT; ++i)
        (this->*DrawStatsFunctions[slotConfig[i]])(i);
}

void TetrisRenderer::DrawStatSlot(int slotNumber, const string& slotText, const string& slotSubText)
{
    const string slotTitle = textDisplay[slotConfig[slotNumber]];

    raylib::Vector2 titleSize = font.MeasureText(slotTitle, fontSize, 1.0);
    raylib::Vector2 titlePosition = statSlotCoords[slotNumber][0];

    raylib::Vector2 textSize = font.MeasureText(slotText, fontSize * 1.65, 1.0); // Bruh
    raylib::Vector2 textPosition = statSlotCoords[slotNumber][1];

    raylib::Vector2 subTextSize = font.MeasureText(slotSubText, fontSize, 1.0);

    if (slotNumber == 4)
    {
        font.DrawText(slotTitle, titlePosition, fontSize, 1.0, RAYWHITE);
        font.DrawText(slotText, textPosition, bigFontSize, 1.0, RAYWHITE);

        if (slotSubText.empty())
            return;

        raylib::Vector2 subTextPos(textPosition.Add(textSize));
        subTextPos.SetY(subTextPos.GetY() - fontSize);
        font.DrawText(slotSubText, subTextPos, fontSize, 1.0, RAYWHITE);

        return;
    }

    font.DrawText(slotTitle, titlePosition, titleSize, 0, fontSize, 1.0, RAYWHITE);

    if (!slotSubText.empty())
    {
        font.DrawText(slotSubText, textPosition, subTextSize, 0, fontSize, 1.0, RAYWHITE);
        textPosition.SetX(textPosition.GetX() - subTextSize.GetX());
    }

    font.DrawText(slotText, textPosition, textSize, 0, bigFontSize, 1.0, RAYWHITE);
}

void TetrisRenderer::DrawMessages()
{
    static raylib::Color color;
    static bool useBigFont;

    static const float SPACING_INC_RATIO = 15;
    static float spacing = 1.0;

    for (size_t i = 0; i < 5; ++i)
    {
        if (messagesTimer[i] <= 0)
            continue;

        messagesTimer[i] -= frametime;

        switch (i) {
            case T_SPIN_MSG:
                useBigFont = false;
                color = raylib::Color(162, 65, 155);
                break;

            case CLEAR_MSG:
                useBigFont = true;
                color = RAYWHITE;
                break;

            case B2B_MSG:
                useBigFont = false;
                color = raylib::Color(182, 156, 53);
                break;

            case COMBO_MSG:
                useBigFont = true;
                color = RAYWHITE;
                break;

            case FC_MSG:
                color = RAYWHITE;
                static float timeElapsed;
                static float localFontSize;
                static const char* text = "  ALL  \nCLEAR";

                timeElapsed = ANIMATION_DURATION - messagesTimer[FC_MSG];

                auto drawTextWithSpacing = [&](float fontSize, float spacingMultiplier, float rotation, raylib::Color drawColor, float alpha = 1.0)
                    {
                        SetTextLineSpacing(fontSize);
                        raylib::Vector2 textSize = font.MeasureText(text, fontSize, 1.0);
                        font.DrawText(text, boardMiddle, textSize.Scale(0.5), rotation, fontSize, spacingMultiplier, drawColor.Alpha(alpha));
                    };

                if (messagesTimer[FC_MSG] >= ANIMATION_DURATION - 0.5)
                {
                    localFontSize = fontSize * 6 * timeElapsed;
                    drawTextWithSpacing(localFontSize, 1.0, 1440 * timeElapsed, color);
                }
                else
                {
                    localFontSize = fontSize * 3 * (1 + timeElapsed * 0.3);
                    drawTextWithSpacing(localFontSize, 1.0, 0, color, 0.2 * getBakedValue(messagesTimer[FC_MSG]));

                    localFontSize = fontSize * 3 * (1 - timeElapsed * 0.1);
                    drawTextWithSpacing(localFontSize, 1.0, 0, color, getBakedValue(messagesTimer[FC_MSG]));
                }

                return;
        }

        spacing = SPACING_INC_RATIO * getBakedValue(ANIMATION_DURATION - messagesTimer[i]);

        raylib::Vector2 textSize = font.MeasureText(
            messagesData[i],
            useBigFont ? bigFontSize : fontSize,
            spacing
        );
        font.DrawText(
            messagesData[i],
            messagesCoords[i].Subtract(textSize),
            useBigFont ? bigFontSize : fontSize,
            spacing,
            color.Alpha(getBakedValue(messagesTimer[i]))
        );
    }
}

void TetrisRenderer::DrawLevel(int slotNumber)
{
    DrawStatSlot(slotNumber, format("{}", stats.level));
}

void TetrisRenderer::DrawScore(int slotNumber)
{
    DrawStatSlot(slotNumber, format("{}", stats.score));
}

void TetrisRenderer::DrawTime(int slotNumber)
{
    int minutes = static_cast<int>(stats.timeElapsed) / 60;
    int seconds = static_cast<int>(stats.timeElapsed) % 60;

    double fractionalSeconds = stats.timeElapsed - static_cast<int>(stats.timeElapsed);
    int milliseconds = static_cast<int>(fractionalSeconds * 1000);

    DrawStatSlot(slotNumber, format("{}:{:02d}", minutes, seconds), format(" .{:03d}", milliseconds));
}

void TetrisRenderer::DrawLineCount(int slotNumber)
{
    DrawStatSlot(slotNumber, format("{}", stats.clearedLineCount), "/40");
}

void TetrisRenderer::DrawPieces(int slotNumber)
{
    float pps = stats.droppedBlockCount / stats.timeElapsed;
    DrawStatSlot(slotNumber, format("{}, ", stats.droppedBlockCount), format("{:.2f}/s", pps));
}

void TetrisRenderer::DrawInputs(int slotNumber)
{
    float ips = 0;
    if (stats.droppedBlockCount)
        ips = 1.0 * stats.keyPressed / stats.droppedBlockCount;
    DrawStatSlot(slotNumber, format("{}, ", stats.keyPressed), format("{:.2f}/p", ips));
}

void TetrisRenderer::DrawFullClear(int slotNumber)
{
    DrawStatSlot(slotNumber, format("{}", stats.fullClearCount));
}
