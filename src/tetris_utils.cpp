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

void rotateIPieceClockwise() {
    std::vector<std::vector<int>> newShape(currentPiece.width, std::vector<int>(currentPiece.height, 0));
    for (int sx = 0; sx < currentPiece.width; ++sx) {
        for (int sy = 0; sy < currentPiece.height; ++sy) {
            newShape[sx][currentPiece.height - 1 - sy] = currentPiece.shape[sy][sx];
        }
    }

    Piece rotatedPiece = currentPiece;
    rotatedPiece.shape = newShape;
    rotatedPiece.width = currentPiece.height;
    rotatedPiece.height = currentPiece.width;
    rotatedPiece.rotation = (currentPiece.rotation + 1) % 4;

    if (rotatedPiece.rotation % 4 == 1 || rotatedPiece.rotation % 4 == 3) 
    {
        // Shift right for vertical I piece
        if (alternateIPieceRotationOffset) {
            rotatedPiece.x += 2;
            alternateIPieceRotationOffset = false;
        }
        else {
            rotatedPiece.x += 1;
            alternateIPieceRotationOffset = true;
        }
    } 
    else {
        if (alternateIPieceRotationOffset) {
            rotatedPiece.x -= 1;
        }
        else {
            rotatedPiece.x -= 2;
        }
    }

    for (const auto& offset: wallKickOffsetsI[rotatedPiece.rotation]) {
        // Check if rotated piece fits at (newX, newY)
        if (checkPlacement(rotatedPiece, board, offset.first, offset.second)) {
            // Apply rotation
            currentPiece.shape = newShape;
            std::swap(currentPiece.width, currentPiece.height);
            currentPiece.rotation = (currentPiece.rotation + 1) % 4;

            currentPiece.x = rotatedPiece.x + offset.first;
            currentPiece.y = rotatedPiece.y + offset.second;

            
            
            break;
        }
    }
}

void rotatePieceClockwise() {
    std::vector<std::vector<int>> newShape(currentPiece.width, std::vector<int>(currentPiece.height, 0));
    for (int sx = 0; sx < currentPiece.width; ++sx) {
        for (int sy = 0; sy < currentPiece.height; ++sy) {
            newShape[sx][currentPiece.height - 1 - sy] = currentPiece.shape[sy][sx];
        }
    }

    Piece rotatedPiece = currentPiece;
    rotatedPiece.shape = newShape;
    rotatedPiece.width = currentPiece.height;
    rotatedPiece.height = currentPiece.width;
    rotatedPiece.rotation = (currentPiece.rotation + 1) % 4;
    
    for (const auto& offset : wallKickOffsets[rotatedPiece.rotation]) {
        
        // Check if rotated piece fits at (newX, newY)
        if (checkPlacement(rotatedPiece, board, offset.first, offset.second)) {
            // Apply rotation and offset
            currentPiece.shape = newShape;
            std::swap(currentPiece.width, currentPiece.height);
            
            currentPiece.x = currentPiece.x + offset.first;
            currentPiece.y = currentPiece.y + offset.second;
            currentPiece.rotation = (currentPiece.rotation + 1) % 4;
            break;
        }
    }
}

void rotateIPieceCounterClockwise() {
    std::vector<std::vector<int>> newShape(currentPiece.width, std::vector<int>(currentPiece.height, 0));
    for (int sx = 0; sx < currentPiece.width; ++sx) {
        for (int sy = 0; sy < currentPiece.height; ++sy) {
            newShape[currentPiece.width - 1 - sx][sy] = currentPiece.shape[sy][sx];
        }
    }

    Piece rotatedPiece = currentPiece;
    rotatedPiece.shape = newShape;
    rotatedPiece.width = currentPiece.height;
    rotatedPiece.height = currentPiece.width;
    rotatedPiece.rotation = (currentPiece.rotation + 3) % 4;

    if (rotatedPiece.rotation % 4 == 1 || rotatedPiece.rotation % 4 == 3) 
    {
        // Shift right for vertical I piece
        if (alternateIPieceRotationOffset) {
            rotatedPiece.x += 2;
            alternateIPieceRotationOffset = false;
        }
        else {
            rotatedPiece.x += 1;
            alternateIPieceRotationOffset = true;
        }
    } 
    else {
        if (alternateIPieceRotationOffset) {
            rotatedPiece.x -= 1;
        }
        else {
            rotatedPiece.x -= 2;
        }
    }

    for (const auto& offset: wallKickOffsetsI[(rotatedPiece.rotation + 8) % 4]) {
        // Check if rotated piece fits at (newX, newY)
        if (checkPlacement(rotatedPiece, board, offset.first, offset.second)) {
            // Apply rotation
            currentPiece.shape = newShape;
            std::swap(currentPiece.width, currentPiece.height);
            currentPiece.rotation = (currentPiece.rotation - 1) % 4;

            currentPiece.x = rotatedPiece.x + offset.first;
            currentPiece.y = rotatedPiece.y + offset.second;
            break;
        }
    }
}

