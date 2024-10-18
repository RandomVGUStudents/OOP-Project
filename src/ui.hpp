#ifndef UI_HPP
#define UI_HPP

#include <array>
#include <deque>
#include <format>
#include <optional>
#include <string>
#include "../include/raylib-cpp.hpp"
#include "board.hpp"
#include "animation.hpp"


// UI elements position
static const float TARGET_ASPECT_RATIO = 16.0 / 9.0;
static const float CANVAS_WIDTH = 0.375;
static const float CANVAS_HEIGHT = 0.65;
static const float HOLDBOX_WIDTH = 0.25;
static const float HOLDBOX_HEIGHT = 0.2;
static const float QUEUE_COLUMN_WIDTH = 0.25;
static const float QUEUE_COLUMN_HEIGHT = 0.8;
static const float CELL_SIZE = 0.05;
static const float FONT_SIZE = 0.03;
static const float TEXT_PADDING = 0.01;
static const int LINE_OFFSET = BOARD_HEIGHT - 20;


// External variables from main driver
extern raylib::Window gameWindow;
extern raylib::Font font;
extern raylib::Texture minoTexture;


// Types of statistics to display
#define STAT_TYPES 8
enum GameStatsDisplay {
    NONE, LEVEL, SCORE, TIME,
    LINECOUNT, BLOCKCOUNT, KEYPRESSCOUNT, FULLCLEARCOUNT
};

// Corresponding title for each statistics type
static const array<string, STAT_TYPES> textDisplay = {{
    "", "level", "score", "time",
    "lines", "pieces", "inputs", "full clears"
}};

// Configurable statistics display, including preset for each gamemode
#define STAT_SLOT_COUNT 5
typedef array<GameStatsDisplay, STAT_SLOT_COUNT> CounterConfig;
static CounterConfig preset40Lines = {{ TIME, LINECOUNT, BLOCKCOUNT, KEYPRESSCOUNT, SCORE }};
static CounterConfig presetBlitz = {{ FULLCLEARCOUNT, LEVEL, LINECOUNT, TIME, SCORE }};


// Types of event-based messages
enum { T_SPIN_MSG, CLEAR_MSG, B2B_MSG, COMBO_MSG, FC_MSG };


struct GameStats
{
    float timeElapsed = 0;
    int droppedBlockCount = 0;
    int clearedLineCount = 0;
    int tSpinCount = 0;
    int tetrisCount = 0;
    int fullClearCount = 0;
    int keyPressed = 0;
    int comboCount = -1;
    int b2bChain = -1;
    int level = 1;
    int score = 0;
};

class GameRenderer
{
public:
    GameRenderer(GameStats& stats);

    // Functions to be called in Draw() loop
    void UpdateScreenSize(); // Run when resizing window
    void DrawHoldBox(const optional<Block>& holdBlock);
    void DrawQueueColumn(const deque<Block>& currentBag);
    void DrawBoard(const Board& board, const Block& currentBlock, int hardDropPos);
    void DrawStats();
    void DrawMessages(const float frametime);

    // Invoking event-based messages
    void InvokeClearMsg(const int clearedLine);
    void InvokeTSpinMsg(const bool normalTspin);
    void InvokeB2BMsg(const int b2bChain);
    void InvokeComboMsg(const int comboCount);
    void InvokeFullClearMsg();

private:
    const GameStats& gameStats;

    int screenWidth;
    int screenHeight;
    float aspectRatio;
    float scaleX;
    float scaleY;
    float fontSize;
    float bigFontSize;
    CounterConfig slotConfig;

    raylib::Rectangle canvas;
    raylib::Rectangle holdBox;
    raylib::Rectangle queueColumn;
    raylib::Rectangle mino;
    raylib::Vector2 bottomLeft;
    raylib::Vector2 bottomRight;
    raylib::Vector2 boardMiddle;

    array<array<raylib::Vector2, 2>, STAT_SLOT_COUNT> statSlotCoords;
    array<raylib::Vector2, 4> messagesCoords;
    array<float, 5> messagesTimer;
    array<string, 4> messagesData;
    array<raylib::Vector2, BLOCK_TYPES> blockSizes;
    array<raylib::Rectangle, BLOCK_TYPES + 1> textureCoords;

    // Responsive to window size
    void CalculateElements();

    // Helper functions to draw statistics
    array<void (GameRenderer::*)(int), STAT_TYPES> DrawStatsFunctions;
    void DrawLevel(int slotNumber);
    void DrawScore(int slotNumber);
    void DrawTime(int slotNumber);
    void DrawLineCount(int slotNumber);
    void DrawPieces(int slotNumber);
    void DrawInputs(int slotNumber);
    void DrawFullClear(int slotNumber);

    void DrawStatSlot(int slotNumber, const string& slotText, const string& slotSubText="");
};

#endif /* UI_HPP */
