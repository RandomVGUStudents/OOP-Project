#include <iostream>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include "board.hpp"
#include "minimalGame.hpp"

namespace nb = nanobind;
class Tetris
{
private:
    Game game;

public:
    Tetris() : game() {};

    nb::tuple getState()
    {
        auto& board = game.board.GetBoard();

        std::array<array<uint8_t, BOARD_HEIGHT - 2>, BOARD_WIDTH> board_data;
        for (size_t i = 0; i < BOARD_WIDTH; ++i) {
            for (size_t j = 2; j < BOARD_HEIGHT; ++j) {
                board_data[i][j - 2] = (board[i][j] == EMPTY) ? 0 : 1;
            }
        }

        // Create nb::ndarray for the board
        nb::ndarray<uint8_t, nb::numpy, nb::shape<10, 20>, nb::device::cpu> board_array(board_data.data());

        // Create nb::ndarray for the queue
        std::array<uint8_t, 5> queue_data;
        for (size_t i = 0; i < 5; ++i) {
            queue_data[i] = this->game.currentBag[i].GetType();
        }
        nb::ndarray<uint8_t, nb::numpy, nb::shape<5>, nb::device::cpu> queue_array(queue_data.data());

        uint8_t currentBlock = this->game.currentBlock ? this->game.currentBlock->GetType() : EMPTY;
        uint8_t holdBlock = this->game.holdBlock ? this->game.holdBlock->GetType() : EMPTY;

        return nb::make_tuple(
            board_array,
            queue_array,
            currentBlock,
            holdBlock
        );
    }

    nb::tuple step(nb::dict action)
    {
        int reward = 0;
        bool useHold = nb::cast<bool>(action["use_hold"]);

        if (useHold)
        {
            if (this->game.usedHold)
                reward = -1000;
            this->game.HoldBlock();
        }
        else
        {
            int col = nb::cast<int>(action["col"]);
            RotateState rotation = static_cast<RotateState>(nb::cast<int>(action["rotation"]));

            reward = this->game.PlaceBlock(col, rotation);
        }

        bool done = this->game.IsGameOver();
        nb::dict info;

        return nb::make_tuple(reward, done, info);
    }

    void reset()
    {
        this->game.Reset();
    }
};


NB_MODULE(tetris_gym_ext, m)
{
    nb::class_<Tetris>(m, "Tetris")
    .def(nb::init<>())
    .def("reset", &Tetris::reset)
    .def("getState", &Tetris::getState)
    .def("step", &Tetris::step);
}
