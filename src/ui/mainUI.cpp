#include "mainUI.hpp"

double stretch(double val, double minIn, double maxIn, double minOut, double maxOut)
{
    return minOut + (val - minIn) * (maxOut - minOut) / (maxIn - minIn);
}

Slider::Slider(float startVal, float endVal)
    : startVal(startVal)
    , endVal(endVal)
    , outputVal(startVal)
    , sliding(false)
{}

float Slider::GetValue()
{
    return outputVal;
}

void Slider::Draw(bool& canDrag)
{
    sliderBar.DrawRoundedLines(1, 5, 3, RAYWHITE);
    if (sliderHandle.CheckCollision(mousePos)) sliderHandle.DrawRounded(1, 5, GRAY);
    else sliderHandle.DrawRounded(1, 5, RAYWHITE);

    if (raylib::Mouse::IsButtonDown(MOUSE_BUTTON_LEFT) && sliderHandle.CheckCollision(mousePos) && canDrag)
    {
        sliding = true;
        canDrag = false;
    }

    if (sliding)
    {
        if (raylib::Mouse::IsButtonReleased(MOUSE_BUTTON_LEFT))
        {
            sliding = false;
            canDrag = true;
        }

        sliderHandle.SetX(clamp(mousePos.GetX(), sliderBar.GetX(),
                                sliderBar.GetX() + sliderBar.GetWidth() - sliderHandle.GetWidth()));

        outputVal = stretch(sliderHandle.GetX(), sliderBar.GetX(),
                            sliderBar.GetX() + sliderBar.GetWidth() - sliderHandle.GetWidth(),
                            startVal, endVal);
    }
}


App::App()
    : screenWidth(0)
    , screenHeight(0)
    , fontSize(0.0)
    , currentPage(PLAY)
    , isMainStarted(false)
{}

void App::Loop()
{
    gameWindow.BeginDrawing();
    gameWindow.ClearBackground();

    if (!isMainStarted)
    {
        mousePos = raylib::Mouse::GetPosition();
        if (raylib::Keyboard::IsKeyPressed(KEY_ESCAPE)) exit(0);

        UpdateScreenSize();

        canvas.DrawRoundedLines(0.06, 5, 3, RAYWHITE);

        for (size_t i = 0; i < BTN_COUNT; ++i)
        {
            auto& btn = tabBtn[i];

            if (btn.CheckCollision(mousePos) || i == currentPage)
            {
                btn.DrawRounded(0.6, 5, RAYWHITE);
                font.DrawText(btnString[i], btnTextPos[i], fontSize, 1.0, BLACK);
            }
            else
        {
                btn.DrawRoundedLines(0.6, 5, 2, RAYWHITE);
                font.DrawText(btnString[i], btnTextPos[i], fontSize, 1.0, WHITE);
            }
        }

        if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT))
            for (size_t i = 0; i < BTN_COUNT; ++i)
                if (tabBtn[i].CheckCollision(mousePos)) currentPage = static_cast<CurrentPage>(i);
    }
    else if (raylib::Keyboard::IsKeyPressed(KEY_ESCAPE))
    {
        isMainStarted = false;
        tetrisGame.NewGame();
        tetrisAI.NewGame();
    }

    switch (currentPage)
    {
        case PLAY:
            PlayPage();
            break;

        case WATCH:
            WatchPage();
            break;

        case TRAIN:
            TrainPage();
            break;
    }
    gameWindow.EndDrawing();
}

void App::UpdateScreenSize()
{
    if (screenWidth == gameWindow.GetRenderWidth() && screenHeight == gameWindow.GetRenderHeight())
        return;

    screenWidth = gameWindow.GetRenderWidth();
    screenHeight = gameWindow.GetRenderHeight();

    CalculateElements();
}

