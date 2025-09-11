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
        
        //The quit flag
        bool quit{ false };

        //The event data
        SDL_Event e;
        SDL_zero( e );
        
        //frames per second timer
        LTimer capTimer;

        Uint64 lastDropTime = SDL_GetTicksNS(); //
        
        Uint64 dropSpeed{ 700000000 }; // Milliseconds between drops

        // Array of piece types
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

        //The main loop
        while( quit == false )
        {

            capTimer.start();

            InputAction action = InputAction::None;

            while( SDL_PollEvent( &e ) == true )
                {
                    if( e.type == SDL_EVENT_QUIT ) //If event is quit type
                    {
                        //End the main loop
                        quit = true;
                    }

                    if (e.type == SDL_EVENT_KEY_DOWN)//Handle keyboard inputs
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

                    if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION)
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

            //rotate helper (can I move this into the case?)
            std::vector<std::vector<int>> newShape(currentPiece.width, std::vector<int>(currentPiece.height, 0));

            switch (action) {
                case InputAction::MoveLeft:
                {
                    if (checkPlacement(currentPiece, board, -1, 0)){ //check if can move left
                        pieceSet(currentPiece, board); //clear current position
                        currentPiece.x -= 1; //move left
                    }
                    break;
                }
                case InputAction::MoveRight:
                {
                    if (checkPlacement(currentPiece, board, 1, 0)){ //check if can move right
                        pieceSet(currentPiece, board); //clear current position
                        currentPiece.x += 1; //move right
                    }
                    break;
                }
                case InputAction::RotateClockwise:
                {
                    // Dont perform rotation if O piece
                    if (currentPiece.width == currentPiece.height) {
                        break;
                    }
                    else if (currentPiece.height == 1 || currentPiece.width == 1) {
                        // I piece rotation special case
                        // Rotate 90 degrees clockwise
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
                                //currentPiece.x = currentPiece.x + 2 + offset.first;
                                alternateIPieceRotationOffset = false;
                            }
                            else {
                                rotatedPiece.x += 1;
                                //currentPiece.x = currentPiece.x + 1 + offset.first;
                                alternateIPieceRotationOffset = true;
                            }
                        } 
                        else {
                            if (alternateIPieceRotationOffset) {
                                rotatedPiece.x -= 1;
                                //currentPiece.x = currentPiece.x - 1 + offset.first;
                            }
                            else {
                                rotatedPiece.x -= 2;
                                //currentPiece.x = currentPiece.x - 2 + offset.first;
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
                    else 
                    {
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
                    

                    break;
                }
                case InputAction::RotateCounterClockwise:
                {
                    // Dont perform rotation if O piece
                    if (currentPiece.width == currentPiece.height) {
                        break;
                    }
                    else if (currentPiece.height == 1 || currentPiece.width == 1) {
                        // I piece rotation special case
                        // Rotate 90 degrees clockwise
                        for (int sx = 0; sx < currentPiece.width; ++sx) {
                            for (int sy = 0; sy < currentPiece.height; ++sy) {
                                newShape[currentPiece.width - 1 - sx][sy] = currentPiece.shape[sy][sx];
                            }
                        }

                        Piece rotatedPiece = currentPiece;
                        rotatedPiece.shape = newShape;
                        rotatedPiece.width = currentPiece.height;
                        rotatedPiece.height = currentPiece.width;

                        // Check if rotated piece fits at (newX, newY)
                        if (checkPlacement(rotatedPiece, board, 0, 0)) {
                            // Apply rotation
                            currentPiece.shape = newShape;
                            std::swap(currentPiece.width, currentPiece.height);
                            currentPiece.rotation = (currentPiece.rotation + 3) % 4; // Equivalent to -1 mod 4
                            if (currentPiece.rotation % 4 == 1 || currentPiece.rotation % 4 == 3) {
                                // Shift right for vertical I piece
                                if (alternateIPieceRotationOffset) {
                                    currentPiece.x += 2;
                                    alternateIPieceRotationOffset = false;
                                }
                                else {
                                    currentPiece.x += 1;
                                    alternateIPieceRotationOffset = true;
                                }
                            } 
                            else {
                                if (alternateIPieceRotationOffset) {
                                    currentPiece.x -= 1;
                                }
                                else {
                                    currentPiece.x -= 2;
                                }
                            }
                            
                            break;
                        }

                        
                    }
                    else 
                    {
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

                        int idk;
                        switch(rotatedPiece.rotation) {
                            case 0: idk = 4; break;
                            case 1: idk = 7; break;
                            case 2: idk = 6; break;
                            case 3: idk = 5; break;
                        };
                        
                        for (const auto& offset : wallKickOffsets[idk]) {
                            
                            // Check if rotated piece fits at (newX, newY)
                            if (checkPlacement(rotatedPiece, board, offset.first, offset.second)) {
                                // Apply rotation and offset
                                currentPiece.shape = newShape;
                                std::swap(currentPiece.width, currentPiece.height);
                                currentPiece.x = currentPiece.x + offset.first;
                                currentPiece.y = currentPiece.y + offset.second;
                                currentPiece.rotation = (currentPiece.rotation - 1) % 4; // Equivalent
                                break;
                            }
                        }
                    }
                    

                    break;
                }
                case InputAction::SoftDrop:
                    if (checkPlacement(currentPiece, board, 0, 1)){ //check if can move down
                        pieceSet(currentPiece, board); //clear current position
                        currentPiece.y += 1; //move down
                    }
                    break;
                case InputAction::HardDrop:
                    {
                        pieceSet(currentPiece, board); //clear current position
                        currentPiece.y = maxDrop(currentPiece, board); //move down to max drop
                        spawnParticles(currentPiece); //spawn particles at hard drop location
                        hardDrop = true; //set hard drop flag
                        break;
                    }
                case InputAction::Hold:
                    // Implement hold functionality here
                    for (int sx = 0; sx < currentPiece.width; ++sx) {
                        for (int sy = 0; sy < currentPiece.height; ++sy) {
                            if (currentPiece.shape[sy][sx] != 0) {
                                int boardX = currentPiece.x + sx;
                                int boardY = currentPiece.y + sy;
                                if (boardX >= 0 && boardX < boardWidth && boardY >= 0 && boardY < boardHeight) {
                                    board.current[boardX][boardY] = 0;
                                }
                            }
                        }
                    }

                    if (!holdUsed){

                        //reset peice rotation

                        // if I piece, reset to original horizontal position
                        if (currentPiece.width == 1 || currentPiece.height == 1) {
                            currentPiece = iPiece; // Reset to I piece
                        }
                        else
                        {
                            while (currentPiece.rotation != 0) {
                                // Rotate back to original orientation
                                for (int sx = 0; sx < currentPiece.width; ++sx) {
                                    for (int sy = 0; sy < currentPiece.height; ++sy) {
                                        newShape[sx][currentPiece.height - 1 - sy] = currentPiece.shape[sy][sx];
                                    }
                                }
                                currentPiece.shape = newShape;
                                std::swap(currentPiece.width, currentPiece.height);
                                currentPiece.rotation = (currentPiece.rotation + 1) % 4;
                                //reset newShape for next potential rotation
                                newShape = std::vector<std::vector<int>>(currentPiece.width, std::vector<int>(currentPiece.height, 0));
                            }
                        }
                        

                        if (holdPiece.shape.empty()) {
                            holdPiece = currentPiece;
                            pickPiece = std::rand() % 7; // Generates a random number between 0 and 6 inclusive
                            //std::cout << "Picked piece: " << pickPiece << std::endl;
                            currentPiece = pieceTypes[nextPickPiece]; // Select a new random piece
                            nextPickPiece = std::rand() % 7; // Randomly select the next piece
                            nextPiece = pieceTypes[nextPickPiece]; // Update next piece
                            currentPiece.y = 0;
                            currentPiece.x = boardWidth / 2;
                            //holdPiece.rotation = 0; // need to implement rotation reset
                        } else {

                            std::swap(holdPiece, currentPiece);
                            currentPiece.y = 0;
                            currentPiece.x = boardWidth / 2;
                        }
                        holdUsed = true; // Mark hold as used for this turn
                    }

                    

                    // holdPiece = currentPiece;
                    // currentPiece = pieceTypes[nextPickPiece]; // Select a new random piece
                    // nextPickPiece = std::rand() % 7; // Randomly select the next piece
                    // nextPiece = pieceTypes[nextPickPiece]; // Update next piece
                    
                    break;
                case InputAction::Pause:
                    paused = !paused;
                default:
                    break;
            }

            

            if (pieceLanded && (action == InputAction::MoveLeft || action == InputAction::MoveRight || action == InputAction::RotateClockwise)) {
                lockDelayCounter = 0;
            }

            float spacing = 2.0f; // Amount of spacing between blocks

            //UI ELEMENTS
            //background black
            SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 255 ); // Use alpha 255 for opaque
            SDL_RenderClear( gRenderer );

            //Set render color to white
            SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255 );

            // Draw grid lines
            SDL_SetRenderDrawColor(gRenderer, 40, 40, 40, 255); // Dark gray for grid lines
            for (int x = 0; x <= boardWidth; ++x) {
                SDL_RenderLine(gRenderer, x * blockSize, 0, x * blockSize, boardHeight * blockSize);
            }
            for (int y = 0; y <= boardHeight; ++y) {
                SDL_RenderLine(gRenderer, 0, y * blockSize, boardWidth * blockSize, y * blockSize);
            }

            //draw line seperating the board and UI
            SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255 );
            SDL_RenderLine( gRenderer, 480, 0, 480, kScreenHeight );

            //render the UI elements
            scoreLabel.render( 520, 40);
            score.render( 520, 80 );
            levelLabel.render( 520, 120 );
            level.render( 520, 160 );
            nextLabel.render( 520, 200 );
            holdLabel.render( 520, 380 );
            highScoreLabel.render( 520, 560 );
            highScore.render( 520, 600 );
            SDL_FRect nextFRect{ 510, 240, 100, 100 };
            SDL_RenderRect( gRenderer, &nextFRect ); // Render a rectangle for the next piece
            SDL_FRect holdFRect{ 510.f, 420.f, 100.f, 100.f };
            SDL_RenderRect( gRenderer, &holdFRect ); // Render a rectangle for the hold piece

            //todo put next piece here
            SDL_SetRenderDrawColor( gRenderer, 128, 128, 128, 255 ); // Gray color for pieces
            for (int sx = 0; sx < nextPiece.width; ++sx) {
                    for (int sy = 0; sy < nextPiece.height; ++sy) {
                        if (nextPiece.shape[sy][sx] != 0) {
                            int boardX = nextPiece.x + sx;
                            int boardY = nextPiece.y + sy;
                            // if (boardX >= 0 && boardX < boardWidth && boardY >= 0 && boardY < boardHeight) {
                            //     board.current[boardX][boardY] = currentPiece.color;
                            // }
                            SDL_FRect rect{ 540.f + sx * blockSize/2 + spacing/2, 265.f + sy * blockSize/2 + spacing/2, blockSize/2-spacing, blockSize/2-spacing };
                            SDL_RenderFillRect(gRenderer, &rect);
                        }
                    }
                }

            //todo put hold piece here
            for (int sx = 0; sx < holdPiece.width; ++sx) {
                    for (int sy = 0; sy < holdPiece.height; ++sy) {
                        if (holdPiece.shape[sy][sx] != 0) {
                            int boardX = holdPiece.x + sx;
                            int boardY = holdPiece.y + sy;
                            // if (boardX >= 0 && boardX < boardWidth && boardY >= 0 && boardY < boardHeight) {
                            //     board.current[boardX][boardY] = currentPiece.color;
                            // }
                            SDL_FRect rect{ 540.f + sx * blockSize/2 + spacing/2, 445.f + sy * blockSize/2 + spacing/2, blockSize/2-spacing, blockSize/2-spacing };
                            SDL_RenderFillRect(gRenderer, &rect);
                        }
                    }
                }

            // std::cout << "Board state:\n";
            //     for (int y = 0; y < boardHeight; ++y) {
            //         for (int x = 0; x < boardWidth; ++x) {
            //             std::cout << board.current[x][y] << " ";
            //         }
            //         std::cout << "\n";
            //     }


            if (paused) {
                //render a "Paused" message
                SDL_Color textColor{ 0xFF, 0xFF, 0xFF, 0xFF };
                gameOverLabel.loadFromRenderedText("PAUSED", textColor);
                gameOverLabel.render(200, 300);
                SDL_RenderPresent(gRenderer);

                //render board with hollow blocks


                // Cap frame rate while paused
                Uint64 nsPerFrame = 1000000000 / kScreenFps;
                Uint64 frameNs{ capTimer.getTicksNS() };
                if (frameNs < nsPerFrame) {
                    SDL_DelayNS(nsPerFrame - frameNs);
                }
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

            // --- RENDER BOARD AND PARTICLES DURING ANIMATION ---
            // Clear background
            SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
            SDL_RenderClear(gRenderer);

            //render the UI elements
            scoreLabel.render( 520, 40);
            score.render( 520, 80 );
            levelLabel.render( 520, 120 );
            level.render( 520, 160 );
            nextLabel.render( 520, 200 );
            holdLabel.render( 520, 380 );
            highScoreLabel.render( 520, 560 );
            highScore.render( 520, 600 );
            
            

            //todo put next piece here
            SDL_SetRenderDrawColor( gRenderer, 128, 128, 128, 255 ); // Gray color for pieces
            for (int sx = 0; sx < nextPiece.width; ++sx) {
                    for (int sy = 0; sy < nextPiece.height; ++sy) {
                        if (nextPiece.shape[sy][sx] != 0) {
                            int boardX = nextPiece.x + sx;
                            int boardY = nextPiece.y + sy;
                            // if (boardX >= 0 && boardX < boardWidth && boardY >= 0 && boardY < boardHeight) {
                            //     board.current[boardX][boardY] = currentPiece.color;
                            // }
                            SDL_FRect rect{ 540.f + sx * blockSize/2 + spacing/2, 265.f + sy * blockSize/2 + spacing/2, blockSize/2-spacing, blockSize/2-spacing };
                            SDL_RenderFillRect(gRenderer, &rect);
                        }
                    }
                }

            //todo put hold piece here
            for (int sx = 0; sx < holdPiece.width; ++sx) {
                    for (int sy = 0; sy < holdPiece.height; ++sy) {
                        if (holdPiece.shape[sy][sx] != 0) {
                            int boardX = holdPiece.x + sx;
                            int boardY = holdPiece.y + sy;
                            // if (boardX >= 0 && boardX < boardWidth && boardY >= 0 && boardY < boardHeight) {
                            //     board.current[boardX][boardY] = currentPiece.color;
                            // }
                            SDL_FRect rect{ 540.f + sx * blockSize/2 + spacing/2, 445.f + sy * blockSize/2 + spacing/2, blockSize/2-spacing, blockSize/2-spacing };
                            SDL_RenderFillRect(gRenderer, &rect);
                        }
                    }
                }

            
            SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255 );
            
            SDL_FRect nextFRect{ 510, 240, 100, 100 };
            SDL_RenderRect( gRenderer, &nextFRect ); // Render a rectangle for the next piece
            SDL_FRect holdFRect{ 510.f, 420.f, 100.f, 100.f };
            SDL_RenderRect( gRenderer, &holdFRect ); // Render a rectangle for the hold piece
            

            // Draw grid lines
            SDL_SetRenderDrawColor(gRenderer, 40, 40, 40, 255);
            for (int x = 0; x <= boardWidth; ++x)
                SDL_RenderLine(gRenderer, x * blockSize, 0, x * blockSize, boardHeight * blockSize);
            for (int y = 0; y <= boardHeight; ++y)
                SDL_RenderLine(gRenderer, 0, y * blockSize, boardWidth * blockSize, y * blockSize);

            //draw line seperating the board and UI
            SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255 );
            SDL_RenderLine( gRenderer, 480, 0, 480, kScreenHeight );

            // Draw the blocks
            float spacing = 2.0f;
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

            // Render particles
            for (auto it = particles.begin(); it != particles.end();) {
                it->x += it->vx;
                it->y += it->vy;
                it->lifetime--;
                if (it->alpha > 0) it->alpha -= 255.0f / (it->lifetime + 1);
                SDL_Color c = it->color;
                c.a = static_cast<Uint8>(std::max(0.0f, it->alpha));
                SDL_SetRenderDrawColor(gRenderer, c.r, c.g, c.b, c.a);
                SDL_FRect rect{it->x, it->y, 2, 2};
                SDL_RenderFillRect(gRenderer, &rect);
                if (it->lifetime <= 0 || it->alpha <= 0)
                    it = particles.erase(it);
                else
                    ++it;
            }

            // Optionally, render UI elements here if you want them visible during animation

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
            // Cap frame rate
            Uint64 nsPerFrame = 1000000000 / kScreenFps;
            Uint64 frameNs{ capTimer.getTicksNS() };
            if (frameNs < nsPerFrame) SDL_DelayNS(nsPerFrame - frameNs);
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
            //currentPiece.y += 1;
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
          
            
            
            
            
            
            
            //&& (currentPiece.y + currentPiece.height < boardHeight) && board.current[currentPiece.x][currentPiece.y + 1 + currentPiece.height] == 0
             

            

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

            // Update and render particles
            for (auto it = particles.begin(); it != particles.end();) {
                it->x += it->vx;
                it->y += it->vy;
                it->lifetime--;
                if (it->alpha > 0) it->alpha -= 255.0f / (it->lifetime + 1); // Fade out
                SDL_Color c = it->color;
                c.a = static_cast<Uint8>(std::max(0.0f, it->alpha));
                SDL_SetRenderDrawColor(gRenderer, c.r, c.g, c.b, c.a);
                SDL_FRect rect{it->x, it->y, 2, 2}; // Small sparkle
                SDL_RenderFillRect(gRenderer, &rect);
                if (it->lifetime <= 0 || it->alpha <= 0)
                    it = particles.erase(it);
                else
                    ++it;
            }

            //update screen
            SDL_RenderPresent( gRenderer );

            // if (!canPlaceNext)
            // {
            //     newPiece = true;
            // }

            


            

            if (!newPiece) {
                    // board.current[currentPiece.x][currentPiece.y] = 0;
                    // currentPiece.y += 1; 

                    //std::cout << canPlaceNext << std::endl;

                    for (int sx = 0; sx < currentPiece.width; ++sx) {
                        for (int sy = 0; sy < currentPiece.height; ++sy) {
                            if (currentPiece.shape[sy][sx] != 0) {
                                int boardX = currentPiece.x + sx;
                                int boardY = currentPiece.y + sy;
                                // if (boardX >= 0 && boardX < boardWidth && boardY >= 0 && boardY < boardHeight) {
                                //     board.current[boardX][boardY] = 0;
                                // }
                                board.current[boardX][boardY] = 0;
                            }
                        }
                    }
                    
            }
            
            Uint64 now = SDL_GetTicksNS();
            if (now - lastDropTime >= dropSpeed && canPlaceNext) {
                currentPiece.y += 1;
                lastDropTime = now;
            }


            if (!canPlaceNext && !hardDrop)
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
            if (newPiece || hardDrop) {
                    //currentPiece.y -= 1; // Move back up to last valid position
                    for (int sx = 0; sx < currentPiece.width; ++sx) {
                        for (int sy = 0; sy < currentPiece.height; ++sy) {
                            if (currentPiece.shape[sy][sx] != 0) {
                                int boardX = currentPiece.x + sx;
                                int boardY = currentPiece.y + sy;
                                // if (boardX >= 0 && boardX < boardWidth && boardY >= 0 && boardY < boardHeight) {
                                //     board.current[boardX][boardY] = 0;
                                // }
                                board.current[boardX][boardY] = currentPiece.color;
                            }
                        }
                    }

                // std::cout << "Board state:\n";
                // for (int y = 0; y < boardHeight; ++y) {
                //     for (int x = 0; x < boardWidth; ++x) {
                //         std::cout << board.current[x][y] << " ";
                //     }
                //     std::cout << "\n";
                // }


                int fullRows[boardHeight];

                for (int i = 0; i < boardHeight; ++i) {
                    fullRows[i] = 0;
                }

                //int rowcount = 0;
                //int maxRowCount = 0;
                for (int y = 0; y < boardHeight; ++y) {
                    int rowcount = 0;
                    for (int x = 0; x < boardWidth; ++x) {
                        if (board.current[x][y] != 0) {
                            rowcount++;
                            //std::cout << "Rowcount: " << rowcount << std::endl;
                        }
                    }
                    // if (rowcount > maxRowCount) {
                    //     maxRowCount = rowcount;
                    // }
                    //std::cout << "maxRowCount: " << maxRowCount << std::endl;
                    if (rowcount == boardWidth) {
                        //std::cout << "Full row at: " << y << std::endl;
                        fullRows[y] = 1;
                    }
                }

                int clearedRows = 0;

                for (int i = 0; i < boardHeight; ++i) 
                {
                    if (fullRows[i] == 1) {
                        clearedRows++;
                        //std::cout << "Row " << i << " is full!" << std::endl;
                        
                        
                    }

                    
                }

                // for (int i = 0; i < boardHeight; ++i) 
                // {
                //     if (fullRows[i] == 1) {
                //         clearedRows++;
                //         //std::cout << "Row " << i << " is full!" << std::endl;
                //         for (int y = i; y > 0; --y) {
                //             for (int x = 0; x < boardWidth; ++x) {
                //                 //board.current[x][0] = 0; // Clear the top row
                //                 board.current[x][y] = board.current[x][y - 1];
                //             }
                //         }
                //         for (int x = 0; x < boardWidth; ++x) {
                //             board.current[x][0] = 0; // Clear the top row
                //         }
                //         // for (int x = 0; x < boardWidth; ++x) {
                //         //     // for (int y = i; y > 0; --y) {
                //         //     //     board.current[x][y] = board.current[x][y - 1];
                //         //     // }
                //         //     // board.current[x][0] = 0; // Clear the top row
                //         //     board.current[x][i] = 0;
                //         // }
                        
                //     }

                    
                // }

                if (!clearingRows && clearedRows > 0) {
                    clearingRows = true;
                    clearAnimStart = SDL_GetTicksNS();
                    clearAnimStep = 0;
                    rowsToClear.clear();
                    for (int i = 0; i < boardHeight; ++i) {
                        if (fullRows[i] == 1) rowsToClear.push_back(i);
                    }
                    // Optionally: Play a sound here
                    //continue; // Skip rest of loop to start animation
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

                // if (scoreValue > 200) {
                    
                //     if (levelValue <= 9) {
                //         levelValue += 1;
                //         level.loadFromRenderedText( std::to_string(levelValue), { 0xFF, 0xFF, 0xFF, 0xFF } );
                //         //levelValue = 0;
                //         if (dropSpeed > 10) {
                //             dropSpeed -= 5; // Increase speed
                //         }
                //     }
                // }

                //std::cout << "newPiece" << std::endl;
                currentPiece.y = 0; // Reset for next falling piece
                currentPiece.x = boardWidth / 2; // Reset horizontal position to center
                //pickPiece = std::rand() % 7; // Generates a random number between 0 and 6 inclusive
                //std::cout << "Picked piece: " << pickPiece << std::endl;
                currentPiece = pieceTypes[nextPickPiece]; // Select a new random piece
                nextPickPiece = std::rand() % 7; // Randomly select the next piece
                nextPiece = pieceTypes[nextPickPiece]; // Update next piece
                newPiece = false;
                hardDrop = false;
                holdUsed = false; // Reset hold usage for the new piece
            }

            // std::cout << "Board state:\n";
            //     for (int y = 0; y < boardHeight; ++y) {
            //         for (int x = 0; x < boardWidth; ++x) {
            //             std::cout << board.current[x][y] << " ";
            //         }
            //         std::cout << "\n";
            //     }
            
            //myTickCount++;


            //Cap frame rate
            //1,000,000,000
            //Uint64 nsPerFrame = dropSpeed / kScreenFps; 
            Uint64 nsPerFrame = 1000000000 / kScreenFps;
            Uint64 frameNs{ capTimer.getTicksNS() };
            if( frameNs < nsPerFrame )
            {
                SDL_DelayNS( nsPerFrame - frameNs );
            }
        } 

        
    }

    

    //Clean up
    close();

    return exitCode;
}