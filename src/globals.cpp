#include "globals.h"
#include "ltimer.h"
#include"tetris_utils.h"
#include "tPieceIcon.h"
#include "Pixeboy_ttf.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <vector>

bool loadMedia()
{
    //File loading flag
    bool success{ true };

    //Load scene font
    // std::string fontPath{ "assets/Pixeboy.ttf" };
    // gFont = TTF_OpenFont( 24 ); 

    SDL_IOStream* io_stream = SDL_IOFromMem(assets_Pixeboy_ttf, assets_Pixeboy_ttf_len);
    gFont = TTF_OpenFontIO(io_stream, 1, 24); // 1 = auto free rw


    if( gFont == nullptr )
    {
        SDL_Log( "Could not load %s! SDL_ttf Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Load text
        SDL_Color textColor{ 0xFF, 0xFF, 0xFF, 0xFF }; // White text
        if( scoreLabel.loadFromRenderedText( "SCORE", textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", SDL_GetError() );
            success = false;
        }
        if( levelLabel.loadFromRenderedText( "LEVEL", textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", SDL_GetError() );
            success = false;
        }
        if( nextLabel.loadFromRenderedText( "NEXT", textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", SDL_GetError() );
            success = false;
        }
        if( score.loadFromRenderedText( std::to_string(scoreValue), textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", SDL_GetError() );
            success = false;
        }
        if( level.loadFromRenderedText( std::to_string(levelValue+1), textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", SDL_GetError() );
            success = false;
        }
        if( holdLabel.loadFromRenderedText( "HOLD", textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", SDL_GetError() );
            success = false;
        }
        if( highScoreLabel.loadFromRenderedText( "HIGH SCORE", textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", SDL_GetError() );
            success = false;
        }
        if( highScore.loadFromRenderedText( std::to_string(highScoreValue), textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", SDL_GetError() );
            success = false;
        }
    }

    return success;
}

bool init(std::string title)
{
    bool success{ true };

    // if (SDL_INIT_GAMEPAD == false) {
    //     SDL_Log("SDL Gamepad subsystem could not initialize! SDL error: %s\n");
    //     success = false;
    // }

    if (SDL_Init( SDL_INIT_VIDEO) == false )
    {
        SDL_Log( "SDL could not initialize! SDL error: %s\n", SDL_GetError() );
        success = false;
    }
    else if (SDL_InitSubSystem( SDL_INIT_GAMEPAD ) == false) {
        SDL_Log("SDL Gamepad subsystem could not initialize! SDL error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        if( SDL_CreateWindowAndRenderer( title.c_str(), kScreenWidth, kScreenHeight, 0, &gWindow, &gRenderer ) == false )
        {
            SDL_Log( "Window could not be created! SDL error: %s\n", SDL_GetError() );
            success = false;
        } else
        {
            // Add this block to set the window icon
            //SDL_Surface* iconSurface = IMG_Load("assets/tPieceIcon.png"); // Use your icon file path
            SDL_IOStream* io_stream = SDL_IOFromMem(assets_tPieceIcon_png, assets_tPieceIcon_png_len);
            SDL_Surface* iconSurface = IMG_Load_IO(io_stream, 1); // 1 = auto free rw
            if (iconSurface != nullptr) {
                SDL_SetWindowIcon(gWindow, iconSurface);
                SDL_DestroySurface(iconSurface);
            } else {
                SDL_Log("Could not load icon! SDL_image error: %s\n", SDL_GetError());
            }

            if ( TTF_Init() == false )
            {
                SDL_Log( "SDL_ttf could not initialize! SDL_ttf error: %s\n", SDL_GetError() );
                success = false;
            }

            //initialize gamepad
            int gamePadCount = 0;
            SDL_JoystickID *ids = SDL_GetGamepads(&gamePadCount);
            SDL_Gamepad* gamepad = nullptr;
            for (int i = 0; i < gamePadCount; ++i) {
                SDL_Gamepad* gamepd = SDL_OpenGamepad(ids[i]);
                if (gamepad == NULL) {
                    gamepad = gamepd;
                }
                //std::cout << "Gamepad connected: " << SDL_GetGamepadName(gamepd) << "\n";
                // Close the other gamepads
                if(i > 0) {
                    SDL_CloseGamepad(gamepd);
                }
            }

        }
    }
    return success;
}

void close()
{

    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    gRenderer = nullptr;
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;

    //Quit SDL subsystems
    SDL_Quit();

     //Clean up textures
    scoreLabel.destroy();
    levelLabel.destroy();
    nextLabel.destroy();
    holdLabel.destroy();
    highScore.destroy();
    score.destroy();
    level.destroy();
    highScore.destroy();
    
    
    //Free font
    TTF_CloseFont( gFont );
    gFont = nullptr;

    // Close gamepad if open
    int gamePadCount = 0;
    SDL_JoystickID *ids = SDL_GetGamepads(&gamePadCount);
    for (int i = 0; i < gamePadCount; ++i) {
        SDL_Gamepad* gamepad = SDL_OpenGamepad(ids[i]);
        if (gamepad != nullptr) {
            SDL_CloseGamepad(gamepad);
        }
    }

    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    gRenderer = nullptr;
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;

    //Quit SDL subsystems
    TTF_Quit();
    SDL_Quit();
}

void capFrameRate(){
    Uint64 nsPerFrame = 1000000000 / kScreenFps;
    Uint64 frameNs{ capTimer.getTicksNS() };
    if( frameNs < nsPerFrame )
    {
        SDL_DelayNS( nsPerFrame - frameNs );
    }
}

void renderUI() {
    //clear screen
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
    SDL_SetRenderDrawColor( gRenderer, 128, 128, 128, 255 ); // Gray color for pieces
    for (int sx = 0; sx < nextPiece.width; ++sx) { // render next piece in the UI
            for (int sy = 0; sy < nextPiece.height; ++sy) {
                if (nextPiece.shape[sy][sx] != 0) {
                    int boardX = nextPiece.x + sx;
                    int boardY = nextPiece.y + sy;
                    SDL_FRect rect{ 540.f + sx * blockSize/2 + spacing/2, 265.f + sy * blockSize/2 + spacing/2, blockSize/2-spacing, blockSize/2-spacing };
                    SDL_RenderFillRect(gRenderer, &rect);
                }
            }
        }
    for (int sx = 0; sx < holdPiece.width; ++sx) { // render hold piece in the UI
            for (int sy = 0; sy < holdPiece.height; ++sy) {
                if (holdPiece.shape[sy][sx] != 0) {
                    int boardX = holdPiece.x + sx;
                    int boardY = holdPiece.y + sy;
                    SDL_FRect rect{ 540.f + sx * blockSize/2 + spacing/2, 445.f + sy * blockSize/2 + spacing/2, blockSize/2-spacing, blockSize/2-spacing };
                    SDL_RenderFillRect(gRenderer, &rect);
                }
            }
        }
    SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255 ); // set render color to white
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
}

void renderParticles() {
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
}

SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
TTF_Font* gFont = nullptr;

LTexture scoreLabel;
LTexture levelLabel;
LTexture nextLabel;
LTexture holdLabel;
LTexture highScoreLabel;
LTexture gameOverLabel;

LTexture score;
LTexture level;
LTexture highScore;

int scoreValue = 0;
int levelValue = 0;
int highScoreValue = 0;

// Define Tetris pieces
Piece iPiece = {
    4, // width
    1, // height
    std::vector<std::vector<int>>{ { 1, 1, 1, 1 } }, // shape
    0, // rotation
    1  // color
};

Piece oPiece = {
    2, // width
    2, // height
    std::vector<std::vector<int>>{ { 1, 1 }, { 1, 1 } }, // shape
    0, // rotation
    2  // color
};

Piece tPiece = {
    3, // width
    2, // height
    std::vector<std::vector<int>>{ { 0, 1, 0 }, { 1, 1, 1 } }, // shape
    0, // rotation
    3  // color
};

Piece lPiece = {
    3, // width
    2, // height
    std::vector<std::vector<int>>{ { 0, 0, 1 }, { 1, 1, 1 } }, // shape
    0, // rotation
    4  // color
};

Piece jPiece = {
    3, // width
    2, // height
    std::vector<std::vector<int>>{ { 1, 0, 0 }, { 1, 1, 1 } }, // shape
    0, // rotation
    5  // color
};

Piece sPiece = {
    3, // width
    2, // height
    std::vector<std::vector<int>>{ { 0, 1, 1 }, { 1, 1, 0 } }, // shape
    0, // rotation
    6  // color
};

Piece zPiece = {
    3, // width
    2, // height
    std::vector<std::vector<int>>{ { 1, 1, 0 }, { 0, 1, 1 } }, // shape
    0, // rotation
    7  // color
};

float spacing = 2.0f; // Amount of spacing between blocks

LTimer capTimer; //frames per second timer
Uint64 lastDropTime = SDL_GetTicksNS(); //
Uint64 dropSpeed{ 700000000 }; // Milliseconds between drops

// Wall kick offset vectors (J, L, S, T, Z pieces)
//state names: 0 = spawn state, R = 1 clockwise rotation from spawn, L = 1 counterclockwise rotation from spawn, 2 = 2 rotations from spawn in either direction
//0->R
std::vector<std::pair<int, int>> wallKickOffsets0R = {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}};
//R->0
std::vector<std::pair<int, int>> wallKickOffsetsR0 = {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}};
//R->2
std::vector<std::pair<int, int>> wallKickOffsetsR2 = {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}};
//2->R
std::vector<std::pair<int, int>> wallKickOffsets2R = {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}};
//2->L
std::vector<std::pair<int, int>> wallKickOffsets2L = {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}};
//L->2
std::vector<std::pair<int, int>> wallKickOffsetsL2 = {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}};
//L->0
std::vector<std::pair<int, int>> wallKickOffsetsL0 = {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}};
//0->L
std::vector<std::pair<int, int>> wallKickOffsets0L = {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}};

std::vector<std::pair<int, int>> wallKickOffsets[8] = {wallKickOffsets0R, wallKickOffsetsR2, wallKickOffsets2L, wallKickOffsetsL0, wallKickOffsets0L, wallKickOffsetsL2, wallKickOffsets2R, wallKickOffsetsR0};

// Wall kick offset vectors (I piece)
//0->R
std::vector<std::pair<int, int>> wallKickOffsetsI0R = {{0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, -2}};
//R->0
std::vector<std::pair<int, int>> wallKickOffsetsIR0 = {{0, 0}, {2, 0}, {-1, 0}, {2, -1}, {-1, 2}};
//R->2
std::vector<std::pair<int, int>> wallKickOffsetsIR2 = {{0, 0}, {-1, 0}, {2, 0}, {-1, -2}, {2, 1}};
//2->R
std::vector<std::pair<int, int>> wallKickOffsetsI2R = {{0, 0}, {1, 0}, {-2, 0}, {1, 2}, {-2, -1}};
//2->L
std::vector<std::pair<int, int>> wallKickOffsetsI2L = {{0, 0}, {2, 0}, {-1, 0}, {2, -1}, {-1, 2}};
//L->2
std::vector<std::pair<int, int>> wallKickOffsetsIL2 = {{0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, -2}};
//L->0
std::vector<std::pair<int, int>> wallKickOffsetsIL0 = {{0, 0}, {1, 0}, {-2, 0}, {1, 2}, {-2, -1}};
//0->L
std::vector<std::pair<int, int>> wallKickOffsetsI0L = {{0, 0}, {-1, 0}, {2, 0}, {-1, -2}, {2, 1}};

std::vector<std::pair<int, int>> wallKickOffsetsI[8] = {wallKickOffsetsI0R, wallKickOffsetsIR2, wallKickOffsetsI2L, wallKickOffsetsIL0, wallKickOffsetsI0L, wallKickOffsetsIL2, wallKickOffsetsI2R, wallKickOffsetsIR0};