void App::CalculateElements()
{
    canvas.SetSize(
        CANVAS_SIZE * min(screenWidth, screenHeight),
        CANVAS_SIZE * min(screenWidth, screenHeight)
    );

    canvas.SetPosition(
        (screenWidth - canvas.GetWidth()) / 2,
        (screenHeight - canvas.GetHeight()) / 2
    );

    fontSize = FONT_SIZE * canvas.GetWidth() * 2;

    for (size_t i = 0; i < BTN_COUNT; ++i)
    {
        tabBtn[i].SetSize(
            canvas.GetWidth() * TAB_BTN_SIZE,
            canvas.GetHeight() * TAB_BTN_SIZE / 4
        );

        float padding = canvas.GetWidth() * BTN_PADDING;
        if (i == 0)
            tabBtn[i].SetPosition(
                canvas.GetX() + padding,
                canvas.GetY() + padding
            );
        else
            tabBtn[i].SetPosition(
                tabBtn[i - 1].GetX() + tabBtn[i - 1].GetWidth() + padding,
                canvas.GetY() + padding
            );

        btnTextPos[i] = raylib::Vector2(
            tabBtn[i].GetX() + (tabBtn[i].GetWidth() - font.MeasureText(btnString[i], fontSize, 1.0).GetX()) / 2,
            tabBtn[i].GetY() + tabBtn[i].GetHeight() * 0.2
        );

        pageTitlePos[i].SetX(canvas.GetX() + (canvas.GetWidth() - font.MeasureText(pageTitle[i], fontSize * TITLE_FONT_SIZE_RATIO, 1.0).GetX()) / 2);
        pageTitlePos[i].SetY(tabBtn[0].GetY() + tabBtn[0].GetHeight() + padding * 2);

        pageBtn[i].SetSize(tabBtn[i].GetSize());
        pageBtn[i].SetPosition(
            tabBtn[i].GetX(),
            canvas.GetY() + canvas.GetHeight() - padding - pageBtn[i].GetHeight()
        );

        p1BtnTextPos[i] = raylib::Vector2(
            pageBtn[i].GetX() + (pageBtn[i].GetWidth() - font.MeasureText(p1BtnString[i], fontSize, 1.0).GetX()) / 2,
            pageBtn[i].GetY() + pageBtn[i].GetHeight() * 0.2
        );

        string currentGen = (i == 1) ? to_string(trainer.generation) : "";

        p2BtnTextPos[i] = raylib::Vector2(
            pageBtn[i].GetX() + (pageBtn[i].GetWidth() - font.MeasureText(p2BtnString[i] + currentGen, fontSize, 1.0).GetX()) / 2,
            pageBtn[i].GetY() + pageBtn[i].GetHeight() * 0.2
        );

        p3BtnTextPos[i] = raylib::Vector2(
            pageBtn[i].GetX() + (pageBtn[i].GetWidth() - font.MeasureText(p3BtnString[i] + currentGen, fontSize, 1.0).GetX()) / 2,
            pageBtn[i].GetY() + pageBtn[i].GetHeight() * 0.2
        );
    }

    float horizontalPadding = canvas.GetWidth() * SLIDER_PADDING;
    for (size_t i = 0; i < P1_SLIDER_COUNT; ++i)
    {
        p1TextPos[i].SetX(canvas.GetX() + horizontalPadding);
        p1TextPos[i].SetY(tabBtn[0].GetY()
                          + tabBtn[0].GetHeight()
                          + fontSize * TITLE_FONT_SIZE_RATIO
                          + canvas.GetHeight() * SLIDER_SPACING * i
                          + horizontalPadding * 2);

        switch (i)
        {
            case ARR:
                p1Slider.push_back(std::move(Slider(0.1, 5.0)));
                break;

            case DAS:
                p1Slider.push_back(std::move(Slider(1.0, 20.0)));
                break;

            case SDF:
                p1Slider.push_back(std::move(Slider(5.0, 40.0)));
                break;

            case GRAVITY:
                p1Slider.push_back(std::move(Slider(1.0 / 60, 1.0)));
                break;
        }

        p1Slider[i].sliderBar.SetSize(
            canvas.GetWidth() * SLIDER_WIDTH,
            canvas.GetHeight() * SLIDER_HEIGHT
        );

        p1Slider[i].sliderBar.SetPosition(
            p1TextPos[i].GetX() + font.MeasureText(p1SliderString[GRAVITY], fontSize, 1.0).GetX() + horizontalPadding,
            p1TextPos[i].GetY() + (fontSize - canvas.GetHeight() * SLIDER_HEIGHT) / 4
        );

        p1Slider[i].sliderHandle.SetSize(
            p1Slider[i].sliderBar.GetHeight(),
            p1Slider[i].sliderBar.GetHeight()
        );

        p1Slider[i].sliderHandle.SetPosition(
            p1Slider[i].sliderBar.GetPosition()
        );
        
        p1ValuePos[i].SetX(p1Slider[i].sliderBar.GetX() + p1Slider[i].sliderBar.GetWidth() + horizontalPadding);
        p1ValuePos[i].SetY(p1TextPos[i].GetY());
    }

    for (size_t i = 0; i < P2_SLIDER_COUNT; ++i)
    {
        p2TextPos[i].SetX(canvas.GetX() + horizontalPadding);
        p2TextPos[i].SetY(tabBtn[0].GetY()
                          + tabBtn[0].GetHeight()
                          + fontSize * TITLE_FONT_SIZE_RATIO
                          + canvas.GetHeight() * SLIDER_SPACING * i
                          + horizontalPadding * 2);

        switch (i)
        {
            case PPS:
                p2Slider.push_back(std::move(Slider(1.0, 20.0)));
                break;

            case REPEAT:
                p2Slider.push_back(std::move(Slider(1.0, 20.0)));
                break;
        }

        p2Slider[i].sliderBar.SetSize(
            canvas.GetWidth() * SLIDER_WIDTH,
            canvas.GetHeight() * SLIDER_HEIGHT
        );

        p2Slider[i].sliderBar.SetPosition(
            p2TextPos[i].GetX() + font.MeasureText(p2SliderString[REPEAT], fontSize, 1.0).GetX() + horizontalPadding,
            p2TextPos[i].GetY() + (fontSize - canvas.GetHeight() * SLIDER_HEIGHT) / 4
        );

        p2Slider[i].sliderHandle.SetSize(
            p2Slider[i].sliderBar.GetHeight(),
            p2Slider[i].sliderBar.GetHeight()
        );

        p2Slider[i].sliderHandle.SetPosition(
            p2Slider[i].sliderBar.GetPosition()
        );
        
        p2ValuePos[i].SetX(p2Slider[i].sliderBar.GetX() + p2Slider[i].sliderBar.GetWidth() + horizontalPadding);
        p2ValuePos[i].SetY(p2TextPos[i].GetY());
    }

    for (size_t i = 0; i < P3_SLIDER_COUNT; ++i)
    {
        p3TextPos[i].SetX(canvas.GetX() + horizontalPadding);
        p3TextPos[i].SetY(tabBtn[0].GetY()
                          + tabBtn[0].GetHeight()
                          + fontSize * TITLE_FONT_SIZE_RATIO
                          + canvas.GetHeight() * SLIDER_SPACING * i
                          + horizontalPadding * 2);

        switch (i)
        {
            case GENERATION:
                p3Slider.push_back(std::move(Slider(0.0, 20.0)));
                break;

            case THREADS:
                p3Slider.push_back(std::move(Slider(1.0, 8.0)));
                break;

            case TIMESTEP:
                p3Slider.push_back(std::move(Slider(3e5, 1e6)));
                break;
        }

        p3Slider[i].sliderBar.SetSize(
            canvas.GetWidth() * SLIDER_WIDTH,
            canvas.GetHeight() * SLIDER_HEIGHT
        );

        p3Slider[i].sliderBar.SetPosition(
            p3TextPos[i].GetX() + font.MeasureText(p3SliderString[THREADS], fontSize, 1.0).GetX() + horizontalPadding,
            p3TextPos[i].GetY() + (fontSize - canvas.GetHeight() * SLIDER_HEIGHT) / 4
        );

        p3Slider[i].sliderHandle.SetSize(
            p3Slider[i].sliderBar.GetHeight(),
            p3Slider[i].sliderBar.GetHeight()
        );

        p3Slider[i].sliderHandle.SetPosition(
            p3Slider[i].sliderBar.GetPosition()
        );
        
        p3ValuePos[i].SetX(p3Slider[i].sliderBar.GetX() + p3Slider[i].sliderBar.GetWidth() + horizontalPadding);
        p3ValuePos[i].SetY(p3TextPos[i].GetY());
    }
}

