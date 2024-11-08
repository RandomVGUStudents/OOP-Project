#include "ui/mainUI.hpp"

raylib::Window gameWindow(1920, 1080, "Tetris++", FLAG_MSAA_4X_HINT);
raylib::Texture minoTexture("./sprite.png");
raylib::Font font("./hun2.ttf", 80);

int main()
{
    gameWindow.SetState(FLAG_WINDOW_RESIZABLE);
    gameWindow.SetExitKey(0);
    SetTraceLogLevel(LOG_FATAL);
    //gameWindow.SetFullscreen(true);
    //gameWindow.SetTargetFPS(60);

    font.GetTexture().GenMipmaps();
    font.GetTexture().SetFilter(TEXTURE_FILTER_BILINEAR);

    App app;

    while (!gameWindow.ShouldClose()) app.Loop();

    gameWindow.Close();
}
