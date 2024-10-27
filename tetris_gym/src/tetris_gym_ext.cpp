#include <iostream>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include "game.hpp"

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

        std::array<array<uint8_t, BOARD_HEIGHT>, BOARD_WIDTH> board_data;
        for (size_t i = 0; i < BOARD_WIDTH; ++i) {
            for (size_t j = 0; j < BOARD_HEIGHT; ++j) {
                board_data[i][j] = (board[i][j] == EMPTY) ? 0 : 1;
            }
        }

        for (const Coord& coord : this->game.currentBlock->GetCoords())
            board_data[coord.x][coord.y] = 1;

        nb::ndarray<uint8_t, nb::numpy, nb::shape<10, 22>, nb::device::cpu> board_array(board_data.data());

        std::array<uint8_t, 6> queue_data;
        queue_data[0] = this->game.currentBlock->GetType();
        for (size_t i = 1; i < 6; ++i) {
            queue_data[i] = this->game.currentBag[i].GetType();
        }
        nb::ndarray<uint8_t, nb::numpy, nb::shape<6>, nb::device::cpu> queue_array(queue_data.data());

        uint8_t holdBlock = this->game.holdBlock ? this->game.holdBlock->GetType() : EMPTY;

        return nb::make_tuple(
            board_array,
            queue_array,
            holdBlock
        );
    }

    nb::tuple step(int action)
    {
        Action a = static_cast<Action>(action);
        int reward = this->game.Update(a);

        bool done = this->game.IsGameOver();
        nb::dict info;

        return nb::make_tuple(reward, done, info);
    }

    void reset()
    {
        this->game.board.Reset();
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