void App::PlayPage()
{
    if (isMainStarted)
    {
        tetrisGame.Draw();
        tetrisGame.Update();
        return;
    }

    font.DrawText(pageTitle[0], pageTitlePos[0], fontSize * TITLE_FONT_SIZE_RATIO, 1.0, RAYWHITE);

    static bool canDrag = true;
    for (size_t i = 0; i < P1_SLIDER_COUNT; ++i)
    {
        p1Slider[i].Draw(canDrag);
        font.DrawText(p1SliderString[i], p1TextPos[i], fontSize, 1.0, RAYWHITE);

        float val = p1Slider[i].GetValue();

        if (val < 10.0) font.DrawText(format("{:.1f}", val), p1ValuePos[i], fontSize, 1.0, RAYWHITE);
        else if (i == SDF && val == 40.0) font.DrawText("INF", p1ValuePos[i], fontSize, 1.0, RAYWHITE);
        else font.DrawText(format("{:.0f}", val), p1ValuePos[i], fontSize, 1.0, RAYWHITE);
    }

    for (size_t i = 0; i < BTN_COUNT; ++i)
    {
        auto& btn = pageBtn[i];

        if (btn.CheckCollision(mousePos))
        {
            btn.DrawRounded(0.6, 5, RAYWHITE);
            font.DrawText(p1BtnString[i], p1BtnTextPos[i], fontSize, 1.0, BLACK);
        }
        else
        {
            btn.DrawRoundedLines(0.6, 5, 2, RAYWHITE);
            font.DrawText(p1BtnString[i], p1BtnTextPos[i], fontSize, 1.0, WHITE);
        }
    }

    if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT))
        for (size_t i = 0; i < BTN_COUNT; ++i)
            if (pageBtn[i].CheckCollision(mousePos))
            {
                isMainStarted = true;
                float cfgArr = p1Slider[ARR].GetValue();
                float cfgDas = p1Slider[DAS].GetValue();
                float cfgSdf = p1Slider[SDF].GetValue();
                float cfgGravity = p1Slider[GRAVITY].GetValue();

                if (cfgSdf == 40.0) cfgSdf = numeric_limits<float>::infinity();
                tetrisGame.SetConfig(cfgArr, cfgDas, cfgSdf, cfgGravity);
                tetrisGame.SetMode((GameMode)i);
                tetrisGame.NewGame();
            }
}

