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
#include <cmath>
#include <random>
#include <array>
#include <algorithm>

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
        if( SDL_CreateWindowAndRenderer( title.c_str(), kScreenWidth, kScreenHeight, SDL_WINDOW_RESIZABLE, &gWindow, &gRenderer ) == false )
        {
            SDL_Log( "Window could not be created! SDL error: %s\n", SDL_GetError() );
            success = false;
        } else
        {
            // Make rendering use a fixed logical size and scale to the window
            // 3-arg variant (SDL3 older API) + set scale mode separately
            if (!SDL_SetRenderLogicalPresentation(gRenderer, kScreenWidth, kScreenHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
                SDL_Log("Failed to set logical presentation: %s", SDL_GetError());
            }
            //SDL_SetRenderScaleMode(gRenderer, SDL_SCALEMODE_NEAREST);

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

            // Optional: tweak double-click timeout (ms)
            SDL_SetHint(SDL_HINT_MOUSE_DOUBLE_CLICK_TIME, "350");
            
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

static inline void ApplyFullscreenCursorState() {
    const bool isFullscreen = (SDL_GetWindowFlags(gWindow) & SDL_WINDOW_FULLSCREEN) != 0;

    // SDL3: boolean ShowCursor
    if (isFullscreen) {SDL_ShowCursor();}
    else {
        SDL_HideCursor();
    } // hide when fullscreen
}

void toggleFullscreen() {
    const bool isFullscreen = (SDL_GetWindowFlags(gWindow) & SDL_WINDOW_FULLSCREEN) != 0;
    SDL_SetWindowFullscreen(gWindow, !isFullscreen);
    ApplyFullscreenCursorState();
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

LTexture resumeTexture;
LTexture quitTexture;

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

// 7-bag RNG (seeded once, no dependency on main())
static std::mt19937 rng([]{
    std::random_device rd;
    return std::mt19937(rd());
}());
static std::array<int, 7> pieceBag{};
static size_t bagIdx = pieceBag.size();

static void refillBag() {
    for (int i = 0; i < 7; ++i) pieceBag[i] = i;
    std::shuffle(pieceBag.begin(), pieceBag.end(), rng);
    bagIdx = 0;
}
int drawPieceIndex() {
    if (bagIdx >= pieceBag.size()) refillBag();
    return pieceBag[bagIdx++];
}

GameState currentState = GameState::MENU;

// Controller repeat config (ms)
Uint64 kDAS_MS          = 167; // delay before auto-repeat
Uint64 kARR_MS          = 42;  // auto-repeat rate (horizontal)
Uint64 kSoftDrop_ARR_MS = 42;  // auto-repeat rate (soft drop)

RepeatState gpLeft{}, gpRight{}, gpDown{};
HDir activeH{ HDir::None };

// Analog stick thresholds with hysteresis
int kAxisPress   = 16000; // press when beyond this magnitude
int kAxisRelease = 12000; // release when within this magnitude

// Track per-source held states
bool kbLeftHeld{false}, kbRightHeld{false}, kbDownHeld{false};
// Gamepad: split DPAD vs Analog, then combine
bool gpDpadLeftHeld{false}, gpDpadRightHeld{false}, gpDpadDownHeld{false};
bool gpAxisLeftHeld{false}, gpAxisRightHeld{false}, gpAxisDownHeld{false};
bool gpLeftHeld{false}, gpRightHeld{false}, gpDownHeld{false};

void recomputeGamepadHeld() {
    gpLeftHeld  = gpDpadLeftHeld  || gpAxisLeftHeld;
    gpRightHeld = gpDpadRightHeld || gpAxisRightHeld;
    gpDownHeld  = gpDpadDownHeld  || gpAxisDownHeld;
}

// Menu/pause analog navigation hysteresis flags
bool menuAxisUpHeld{false}, menuAxisDownHeld{false};
bool pauseAxisLeftHeld{false}, pauseAxisRightHeld{false};

LTexture titleTexture;
LTexture playTexture;
LTexture optionsTexture;
LTexture backTexture;
LTexture exitTexture;

LTexture optionsTitleTexture;
LTexture optionsGridLabel;

LTexture optionsTitleTexture2;
LTexture windowSizeLabel;
LTexture fullscreenLabel;


LTexture optionsTitleTexture3;
LTexture inputConfigLabel;


int menuSelection = 0;

// Helper to move menu selection (0..2) with wrap-around
static inline void moveMenuSelection(int delta) {
    const int count = 3; // Play, Options, Exit
    menuSelection = (menuSelection + delta + count) % count;
}

void renderMenu() {
    // Clear screen
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    // Use logical size for layout; renderer scales to window
    const int winW = kScreenWidth;
    const int winH = kScreenHeight;
    int rightX = winW - 250;
    int centerY = winH / 4;

    // Prepare text first so widths/heights are valid
    titleTexture.loadFromRenderedText("TETRIS", {255,255,255,255});
    playTexture.loadFromRenderedText("Play", {255,255,255,255});
    optionsTexture.loadFromRenderedText("Options", {255,255,255,255});
    exitTexture.loadFromRenderedText("Exit", {255,255,255,255});

    // Row positions
    const int yPlay    = centerY + 60;
    const int yOptions = centerY + 110;
    const int yExit    = centerY + 160;

    // X positions (kept similar to your existing offsets)
    const int xPlay    = rightX;
    const int xOptions = rightX - 15;
    const int xExit    = rightX - 5;

    // Selection rectangle around the chosen option
    const LTexture* selTex = (menuSelection == 0) ? &playTexture
                           : (menuSelection == 1) ? &optionsTexture
                           : &exitTexture;
    const int selX = (menuSelection == 0) ? xPlay
                   : (menuSelection == 1) ? xOptions
                   : xExit;
    const int selY = (menuSelection == 0) ? yPlay
                   : (menuSelection == 1) ? yOptions
                   : yExit;

    const int padX = 18;
    const int padY = 10;

    SDL_SetRenderDrawColor(gRenderer, 49, 117, 73, 70);
    SDL_FRect selectRect{
        static_cast<float>(selX - padX),
        static_cast<float>(selY - padY),
        static_cast<float>(selTex->getWidth() + padX * 2 - 2),
        static_cast<float>(selTex->getHeight() + padY * 2 - 2)
    };
    SDL_RenderFillRect(gRenderer, &selectRect);

    // Draw
    titleTexture.render(rightX-250, centerY-100, nullptr, 160, 100);
    playTexture.render(xPlay, yPlay);
    optionsTexture.render(xOptions, yOptions);
    exitTexture.render(xExit, yExit);


    // Selection rectangle
    // SDL_SetRenderDrawColor(gRenderer, 49, 117, 73, 70);
    // int rectY = (menuSelection == 0) ? centerY + 60 : centerY + 110;
    // int rectW = playTexture.getWidth() + 20;
    // int rectH = playTexture.getHeight() + 10;
    // SDL_FRect selectRect{static_cast<float>(rightX - 18), static_cast<float>(rectY - 10), static_cast<float>(rectW + 20), static_cast<float>(rectH + 10)};
    // SDL_RenderFillRect(gRenderer, &selectRect);

    // // Draw
    // titleTexture.render(rightX-250, centerY-100, nullptr, 160, 100);
    // playTexture.render(rightX, centerY + 60);
    // optionsTexture.render(rightX - 15, centerY + 110);

    // SDL_RenderPresent moved to main
}

int handleMenuEvent(const SDL_Event& e) {
    //handke keyboard input for menu navigation
    if (e.type == SDL_EVENT_KEY_DOWN) {
        if (e.key.key == SDLK_UP) {
            moveMenuSelection(-1);
        } else if (e.key.key == SDLK_DOWN) {
            moveMenuSelection(1);
        } else if (e.key.key == SDLK_RETURN || e.key.key == SDLK_KP_ENTER) {
            return menuSelection; // Return the selected option
        }
    }

    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP) {
            moveMenuSelection(-1);
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
            menuSelection = (menuSelection + 1) % 2;
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
            return menuSelection; // Return the selected option
        }
    }

    // Analog stick up/down for menu
    if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
        const int v = e.gaxis.value;
        if (v <= -kAxisPress) {
            if (!menuAxisUpHeld) {
                moveMenuSelection(-1);
                menuAxisUpHeld = true;
                menuAxisDownHeld = false;
            }
        } else if (v >= kAxisPress) {
            if (!menuAxisDownHeld) {
                moveMenuSelection(1);
                menuAxisDownHeld = true;
                menuAxisUpHeld = false;
            }
        } else if (std::abs(v) < kAxisRelease) {
            menuAxisUpHeld = false;
            menuAxisDownHeld = false;
        }
    }

    return -1; // No selection made
}