void rotatePieceCounterClockwise() {
    std::vector<std::vector<int>> newShape(currentPiece.width, std::vector<int>(currentPiece.height, 0));
    for (int sx = 0; sx < currentPiece.width; ++sx) {
        for (int sy = 0; sy < currentPiece.height; ++sy) {
            newShape[currentPiece.width - 1 - sx][sy] = currentPiece.shape[sy][sx];
        }
    }

    Piece rotatedPiece = currentPiece;
    rotatedPiece.shape = newShape;
    rotatedPiece.width = currentPiece.height;
    rotatedPiece.height = currentPiece.width;
    rotatedPiece.rotation = (currentPiece.rotation - 1) % 4;
    
    for (const auto& offset : wallKickOffsets[(rotatedPiece.rotation + 8) % 4]) {
        
        // Check if rotated piece fits at (newX, newY)
        if (checkPlacement(rotatedPiece, board, offset.first, offset.second)) {
            // Apply rotation and offset
            currentPiece.shape = newShape;
            std::swap(currentPiece.width, currentPiece.height);
            
            currentPiece.x = currentPiece.x + offset.first;
            currentPiece.y = currentPiece.y + offset.second;
            currentPiece.rotation = (currentPiece.rotation - 1) % 4;
            break;
        }
    }
}

void resetRotation() {
    if (currentPiece.width == 1 || currentPiece.height == 1) { // I piece
        currentPiece = iPiece; // Reset to I piece
    }
    else 
    {
        while (currentPiece.rotation != 0) { // rotate back to original position
            std::vector<std::vector<int>> newShape(currentPiece.width, std::vector<int>(currentPiece.height, 0));
            for (int sx = 0; sx < currentPiece.width; ++sx) {
                for (int sy = 0; sy < currentPiece.height; ++sy) {
                    newShape[sx][currentPiece.height - 1 - sy] = currentPiece.shape[sy][sx];
                }
            }
            currentPiece.shape = newShape;
            std::swap(currentPiece.width, currentPiece.height);
            currentPiece.rotation = (currentPiece.rotation + 1) % 4;
            newShape = std::vector<std::vector<int>>(currentPiece.width, std::vector<int>(currentPiece.height, 0));
        }
    }
}

void firstHold() {
    holdPiece = currentPiece;
    pickPiece = std::rand() % 7; // Generates a random number between 0 and 6 inclusive
    //std::cout << "Picked piece: " << pickPiece << std::endl;
    currentPiece = pieceTypes[nextPickPiece]; // Select a new random piece
    nextPickPiece = std::rand() % 7; // Randomly select the next piece
    nextPiece = pieceTypes[nextPickPiece]; // Update next piece
    currentPiece.y = 0;
    currentPiece.x = boardWidth / 2;
}