void App::WatchPage()
{
    static int runTimes = 0;
    static int repeatTimes = 0;
    if (isMainStarted)
    {
        tetrisAI.Update();
        if (tetrisAI.IsOver() || runTimes >= repeatTimes) tetrisAI.NewGame();
        tetrisAI.Draw();
        return;
    }

    font.DrawText(pageTitle[1], pageTitlePos[1], fontSize * TITLE_FONT_SIZE_RATIO, 1.0, RAYWHITE);

    static bool canDrag = true;
    for (size_t i = 0; i < P2_SLIDER_COUNT; ++i)
    {
        p2Slider[i].Draw(canDrag);
        font.DrawText(p2SliderString[i], p2TextPos[i], fontSize, 1.0, RAYWHITE);

        float val = p2Slider[i].GetValue();

        if (val < 10.0) font.DrawText(format("{:.1f}", val), p2ValuePos[i], fontSize, 1.0, RAYWHITE);
        else if (i == SDF && val == 40.0) font.DrawText("INF", p2ValuePos[i], fontSize, 1.0, RAYWHITE);
        else font.DrawText(format("{:.0f}", val), p2ValuePos[i], fontSize, 1.0, RAYWHITE);
    }

    for (size_t i = 0; i < BTN_COUNT; ++i)
    {
        auto& btn = pageBtn[i];
        string currentGen = (i == 1) ? to_string(trainer.generation) : "";

        if (btn.CheckCollision(mousePos))
        {
            btn.DrawRounded(0.6, 5, RAYWHITE);
            font.DrawText(p2BtnString[i] + currentGen, p2BtnTextPos[i], fontSize, 1.0, BLACK);
        }
        else
        {
            btn.DrawRoundedLines(0.6, 5, 2, RAYWHITE);
            font.DrawText(p2BtnString[i] + currentGen, p2BtnTextPos[i], fontSize, 1.0, WHITE);
        }
    }

    if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT))
        for (size_t i = 0; i < BTN_COUNT; ++i)
            if (pageBtn[i].CheckCollision(mousePos))
            {
                if (i == 1) return;

                isMainStarted = true;
                float pps = p2Slider[PPS].GetValue();
                repeatTimes = p2Slider[REPEAT].GetValue();

                tetrisAI.SetPPS(pps == 20.0 ? 0 : pps);
                tetrisAI.UpdateHeuristics(trainer.GetBestIndividual().chromosome);
                tetrisAI.NewGame();
            }
}

