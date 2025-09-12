#ifndef TETRIS_UTILS_H
#define TETRIS_UTILS_H

#include "board.h"
#include "piece.h"
#include "globals.h"
#include <vector>
#include <string>

bool checkPlacement(const Piece&, const Board&, int, int);

void pieceSet(const Piece& piece, Board& board, int color = 0);

int maxDrop(const Piece&, const Board&);

void rotateIPieceClockwise();
void rotatePieceClockwise();

void rotateIPieceCounterClockwise();
void rotatePieceCounterClockwise();

void resetRotation();
void firstHold();
void pieceSwap();

void spawnParticles(const Piece&);
void spawnParticlesAt(int, int, int);

void renderBoardBlocks();
void renderBoardBlocksDuringAnimation();

void renderPauseMenu();

void animateRowClear();

bool checkGameOver();

void autoDrop(bool canPlaceNextPiece);

void handleLockDelay(bool canPlaceNextPiece);

void handlePieceLanded();

std::string chooseWindowTitle();

extern Piece pieceTypes[7];
extern int pickPiece;
extern int nextPickPiece;
extern Piece currentPiece;
extern Piece nextPiece;

extern bool clearingRows;
extern std::vector<int> rowsToClear;
extern Uint64 clearAnimStart;
extern int clearAnimStep;
extern const int clearAnimSteps;
extern const Uint64 clearAnimDuration;

extern Piece holdPiece;
extern bool newPiece;
extern bool holdUsed;
extern int rowsCleared;
extern int levelIncrease;
extern bool hardDropFlag;
extern bool alternateIPieceRotationOffset;
extern bool paused;

extern int lockDelayFrames;
extern int lockDelayCounter;
extern bool pieceLanded;

extern Board board;

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

void moveLeft();
void moveRight();
void rotateClockwise();
void rotateCounterClockwise();
void softDrop();
void hardDrop();
void hold();
void pauseGame();

#endif