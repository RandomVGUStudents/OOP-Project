#include "renderer.hpp"

// Animation helper function
float getBakedValue(float currentTime)
{
    int index = static_cast<int>(currentTime / TIME_STEP);
    return ANIMATION_DATA[index];
}

TetrisRenderer::TetrisRenderer(GameStats& gameStats, CounterConfig config)
    : stats(gameStats)
    , screenWidth(0)
    , screenHeight(0)
    , fontSize(0)
    , slotConfig(config)
{
    messagesData.fill("");
    messagesTimer.fill(0);

    DrawStatsFunctions = {{
        nullptr,
        &TetrisRenderer::DrawLevel,
        &TetrisRenderer::DrawScore,
        &TetrisRenderer::DrawTime,
        &TetrisRenderer::DrawLineSpeed,
        &TetrisRenderer::DrawLineCount,
        &TetrisRenderer::DrawPieces,
        &TetrisRenderer::DrawInputs,
        &TetrisRenderer::DrawFullClear
    }};

    UpdateScreenSize();
}

void TetrisRenderer::SetConfig(CounterConfig config)
{
    slotConfig = config;
}

void TetrisRenderer::UpdateScreenSize()
{
    if (screenWidth == gameWindow.GetRenderWidth() && screenHeight == gameWindow.GetRenderHeight())
        return;

    screenWidth = gameWindow.GetRenderWidth();
    screenHeight = gameWindow.GetRenderHeight();

    CalculateElements();
}