void pieceSwap() {
    std::swap(holdPiece, currentPiece);
    currentPiece.y = 0;
    currentPiece.x = boardWidth / 2;
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

void renderBoardBlocks() {
    // Render the current blocks on the board
    std::vector<SDL_FRect> blockRects;
    for (int x = 0; x < boardWidth; ++x) {
        for (int y = 0; y < boardHeight; ++y) {
            if (board.current[x][y] != 0) {
                SDL_FRect rect{ static_cast<float>(x * blockSize), static_cast<float>(y * blockSize), blockSize, blockSize };
                blockRects.push_back(rect);
            }
        }
    }

    // Render the rectangles for the blocks
    if (!blockRects.empty()) {
        // Render rectangles for the blocks with individual colors
        for (size_t i = 0; i < blockRects.size(); ++i) {
            for (int x = 0; x < boardWidth; ++x) {
                for (int y = 0; y < boardHeight; ++y) {
                    int val = board.current[x][y];
                    if (val != 0) {
                        SDL_FRect rect{ static_cast<float>(x * blockSize) + spacing / 2,
                                        static_cast<float>(y * blockSize) + spacing / 2,
                                        blockSize - spacing,
                                        blockSize - spacing };
                        SDL_Color color;
                        switch (val) {
                            case 1: color = {0, 255, 255, 255}; break; //cyan
                            case 2: color = {255, 255, 0, 255}; break; //yellow
                            case 3: color = {128, 0, 128, 255}; break; //purple
                            case 4: color = {255, 0, 0, 255}; break; //blue
                            case 5: color = {0, 0, 255, 255}; break; //orange
                            case 6: color = {0, 255, 0, 255}; break; //green
                            case 7: color = {255, 0, 0, 255}; break; //red
                            default: color = {127, 127, 127, 255}; break; //grey
                        }
                        // Check if this block is part of the current falling piece
                        bool isCurrentPieceBlock = false;
                        for (int sx = 0; sx < currentPiece.width; ++sx) {
                            for (int sy = 0; sy < currentPiece.height; ++sy) {
                                if (currentPiece.shape[sy][sx] != 0 &&
                                    x == currentPiece.x + sx &&
                                    y == currentPiece.y + sy) {
                                    isCurrentPieceBlock = true;
                                    break;
                                }
                            }
                            if (isCurrentPieceBlock) break;
                        }
                        // Only darken locked pieces
                        if (!isCurrentPieceBlock) {
                            color.r = static_cast<Uint8>(color.r * 0.7f);
                            color.g = static_cast<Uint8>(color.g * 0.7f);
                            color.b = static_cast<Uint8>(color.b * 0.7f);
                        }
                        SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, color.a);
                        SDL_RenderFillRect(gRenderer, &rect);
                    }
                }
            }
        }
    }
}

void renderBoardBlocksDuringAnimation() {
    // Draw the blocks
    for (int x = 0; x < boardWidth; ++x) {
        for (int y = 0; y < boardHeight; ++y) {
            int val = board.current[x][y];
            if (val != 0) {
                SDL_FRect rect{ static_cast<float>(x * blockSize) + spacing / 2,
                                static_cast<float>(y * blockSize) + spacing / 2,
                                blockSize - spacing,
                                blockSize - spacing };
                SDL_Color color;
                switch (val) {
                    case 1: color = {0, 255, 255, 255}; break; //cyan
                    case 2: color = {255, 255, 0, 255}; break; //yellow
                    case 3: color = {128, 0, 128, 255}; break; //purple
                    case 4: color = {255, 0, 0, 255}; break; //blue
                    case 5: color = {0, 0, 255, 255}; break; //orange
                    case 6: color = {0, 255, 0, 255}; break; //green
                    case 7: color = {255, 0, 0, 255}; break; //red
                    default: color = {127, 127, 127, 255}; break; //grey
                }
                
                color.r = static_cast<Uint8>(color.r * 0.7f);
                color.g = static_cast<Uint8>(color.g * 0.7f);
                color.b = static_cast<Uint8>(color.b * 0.7f);
                        
                SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(gRenderer, &rect);
                
            }
        }
    }
}

void renderPauseMenu() {
    //render a "Paused" message
    SDL_Color textColor{ 0xFF, 0xFF, 0xFF, 0xFF };
    gameOverLabel.loadFromRenderedText("PAUSED", textColor);
    gameOverLabel.render(200, 300);
    SDL_RenderPresent(gRenderer);

    //todo render board with hollow blocks

    capFrameRate();
}

void animateRowClear() {
    Uint64 now = SDL_GetTicksNS();
    int animFrame = ((now - clearAnimStart) * clearAnimSteps) / clearAnimDuration;
    if (animFrame > clearAnimStep) {
        clearAnimStep = animFrame;
        // For each row, clear blocks from center out
        for (int row : rowsToClear) {
            int center = boardWidth / 2;
            for (int offset = 0; offset <= clearAnimStep; ++offset) {
                int left = center - offset;
                int right = center + offset;
                if (left >= 0 && board.current[left][row] != 0) {
                    spawnParticlesAt(left, row, board.current[left][row]);
                    board.current[left][row] = 0;
                }
                if (right < boardWidth && right != left && board.current[right][row] != 0) {
                    spawnParticlesAt(right, row, board.current[right][row]);
                    board.current[right][row] = 0;
                }
            }
        }
    }

    renderUI();

    renderBoardBlocksDuringAnimation();

    renderParticles();

    SDL_RenderPresent(gRenderer);

    // Pause for animation duration
    if (now - clearAnimStart >= clearAnimDuration) {
        // Shift rows down
        for (int row : rowsToClear) {
            for (int y = row; y > 0; --y) {
                for (int x = 0; x < boardWidth; ++x) {
                    board.current[x][y] = board.current[x][y - 1];
                }
            }
            for (int x = 0; x < boardWidth; ++x) {
                board.current[x][0] = 0;
            }
        }
        clearingRows = false;
        rowsToClear.clear();
    }
    capFrameRate();
}

