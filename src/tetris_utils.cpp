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

void pieceSet(const Piece& piece, Board board, int color) {
    for (int sx = 0; sx < piece.width; ++sx) {
        for (int sy = 0; sy < piece.height; ++sy) {
            if (piece.shape[sy][sx] != 0) {
                int boardX = piece.x + sx;
                int boardY = piece.y + sy;
                board.current[boardX][boardY] = color;
            }
        }
    }
}

int maxDrop(const Piece& piece, const Board& board) {
    int maxDrop = boardHeight - piece.height;
    for (int dropY = piece.y; dropY <= maxDrop; ++dropY) {
        bool collision = false;
        for (int sx = 0; sx < piece.width; ++sx) {
            for (int sy = 0; sy < piece.height; ++sy) {
                if (piece.shape[sy][sx] != 0) {
                    int boardX = piece.x + sx;
                    int boardY = dropY + sy;
                    if (boardY >= boardHeight || board.current[boardX][boardY] != 0) {
                        collision = true;
                        break;
                    }
                }
            }
            if (collision) break;
        }
        if (collision) {
            return dropY - 1;
        }
        // If we reached the last possible position, return maxDrop
        if (dropY == maxDrop) {
            return maxDrop;
        }
    }
    return piece.y; // No drop possible
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

std::string chooseWindowTitle() {
    int alternateIndex = std::rand() % 3;
    if (alternateIndex == 0) {
        int index = std::rand() % windowTitles.size();
        return windowTitles[index];
    }
    return "Tetris (CopBoat's Version)";
}

Piece pieceTypes[7] = { iPiece, oPiece, tPiece, lPiece, jPiece, sPiece, zPiece }; // Array of piece types
int pickPiece = std::rand() % 7;  // Randomly select the current piece from pieceTypes 
int nextPickPiece = std::rand() % 7; // Randomly select the next piece from pieceTypes
Piece currentPiece = pieceTypes[pickPiece]; // Initialize current piece
Piece nextPiece = pieceTypes[nextPickPiece]; // Initialize next piece

//row clearing animation variables
bool clearingRows = false;
std::vector<int> rowsToClear;
Uint64 clearAnimStart = 0;
int clearAnimStep = 0;
const int clearAnimSteps = boardWidth / 2 + 1; // Number of steps for center-out
const Uint64 clearAnimDuration = 500000000; // 0.5 seconds in ns

Piece holdPiece; // Piece to hold
bool newPiece{ false }; // To track if a new piece is needed
bool holdUsed{ false }; // To track if hold was used in the current turn
int rowsCleared = 0; // To track number of cleared rows
int levelIncrease = 0; // To track level increase threshold
bool hardDrop = false; // To track if hard drop was used
bool alternateIPieceRotationOffset = false; // To alternate I piece rotation offsets
bool paused = false; // To track if the game is paused

//piece state variables for lock delay
int lockDelayFrames = 30; // Number of frames to allow after landing (adjust as desired)
int lockDelayCounter = 0; // Counts frames since landing (reset on move/rotate)
bool pieceLanded = false; // True if just landed, false if still falling

Board board; // The game board