void renderGameOptions() {
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    optionsTitleTexture.loadFromRenderedText("Game", {255,255,255,255});
    optionsTitleTexture.render(200, 20);
    optionsTitleTexture2.loadFromRenderedText("Video", {255,255,255,255});
    optionsTitleTexture2.render(350, 20);
    optionsTitleTexture3.loadFromRenderedText("Input", {255,255,255,255});
    optionsTitleTexture3.render(500, 20);

    optionsGridLabel.loadFromRenderedText("Grid lines < ON >", {255,255,255,255});
    optionsGridLabel.render(400, 100);

    backTexture.loadFromRenderedText("Return", {255,255,255,255});
    backTexture.render(200, 400);
}

void renderVideoOptions() {
    optionsTitleTexture.loadFromRenderedText("Game", {255,255,255,255});
    optionsTitleTexture.render(200, 20);
    optionsTitleTexture2.loadFromRenderedText("Video", {255,255,255,255});
    optionsTitleTexture2.render(350, 20);
    optionsTitleTexture3.loadFromRenderedText("Input", {255,255,255,255});
    optionsTitleTexture3.render(500, 20);

    windowSizeLabel.loadFromRenderedText("Window Size < Standard >", {255,255,255,255});
    windowSizeLabel.render(400, 100);
    
    backTexture.loadFromRenderedText("Return", {255,255,255,255});
    backTexture.render(200, 400);
}

