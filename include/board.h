#ifndef BOARD_H
#define BOARD_H

#include <SDL3/SDL.h>
#include <vector>

constexpr int boardWidth{ 15 }; // Width of the board in blocks
constexpr int boardHeight{ 20 }; // Height of the board in blocks

class Board
{
    public:

        int current[boardWidth][boardHeight];

        // Initialize the board with empty blocks
        Board() {
            for (int x = 0; x < boardWidth; ++x)
                for (int y = 0; y < boardHeight; ++y)
                    current[x][y] = 0; 
        }
};

struct Particle {
    float x, y;
    float vx, vy;
    int lifetime;
    SDL_Color color;
    float alpha; // Add alpha for fading
};

extern std::vector<Particle> particles;

#endif