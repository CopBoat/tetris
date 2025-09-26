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
                if( e.type == SDL_EVENT_QUIT ) { quit = true; }

                // Double-click anywhere in the client area to toggle fullscreen
                if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                    e.button.button == SDL_BUTTON_LEFT &&
                    e.button.clicks == 2) {
                    toggleFullscreen();
                    continue;
                }

                if (currentState == GameState::MENU) {
                    
                    switch (handleMenuEvent(e)) {
                        case 0: // Start Game
                            currentState = GameState::PLAYING;
                            renderWipeIntro(gRenderer, kScreenWidth, kScreenHeight);
                            break;
                        case 1: // Options menu
                            currentState = GameState::OPTIONS;
                            break;
                        case 2: // Exit program
                            quit = true;
                            break;
                        default:
                            break;
                    }

                } else if (currentState == GameState::OPTIONS) {
                    // if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE)
                    //     currentState = GameState::MENU;

                    switch (optionsTab) {
                        case 0: { // Game // turns out i prob dont need a swtich here
                                    switch (handleGameOptionsMenuEvent(e)) {
                                        case 0: // Game
                                            // move menus
                                            break;
                                        case 1: 
                                            break;
                                        case 2: 
                                            break;
                                        case 3: 
                                            break;
                                        case 4: // Back
                                            currentState = GameState::MENU;
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            break;
                        case 1: // Video
                            break;
                        case 2: // Input
                            break;
                        default:
                            break;
                    }

                    





                        
                } else if (currentState == GameState::PUASE) {
                    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                        if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_LEFT) {
                            pauseMenuSelection = (pauseMenuSelection - 1 + 2) % 2;
                        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
                            pauseMenuSelection = (pauseMenuSelection + 1) % 2;
                        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
                            if (pauseMenuSelection == 0) { currentState = GameState::PLAYING; continue; }
                            else if (pauseMenuSelection == 1) { currentState = GameState::MENU; quitToMenu(); }
                        }
                    }
                    if (e.type == SDL_EVENT_KEY_DOWN) {
                        if (e.key.key == SDLK_LEFT) {
                            pauseMenuSelection = (pauseMenuSelection - 1 + 2) % 2;
                        } else if (e.key.key == SDLK_RIGHT) {
                            pauseMenuSelection = (pauseMenuSelection + 1) % 2;
                        } else if (e.key.key == SDLK_RETURN || e.key.key == SDLK_KP_ENTER) {
                            if (pauseMenuSelection == 0) { currentState = GameState::PLAYING; continue; }
                            else if (pauseMenuSelection == 1) { currentState = GameState::MENU; quitToMenu(); }
                        } else if (e.key.key == SDLK_ESCAPE) {
                            currentState = GameState::PLAYING;
                        }
                    }
                    // Analog stick left/right for pause menu
                    if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
                        const int v = e.gaxis.value;
                        if (v <= -kAxisPress) {
                            if (!pauseAxisLeftHeld) {
                                pauseMenuSelection = (pauseMenuSelection - 1 + 2) % 2;
                                pauseAxisLeftHeld = true;
                                pauseAxisRightHeld = false;
                            }
                        } else if (v >= kAxisPress) {
                            if (!pauseAxisRightHeld) {
                                pauseMenuSelection = (pauseMenuSelection + 1) % 2;
                                pauseAxisRightHeld = true;
                                pauseAxisLeftHeld = false;
                            }
                        } else if (std::abs(v) < kAxisRelease) {
                            pauseAxisLeftHeld = false;
                            pauseAxisRightHeld = false;
                        }
                    }


                }

                // Only collect game input while playing
                if (currentState == GameState::PLAYING) {
                    playing = true;
                    if (e.type == SDL_EVENT_KEY_DOWN) {
                        // Ignore OS key repeat; we implement DAS/ARR ourselves
                        if (e.key.repeat) {
                            // no-op
                        } else {
                            switch (e.key.key)
                            {
                                case SDLK_LEFT: {
                                    action = InputAction::MoveLeft;
                                    kbLeftHeld = true;
                                    gpLeft.held = true;
                                    gpLeft.pressedAt = gpLeft.lastRepeatAt = SDL_GetTicks();
                                    activeH = HDir::Left;
                                } break;
                                case SDLK_RIGHT: {
                                    action = InputAction::MoveRight;
                                    kbRightHeld = true;
                                    gpRight.held = true;
                                    gpRight.pressedAt = gpRight.lastRepeatAt = SDL_GetTicks();
                                    activeH = HDir::Right;
                                } break;
                                case SDLK_UP: action = InputAction::RotateClockwise; break;
                                case SDLK_DOWN: {
                                    action = InputAction::SoftDrop;
                                    kbDownHeld = true;
                                    gpDown.held = true;
                                    gpDown.pressedAt = gpDown.lastRepeatAt = SDL_GetTicks();
                                } break;
                                case SDLK_H: action = InputAction::Hold; break;
                                case SDLK_SPACE: action = InputAction::HardDrop; break;
                                case SDLK_ESCAPE: action = InputAction::Pause; break;
                                default: break;
                            }
                        }
                    }
                    if (e.type == SDL_EVENT_KEY_UP) {
                        switch (e.key.key) {
                            case SDLK_LEFT: {
                                kbLeftHeld = false;
                                if (!gpLeftHeld) {
                                    gpLeft.held = false;
                                    if (activeH == HDir::Left) {
                                        if (kbRightHeld || gpRightHeld) {
                                            activeH = HDir::Right;
                                            gpRight.held = true;
                                            gpRight.pressedAt = gpRight.lastRepeatAt = SDL_GetTicks();
                                        } else {
                                            activeH = HDir::None;
                                        }
                                    }
                                }
                            } break;
                            case SDLK_RIGHT: {
                                kbRightHeld = false;
                                if (!gpRightHeld) {
                                    gpRight.held = false;
                                    if (activeH == HDir::Right) {
                                        if (kbLeftHeld || gpLeftHeld) {
                                            activeH = HDir::Left;
                                            gpLeft.held = true;
                                            gpLeft.pressedAt = gpLeft.lastRepeatAt = SDL_GetTicks();
                                        } else {
                                            activeH = HDir::None;
                                        }
                                    }
                                }
                            } break;
                            case SDLK_DOWN: {
                                kbDownHeld = false;
                                if (!gpDownHeld) {
                                    gpDown.held = false;
                                }
                            } break;
                            default: break;
                        }
                    }
                    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                        switch (e.gbutton.button) {
                            case SDL_GAMEPAD_BUTTON_DPAD_LEFT: {
                                action = InputAction::MoveLeft;
                                gpDpadLeftHeld = true;
                                recomputeGamepadHeld();
                                gpLeft.held = true;
                                gpLeft.pressedAt = gpLeft.lastRepeatAt = SDL_GetTicks();
                                activeH = HDir::Left;
                            } break;
                            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: {
                                action = InputAction::MoveRight;
                                gpDpadRightHeld = true;
                                recomputeGamepadHeld();
                                gpRight.held = true;
                                gpRight.pressedAt = gpRight.lastRepeatAt = SDL_GetTicks();
                                activeH = HDir::Right;
                            } break;
                            case SDL_GAMEPAD_BUTTON_WEST:          action = InputAction::RotateClockwise;        break;
                            case SDL_GAMEPAD_BUTTON_EAST:          action = InputAction::RotateCounterClockwise; break;
                            case SDL_GAMEPAD_BUTTON_DPAD_DOWN: {
                                action = InputAction::SoftDrop;
                                gpDpadDownHeld = true;
                                recomputeGamepadHeld();
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
                                gpDpadLeftHeld = false;
                                recomputeGamepadHeld();
                                if (!kbLeftHeld && !gpLeftHeld) {
                                    gpLeft.held = false;
                                    if (activeH == HDir::Left) {
                                        if (kbRightHeld || gpRightHeld) {
                                            activeH = HDir::Right;
                                            gpRight.held = true;
                                            gpRight.pressedAt = gpRight.lastRepeatAt = SDL_GetTicks();
                                        } else {
                                            activeH = HDir::None;
                                        }
                                    }
                                }
                            } break;
                            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: {
                                gpDpadRightHeld = false;
                                recomputeGamepadHeld();
                                if (!kbRightHeld && !gpRightHeld) {
                                    gpRight.held = false;
                                    if (activeH == HDir::Right) {
                                        if (kbLeftHeld || gpLeftHeld) {
                                            activeH = HDir::Left;
                                            gpLeft.held = true;
                                            gpLeft.pressedAt = gpLeft.lastRepeatAt = SDL_GetTicks();
                                        } else {
                                            activeH = HDir::None;
                                        }
                                    }
                                }
                            } break;
                            case SDL_GAMEPAD_BUTTON_DPAD_DOWN: {
                                gpDpadDownHeld = false;
                                recomputeGamepadHeld();
                                if (!kbDownHeld && !gpDownHeld) {
                                    gpDown.held = false;
                                }
                            } break;
                            default: break;
                        }
                    }
                    if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
                        const int v = e.gaxis.value;
                        if (e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
                            const Uint64 now = SDL_GetTicks();

                            // Switch or press Left
                            if (v <= -kAxisPress) {
                                if (!gpAxisLeftHeld) {
                                    // If we were holding Right via axis, release it
                                    if (gpAxisRightHeld) {
                                        gpAxisRightHeld = false;
                                        recomputeGamepadHeld();
                                        if (!kbRightHeld && !gpRightHeld) gpRight.held = false;
                                    }
                                    gpAxisLeftHeld = true;
                                    recomputeGamepadHeld();

                                    action = InputAction::MoveLeft;
                                    gpLeft.held = true;
                                    gpLeft.pressedAt = gpLeft.lastRepeatAt = now;
                                    activeH = HDir::Left;
                                }
                            }
                            // Switch or press Right
                            else if (v >= kAxisPress) {
                                if (!gpAxisRightHeld) {
                                    if (gpAxisLeftHeld) {
                                        gpAxisLeftHeld = false;
                                        recomputeGamepadHeld();
                                        if (!kbLeftHeld && !gpLeftHeld) gpLeft.held = false;
                                    }
                                    gpAxisRightHeld = true;
                                    recomputeGamepadHeld();

                                    action = InputAction::MoveRight;
                                    gpRight.held = true;
                                    gpRight.pressedAt = gpRight.lastRepeatAt = now;
                                    activeH = HDir::Right;
                                }
                            }
                            // Release to neutral (hysteresis)
                            else if (std::abs(v) < kAxisRelease) {
                                bool releasedLeft = false, releasedRight = false;
                                if (gpAxisLeftHeld)  { gpAxisLeftHeld  = false; releasedLeft  = true; }
                                if (gpAxisRightHeld) { gpAxisRightHeld = false; releasedRight = true; }
                                if (releasedLeft || releasedRight) {
                                    recomputeGamepadHeld();
                                    if (releasedLeft && !kbLeftHeld && !gpLeftHeld) {
                                        gpLeft.held = false;
                                        if (activeH == HDir::Left) {
                                            if (kbRightHeld || gpRightHeld) {
                                                activeH = HDir::Right;
                                                gpRight.held = true;
                                                gpRight.pressedAt = gpRight.lastRepeatAt = now;
                                            } else {
                                                activeH = HDir::None;
                                            }
                                        }
                                    }
                                    if (releasedRight && !kbRightHeld && !gpRightHeld) {
                                        gpRight.held = false;
                                        if (activeH == HDir::Right) {
                                            if (kbLeftHeld || gpLeftHeld) {
                                                activeH = HDir::Left;
                                                gpLeft.held = true;
                                                gpLeft.pressedAt = gpLeft.lastRepeatAt = now;
                                            } else {
                                                activeH = HDir::None;
                                            }
                                        }
                                    }
                                }
                            }
                        } else if (e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
                            const Uint64 now = SDL_GetTicks();
                            // Soft drop on down only
                            if (v >= kAxisPress) {
                                if (!gpAxisDownHeld) {
                                    gpAxisDownHeld = true;
                                    recomputeGamepadHeld();

                                    action = InputAction::SoftDrop;
                                    gpDown.held = true;
                                    gpDown.pressedAt = gpDown.lastRepeatAt = now;
                                }
                            } else if (v < kAxisRelease) {
                                if (gpAxisDownHeld) {
                                    gpAxisDownHeld = false;
                                    recomputeGamepadHeld();
                                    if (!kbDownHeld && !gpDownHeld) {
                                        gpDown.held = false;
                                    }
                                }
                            }
                        }
                    }
                } else {
                    playing = false;
                    // Clear held states when leaving PLAYING
                    gpLeft = {}; gpRight = {}; gpDown = {}; activeH = HDir::None;
                    kbLeftHeld = kbRightHeld = kbDownHeld = false;
                    gpDpadLeftHeld = gpDpadRightHeld = gpDpadDownHeld = false;
                    gpAxisLeftHeld = gpAxisRightHeld = gpAxisDownHeld = false;
                    recomputeGamepadHeld();
                }
            }

            // After processing all events, render exactly once based on state
            if (currentState == GameState::MENU) {
                renderMenu();
                SDL_RenderPresent(gRenderer);
                capFrameRate();
                continue;
            } else if (currentState == GameState::OPTIONS) {
                switch (optionsTab) {
                    case 0: renderGameOptions(); break;
                    case 1: renderVideoOptions(); break;
                    case 2: renderInputOptions(); break;
                    default: renderGameOptions(); break;
                }
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