void TetrisRenderer::CalculateElements()
{
    canvas.SetSize(
        CANVAS_SIZE * min(screenWidth, screenHeight),
        CANVAS_SIZE * min(screenWidth, screenHeight)
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
        switch (i)
        {
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

    textureCoords[BLOCK_TYPES + 1].SetSize(96, 96);
    textureCoords[BLOCK_TYPES + 1].SetPosition(96 * BLOCK_TYPES + 96, 0);

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

void TetrisRenderer::InvokeClearMsg(int clearedLine)
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

void TetrisRenderer::InvokeTSpinMsg(const bool isNormalTspin)
{
    string msg = "t-spin";
    if (!isNormalTspin) msg = "mini " + msg;

    messagesTimer[T_SPIN_MSG] = ANIMATION_DURATION;
    messagesData[T_SPIN_MSG] = msg;
}

void TetrisRenderer::InvokeB2BMsg(const int b2bCount)
{
    messagesTimer[B2B_MSG] = ANIMATION_DURATION;
    messagesData[B2B_MSG] = format("back-to-back x{}", b2bCount);
}

void TetrisRenderer::InvokeComboMsg(const int comboCount)
{
    messagesTimer[COMBO_MSG] = ANIMATION_DURATION;
    messagesData[COMBO_MSG] = format("{} COMBO", comboCount);
}

void TetrisRenderer::InvokeFullClearMsg()
{
    messagesTimer[FC_MSG] = ANIMATION_DURATION;
}

void TetrisRenderer::DrawHoldBox(Block& holdBlock)
{
    holdBox.Draw(BLACK);
    holdBox.DrawLines(RAYWHITE, 2.0);

    if (!holdBlock) return;

    const BlockType type = holdBlock.GetType();

    for (const Coord& coord : holdBlock.GetCoords())
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

void TetrisRenderer::DrawQueueColumn(deque<BlockType>& currentBag)
{
    queueColumn.Draw(BLACK);
    queueColumn.DrawLines(RAYWHITE, 2.0);

    for (size_t i = 0; i < 5; ++i)
    {
        const BlockType type = currentBag.at(i);
        for (const Coord& coord : Block(currentBag.at(i)).GetCoords())
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

void TetrisRenderer::DrawBoard(Block& currentBlock, int hardDropPos, Board& board)
{
    BlockType type = currentBlock.GetType();

    for (const Coord& coord : currentBlock.GetCoords())
    {
        mino.SetPosition(
            holdBox.GetWidth() + canvas.GetX() + coord.x * mino.GetWidth(),
            canvas.GetY() + (coord.y - LINE_OFFSET + hardDropPos) * mino.GetHeight()
        );

        minoTexture.Draw(textureCoords[EMPTY], mino);

        mino.SetPosition(
            holdBox.GetWidth() + canvas.GetX() + coord.x * mino.GetWidth(),
            canvas.GetY() + (coord.y - LINE_OFFSET) * mino.GetHeight()
        );

        minoTexture.Draw(textureCoords[type], mino);
    }

    for (size_t i = 0; i < BOARD_WIDTH; ++i)
        for (size_t j = LINE_OFFSET; j < BOARD_HEIGHT; ++j)
        {
            j -= LINE_OFFSET;

            mino.SetPosition(
                holdBox.GetWidth() + canvas.GetX() + i * mino.GetWidth(),
                canvas.GetY() + j * mino.GetHeight()
            );

            j += LINE_OFFSET;

            mino.DrawLines(DARKGRAY, 1.0);
            type = board.GetCell(i, j);
            if (type != EMPTY) minoTexture.Draw(textureCoords[type], mino);
        }

    canvas.GetPosition().Add(holdBox.GetSize()).DrawLine(bottomLeft, 2.0, RAYWHITE);
    bottomLeft.DrawLine(bottomRight, 2.0, RAYWHITE);
    bottomRight.DrawLine(queueColumn.GetPosition(), 2.0, RAYWHITE);
}

void TetrisRenderer::DrawStats()
{
    for (size_t i = 0; i < STAT_SLOT_COUNT; ++i)
        if (slotConfig[i] != NONE && slotConfig[i] != CUSTOM)
            (this->*DrawStatsFunctions[slotConfig[i]])(i);
}

void TetrisRenderer::DrawCustomStats(int slotNumber, const string& slotTitle, const string& slotText, const string& slotSubText)
{
    raylib::Vector2 titleSize = font.MeasureText(slotTitle, fontSize, 1.0);
    raylib::Vector2 titlePosition = statSlotCoords[slotNumber][0];

    raylib::Vector2 textSize = font.MeasureText(slotText, fontSize * 1.65, 1.0); // Bruh
    raylib::Vector2 textPosition = statSlotCoords[slotNumber][1];

    raylib::Vector2 subTextSize = font.MeasureText(slotSubText, fontSize, 1.0);

    if (slotNumber == 4)
    {
        font.DrawText(slotTitle, titlePosition, fontSize, 1.0, RAYWHITE);
        font.DrawText(slotText, textPosition, bigFontSize, 1.0, RAYWHITE);

        if (slotSubText.empty()) return;

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

void TetrisRenderer::DrawStatSlot(int slotNumber, const string& slotText, const string& slotSubText)
{
    const string slotTitle = textDisplay[slotConfig[slotNumber]];
    DrawCustomStats(slotNumber, slotTitle, slotText, slotSubText);
}

void TetrisRenderer::DrawMessages()
{
    static raylib::Color color;
    static bool useBigFont;

    static const float SPACING_INC_RATIO = 15;
    static float spacing = 1.0;

    for (size_t i = 0; i < 5; ++i)
    {
        if (messagesTimer[i] <= 0) continue;

        messagesTimer[i] -= gameWindow.GetFrameTime();

        switch (i)
        {
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

void TetrisRenderer::DrawGameOver(Board& board)
{
    for (size_t i = 0; i < BOARD_WIDTH; ++i)
        for (size_t j = LINE_OFFSET; j < BOARD_HEIGHT; ++j)
        {
            j -= LINE_OFFSET;

            mino.SetPosition(
                holdBox.GetWidth() + canvas.GetX() + i * mino.GetWidth(),
                canvas.GetY() + j * mino.GetHeight()
            );

            j += LINE_OFFSET;

            mino.DrawLines(DARKGRAY, 1.0);
            if (board.GetCell(i, j) != EMPTY) minoTexture.Draw(textureCoords[EMPTY + 1], mino);
        }

    messagesTimer[T_SPIN_MSG] = ANIMATION_DURATION;
    messagesData[T_SPIN_MSG] = "press r to restart";
    messagesTimer[CLEAR_MSG] = ANIMATION_DURATION;
    messagesData[CLEAR_MSG] = "GAME OVER";
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
    auto duration = stats.timeElapsed;

    int minutes = static_cast<int>(chrono::duration_cast<chrono::minutes>(duration).count());
    duration -= chrono::minutes(minutes);

    int seconds = static_cast<int>(chrono::duration_cast<chrono::seconds>(duration).count());
    duration -= chrono::seconds(seconds);

    int milliseconds = static_cast<int>(chrono::duration_cast<chrono::milliseconds>(duration).count());
    DrawStatSlot(slotNumber, format("{}:{:02d}", minutes, seconds), format(" .{:03d}", milliseconds));
}

void TetrisRenderer::DrawLineSpeed(int slotNumber)
{
    float lps = stats.clearedLineCount / stats.timeElapsed.count();
    DrawStatSlot(slotNumber, format("{}, ", stats.clearedLineCount), format("{:.2f}/s", lps));
}

void TetrisRenderer::DrawLineCount(int slotNumber)
{
    DrawStatSlot(slotNumber, format("{}", stats.clearedLineCount), "/40");
}

void TetrisRenderer::DrawPieces(int slotNumber)
{
    float pps = stats.droppedBlockCount / stats.timeElapsed.count();
    DrawStatSlot(slotNumber, format("{}, ", stats.droppedBlockCount), format("{:.2f}/s", pps));
}

void TetrisRenderer::DrawInputs(int slotNumber)
{
    float ips = 0;
    if (stats.droppedBlockCount) ips = 1.0 * stats.keyPressed / stats.droppedBlockCount;
    DrawStatSlot(slotNumber, format("{}, ", stats.keyPressed), format("{:.2f}/p", ips));
}

void TetrisRenderer::DrawFullClear(int slotNumber)
{
    DrawStatSlot(slotNumber, format("{}", stats.fullClearCount));
}
