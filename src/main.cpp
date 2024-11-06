#include "ui/ui.hpp"
#include "ai/genetic.hpp"

raylib::Window gameWindow(1920, 1080, "Tetris++");
raylib::Texture minoTexture("./sprite.png");
raylib::Font font("./hun2.ttf", 80);

int main()
{
    gameWindow.SetFullscreen(true);
    gameWindow.SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    gameWindow.SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetTraceLogLevel(LOG_ERROR);
    //gameWindow.SetTargetFPS(60);
    font.GetTexture().GenMipmaps();
    font.GetTexture().SetFilter(TEXTURE_FILTER_BILINEAR);

    TetrisUI tetrisGame;
    TetrisHeurAI tetrisAI;
    Trainer trainer;

    cout << CLEAR_SCREEN;

    stringstream ss;
    ss << CLI_BLUE << "1. Play Tetris! " << RESET << endl;
    ss << CLI_GREEN << "2. Watch AI play Tetris! " << RESET << endl;
    ss << CLI_RED << "3. Train an AI to play Tetris!" << RESET << endl;

    cout << createBox(BOLD + "Tetris++" + RESET, ss.str()) << endl
        << "Enter your choice: ";

    switch (getchar())
    {
        case '1':
            cout << "Have fun!" << endl;
            while (!gameWindow.ShouldClose())
            {
                gameWindow.BeginDrawing();
                gameWindow.ClearBackground();
                tetrisGame.Update();
                if (tetrisGame.IsOver()) tetrisGame.NewGame();
                tetrisGame.Draw();
                gameWindow.EndDrawing();
            }
            cout << "See you again!" << endl;
            break;

        case '2':
            cout << "Sit back and relax" << endl;
            tetrisAI.UpdateHeuristics(trainer.GetBestIndividual().chromosome);
            while (!gameWindow.ShouldClose())
            {
                gameWindow.BeginDrawing();
                gameWindow.ClearBackground();
                tetrisAI.Update();
                if (tetrisAI.IsOver()) tetrisAI.NewGame();
                tetrisAI.Draw();
                gameWindow.EndDrawing();
            }
            cout << "See you again!" << endl;
            break;

        case '3':
            cout << "Burning your PC..." << endl;
            while(!trainer.ShouldStop()) trainer.StartTraining();
            cout << "See you again!" << endl;
            break;

        default:
            cout << "See you again!" << endl;
            break;
    }

    gameWindow.Close();
}
