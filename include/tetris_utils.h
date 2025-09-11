#ifndef TETRIS_UTILS_H
#define TETRIS_UTILS_H

#include "board.h"
#include "piece.h"
#include "globals.h"
#include <vector>
#include <string>

bool checkPlacement(const Piece&, const Board&, int, int);

void pieceSet(const Piece& piece, Board board, int color = 0);

int maxDrop(const Piece&, const Board&);

void spawnParticles(const Piece&);

void spawnParticlesAt(int, int, int);

std::string chooseWindowTitle();

enum class InputAction {
    None,
    MoveLeft,
    MoveRight,
    RotateClockwise,
    RotateCounterClockwise,
    SoftDrop,
    HardDrop,
    Hold,
    Pause
};

#endif