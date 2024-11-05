//#include "ui/ui.hpp"
#include "ai/heuristics.hpp"

raylib::Window gameWindow(1920, 1080, "Tetr.io Clone");
raylib::Texture minoTexture("./sprite.png");
raylib::Font font("./hun2.ttf", 80);

int main()
{
    gameWindow.SetFullscreen(true);
    gameWindow.SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    gameWindow.SetConfigFlags(FLAG_MSAA_4X_HINT);
    gameWindow.SetTargetFPS(60);
    font.GetTexture().GenMipmaps();
    font.GetTexture().SetFilter(TEXTURE_FILTER_BILINEAR);

    //TetrisUI tetrisGame;
    TetrisHeurAI tetrisGame;

    while (!gameWindow.ShouldClose())
    {
        gameWindow.BeginDrawing();
        gameWindow.ClearBackground();
        tetrisGame.Update();
        tetrisGame.Draw();
        gameWindow.EndDrawing();
    }

    gameWindow.Close();
}