void App::TrainPage()
{
    if (isMainStarted)
    {
        while (!trainer.ShouldStop()) trainer.StartTraining();
        isMainStarted = false;
        return;
    }

    font.DrawText(pageTitle[2], pageTitlePos[2], fontSize * TITLE_FONT_SIZE_RATIO, 1.0, RAYWHITE);

    static bool canDrag = true;
    for (size_t i = 0; i < P3_SLIDER_COUNT; ++i)
    {
        p3Slider[i].Draw(canDrag);
        font.DrawText(p3SliderString[i], p3TextPos[i], fontSize, 1.0, RAYWHITE);

        float val = p3Slider[i].GetValue();
        font.DrawText(format("{:.0f}", val), p3ValuePos[i], fontSize, 1.0, RAYWHITE);
    }

    for (size_t i = 0; i < BTN_COUNT; ++i)
    {
        auto& btn = pageBtn[i];
        string currentGen = (i == 1) ? to_string(trainer.generation) : "";

        if (btn.CheckCollision(mousePos))
        {
            btn.DrawRounded(0.6, 5, RAYWHITE);
            font.DrawText(p3BtnString[i] + currentGen, p3BtnTextPos[i], fontSize, 1.0, BLACK);
        }
        else
        {
            btn.DrawRoundedLines(0.6, 5, 2, RAYWHITE);
            font.DrawText(p3BtnString[i] + currentGen, p3BtnTextPos[i], fontSize, 1.0, WHITE);
        }
    }

    if (raylib::Mouse::IsButtonPressed(MOUSE_BUTTON_LEFT))
        for (size_t i = 0; i < BTN_COUNT; ++i)
            if (pageBtn[i].CheckCollision(mousePos))
            {
                switch (i)
                {
                    case 0:
                        isMainStarted = true;
                        trainer.SetConfig(static_cast<int>(p3Slider[1].GetValue()), static_cast<int>(p3Slider[0].GetValue()));
                        break;

                    case 1:
                        return;

                    case 2:
                        return;
                }
            }
}
