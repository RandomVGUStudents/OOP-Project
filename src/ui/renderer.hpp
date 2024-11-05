#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <deque>
#include <format>
#include "../include/raylib-cpp.hpp"
#include "core/common.hpp"
#include "core/block.hpp"
#include "core/board.hpp"
#include "animation.hpp"


// UI elements position
constexpr float TARGET_ASPECT_RATIO = 16.0 / 9.0;
constexpr float CANVAS_WIDTH = 0.375;
constexpr float CANVAS_HEIGHT = 0.65;
constexpr float HOLDBOX_WIDTH = 0.25;
constexpr float HOLDBOX_HEIGHT = 0.2;
constexpr float QUEUE_COLUMN_WIDTH = 0.25;
constexpr float QUEUE_COLUMN_HEIGHT = 0.8;
constexpr float CELL_SIZE = 0.05;
constexpr float FONT_SIZE = 0.03;
constexpr float TEXT_PADDING = 0.01;
constexpr int LINE_OFFSET = 2;


// Types of statistics to display
constexpr int STAT_TYPES = 9;
enum GameStatsDisplay {
    NONE, LEVEL, SCORE, TIME,
    LINESPEED, LINECOUNT, BLOCKCOUNT,
    KEYPRESSCOUNT, FULLCLEARCOUNT
};

// Corresponding title for each statistics type
constexpr array<string, STAT_TYPES> textDisplay = {{
    "", "level", "score", "time",
    "lines", "lines", "pieces", "inputs", "full clears"
}};

// Configurable statistics display, including preset for each gamemode
constexpr int STAT_SLOT_COUNT = 5;
typedef array<GameStatsDisplay, STAT_SLOT_COUNT> CounterConfig;
constexpr CounterConfig preset40Lines = {{ TIME, LINECOUNT, BLOCKCOUNT, KEYPRESSCOUNT, SCORE }};
constexpr CounterConfig presetBlitz = {{ FULLCLEARCOUNT, LEVEL, LINECOUNT, TIME, SCORE }};


// Types of event-based messages
enum { T_SPIN_MSG, CLEAR_MSG, B2B_MSG, COMBO_MSG, FC_MSG };


// External variables from main driver
extern raylib::Window gameWindow;
extern raylib::Font font;
extern raylib::Texture minoTexture;


class TetrisRenderer
{
public:
    TetrisRenderer(GameStats& gameStats, CounterConfig config=preset40Lines);

    // Functions to be called in Draw() loop
    void UpdateScreenSize(); // Run when resizing window
    void DrawHoldBox(Block& holdBlock);
    void DrawQueueColumn(deque<BlockType>& currentBag);
    void DrawBoard(Block& currentBlock, int hardDropPos, Board& board);
    void DrawStats();
    void DrawMessages();

    // Invoking event-based messages
    void InvokeClearMsg(const int clearedLine);
    void InvokeTSpinMsg(const bool isNormalTspin);
    void InvokeB2BMsg(const int b2bCount);
    void InvokeComboMsg(const int comboCount);
    void InvokeFullClearMsg();

private:
    GameStats& stats;

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
    array<void (TetrisRenderer::*)(int), STAT_TYPES> DrawStatsFunctions;
    void DrawLevel(int slotNumber);
    void DrawScore(int slotNumber);
    void DrawTime(int slotNumber);
    void DrawLineSpeed(int slotNumber);
    void DrawLineCount(int slotNumber);
    void DrawPieces(int slotNumber);
    void DrawInputs(int slotNumber);
    void DrawFullClear(int slotNumber);

    void DrawStatSlot(int slotNumber, const string& slotText, const string& slotSubText="");
};

#endif /* RENDERER_HPP */
