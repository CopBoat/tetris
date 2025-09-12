#include "globals.h"
#include "ltexture.h"
#include "ltimer.h"
#include "board.h"
#include "piece.h"
#include "tetris_utils.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#include <math.h>

int main( int argc, char* args[] )
{
    //Final exit code
    int exitCode{ 0 };

    //Seed random number generator
    std::srand(static_cast<unsigned int>(time(0)));
    std::srand(static_cast<unsigned int>(std::time(0)));

    //Initialize
    if( init(chooseWindowTitle()) == false ) //initialize SDL Components
    {
        SDL_Log( "Unable to initialize program!\n" );
        exitCode = 1;
    }
    else if( loadMedia() == false ) //intialize font and font textures
    {
        SDL_Log( "Unable to load media!\n" );
        exitCode = 1;
    }
    else //if everything initialized fine
    {
        bool quit{ false }; //The quit flag

        //The event data
        SDL_Event e;
        SDL_zero( e );

        while( quit == false ) //The main loop
        {
            capTimer.start();

            InputAction action = InputAction::None;

            while( SDL_PollEvent( &e ) == true ) //While there are events to handle
            {
                if( e.type == SDL_EVENT_QUIT ) //If event is quit type
                {
                    quit = true; //End the main loop
                }

                if (e.type == SDL_EVENT_KEY_DOWN) //Handle keyboard inputs
                {
                    switch (e.key.key)
                    {
                        case SDLK_LEFT: action = InputAction::MoveLeft;  break;
                        case SDLK_RIGHT: action = InputAction::MoveRight; break;
                        case SDLK_UP: action = InputAction::RotateClockwise; break;
                        case SDLK_DOWN: action = InputAction::SoftDrop;  break;
                        case SDLK_H: action = InputAction::Hold;      break;
                        case SDLK_SPACE: action = InputAction::HardDrop;  break;
                        case SDLK_ESCAPE: action = InputAction::Pause; break;
                        default: break;
                    }
                }

                if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) //Handle gamepad inputs
                {
                    switch (e.gbutton.button) {
                        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:     action = InputAction::MoveLeft;               break;
                        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:    action = InputAction::MoveRight;              break;
                        case SDL_GAMEPAD_BUTTON_WEST:          action = InputAction::RotateClockwise;        break;
                        case SDL_GAMEPAD_BUTTON_EAST:          action = InputAction::RotateCounterClockwise; break;
                        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:     action = InputAction::SoftDrop;               break;
                        case SDL_GAMEPAD_BUTTON_SOUTH:         action = InputAction::HardDrop;               break;
                        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: action = InputAction::Hold;                   break;
                        case SDL_GAMEPAD_BUTTON_START:         action = InputAction::Pause;                  break;
                        default: break;
                    }
                }

                if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) //Handle gamepad axis motion
                {
                    if (e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
                        if (e.gaxis.value < -8000) {
                            action = InputAction::MoveLeft;
                        } else if (e.gaxis.value > 8000) {
                            action = InputAction::MoveRight;
                        }
                    } else if (e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
                        if (e.gaxis.value > 8000) {
                            action = InputAction::SoftDrop;
                        }
                    }
                }
            }

            switch (action) { //handle input actions
                case InputAction::MoveLeft: { moveLeft(); break; }
                case InputAction::MoveRight: { moveRight(); break; }
                case InputAction::RotateClockwise: { rotateClockwise(); break; }
                case InputAction::RotateCounterClockwise: { rotateCounterClockwise(); break; }
                case InputAction::SoftDrop: { softDrop(); break; }
                case InputAction::HardDrop: { hardDrop(); break; }
                case InputAction::Hold: { hold(); break; }
                case InputAction::Pause: { pauseGame(); break; }
                default: break;
            }

            //only allow rotation and x movement to reset lockDelayCounter
            if (pieceLanded && (action == InputAction::MoveLeft || action == InputAction::MoveRight || action == InputAction::RotateClockwise || action == InputAction::RotateCounterClockwise)) {
                lockDelayCounter = 0;
            }

            renderUI();

            if (paused) {
                //render a "Paused" message
                SDL_Color textColor{ 0xFF, 0xFF, 0xFF, 0xFF };
                gameOverLabel.loadFromRenderedText("PAUSED", textColor);
                gameOverLabel.render(200, 300);
                SDL_RenderPresent(gRenderer);

                //todo render board with hollow blocks

                capFrameRate();
                continue; // Skip the rest of the loop
            }

            // ...in your main loop, before normal rendering...
            if (clearingRows) {
            
            
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
                // Resume normal game logic
            }
            capFrameRate();
            continue; // Skip rest of loop while animating
        }

            //check game over
            if (currentPiece.y == 0) {
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
                    continue; // Skip rest of loop and start new game
                }
            }

            // Check if the piece can be placed at its next position
            bool canPlaceNext = checkPlacement(currentPiece, board, 0, 1);

            // Place the current piece's shape onto the board at its current position
            for (int sx = 0; sx < currentPiece.width; ++sx) {
                for (int sy = 0; sy < currentPiece.height; ++sy) {
                    if (currentPiece.shape[sy][sx] != 0) {
                        int boardX = currentPiece.x + sx;
                        int boardY = currentPiece.y + sy;
                        if (boardX >= 0 && boardX < boardWidth && boardY >= 0 && boardY < boardHeight) {
                            board.current[boardX][boardY] = currentPiece.color;
                        }
                    }
                }
            }
             
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

            renderParticles();

            SDL_RenderPresent( gRenderer ); //update screen

            if (!newPiece) {
                for (int sx = 0; sx < currentPiece.width; ++sx) {
                    for (int sy = 0; sy < currentPiece.height; ++sy) {
                        if (currentPiece.shape[sy][sx] != 0) {
                            int boardX = currentPiece.x + sx;
                            int boardY = currentPiece.y + sy;
                            board.current[boardX][boardY] = 0;
                        }
                    }
                }
            }
            
            // Handle automatic piece dropping based on drop speed
            Uint64 now = SDL_GetTicksNS();
            if (now - lastDropTime >= dropSpeed && canPlaceNext) {
                currentPiece.y += 1;
                lastDropTime = now;
            }


            if (!canPlaceNext && !hardDropFlag)
            {
                if (!pieceLanded) {
                    pieceLanded = true;
                    lockDelayCounter = 0;
                } else {
                    lockDelayCounter++;
                    if (lockDelayCounter >= lockDelayFrames) {
                        newPiece = true;
                        pieceLanded = false;
                        lockDelayCounter = 0;
                    }
                }
            } else {
                pieceLanded = false;
                lockDelayCounter = 0;
            }

            //use  if (newPiece) to test rotation after hard drop
            if (newPiece || hardDropFlag) {
                    
                for (int sx = 0; sx < currentPiece.width; ++sx) {
                    for (int sy = 0; sy < currentPiece.height; ++sy) {
                        if (currentPiece.shape[sy][sx] != 0) {
                            int boardX = currentPiece.x + sx;
                            int boardY = currentPiece.y + sy;
                            board.current[boardX][boardY] = currentPiece.color;
                        }
                    }
                }

                int fullRows[boardHeight];

                for (int i = 0; i < boardHeight; ++i) {
                    fullRows[i] = 0;
                }

                for (int y = 0; y < boardHeight; ++y) {
                    int rowcount = 0;
                    for (int x = 0; x < boardWidth; ++x) {
                        if (board.current[x][y] != 0) {
                            rowcount++;
                        }
                    }
                    if (rowcount == boardWidth) {
                        fullRows[y] = 1;
                    }
                }

                int clearedRows = 0;

                for (int i = 0; i < boardHeight; ++i) 
                {
                    if (fullRows[i] == 1) {
                        clearedRows++;
                    }
                }

                if (!clearingRows && clearedRows > 0) {
                    clearingRows = true;
                    clearAnimStart = SDL_GetTicksNS();
                    clearAnimStep = 0;
                    rowsToClear.clear();
                    for (int i = 0; i < boardHeight; ++i) {
                        if (fullRows[i] == 1) rowsToClear.push_back(i);
                    }
                    // Play a sound here?
                }

                switch (clearedRows)
                    {
                    case 1:
                        scoreValue += 40*(levelValue+1); // Bonus for clearing one row
                        break;
                    case 2:
                        scoreValue += 100*(levelValue+1); // Bonus for clearing two rows
                        break;
                    case 3:
                        scoreValue += 300*(levelValue+1); // Bonus for clearing three rows
                        break;
                    case 4:
                        scoreValue += 1200*(levelValue+1); // Bonus for clearing four rows (Tetris)
                    default:
                        break;
                    }

                    score.loadFromRenderedText( std::to_string(scoreValue), { 0xFF, 0xFF, 0xFF, 0xFF } );
                    if (scoreValue > highScoreValue) {
                        highScoreValue = scoreValue;
                        highScore.loadFromRenderedText( std::to_string(highScoreValue), { 0xFF, 0xFF, 0xFF, 0xFF } );
                    }

                    rowsCleared += clearedRows;

                    
                    levelValue = rowsCleared / 10;
                    level.loadFromRenderedText( std::to_string(levelValue+1), { 0xFF, 0xFF, 0xFF, 0xFF } );

                    if (levelValue > levelIncrease) {
                        if (levelValue < 5) {
                            dropSpeed -= 200000000; // Increase speed- may cause crash after level 4
                        } else if (levelValue == 5) {
                            dropSpeed -= 20000000; // Increase speed- may cause crash after level 4
                            level.loadFromRenderedText( "MAX", { 0xFF, 0xFF, 0xFF, 0xFF } );
                        }
                        levelIncrease = levelValue;
                    }

                currentPiece.y = 0; // Reset for next falling piece
                currentPiece.x = boardWidth / 2; // Reset horizontal position to center
                currentPiece = pieceTypes[nextPickPiece]; // Select a new random piece
                nextPickPiece = std::rand() % 7; // Randomly select the next piece
                nextPiece = pieceTypes[nextPickPiece]; // Update next piece
                newPiece = false;
                hardDropFlag = false;
                holdUsed = false; // Reset hold usage for the new piece
            }
            capFrameRate();
        } 
    }
    close(); //Clean up
    return exitCode; //End program
}