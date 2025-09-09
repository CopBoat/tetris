#include "tetris_utils.h"
#include "globals.h"
#include <iostream>
#include <math.h>

bool checkPlacement(const Piece& piece, const Board& board, int newX, int newY) {
    bool placementValid = true;
    for (int sx = 0; sx < piece.width; ++sx) {
        for (int sy = 0; sy < piece.height; ++sy) {
            if (piece.shape[sy][sx] != 0) {
                int boardX = piece.x + sx + newX;
                int boardY = piece.y + sy + newY;
                if (boardX < 0 || boardX >= boardWidth || boardY < 0 || boardY >= boardHeight || board.current[boardX][boardY] != 0) {
                    placementValid = false;
                }
            }
        }
        if (!placementValid) break;
    }
    return placementValid; 
}

void spawnParticles(const Piece& piece) {
    for (int sx = 0; sx < piece.width; ++sx) {
        for (int sy = 0; sy < piece.height; ++sy) {
            if (piece.shape[sy][sx] != 0) {
                int numSparkles = 8 + std::rand() % 8; // More sparkles per block
                for (int i = 0; i < numSparkles; ++i) {
                    Particle p;
                    p.x = (piece.x + sx) * blockSize + blockSize / 2;
                    p.y = (piece.y + sy) * blockSize + blockSize / 2;
                    float angle = (std::rand() % 360) * 3.14159f / 180.0f;
                    float speed = 1.0f + (std::rand() % 100) / 100.0f;
                    p.vx = cos(angle) * speed;
                    p.vy = sin(angle) * speed;
                    p.lifetime = 15 + std::rand() % 10;
                    // Sparkle colors: white, yellow, cyan, light blue
                    int c = std::rand() % 4;
                    switch (c) {
                        case 0: p.color = {255, 255, 255, 255}; break; // White
                        case 1: p.color = {255, 255, 128, 255}; break; // Yellowish
                        case 2: p.color = {128, 255, 255, 255}; break; // Cyan
                        case 3: p.color = {200, 200, 255, 255}; break; // Light blue
                    }
                    p.alpha = 255.0f;
                    particles.push_back(p);
                }
            }
        }
    }
}

void spawnParticlesAt(int x, int y, int color) {
    int numSparkles = 8 + std::rand() % 8;
    for (int i = 0; i < numSparkles; ++i) {
        Particle p;
        p.x = x * blockSize + blockSize / 2;
        p.y = y * blockSize + blockSize / 2;
        float angle = (std::rand() % 360) * 3.14159f / 180.0f;
        float speed = 1.0f + (std::rand() % 100) / 100.0f;
        p.vx = cos(angle) * speed;
        p.vy = sin(angle) * speed;
        p.lifetime = 15 + std::rand() % 10;
        // Set color based on block color if desired
        switch (color) {
            case 1: p.color = {255, 0, 0, 255}; break;
            case 2: p.color = {0, 0, 255, 255}; break;
            case 3: p.color = {255, 255, 0, 255}; break;
            case 4: p.color = {0, 255, 255, 255}; break;
            case 5: p.color = {0, 255, 0, 255}; break;
            case 6: p.color = {255, 0, 255, 255}; break;
            case 7: p.color = {255, 128, 0, 255}; break;
            default: p.color = {255, 255, 255, 255}; break;
        }
        p.alpha = 255.0f;
        particles.push_back(p);
    }
}