void renderInputOptions() {
    optionsTitleTexture.loadFromRenderedText("Game", {255,255,255,255});
    optionsTitleTexture.render(200, 20);
    optionsTitleTexture2.loadFromRenderedText("Video", {255,255,255,255});
    optionsTitleTexture2.render(350, 20);
    optionsTitleTexture3.loadFromRenderedText("Input", {255,255,255,255});
    optionsTitleTexture3.render(500, 20);

    inputConfigLabel.loadFromRenderedText("Hard Drop (A) Select to change binding", {255,255,255,255});
    inputConfigLabel.render(400, 100);



    backTexture.loadFromRenderedText("Return", {255,255,255,255});
    backTexture.render(200, 400);
}

int pauseMenuSelection = 0;

void renderPauseMenu() {
    // Redraw game scene behind pause menu
    renderUI();
    renderBoardBlocks();
    renderParticles();

    // Enable blending only for translucent overlay + selection
    SDL_BlendMode prevBlend;
    SDL_GetRenderDrawBlendMode(gRenderer, &prevBlend);
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);

    // Use logical size for layout; renderer scales to window
    const int winW = kScreenWidth - 160;
    const int winH = kScreenHeight;

    // Dim overlay
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 160);
    SDL_FRect overlay{0.f, 0.f, static_cast<float>(winW), static_cast<float>(winH)};
    SDL_RenderFillRect(gRenderer, &overlay);

    // Text
    SDL_Color textColor{255, 255, 255, 255};
    gameOverLabel.loadFromRenderedText("PAUSED", textColor);
    resumeTexture.loadFromRenderedText("Resume", textColor);
    quitTexture.loadFromRenderedText("Quit", textColor);

    // Layout
    const int centerX = winW / 2;
    const int titleY  = winH / 4;

    const int titleW = gameOverLabel.getWidth();
    const int titleH = gameOverLabel.getHeight();

    const int afterTitle = 40;   // vertical gap under title
    const int optionsY   = titleY + titleH + afterTitle;

    // Measure options and center the pair as a group
    const int resumeW = resumeTexture.getWidth();
    const int resumeH = resumeTexture.getHeight();
    const int quitW   = quitTexture.getWidth();
    const int quitH   = quitTexture.getHeight();
    const int gapX    = 60; // horizontal gap between Resume and Quit

    const int groupW = resumeW + gapX + quitW;
    const int resumeX = centerX - groupW / 2;
    const int quitX   = resumeX + resumeW + gapX;

    // Selection rectangle around the chosen option
    const int padX = 12;
    const int padY = 8;

    int rectX, rectY, rectW, rectH;
    if (pauseMenuSelection == 0) {
        rectX = resumeX - padX;
        rectY = optionsY - padY;
        rectW = resumeW + padX * 2;
        rectH = resumeH + padY * 2;
    } else {
        rectX = quitX - padX;
        rectY = optionsY - padY;
        rectW = quitW + padX * 2;
        rectH = quitH + padY * 2;
    }

    SDL_SetRenderDrawColor(gRenderer, 49, 117, 73, 180);
    SDL_FRect selectRect{
        static_cast<float>(rectX),
        static_cast<float>(rectY),
        static_cast<float>(rectW),
        static_cast<float>(rectH)
    };
    SDL_RenderFillRect(gRenderer, &selectRect);

    // Draw title and options
    gameOverLabel.render(centerX - titleW / 2, titleY);
    resumeTexture.render(resumeX, optionsY);
    quitTexture.render(quitX, optionsY);

    // Restore previous blend mode
    SDL_SetRenderDrawBlendMode(gRenderer, prevBlend);
}

