#ifndef MAIN_UI_HPP
#define MAIN_UI_HPP

#include "../include/raylib-cpp.hpp"
#include "tetrisUI.hpp"
#include "ai/genetic.hpp"
#include <array>

using namespace std;


constexpr int BTN_COUNT = 3;
enum CurrentPage { PLAY, WATCH, TRAIN };
constexpr array<string, BTN_COUNT> btnString = {{ "PLAY", "EVAL", "TRAIN" }};
constexpr array<string, BTN_COUNT> pageTitle = {{ "GAME CONFIG", "AI CONFIG", "TRAIN CONFIG" }};

constexpr int P1_SLIDER_COUNT = 4;
enum P1Slider { ARR, DAS, SDF, GRAVITY };
constexpr array<string, P1_SLIDER_COUNT> p1SliderString = {{ "ARR", "DAS", "SDF", "GRAVITY" }};
constexpr array<string, BTN_COUNT> p1BtnString = {{ "40 LINES", "BLITZ", "ZEN" }};

constexpr int P2_SLIDER_COUNT = 3;
enum P2Slider { PPS, REPEAT, GEN };
constexpr array<string, P2_SLIDER_COUNT> p2SliderString = {{ "PPS", "REPEAT", "GEN." }};
constexpr array<string, BTN_COUNT> p2BtnString = {{ "RUN GA", "CURRENT", "GEN. " }};

constexpr int P3_SLIDER_COUNT = 1;
enum P3Slider { RENDER };
constexpr array<string, P3_SLIDER_COUNT> p3SliderString = {{ "RENDER" }};
constexpr array<string, BTN_COUNT> p3BtnString = {{ "TRAIN GA", "CURRENT", "GEN. " }};

constexpr float TAB_BTN_SIZE = 0.3;
constexpr float BTN_PADDING = (1 - TAB_BTN_SIZE * BTN_COUNT) / (BTN_COUNT + 1);

constexpr float TITLE_FONT_SIZE_RATIO = 1.5;

constexpr float SLIDER_WIDTH = 0.42;
constexpr float SLIDER_HEIGHT = 0.03;
constexpr float SLIDER_SPACING = 0.15;
constexpr float SLIDER_PADDING = 0.05;

// External variables from main driver
extern raylib::Window gameWindow;
extern raylib::Font font;
extern raylib::Texture minoTexture;

static raylib::Vector2 mousePos(raylib::Mouse::GetPosition());

class Slider
{
public:
    Slider(float startVal, float endVal);

    raylib::Rectangle sliderBar;
    raylib::Rectangle sliderHandle;

    float GetValue();
    void Draw(bool& canDrag);

private:
    float startVal;
    float endVal;
    float outputVal;
    bool sliding;
};


class App
{
public:
    App();

    void Loop();

private:
    int screenWidth;
    int screenHeight;

    float fontSize;
    raylib::Rectangle canvas;
    array<raylib::Rectangle, BTN_COUNT> tabBtn;
    array<raylib::Vector2, BTN_COUNT> btnTextPos;
    array<raylib::Vector2, BTN_COUNT> pageTitlePos;
    array<raylib::Rectangle, BTN_COUNT> pageBtn;

    // PlayPage elements
    vector<Slider> p1Slider;
    array<raylib::Vector2, P1_SLIDER_COUNT> p1ValuePos;
    array<raylib::Vector2, P1_SLIDER_COUNT> p1TextPos;
    array<raylib::Vector2, BTN_COUNT> p1BtnTextPos;

    // EvalPage elements
    vector<Slider> p2Slider;
    array<raylib::Vector2, P2_SLIDER_COUNT> p2ValuePos;
    array<raylib::Vector2, P2_SLIDER_COUNT> p2TextPos;
    array<raylib::Vector2, BTN_COUNT> p2BtnTextPos;

    // TrainPage elements
    vector<Slider> p3Slider;
    array<raylib::Vector2, P3_SLIDER_COUNT> p3ValuePos;
    array<raylib::Vector2, P3_SLIDER_COUNT> p3TextPos;
    array<raylib::Vector2, BTN_COUNT> p3BtnTextPos;

    CurrentPage currentPage;
    bool isMainStarted;

    TetrisUI tetrisGame;
    TetrisHeurAI tetrisAI;
    Trainer trainer;
    
    void UpdateScreenSize();
    void CalculateElements();

    void PlayPage();
    void EvalPage();
    void TrainPage();
};

#endif /* MAIN_UI_HPP */