bool checkGameOver() {
    bool gameOver = false;
    for (int sx = 0; sx < currentPiece.width; ++sx) 
    {
        for (int sy = 0; sy < currentPiece.height; ++sy) 
        {
            if (currentPiece.shape[sy][sx] != 0) 
            {
                int boardX = currentPiece.x + sx;
                int boardY = currentPiece.y + sy;
                if (board.current[boardX][boardY] != 0) 
                {
                    gameOver = true;
                    break;
                }
            }
        }
        if (gameOver) break;
    }
    if (gameOver) {
        // Render "Game Over" message
        SDL_Color textColor{ 0xFF, 0x00, 0x00, 0xFF }; // Red text
        gameOverLabel.loadFromRenderedText( "GAME OVER", textColor );
        gameOverLabel.render( 200, 300 );
        SDL_RenderPresent( gRenderer );
        SDL_Delay(4000); // Pause for 3 seconds to show the message
        //quit = true; // Exit the main loop
        //restart the game
        // Reset game state instead of restarting main
        scoreValue = 0;
        levelValue = 0;
        rowsCleared = 0;
        levelIncrease = 0;
        dropSpeed = 900000000;
        holdPiece = Piece();
        holdUsed = false;
        for (int x = 0; x < boardWidth; ++x)
            for (int y = 0; y < boardHeight; ++y)
                board.current[x][y] = 0;
        pickPiece = std::rand() % 7;
        nextPickPiece = std::rand() % 7;
        currentPiece = pieceTypes[pickPiece];
        nextPiece = pieceTypes[nextPickPiece];
        currentPiece.x = boardWidth / 2;
        currentPiece.y = 0;
        score.loadFromRenderedText(std::to_string(scoreValue), { 0xFF, 0xFF, 0xFF, 0xFF });
        level.loadFromRenderedText(std::to_string(levelValue+1), { 0xFF, 0xFF, 0xFF, 0xFF });
        newPiece = false;
        
    }
    return gameOver;
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
bool hardDropFlag = false; // To track if hard drop was used
bool alternateIPieceRotationOffset = false; // To alternate I piece rotation offsets
bool paused = false; // To track if the game is paused

//piece state variables for lock delay
int lockDelayFrames = 30; // Number of frames to allow after landing (adjust as desired)
int lockDelayCounter = 0; // Counts frames since landing (reset on move/rotate)
bool pieceLanded = false; // True if just landed, false if still falling

Board board; // The game board

void moveLeft() {
    if (checkPlacement(currentPiece, board, -1, 0)){ //check if can move left
        pieceSet(currentPiece, board); //clear current position
        currentPiece.x -= 1; //move left
    }
}

void moveRight() {
    if (checkPlacement(currentPiece, board, 1, 0)){ //check if can move right
        pieceSet(currentPiece, board); //clear current position
        currentPiece.x += 1; //move right
    }
}

void rotateClockwise() {
    if (currentPiece.width == currentPiece.height) { // Dont perform rotation if O piece
        return;
    }
    else if (currentPiece.height == 1 || currentPiece.width == 1) {
        rotateIPieceClockwise();
    }
    else 
    {
        rotatePieceClockwise();
    }
}

void rotateCounterClockwise() {
    if (currentPiece.width == currentPiece.height) { // Dont perform rotation if O piece
        return;
    }
    else if (currentPiece.height == 1 || currentPiece.width == 1) {
        rotateIPieceCounterClockwise();
    }
    else 
    {
        rotatePieceCounterClockwise();
    }
}

void softDrop() {
    if (checkPlacement(currentPiece, board, 0, 1)){ //check if can move down
        pieceSet(currentPiece, board); //clear current position
        currentPiece.y += 1; //move down
    }
}

void hardDrop() {
    pieceSet(currentPiece, board); //clear current position
    currentPiece.y = maxDrop(currentPiece, board); //move down to max drop
    spawnParticles(currentPiece); //spawn particles at hard drop location
    hardDropFlag = true; //set hard drop flag
}

void hold() {
    pieceSet(currentPiece, board); //clear current position
    if (!holdUsed){ //if hold not used this turn
        resetRotation();
        if (holdPiece.shape.empty()) {
            firstHold(); //first time holding a piece
        } else {
            pieceSwap(); //swap current and hold pieces
        }
        holdUsed = true; // Mark hold as used for this turn
    }
}

void pauseGame() {
    paused = !paused;
}