void quitToMenu() {
    // Reset game state
    currentState = GameState::MENU;
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
    pickPiece = drawPieceIndex();
    nextPickPiece = drawPieceIndex();
    currentPiece = pieceTypes[pickPiece];
    nextPiece = pieceTypes[nextPickPiece];
    currentPiece.x = boardWidth / 2;
    currentPiece.y = 0;
    score.loadFromRenderedText(std::to_string(scoreValue), { 0xFF, 0xFF, 0xFF, 0xFF });
    level.loadFromRenderedText(std::to_string(levelValue+1), { 0xFF, 0xFF, 0xFF, 0xFF });
    newPiece = false;
}

void renderWipeIntro(SDL_Renderer* renderer, int screenWidth, int screenHeight) {
    const int durationMs = 1200; // Animation duration
    Uint32 startTime = SDL_GetTicks();
    bool done = false;

    while (!done) {
        Uint32 now = SDL_GetTicks();
        float t = (now - startTime) / (float)durationMs;
        if (t > 1.0f) t = 1.0f;

        // Ease-out cubic: y = 1 - (1-t)^3
        float eased = 1.0f - pow(1.0f - t, 3);

        int wipeHeight = static_cast<int>(screenHeight * eased);

        renderBoardBlocks();
        renderUI();
        renderParticles();
        //SDL_RenderPresent(gRenderer);

        // Draw overlay
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_FRect overlay = {0.0f, static_cast<float>(wipeHeight), static_cast<float>(screenWidth), static_cast<float>(screenHeight - wipeHeight)};
        SDL_RenderFillRect(renderer, &overlay);

        SDL_RenderPresent(renderer);

        if (t >= 1.0f) done = true;
        SDL_Delay(16); // ~60 FPS
    }
}