#ifndef PIECE_H
#define PIECE_H

#include "board.h"
#include <vector>

class Piece
{
    public:
        // Piece properties
        int x{ boardWidth / 2 }; // X position on the board
        int y{ 0 }; // Y position on the board
        int width{ 0 }; // Width of the piece in blocks
        int height{ 0 }; // Height of the piece in blocks
        std::vector<std::vector<int>> shape; // 2D vector representing the piece shape
        int rotation{ 0 }; // Current rotation state of the piece
        int color; // Color for each block in the piece

};

#endif