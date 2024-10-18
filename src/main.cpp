#include "game.hpp"

raylib::Window gameWindow(1920, 1080, "Tetr.io Clone");
raylib::Texture minoTexture("./sprite.png");
raylib::Font font("./hun2.ttf", 80);


int main()
{
    gameWindow.SetFullscreen(true);
    gameWindow.SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    gameWindow.SetConfigFlags(FLAG_MSAA_4X_HINT);
    font.GetTexture().GenMipmaps();
    font.GetTexture().SetFilter(TEXTURE_FILTER_BILINEAR);

    Game game;

    while (!gameWindow.ShouldClose())
    {
        gameWindow.BeginDrawing();
        gameWindow.ClearBackground();
        game.Update();
        game.Draw();
        gameWindow.EndDrawing();
    }

    gameWindow.Close();
}
