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

    // Controller repeat config (ms)
    constexpr Uint64 kDAS_MS          = 167; // delay before auto-repeat
    constexpr Uint64 kARR_MS          = 33;  // auto-repeat rate (horizontal)
    constexpr Uint64 kSoftDrop_ARR_MS = 33;  // auto-repeat rate (soft drop)

    struct RepeatState {
        bool   held{false};
        Uint64 pressedAt{0};
        Uint64 lastRepeatAt{0};
    };
    enum class HDir { None, Left, Right };

    RepeatState gpLeft{}, gpRight{}, gpDown{};
    HDir activeH{ HDir::None };

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
                if( e.type == SDL_EVENT_QUIT ) { quit = true; }

                if (currentState == GameState::MENU) {
                    // input only; no rendering here
                    if (e.type == SDL_EVENT_KEY_DOWN) {
                        if (e.key.key == SDLK_UP) {
                            menuSelection = (menuSelection - 1 + 2) % 2;
                        } else if (e.key.key == SDLK_DOWN) {
                            menuSelection = (menuSelection + 1) % 2;
                        } else if (e.key.key == SDLK_RETURN || e.key.key == SDLK_KP_ENTER) {
                            if (menuSelection == 0) { currentState = GameState::PLAYING; renderWipeIntro(gRenderer, kScreenWidth, kScreenHeight); continue; }
                            else if (menuSelection == 1) currentState = GameState::OPTIONS;
                        }
                    }
                    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                        if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP) {
                            menuSelection = (menuSelection - 1 + 2) % 2;
                        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
                            menuSelection = (menuSelection + 1) % 2;
                        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
                            if (menuSelection == 0) { currentState = GameState::PLAYING; renderWipeIntro(gRenderer, kScreenWidth, kScreenHeight); continue;}
                            else if (menuSelection == 1) currentState = GameState::OPTIONS;
                        }
                    }
                } else if (currentState == GameState::OPTIONS) {
                    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE)
                        currentState = GameState::MENU;
                } else if (currentState == GameState::PUASE) {
                    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                        if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_LEFT) {
                            pauseMenuSelection = (pauseMenuSelection - 1 + 2) % 2;
                        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
                            pauseMenuSelection = (pauseMenuSelection + 1) % 2;
                        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
                            if (pauseMenuSelection == 0) { currentState = GameState::PLAYING; }
                            else if (pauseMenuSelection == 1) { currentState = GameState::MENU; quitToMenu(); }
                        }
                    }
                }

                // Only collect game input while playing
                if (currentState == GameState::PLAYING) {
                    playing = true;
                    if (e.type == SDL_EVENT_KEY_DOWN) {
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
                    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                        switch (e.gbutton.button) {
                            case SDL_GAMEPAD_BUTTON_DPAD_LEFT: {
                                action = InputAction::MoveLeft;
                                gpLeft.held = true;
                                gpLeft.pressedAt = gpLeft.lastRepeatAt = SDL_GetTicks();
                                activeH = HDir::Left;
                            } break;
                            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: {
                                action = InputAction::MoveRight;
                                gpRight.held = true;
                                gpRight.pressedAt = gpRight.lastRepeatAt = SDL_GetTicks();
                                activeH = HDir::Right;
                            } break;
                            case SDL_GAMEPAD_BUTTON_WEST:          action = InputAction::RotateClockwise;        break;
                            case SDL_GAMEPAD_BUTTON_EAST:          action = InputAction::RotateCounterClockwise; break;
                            case SDL_GAMEPAD_BUTTON_DPAD_DOWN: {
                                action = InputAction::SoftDrop;
                                gpDown.held = true;
                                gpDown.pressedAt = gpDown.lastRepeatAt = SDL_GetTicks();
                            } break;
                            case SDL_GAMEPAD_BUTTON_SOUTH:         action = InputAction::HardDrop;               break;
                            case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: action = InputAction::Hold;                   break;
                            case SDL_GAMEPAD_BUTTON_START:         action = InputAction::Pause;                  break;
                            default: break;
                        }
                    }
                    // Handle button releases to stop repeating and resolve LR conflict
                    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
                        switch (e.gbutton.button) {
                            case SDL_GAMEPAD_BUTTON_DPAD_LEFT: {
                                gpLeft.held = false;
                                if (activeH == HDir::Left) {
                                    if (gpRight.held) {
                                        // switch to right: restart DAS timing
                                        activeH = HDir::Right;
                                        gpRight.pressedAt = gpRight.lastRepeatAt = SDL_GetTicks();
                                    } else {
                                        activeH = HDir::None;
                                    }
                                }
                            } break;
                            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: {
                                gpRight.held = false;
                                if (activeH == HDir::Right) {
                                    if (gpLeft.held) {
                                        activeH = HDir::Left;
                                        gpLeft.pressedAt = gpLeft.lastRepeatAt = SDL_GetTicks();
                                    } else {
                                        activeH = HDir::None;
                                    }
                                }
                            } break;
                            case SDL_GAMEPAD_BUTTON_DPAD_DOWN: {
                                gpDown.held = false;
                            } break;
                            default: break;
                        }
                    }
                    if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
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
                } else {
                    playing = false;
                    // Clear held states when leaving PLAYING
                    gpLeft = {}; gpRight = {}; gpDown = {}; activeH = HDir::None;
                }
            }

            // After processing all events, render exactly once based on state
            if (currentState == GameState::MENU) {
                renderMenu();
                SDL_RenderPresent(gRenderer);
                capFrameRate();
                continue;
            } else if (currentState == GameState::OPTIONS) {
                renderOptions();
                SDL_RenderPresent(gRenderer);
                capFrameRate();
                continue;
            } else if (currentState == GameState::PUASE) {
                //renderUI();
                renderPauseMenu();
                 // draw the game scene behind the pause menu
                //renderBoardBlocks(); // draw board on top of UI
                
                SDL_RenderPresent(gRenderer);
                capFrameRate();
                continue;
            }

            if (!playing) continue; // Skip the rest of the loop if not playing

            // One-shot action from this frame's events
            switch (action) { //handle input actions
                case InputAction::MoveLeft: { moveLeft(); break; }
                case InputAction::MoveRight: { moveRight(); break; }
                case InputAction::RotateClockwise: { rotateClockwise(); break; }
                case InputAction::RotateCounterClockwise: { rotateCounterClockwise(); break; }
                case InputAction::SoftDrop: { softDrop(); break; }
                case InputAction::HardDrop: { hardDrop(); break; }
                case InputAction::Hold: { hold(); break; }
                case InputAction::Pause: { currentState = GameState::PUASE; break; }
                default: break;
            }

            // Inject auto-repeat moves for held D-pad buttons (DAS/ARR)
            bool repeatedHorizontalThisFrame = false;
            const Uint64 now = SDL_GetTicks();

            // Horizontal (last-direction-wins)
            if (activeH == HDir::Left && gpLeft.held) {
                if (now - gpLeft.pressedAt >= kDAS_MS && now - gpLeft.lastRepeatAt >= kARR_MS) {
                    moveLeft();
                    gpLeft.lastRepeatAt = now;
                    repeatedHorizontalThisFrame = true;
                }
            } else if (activeH == HDir::Right && gpRight.held) {
                if (now - gpRight.pressedAt >= kDAS_MS && now - gpRight.lastRepeatAt >= kARR_MS) {
                    moveRight();
                    gpRight.lastRepeatAt = now;
                    repeatedHorizontalThisFrame = true;
                }
            }

            // Soft drop repeat
            if (gpDown.held) {
                if (now - gpDown.pressedAt >= kDAS_MS && now - gpDown.lastRepeatAt >= kSoftDrop_ARR_MS) {
                    softDrop();
                    gpDown.lastRepeatAt = now;
                }
            }

            //only allow rotation and x movement to reset lockDelayCounter
            if (pieceLanded && (action == InputAction::MoveLeft || action == InputAction::MoveRight || action == InputAction::RotateClockwise || action == InputAction::RotateCounterClockwise
                                || repeatedHorizontalThisFrame)) {
                lockDelayCounter = 0;
            }

            renderUI();

            // if (paused) // todo add paused as a game state
            // { 
            //     currentState = GameState::PUASE;
            //     continue; 
            // } // Skip the rest of the loop while paused

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