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

    bool playing = false;

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

                if (currentState == GameState::MENU) {
                    //handleMenuEvent(e, quit, currentState);
                    if (e.type == SDL_EVENT_KEY_DOWN) {
                        // if (e.key.key == SDLK_1) currentState = GameState::PLAYING;
                        // if (e.key.key == SDLK_2) currentState = GameState::OPTIONS;

                        if (e.key.key == SDLK_UP) {
                            menuSelection = (menuSelection - 1 + 2) % 2; // Wrap around for 2 options
                        } else if (e.key.key == SDLK_DOWN) {
                            menuSelection = (menuSelection + 1) % 2; // Wrap around for 2 options
                        } else if (e.key.key == SDLK_RETURN || e.key.key == SDLK_KP_ENTER) {
                            if (menuSelection == 0) { currentState = GameState::PLAYING; renderWipeIntro(gRenderer, kScreenWidth, kScreenHeight); }
                            else if (menuSelection == 1) currentState = GameState::OPTIONS;
                        }
                    }

                    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                        if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP) {
                            menuSelection = (menuSelection - 1 + 2) % 2; // Wrap around for 2 options
                        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
                            menuSelection = (menuSelection + 1) % 2; // Wrap around for 2 options
                        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
                            if (menuSelection == 0) { currentState = GameState::PLAYING; renderWipeIntro(gRenderer, kScreenWidth, kScreenHeight); continue; }
                            else if (menuSelection == 1) currentState = GameState::OPTIONS;
                        }
                    }
                    //continue; // Skip the rest of the loop while in menu
                } else if (currentState == GameState::OPTIONS) {
                    //handleOptionsEvent(e, currentState);
                    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE)
                        currentState = GameState::MENU;
                    //continue; // Skip the rest of the loop while in options
                }

                // Render based on state
                if (currentState == GameState::MENU) { renderMenu(); }
                else if (currentState == GameState::OPTIONS) { renderOptions(); }
                else 
                {
                    playing = true;
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
                
               
            }

            if (!playing) continue; // Skip the rest of the loop if not playing

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

            if (paused) // todo add paused as a game state
            { 
                if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                    if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP) {
                        pauseMenuSelection = (pauseMenuSelection - 1 + 2) % 2; // Wrap around for 2 options
                    } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
                        pauseMenuSelection = (pauseMenuSelection + 1) % 2; // Wrap around for 2 options
                    } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
                        if (pauseMenuSelection == 0) { currentState == GameState::MENU; } // Resume
                        //else if (pauseMenuSelection == 1) { currentState = GameState::MENU; playing = false; pauseGame(); renderWipeIntro(gRenderer, kScreenWidth, kScreenHeight); } // Quit to menu
                    }
                }
                renderPauseMenu(); 
                continue; 
            } // Skip the rest of the loop while paused

            if (clearingRows) { animateRowClear(); continue; } // Skip rest of loop while animating

            //check game over
            if (currentPiece.y == 0) { if (checkGameOver()) { continue; } } // Skip rest of loop and start new game

            // Check if the piece can be placed at its next position
            bool canPlaceNext = checkPlacement(currentPiece, board, 0, 1);

            // Place the current piece's shape onto the board at its current position
            pieceSet(currentPiece, board, currentPiece.color);
             
            renderBoardBlocks();

            renderParticles();

            SDL_RenderPresent( gRenderer ); //update screen

            if (!newPiece) { pieceSet(currentPiece, board); } // Clear the piece's current position on the board

            autoDrop(canPlaceNext); // Handle automatic piece dropping based on drop speed

            handleLockDelay(canPlaceNext); // Handle lock delay if the piece has landed

            handlePieceLanded(); // Handle piece landing and row clearing
            
            capFrameRate();
        } 
    }
    close(); //Clean up
    return exitCode; //End program
}