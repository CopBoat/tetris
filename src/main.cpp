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

            if (paused) { renderPauseMenu(); continue; } // Skip the rest of the loop while paused

            if (clearingRows) { animateRowClear(); continue; } // Skip rest of loop while animating

            //check game over
            if (currentPiece.y == 0) { if (checkGameOver()) { continue; } } // Skip rest of loop and start new game

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
             
            renderBoardBlocks();

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