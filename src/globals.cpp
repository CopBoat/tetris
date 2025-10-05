#include "globals.h"
#include "ltimer.h"
#include "tetris_utils.h"
#include "tPieceIcon.h"
#include "Pixeboy_ttf.h"
#include "splashLogo.h"
#include "Logo.h"
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

//define controller inputs
SDL_GamepadButton hardDropControllerBind = static_cast<SDL_GamepadButton>(SDL_GAMEPAD_BUTTON_SOUTH);
SDL_GamepadButton holdControllerBind = static_cast<SDL_GamepadButton>(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
SDL_GamepadButton rotateClockwiseControllerBind = static_cast<SDL_GamepadButton>(SDL_GAMEPAD_BUTTON_WEST);
SDL_GamepadButton rotateCounterClockwiseControllerBind = static_cast<SDL_GamepadButton>(SDL_GAMEPAD_BUTTON_EAST);

SDL_Gamepad* gActiveGamepad = nullptr;

static const char* buttonName(SDL_GamepadButton b) {
    switch (b) {
        case SDL_GAMEPAD_BUTTON_SOUTH: return "A";
        case SDL_GAMEPAD_BUTTON_EAST:  return "B";
        case SDL_GAMEPAD_BUTTON_WEST:  return "X";
        case SDL_GAMEPAD_BUTTON_NORTH: return "Y";
        case SDL_GAMEPAD_BUTTON_DPAD_UP: return "DPad Up";
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return "LB";
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return "RB";
        case SDL_GAMEPAD_BUTTON_LEFT_STICK: return "Left Stick";
        case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return "Right Stick";
        default: return "?";
    }
}

// Add helper to acquire first available gamepad
void AcquireFirstGamepadIfNone() {
    if (gActiveGamepad) return;
    int count = 0;
    SDL_JoystickID* ids = SDL_GetGamepads(&count);
    for (int i = 0; i < count; ++i) {
        SDL_Gamepad* pad = SDL_OpenGamepad(ids[i]);
        if (pad) {
            gActiveGamepad = pad;
            SDL_Log("Gamepad connected: %s", SDL_GetGamepadName(pad));
            break;
        }
    }
}

//define keyboard inputs
SDL_Keycode hardDropKey = SDLK_SPACE;
SDL_Keycode holdKey = SDLK_H;
SDL_Keycode rotateClockwiseKey = SDLK_UP;
SDL_Keycode rotateCounterClockwiseKey = SDLK_LCTRL;

void showSplashScreen()
{
    //SDL_Log("Showing splash screen...");

    // Load logo using SDL_image
    SDL_Texture* logoTex = nullptr;
    SDL_IOStream* io_stream = SDL_IOFromMem(assets_splashLogo_png, assets_splashLogo_png_len);
    SDL_Surface* splashSurface = IMG_Load_IO(io_stream, 1); // 1 = auto free rw
    if (splashSurface != nullptr) {
        logoTex = SDL_CreateTextureFromSurface(gRenderer, splashSurface);
        SDL_DestroySurface(splashSurface);
    }
    
    if (!logoTex) {
        SDL_Log("Splash logo failed to load: %s", SDL_GetError());
        return;
    }

    // SDL3: width/height outputs are float*
    float texW = 0.0f, texH = 0.0f;
    SDL_GetTextureSize(logoTex, &texW, &texH);
    int logoW = static_cast<int>(texW);
    int logoH = static_cast<int>(texH);

    const float maxW = kScreenWidth * 0.6f;
    const float maxH = kScreenHeight * 0.6f;
    float scale = 1.0f;
    if (logoW > 0 && logoH > 0) {
        scale = std::min(maxW / logoW, maxH / logoH);
        if (scale > 1.0f) scale = 1.0f;
    }
    int drawW = static_cast<int>(logoW * scale);
    int drawH = static_cast<int>(logoH * scale);
    SDL_FRect dstLogo = {
        (kScreenWidth - drawW) * 0.5f,
        (kScreenHeight - drawH) * 0.5f - 20.0f, // room for text
        static_cast<float>(drawW),
        static_cast<float>(drawH)
    };

    // Text (created after small delay so logo shows first)
    const char* splashText = "CopBoat's Version";
    SDL_Texture* textTex = nullptr;
    int textW = 0, textH = 0;

    // Enable blending for fade
    SDL_SetTextureBlendMode(logoTex, SDL_BLENDMODE_BLEND);

    // Timeline
    const Uint32 fadeInMS  = 900;
    const Uint32 holdMS    = 1200;
    const Uint32 fadeOutMS = 900;
    const Uint32 totalMS   = fadeInMS + holdMS + fadeOutMS;

    // Text appears shortly after the logo starts fading in
    const Uint32 textDelayMS = 400;

    const Uint32 start = SDL_GetTicks();

    while ((SDL_GetTicks() - start) < totalMS) {
        // Handle quit events so user can close during splash
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                if (textTex) SDL_DestroyTexture(textTex);
                SDL_DestroyTexture(logoTex);
                return;
            }
        }

        Uint32 t = SDL_GetTicks() - start;

        // Compute alpha with simple ease-in/out
        float alphaF = 1.0f;
        if (t < fadeInMS) {
            float x = (float)t / (float)fadeInMS;        // 0..1
            alphaF = x * x * (3 - 2 * x);                // smoothstep
        } else if (t < fadeInMS + holdMS) {
            alphaF = 1.0f;
        } else {
            float x = (float)(t - fadeInMS - holdMS) / (float)fadeOutMS; // 0..1
            x = std::min(std::max(x, 0.0f), 1.0f);
            float inv = 1.0f - x;
            alphaF = inv * inv * (3 - 2 * inv);          // smoothstep out
        }
        Uint8 alpha = static_cast<Uint8>(std::round(alphaF * 255.0f));

        // Create text texture after delay (once)
        if (!textTex && t >= textDelayMS) {
            SDL_Color white{255, 255, 255, 255};
            extern TTF_Font* gFont;
            if (gFont) {
                SDL_Surface* surf = TTF_RenderText_Blended(gFont, splashText, strlen(splashText), white);
                if (surf) {
                    textTex = SDL_CreateTextureFromSurface(gRenderer, surf);
                    SDL_DestroySurface(surf);
                    if (textTex) {
                        SDL_SetTextureBlendMode(textTex, SDL_BLENDMODE_BLEND);
                        // Query text size
                        float tw = 0.0f, th = 0.0f;
                        SDL_GetTextureSize(textTex, &tw, &th);
                        textW = static_cast<int>(tw);
                        textH = static_cast<int>(th);
                    }
                }
            }
        }

        // Apply alpha to textures
        SDL_SetTextureAlphaMod(logoTex, alpha);
        if (textTex) SDL_SetTextureAlphaMod(textTex, alpha);

        // Render
        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
        SDL_RenderClear(gRenderer);

        SDL_RenderTexture(gRenderer, logoTex, nullptr, &dstLogo);

        if (textTex) {
            SDL_FRect dstText = {
                (kScreenWidth - textW) * 0.5f,
                dstLogo.y + dstLogo.h + 16.0f,
                static_cast<float>(textW),
                static_cast<float>(textH)
            };
            if (dstText.y + dstText.h > kScreenHeight - 8) {
                dstText.y = kScreenHeight - 8 - dstText.h;
            }
            SDL_RenderTexture(gRenderer, textTex, nullptr, &dstText);
        }

        SDL_RenderPresent(gRenderer);
        SDL_Delay(16);
    }

    // Cleanup
    if (textTex) SDL_DestroyTexture(textTex);
    SDL_DestroyTexture(logoTex);
}

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

int kScreenWidthStandard = 800;
int kScreenHeightStandard = 800;

int kScreenWidthSmall = 640;
int kScreenHeightSmall = 640;

int kScreenWidthLarge = 960;
int kScreenHeightLarge = 960;

static void SetScreenSizeFromDesktop() {
    SDL_DisplayID display = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(display);
    if (!mode) {
        SDL_Log("GetDesktopDisplayMode failed: %s", SDL_GetError());
        return;
    }
    const int desktopH = (int)mode->h;
    if (desktopH <= 0) return;

     float scale = 1080.0f / desktopH ;
    if (scale < 1.0f) {
        scale = 1.0f;
    }

    kScreenWidthStandard = (int)(800 * scale);
    kScreenHeightStandard = (int)(800 * scale);

    kScreenWidthSmall = (int)(640 * scale);
    kScreenHeightSmall = (int)(640 * scale);

    kScreenWidthLarge = (int)(960 * scale);
    kScreenHeightLarge = (int)(960 * scale);

    SDL_Log("Set standard window size to %dx%d (monitor resolution %dx%d, calculated scale %.2f)", kScreenWidth, kScreenHeight, (int)mode->w, (int)mode->h, scale);
    SDL_Log("Set small window size to %dx%d", kScreenWidthSmall, kScreenHeightSmall);
    SDL_Log("Set large window size to %dx%d", kScreenWidthLarge, kScreenHeightLarge);
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

            const float aspect = static_cast<float>(kScreenWidth) / static_cast<float>(kScreenHeight);
            if (!SDL_SetWindowAspectRatio(gWindow, aspect, aspect)) {
                SDL_Log("Could not set window aspect ratio: %s", SDL_GetError());
            }

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

            AcquireFirstGamepadIfNone();   // hot-plug safe initial grab

            // Optional: tweak double-click timeout (ms)
            SDL_SetHint(SDL_HINT_MOUSE_DOUBLE_CLICK_TIME, "350");
            
        }
    }

    SetScreenSizeFromDesktop();
    SDL_SetWindowSize(gWindow, kScreenWidthStandard, kScreenHeightStandard);

    return success;
}

void pollForNewGamepad() {
    if (!gActiveGamepad) {
        AcquireFirstGamepadIfNone();
    }
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
    fullscreenEnabled = !isFullscreen;

    // Disable aspect lock in fullscreen; re-enable in windowed
    // const float aspect = static_cast<float>(kScreenWidth) / static_cast<float>(kScreenHeight);
    // if (isFullscreen) {
    //     SDL_SetWindowAspectRatio(gWindow, 0.0f, 0.0f); // no constraint
    // } else {
    //     SDL_SetWindowAspectRatio(gWindow, aspect, aspect); // lock ratio
    // }
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
    
    if (gridLinesEnabled) { // Draw grid lines
        SDL_SetRenderDrawColor(gRenderer, 40, 40, 40, 255);
    for (int x = 0; x <= boardWidth; ++x)
        SDL_RenderLine(gRenderer, x * blockSize, 0, x * blockSize, boardHeight * blockSize);
    for (int y = 0; y <= boardHeight; ++y)
        SDL_RenderLine(gRenderer, 0, y * blockSize, boardWidth * blockSize, y * blockSize);
    } 
    
    //draw line seperating the board and UI
    SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255 );
    SDL_RenderLine( gRenderer, 480, 0, 480, kScreenHeight );

    if (!gridLinesEnabled) {
        SDL_RenderLine( gRenderer, 0, 0, 0, kScreenHeight );
    }
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
LTexture optionsBlockGapLabel;
LTexture optionsPlacementPreviewLabel;

LTexture optionsTitleTexture2;
LTexture windowSizeLabel;
LTexture fullscreenLabel;
LTexture fullscreenTipLabel;


LTexture optionsTitleTexture3;
LTexture inputConfigHardDropLabel;
LTexture inputConfigHoldLabel;
LTexture inputConfigRotateCWLabel;
LTexture inputConfigRotateCCWLabel;
LTexture inputConfigKeyDirectionLabel;


int menuSelection = 0;

// Helper to move menu selection (0..2) with wrap-around
static inline void moveMenuSelection(int delta) {
    const int count = 3; // Play, Options, Exit
    menuSelection = (menuSelection + delta + count) % count;
}




// ---- Menu background bouncing pieces ----
struct MenuBouncePiece {
    const Piece* piece;
    float x, y;
    float vx, vy;
    float rot;       // (optional, not used for shape yet)
    float alpha;
    SDL_Color color;
    float cellSize;
};

static std::vector<MenuBouncePiece> gMenuPieces;
static bool gMenuPiecesInit = false;
static SDL_Texture* gMenuLogoTex = nullptr;

static SDL_Color kMenuColors[] = {
    {  80, 200, 255, 90},
    { 255, 210,  60, 90},
    { 200, 120, 255, 90},
    { 255, 140, 140, 90},
    { 140, 255, 160, 90},
    { 255, 100, 200, 90},
    { 120, 220, 120, 90},
};

static const Piece* kAllPiecesPtr[] = {
    &iPiece,&oPiece,&tPiece,&lPiece,&jPiece,&sPiece,&zPiece
};

static void initMenuBackgroundPieces() {
    gMenuPieces.clear();
    int count = 14;
    std::uniform_real_distribution<float> sx(30.f, kScreenWidth - 150.f);
    std::uniform_real_distribution<float> sy(20.f, kScreenHeight - 200.f);
    std::uniform_real_distribution<float> sv(-70.f, 70.f);
    std::uniform_real_distribution<float> sc(18.f, 30.f);

    for (int i = 0; i < count; ++i) {
        const Piece* p = kAllPiecesPtr[i % 7];
        MenuBouncePiece mb{
            p,
            sx(rng), sy(rng),
            0.f, 0.f,
            0.f,
            1.f,
            kMenuColors[i % 7],
            sc(rng)
        };
        // ensure non-zero velocity
        mb.vx = (sv(rng) >= 0 ? sv(rng) + 20.f : sv(rng) - 20.f);
        mb.vy = (sv(rng) >= 0 ? sv(rng) + 20.f : sv(rng) - 20.f);
        gMenuPieces.push_back(mb);
    }
    gMenuPiecesInit = true;
}

static void updateMenuBackgroundPieces(float dt) {
    for (auto& m : gMenuPieces) {
        m.x += m.vx * dt;
        m.y += m.vy * dt;

        float w = m.piece->width * m.cellSize;
        float h = m.piece->height * m.cellSize;

        if (m.x < 0.f) { m.x = 0.f; m.vx = -m.vx; }
        if (m.x + w > kScreenWidth) { m.x = kScreenWidth - w; m.vx = -m.vx; }
        if (m.y < 0.f) { m.y = 0.f; m.vy = -m.vy; }
        if (m.y + h > kScreenHeight) { m.y = kScreenHeight - h; m.vy = -m.vy; }
    }
}

static void renderMenuBackgroundPieces() {
    for (auto& m : gMenuPieces) {
        SDL_SetRenderDrawColor(gRenderer, m.color.r, m.color.g, m.color.b, m.color.a);
        for (int yy = 0; yy < m.piece->height; ++yy) {
            for (int xx = 0; xx < m.piece->width; ++xx) {
                if (m.piece->shape[yy][xx]) {
                    SDL_FRect r{
                        m.x + xx * m.cellSize,
                        m.y + yy * m.cellSize,
                        m.cellSize - 2.f,
                        m.cellSize - 2.f
                    };
                    SDL_RenderFillRect(gRenderer, &r);
                }
            }
        }
    }
}

// Lazy-load and cache logo texture (avoid per-frame load cost)
static SDL_Texture* getMenuLogoTexture() {
    if (gMenuLogoTex) return gMenuLogoTex;
    SDL_IOStream* io_stream = SDL_IOFromMem(assets_Logo_png, assets_Logo_png_len);
    SDL_Surface* logoSurface = IMG_Load_IO(io_stream, 1);
    if (logoSurface) {
        gMenuLogoTex = SDL_CreateTextureFromSurface(gRenderer, logoSurface);
        SDL_DestroySurface(logoSurface);
    }
    return gMenuLogoTex;
}

// ---- Falling menu background pieces (main menu only) ----
struct MenuFallingPiece {
    const Piece* piece;
    float x, y;
    float vy;
    float drift;      // small horizontal drift
    float cellSize;
    SDL_Color color;
    Uint8 alpha;
};

static std::vector<MenuFallingPiece> gMenuFallingPieces;
static bool gMenuFallingInit = false;

static void initMenuFallingPieces() {
    gMenuFallingPieces.clear();
    int count = 26; // denser field than options menu
    std::uniform_real_distribution<float> sx(0.f, (float)kScreenWidth - 120.f);
    std::uniform_real_distribution<float> sy(-600.f, (float)kScreenHeight);
    std::uniform_real_distribution<float> sc(14.f, 28.f);
    std::uniform_real_distribution<float> sv(40.f, 140.f);
    std::uniform_real_distribution<float> sd(-10.f, 10.f);
    std::uniform_int_distribution<int> ca(55, 95); // darker than before

    for (int i = 0; i < count; ++i) {
        const Piece* p = kAllPiecesPtr[i % 7];
        gMenuFallingPieces.push_back(MenuFallingPiece{
            p,
            sx(rng),
            sy(rng),
            sv(rng),
            sd(rng),
            sc(rng),
            kMenuColors[i % 7],
            (Uint8)ca(rng)
        });
    }
    gMenuFallingInit = true;
}

static void respawnFallingPiece(MenuFallingPiece& m) {
    std::uniform_real_distribution<float> sx(0.f, (float)kScreenWidth - 100.f);
    std::uniform_real_distribution<float> sc(14.f, 28.f);
    std::uniform_real_distribution<float> sv(55.f, 150.f);
    std::uniform_real_distribution<float> sd(-12.f, 12.f);
    std::uniform_int_distribution<int> ca(55, 95);
    m.cellSize = sc(rng);
    float w = m.piece->width * m.cellSize;
    m.x = std::min(std::max(0.f, sx(rng)), (float)kScreenWidth - w);
    m.y = -m.piece->height * m.cellSize - (float)(rand()%120);
    m.vy = sv(rng);
    m.drift = sd(rng);
    m.alpha = (Uint8)ca(rng);
}

static void updateMenuFallingPieces(float dt) {
    for (auto& m : gMenuFallingPieces) {
        m.y += m.vy * dt;
        m.x += m.drift * dt;
        float w = m.piece->width * m.cellSize;
        if (m.x < 0) m.x = 0;
        if (m.x + w > kScreenWidth) m.x = kScreenWidth - w;
        if (m.y > kScreenHeight + 10.f) {
            respawnFallingPiece(m);
        }
    }
}

static void renderMenuFallingPieces() {
    for (auto& m : gMenuFallingPieces) {
        SDL_SetRenderDrawColor(gRenderer, m.color.r, m.color.g, m.color.b, m.alpha);
        for (int yy = 0; yy < m.piece->height; ++yy) {
            for (int xx = 0; xx < m.piece->width; ++xx) {
                if (m.piece->shape[yy][xx]) {
                    SDL_FRect r{
                        m.x + xx * m.cellSize,
                        m.y + yy * m.cellSize,
                        m.cellSize - 2.f,
                        m.cellSize - 2.f
                    };
                    SDL_RenderFillRect(gRenderer, &r);
                }
            }
        }
    }
}


static void destroyMenuLogoTexture() {
    if (gMenuLogoTex) {
        SDL_DestroyTexture(gMenuLogoTex);
        gMenuLogoTex = nullptr;
    }
}

void renderMenu() {

    // SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    // SDL_RenderClear(gRenderer);


    static Uint64 lastTicks = SDL_GetTicksNS();
    Uint64 now = SDL_GetTicksNS();
    float dt = (now - lastTicks) / 1'000'000'000.0f;
    if (dt > 0.05f) dt = 0.05f;
    lastTicks = now;

    if (!gMenuFallingInit) initMenuFallingPieces();

    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    updateMenuFallingPieces(dt);
    renderMenuFallingPieces();

    // Dim overlay for readability
    SDL_BlendMode prev;
    SDL_GetRenderDrawBlendMode(gRenderer, &prev);
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 175);
    SDL_FRect dim{0.f,0.f,(float)kScreenWidth,(float)kScreenHeight};
    SDL_RenderFillRect(gRenderer, &dim);
    SDL_SetRenderDrawBlendMode(gRenderer, prev);

    SDL_Texture* logoTex = getMenuLogoTexture();
    if (logoTex) {
        float texW=0.f, texH=0.f;
        SDL_GetTextureSize(logoTex, &texW, &texH);
        float maxW = kScreenWidth * 0.55f;
        float maxH = kScreenHeight * 0.35f;
        float scale = std::min(maxW / texW, maxH / texH);
        SDL_FRect dst{
            (kScreenWidth - texW * scale) * 0.3f,
            kScreenHeight * 0.12f,
            texW * scale,
            texH * scale
        };
        SDL_RenderTexture(gRenderer, logoTex, nullptr, &dst);
    }

    // Textures regenerated each frame (quick, but could be cached)
    titleTexture.loadFromRenderedText("TETRIS", {255,255,255,255});
    playTexture.loadFromRenderedText("Play", {255,255,255,255});
    optionsTexture.loadFromRenderedText("Options", {255,255,255,255});
    exitTexture.loadFromRenderedText("Exit", {255,255,255,255});

    int rightX = kScreenWidth - 250;
    int centerY = kScreenHeight / 2;

    const int yPlay    = centerY - 10;
    const int yOptions = centerY + 40;
    const int yExit    = centerY + 90;

    const int xPlay    = rightX;
    const int xOptions = rightX - 15;
    const int xExit    = rightX - 5;

    const LTexture* selTex = (menuSelection == 0) ? &playTexture
                           : (menuSelection == 1) ? &optionsTexture
                           : &exitTexture;
    const int selX = (menuSelection == 0) ? xPlay
                   : (menuSelection == 1) ? xOptions
                   : xExit;
    const int selY = (menuSelection == 0) ? yPlay
                   : (menuSelection == 1) ? yOptions
                   : yExit;

    SDL_SetRenderDrawColor(gRenderer, 49,117,73,95);
    SDL_FRect selectRect{
        (float)(selX - 18),
        (float)(selY - 10),
        (float)(selTex->getWidth() + 36),
        (float)(selTex->getHeight() + 20)
    };
    SDL_RenderFillRect(gRenderer, &selectRect);

    playTexture.render(xPlay, yPlay);
    optionsTexture.render(xOptions, yOptions);
    exitTexture.render(xExit, yExit);
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
            moveMenuSelection(1);
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

int optionsTab = 0; // 0 = Game, 1 = Video, 2 = Input

int GameOptionsMenuSelection = 0;

bool gridLinesEnabled = true;

int blockGapSelection = 0;

float blockGapValues[] = {2.0f, 5.0f, 10.0f, 0.0f};

int placementPreviewSelection = 0; // 0 = ghost piece + highlights, 1 = ghost piece only, 2 = off

// Helper to move menu selection (0..3) with wrap-around
static inline void moveBlockGapSelection(int delta) {
    const int count = 4; // tab, grid, gap, preview, back
    blockGapSelection = (blockGapSelection + delta + count) % count;
}

// Helper to move menu selection (0..4) with wrap-around
static inline void moveGameOptionsMenuSelection(int delta) {
    const int count = 5; // tab, grid, gap, preview, back
    GameOptionsMenuSelection = (GameOptionsMenuSelection + delta + count) % count;
}

void renderGameOptions() {
    // SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    // SDL_RenderClear(gRenderer);


    static Uint64 lastTicks = SDL_GetTicksNS();
    Uint64 now = SDL_GetTicksNS();
    float dt = (now - lastTicks) / 1'000'000'000.0f;
    if (dt > 0.05f) dt = 0.05f;
    lastTicks = now;

    if (!gMenuPiecesInit) initMenuBackgroundPieces();

    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    updateMenuBackgroundPieces(dt);
    renderMenuBackgroundPieces();

    // --- dim overlay to darken background pieces ---
    SDL_BlendMode prev;
    SDL_GetRenderDrawBlendMode(gRenderer, &prev);
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 175); // raise alpha for stronger darkening
    SDL_FRect dim{0.f, 0.f, (float)kScreenWidth, (float)kScreenHeight};
    SDL_RenderFillRect(gRenderer, &dim);
    SDL_SetRenderDrawBlendMode(gRenderer, prev);
    // --- end dim overlay ---

    // Use logical size for layout; renderer scales to window
    const int winW = kScreenWidth;
    const int winH = kScreenHeight;
    int rightX = winW / 3.2;
    int centerY = winH / 32;

    //row positions
    const int yGame = centerY;
    const int yVideo = centerY;
    const int yInput = centerY;
    const int yGridLines = centerY + 150;
    const int yBlockGap = centerY + 250;
    const int yPlacementPreview = centerY + 350;
    const int yBack = centerY + 450;

    //x positions
    const int xGame = rightX - 60; //140
    const int xVideo = rightX + 120; //380
    const int xInput = rightX + 300; //500
    const int xGridLines = rightX - 150;
    const int xBlockGap = rightX - 150;
    const int xPlacementPreview = rightX - 150;
    const int xBack = rightX - 150;

    optionsTitleTexture.loadFromRenderedText("Game", {255,255,255,255});
    optionsTitleTexture2.loadFromRenderedText("Video", {255,255,255,255});
    optionsTitleTexture3.loadFromRenderedText("Input", {255,255,255,255});
                optionsGridLabel.loadFromRenderedText( gridLinesEnabled ? "Grid lines        < ON >" : "Grid lines        < OFF >", {255,255,255,255});
            optionsBlockGapLabel.loadFromRenderedText((blockGapSelection == 0) ? "Block Gap         < 2px >" 
                                                        : (blockGapSelection == 1) ? "Block Gap         < 5px >" 
                                                        : (blockGapSelection == 2) ? "Block Gap         < 10px >"
                                                        : "Block Gap         < 0px >" , {255,255,255,255});
    optionsPlacementPreviewLabel.loadFromRenderedText( (placementPreviewSelection == 0) ? "Placement Preview    < Ghost Piece & Highlights >"
                                                        : (placementPreviewSelection == 1) ? "Placement Preview    < Ghost Piece Only >"
                                                        : "Placement Preview    < None >" , {255,255,255,255});
    backTexture.loadFromRenderedText("Return", {255,255,255,255});

    // Selection rectangle around the chosen option
    const LTexture* selTex = (GameOptionsMenuSelection == 0) ? &optionsTitleTexture
                           : (GameOptionsMenuSelection == 1) ? &optionsGridLabel
                           : (GameOptionsMenuSelection == 2) ? &optionsBlockGapLabel
                           : (GameOptionsMenuSelection == 3) ? &optionsPlacementPreviewLabel
                           : &backTexture;
    const int selX = (GameOptionsMenuSelection == 0) ? xGame
                   : (GameOptionsMenuSelection == 1) ? xGridLines
                   : (GameOptionsMenuSelection == 2) ? xBlockGap
                   : (GameOptionsMenuSelection == 3) ? xPlacementPreview
                   : xBack;
    const int selY = (GameOptionsMenuSelection == 0) ? yGame
                   : (GameOptionsMenuSelection == 1) ? yGridLines
                   : (GameOptionsMenuSelection == 2) ? yBlockGap
                   : (GameOptionsMenuSelection == 3) ? yPlacementPreview
                   : yBack;

    const int padX = 18;
    const int padY = 10;
    

    //rect to show current tab
    SDL_SetRenderDrawColor(gRenderer, 128, 128, 128, 70); 
    SDL_FRect tabRect{122, 10, 78, 32};
    SDL_RenderFillRect(gRenderer, &tabRect);
    
    SDL_SetRenderDrawColor(gRenderer, 49, 117, 73, 70);
    SDL_FRect selectRect{
        static_cast<float>(selX - padX),
        static_cast<float>(selY - padY),
        static_cast<float>(selTex->getWidth() + padX * 2 - 2),
        static_cast<float>(selTex->getHeight() + padY * 2 - 2)
    };
    SDL_RenderFillRect(gRenderer, &selectRect);    

    //draw text
    optionsTitleTexture.render(xGame, yGame);
    optionsTitleTexture2.render(xVideo, yVideo);
    optionsTitleTexture3.render(xInput, yInput);
    optionsGridLabel.render(xGridLines, yGridLines);
    optionsBlockGapLabel.render(xBlockGap, yBlockGap);
    optionsPlacementPreviewLabel.render(xPlacementPreview, yPlacementPreview);
    backTexture.render(xBack, yBack);

}

int handleGameOptionsMenuEvent(const SDL_Event& e) {
    //handke keyboard input for menu navigation
    if (e.type == SDL_EVENT_KEY_DOWN) {
        if (e.key.key == SDLK_UP) {
            moveGameOptionsMenuSelection(-1);
        } else if (e.key.key == SDLK_DOWN) {
            moveGameOptionsMenuSelection(1);
        }else if (e.key.key == SDLK_LEFT) {
            if (GameOptionsMenuSelection == 0) { // Game tab
                optionsTab = 2;
            } else if (GameOptionsMenuSelection == 1) { // Grid lines
                gridLinesEnabled = !gridLinesEnabled;
            } else if (GameOptionsMenuSelection == 2) { // Block gap
                blockGapSelection = (blockGapSelection - 1 + 4) % 4;
                spacing = blockGapValues[blockGapSelection];
            } else if (GameOptionsMenuSelection == 3) { // Placement preview
                placementPreviewSelection = (placementPreviewSelection - 1 + 3) % 3;
            }
        } else if (e.key.key == SDLK_RIGHT) {
            if (GameOptionsMenuSelection == 0) { // Game tab
                optionsTab = 1;
            } else if (GameOptionsMenuSelection == 1) { // Grid lines
                gridLinesEnabled = !gridLinesEnabled;
            } else if (GameOptionsMenuSelection == 2) { // Block gap
                blockGapSelection = (blockGapSelection + 1) % 4;
                spacing = blockGapValues[blockGapSelection];
            } else if (GameOptionsMenuSelection == 3) { // Placement preview
                placementPreviewSelection = (placementPreviewSelection + 1) % 3;
            }

        } else if (e.key.key == SDLK_ESCAPE) {
            GameOptionsMenuSelection = 0;
            return 4; // Return to main menu
        } else if (e.key.key == SDLK_RETURN || e.key.key == SDLK_KP_ENTER) {
            GameOptionsMenuSelection = 0;
            return 4; // Return the selected option
        }
    }

    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP) {
            moveGameOptionsMenuSelection(-1);
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
            moveGameOptionsMenuSelection(1);
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
            if (GameOptionsMenuSelection == 0) { // Game tab
                optionsTab = 1;
            } else if (GameOptionsMenuSelection == 1) { // Grid lines
                gridLinesEnabled = !gridLinesEnabled;
            } else if (GameOptionsMenuSelection == 2) { // Block gap
                blockGapSelection = (blockGapSelection + 1) % 4;
                spacing = blockGapValues[blockGapSelection];
            } else if (GameOptionsMenuSelection == 3) { // Placement preview
                placementPreviewSelection = (placementPreviewSelection + 1) % 3;
            }
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_LEFT) {
            if (GameOptionsMenuSelection == 0) { // Game tab
                optionsTab = 2;
            } else if (GameOptionsMenuSelection == 1) { // Grid lines
                gridLinesEnabled = !gridLinesEnabled;
            } else if (GameOptionsMenuSelection == 2) { // Block gap
                blockGapSelection = (blockGapSelection - 1 + 4) % 4;
                spacing = blockGapValues[blockGapSelection];
            } else if (GameOptionsMenuSelection == 3) { // Placement preview
                placementPreviewSelection = (placementPreviewSelection - 1 + 3) % 3;
            }
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_EAST) {
            GameOptionsMenuSelection = 0;
            return 4;
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
            if (GameOptionsMenuSelection == 4) { // Back
                GameOptionsMenuSelection = 0;
                return 4; // Return to main menu
            } 
            
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER) {
            GameOptionsMenuSelection = 0;
            optionsTab = 2;
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) {
            GameOptionsMenuSelection = 0;
            optionsTab = 1;
        }
    }

    // Analog stick up/down for menu
    if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
        const int v = e.gaxis.value;
        if (v <= -kAxisPress) {
            if (!menuAxisUpHeld) {
                moveGameOptionsMenuSelection(-1);
                menuAxisUpHeld = true;
                menuAxisDownHeld = false;
            }
        } else if (v >= kAxisPress) {
            if (!menuAxisDownHeld) {
                moveGameOptionsMenuSelection(1);
                menuAxisDownHeld = true;
                menuAxisUpHeld = false;
            }
        } else if (std::abs(v) < kAxisRelease) {
            menuAxisUpHeld = false;
            menuAxisDownHeld = false;
        }
    } 
    
    if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
        const int v = e.gaxis.value;
        if (v <= -kAxisPress) {
            if (!pauseAxisLeftHeld) {
                if (GameOptionsMenuSelection == 0) { // Game tab
                    optionsTab = 2;
                } else if (GameOptionsMenuSelection == 1) { // Grid lines
                    gridLinesEnabled = !gridLinesEnabled;
                } else if (GameOptionsMenuSelection == 2) { // Block gap
                    blockGapSelection = (blockGapSelection - 1 + 4) % 4;
                    spacing = blockGapValues[blockGapSelection];
                } else if (GameOptionsMenuSelection == 3) { // Placement preview
                    placementPreviewSelection = (placementPreviewSelection - 1 + 3) % 3;
                }
                pauseAxisLeftHeld = true;
                pauseAxisRightHeld = false;
            }
        } else if (v >= kAxisPress) {
            if (!pauseAxisRightHeld) {
                if (GameOptionsMenuSelection == 0) { // Game tab
                    optionsTab = 1;
                } else if (GameOptionsMenuSelection == 1) { // Grid lines
                    gridLinesEnabled = !gridLinesEnabled;
                } else if (GameOptionsMenuSelection == 2) { // Block gap
                    blockGapSelection = (blockGapSelection + 1) % 4;
                    spacing = blockGapValues[blockGapSelection];
                } else if (GameOptionsMenuSelection == 3) { // Placement preview
                    placementPreviewSelection = (placementPreviewSelection + 1) % 3;
                }
                pauseAxisRightHeld = true;
                pauseAxisLeftHeld = false;
            }
        } else if (std::abs(v) < kAxisRelease) {
            pauseAxisLeftHeld = false;
            pauseAxisRightHeld = false;
        }
    }

    return -1; // No selection made
}

int VideoOptionsMenuSelection = 0;

int WindowSizeMenuSelection = 0;

bool fullscreenEnabled = false;

void applyWindowSize(int selection) {

    if (fullscreenEnabled) {
        toggleFullscreen();
        fullscreenEnabled = false; 
    }

    // Center the window on screen after resizing
    //SDL_SetWindowPosition(gWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    if (WindowSizeMenuSelection == 0) { // Standard
        SDL_SetWindowSize(gWindow, kScreenWidthStandard, kScreenHeightStandard);
    } else if (WindowSizeMenuSelection == 1) { // Large
        SDL_SetWindowSize(gWindow, kScreenWidthLarge, kScreenHeightLarge);
    } else if (WindowSizeMenuSelection == 2) { // Small
        SDL_SetWindowSize(gWindow, kScreenWidthSmall, kScreenHeightSmall);
    }
    
}

// Helper to move menu selection (0..4) with wrap-around
static inline void moveVideoOptionsMenuSelection(int delta) {
    const int count = 4; // tab, window-size, fullscreen, back
    VideoOptionsMenuSelection = (VideoOptionsMenuSelection + delta + count) % count;
}

void renderVideoOptions() {
    // SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    // SDL_RenderClear(gRenderer);


    static Uint64 lastTicks = SDL_GetTicksNS();
    Uint64 now = SDL_GetTicksNS();
    float dt = (now - lastTicks) / 1'000'000'000.0f;
    if (dt > 0.05f) dt = 0.05f;
    lastTicks = now;

    if (!gMenuPiecesInit) initMenuBackgroundPieces();

    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    updateMenuBackgroundPieces(dt);
    renderMenuBackgroundPieces();

    // --- dim overlay to darken background pieces ---
    SDL_BlendMode prev;
    SDL_GetRenderDrawBlendMode(gRenderer, &prev);
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 175); // raise alpha for stronger darkening
    SDL_FRect dim{0.f, 0.f, (float)kScreenWidth, (float)kScreenHeight};
    SDL_RenderFillRect(gRenderer, &dim);
    SDL_SetRenderDrawBlendMode(gRenderer, prev);
    // --- end dim overlay ---

    // Use logical size for layout; renderer scales to window
    const int winW = kScreenWidth;
    const int winH = kScreenHeight;
    int rightX = winW / 3.2;
    int centerY = winH / 32;

    //row positions
    const int yGame = centerY;
    const int yVideo = centerY;
    const int yInput = centerY;
    const int yWindowSize = centerY + 150;
    const int yfullscreen = centerY + 250;
    const int yfullscreenTip = centerY + 280;
    const int yBack = centerY + 450;

    //x positions
    const int xGame = rightX - 60; //140
    const int xVideo = rightX + 120; //380
    const int xInput = rightX + 300; //500
    const int xWindowSize = rightX - 150;
    const int xfullscreen = rightX - 150;
    const int xfullscreenTip = rightX - 150;
    const int xBack = rightX - 150;

    optionsTitleTexture.loadFromRenderedText("Game", {255,255,255,255});
    optionsTitleTexture2.loadFromRenderedText("Video", {255,255,255,255});
    optionsTitleTexture3.loadFromRenderedText("Input", {255,255,255,255});
    windowSizeLabel.loadFromRenderedText( (WindowSizeMenuSelection == 0 ) ? "Window Size < Standard >"
                                            : (WindowSizeMenuSelection == 1) ? "Window Size < Large >"
                                            : "Window Size < Small >" , {255,255,255,255});
    fullscreenLabel.loadFromRenderedText( (!fullscreenEnabled) ? "Fullscreen < OFF >"
                                            : "Fullscreen < ON >", {255,255,255,255});
    fullscreenTipLabel.loadFromRenderedText("*Toggle at any time by double clicking on the window*", {255,255,255,255});
    backTexture.loadFromRenderedText("Return", {255,255,255,255});

    // Selection rectangle around the chosen option
    const LTexture* selTex = (VideoOptionsMenuSelection == 0) ? &optionsTitleTexture2
                           : (VideoOptionsMenuSelection == 1) ? &windowSizeLabel
                           : (VideoOptionsMenuSelection == 2) ? &fullscreenLabel
                           : &backTexture;
    const int selX = (VideoOptionsMenuSelection == 0) ? xVideo
                   : (VideoOptionsMenuSelection == 1) ? xWindowSize
                   : (VideoOptionsMenuSelection == 2) ? xfullscreen
                   : xBack;
    const int selY = (VideoOptionsMenuSelection == 0) ? yVideo
                   : (VideoOptionsMenuSelection == 1) ? yWindowSize
                   : (VideoOptionsMenuSelection == 2) ? yfullscreen
                   : yBack;

    const int padX = 18;
    const int padY = 10;
    

    //rect to show current tab
    SDL_SetRenderDrawColor(gRenderer, 128, 128, 128, 70); 
    SDL_FRect tabRect{302, 10, 89, 32};
    SDL_RenderFillRect(gRenderer, &tabRect);
    
    SDL_SetRenderDrawColor(gRenderer, 49, 117, 73, 70);
    SDL_FRect selectRect{
        static_cast<float>(selX - padX),
        static_cast<float>(selY - padY),
        static_cast<float>(selTex->getWidth() + padX * 2 - 2),
        static_cast<float>(selTex->getHeight() + padY * 2 - 2)
    };
    SDL_RenderFillRect(gRenderer, &selectRect); 


    //draw text
    optionsTitleTexture.render(xGame, yGame);
    optionsTitleTexture2.render(xVideo, yVideo);
    optionsTitleTexture3.render(xInput, yInput);
    windowSizeLabel.render(xWindowSize, yWindowSize);
    fullscreenLabel.render(xfullscreen, yfullscreen);
    fullscreenTipLabel.render(xfullscreenTip, yfullscreenTip);
    backTexture.render(xBack, yBack);
}

int handleVideoOptionsMenuEvent(const SDL_Event& e) {
    //handke keyboard input for menu navigation
    if (e.type == SDL_EVENT_KEY_DOWN) {
        if (e.key.key == SDLK_UP) {
            moveVideoOptionsMenuSelection(-1);
        } else if (e.key.key == SDLK_DOWN) {
            moveVideoOptionsMenuSelection(1);
        }else if (e.key.key == SDLK_LEFT) {
            if (VideoOptionsMenuSelection == 0) { // Game tab
                optionsTab = 0;
            } else if (VideoOptionsMenuSelection == 1) { // window size
                WindowSizeMenuSelection = (WindowSizeMenuSelection - 1 + 3) % 3;
                applyWindowSize(WindowSizeMenuSelection);
            } else if (VideoOptionsMenuSelection == 2) { // full screen
                fullscreenEnabled = !fullscreenEnabled;
                toggleFullscreen();
            }
        } else if (e.key.key == SDLK_RIGHT) {
            if (VideoOptionsMenuSelection == 0) { // Game tab
                optionsTab = 2;
            } else if (VideoOptionsMenuSelection == 1) { // window size
                WindowSizeMenuSelection = (WindowSizeMenuSelection + 1) % 3;
                applyWindowSize(WindowSizeMenuSelection);
            } else if (VideoOptionsMenuSelection == 2) { // full screen
                fullscreenEnabled = !fullscreenEnabled;
                toggleFullscreen();
            }
        } else if (e.key.key == SDLK_ESCAPE) {
            VideoOptionsMenuSelection = 0;
            return 3; // Return to main menu
        } else if (e.key.key == SDLK_RETURN || e.key.key == SDLK_KP_ENTER) {
            VideoOptionsMenuSelection = 0;
            return 3; // Return the selected option
        }
    }

    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP) {
            moveVideoOptionsMenuSelection(-1);
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
            moveVideoOptionsMenuSelection(1);
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
            if (VideoOptionsMenuSelection == 0) { // Game tab
                optionsTab = 2;
            } else if (VideoOptionsMenuSelection == 1) { // window size
                WindowSizeMenuSelection = (WindowSizeMenuSelection + 1) % 3;
                applyWindowSize(WindowSizeMenuSelection);
            } else if (VideoOptionsMenuSelection == 2) { // full screen
                fullscreenEnabled = !fullscreenEnabled;
                toggleFullscreen();
            }
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_LEFT) {
            if (VideoOptionsMenuSelection == 0) { // Game tab
                    optionsTab = 0;
                } else if (VideoOptionsMenuSelection == 1) { // window size
                    WindowSizeMenuSelection = (WindowSizeMenuSelection - 1 + 3) % 3;
                    applyWindowSize(WindowSizeMenuSelection);
                } else if (VideoOptionsMenuSelection == 2) { // full screen
                    fullscreenEnabled = !fullscreenEnabled;
                    toggleFullscreen();
                }
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_EAST) {
            VideoOptionsMenuSelection = 0;
            return 3;
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
            if (VideoOptionsMenuSelection == 3) { // Back
                VideoOptionsMenuSelection = 0;
                return 3; // Return to main menu
            }
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER) {
            VideoOptionsMenuSelection = 0;
            optionsTab = 0;
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) {
            VideoOptionsMenuSelection = 0;
            optionsTab = 2;
        }
    }

    // Analog stick up/down for menu
    if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
        const int v = e.gaxis.value;
        if (v <= -kAxisPress) {
            if (!menuAxisUpHeld) {
                moveVideoOptionsMenuSelection(-1);
                menuAxisUpHeld = true;
                menuAxisDownHeld = false;
            }
        } else if (v >= kAxisPress) {
            if (!menuAxisDownHeld) {
                moveVideoOptionsMenuSelection(1);
                menuAxisDownHeld = true;
                menuAxisUpHeld = false;
            }
        } else if (std::abs(v) < kAxisRelease) {
            menuAxisUpHeld = false;
            menuAxisDownHeld = false;
        }
    } 
    
    if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
        const int v = e.gaxis.value;
        if (v <= -kAxisPress) {
            if (!pauseAxisLeftHeld) {
                if (VideoOptionsMenuSelection == 0) { // Game tab
                    optionsTab = 0;
                } else if (VideoOptionsMenuSelection == 1) { // window size
                    WindowSizeMenuSelection = (WindowSizeMenuSelection - 1 + 3) % 3;
                    applyWindowSize(WindowSizeMenuSelection);
                } else if (VideoOptionsMenuSelection == 2) { // full screen
                    fullscreenEnabled = !fullscreenEnabled;
                    toggleFullscreen();
                }
                pauseAxisLeftHeld = true;
                pauseAxisRightHeld = false;
            }
        } else if (v >= kAxisPress) {
            if (!pauseAxisRightHeld) {
                if (VideoOptionsMenuSelection == 0) { // Game tab
                    optionsTab = 2;
                } else if (VideoOptionsMenuSelection == 1) { // window size
                    WindowSizeMenuSelection = (WindowSizeMenuSelection + 1) % 3;
                    applyWindowSize(WindowSizeMenuSelection);
                } else if (VideoOptionsMenuSelection == 2) { // full screen
                    fullscreenEnabled = !fullscreenEnabled;
                    toggleFullscreen();
                }
                pauseAxisRightHeld = true;
                pauseAxisLeftHeld = false;
            }
        } else if (std::abs(v) < kAxisRelease) {
            pauseAxisLeftHeld = false;
            pauseAxisRightHeld = false;
        }
    }

    return -1; // No selection made
}

int InputOptionsMenuSelection = 0;

bool waitingForKeyRebind = false;
int keyToRebind = -1; // 0 = direction, 1 = hard drop, 2 = hold, 3 = rotate CW, 4 = rotate CCW
bool invalidRebindAttempt = false;

static void applyRebind(const SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode k = e.key.key;
        if (keyToRebind == 1) hardDropKey  = k;
        else if (keyToRebind == 2) holdKey = k;
        else if (keyToRebind == 3) rotateClockwiseKey = k;
        else if (keyToRebind == 4) rotateCounterClockwiseKey = k;
    } else if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        SDL_GamepadButton b = (SDL_GamepadButton)e.gbutton.button;
        if (keyToRebind == 1) {
            hardDropControllerBind = b;
        } else if (keyToRebind == 2) {
            holdControllerBind = b;
        } else if (keyToRebind == 3) {
            rotateClockwiseControllerBind = b;
        } else if (keyToRebind == 4) {
            rotateCounterClockwiseControllerBind = b;
        }
        // Extend for others if you later support controller binds for them
    }
}

int validateKeyRebind(SDL_Event e) {
    SDL_Log("Validating key rebind...");
    // Prevent binding to keys already in use
    if (e.type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode newKey = e.key.key;
        if (newKey == SDLK_LEFT || newKey == SDLK_RIGHT || newKey == SDLK_DOWN || newKey == SDLK_ESCAPE) {
            return 0; // invalid
        }
        return 1; // valid
    } else if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        SDL_GamepadButton newButton = static_cast<SDL_GamepadButton>(e.gbutton.button);
        if (newButton == SDL_GAMEPAD_BUTTON_DPAD_LEFT|| newButton == SDL_GAMEPAD_BUTTON_DPAD_RIGHT|| newButton == SDL_GAMEPAD_BUTTON_DPAD_DOWN || newButton == SDL_GAMEPAD_BUTTON_START) {
            return 0; // invalid
        }
        SDL_Log("Rebinding to button %d", newButton);
        return 1; // valid
    }

    return -1; // invalid by default (not recognized)
}

static bool handleRebindCapture(const SDL_Event& e) {
    // Cancel keys/buttons
    bool cancel = (e.type == SDL_EVENT_KEY_DOWN  && e.key.key == SDLK_ESCAPE) ||
                  (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN && 
                   (e.gbutton.button == SDL_GAMEPAD_BUTTON_START));
    if (cancel) {
        SDL_Log("Rebind cancelled");
        waitingForKeyRebind = false;
        keyToRebind = -1;
        return true;
    }

    // Only accept first key/button press (ignore navigation inputs that are disallowed)
    if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        if (validateKeyRebind(e) == 1) {
            SDL_Log("Rebind valid, applying...");
            applyRebind(e);
            waitingForKeyRebind = false;
            keyToRebind = -1;
            return true;
        } else {
            SDL_Log("Invalid rebind attempt");
            invalidRebindAttempt = true;
            waitingForKeyRebind = false;
            keyToRebind = -1;
            return false;
        }
    }

    //SDL_Log("Rebind capture received unhandled event type %d", e.type);
    return false;
}





// Helper to move menu selection (0..5) with wrap-around
static inline void moveInputOptionsMenuSelection(int delta) {
    const int count = 6; // tab, window-size, fullscreen, back
    InputOptionsMenuSelection = (InputOptionsMenuSelection + delta + count) % count;
}

void renderInputOptions() {
    // SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    // SDL_RenderClear(gRenderer);

    static Uint64 lastTicks = SDL_GetTicksNS();
    Uint64 now = SDL_GetTicksNS();
    float dt = (now - lastTicks) / 1'000'000'000.0f;
    if (dt > 0.05f) dt = 0.05f;
    lastTicks = now;

    if (!gMenuPiecesInit) initMenuBackgroundPieces();

    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    updateMenuBackgroundPieces(dt);
    renderMenuBackgroundPieces();

    // --- dim overlay to darken background pieces ---
    SDL_BlendMode prev;
    SDL_GetRenderDrawBlendMode(gRenderer, &prev);
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 175); // raise alpha for stronger darkening
    SDL_FRect dim{0.f, 0.f, (float)kScreenWidth, (float)kScreenHeight};
    SDL_RenderFillRect(gRenderer, &dim);
    SDL_SetRenderDrawBlendMode(gRenderer, prev);
    // --- end dim overlay ---

    // Use logical size for layout; renderer scales to window
    const int winW = kScreenWidth;
    const int winH = kScreenHeight;
    int rightX = winW / 3.2;
    int centerY = winH / 32;

    //row positions
    const int yGame = centerY;
    const int yVideo = centerY;
    const int yInput = centerY;
    const int yKeyDir = centerY + 150;
    const int yHardDrop = centerY + 200;
    const int yHold = centerY + 250;
    const int yRCW = centerY + 300;
    const int yRCCW = centerY + 350;
    const int yBack = centerY + 450;

    //x positions
    const int xGame = rightX - 60; //140
    const int xVideo = rightX + 120; //380
    const int xInput = rightX + 300; //500
    const int xKeyDir = rightX - 150;
    const int xHardDrop = rightX - 150;
    const int xHold = rightX - 150;
    const int xRCW = rightX - 150;
    const int xRCCW = rightX - 150;
    const int xBack = rightX - 150;

    char hardDropLine[128];
    snprintf(hardDropLine, sizeof(hardDropLine),
             "Hard Drop: (%s) <> (%s)",
             buttonName(hardDropControllerBind),
             SDL_GetKeyName(hardDropKey));

    char holdLine[128];
    snprintf(holdLine, sizeof(holdLine),
             "Hold: (%s) <> (%s)",
             buttonName(holdControllerBind),
             SDL_GetKeyName(holdKey));
    
    char rotateCWLine[128];
    snprintf(rotateCWLine, sizeof(rotateCWLine),
             "Rotate Clockwise: (%s) <> (%s)",
             buttonName(rotateClockwiseControllerBind),
             SDL_GetKeyName(rotateClockwiseKey));

    char rotateCCWLine[128];
    snprintf(rotateCCWLine, sizeof(rotateCCWLine),
             "Rotate Counter Clockwise: (%s) <> (%s)",
             buttonName(rotateCounterClockwiseControllerBind),
             SDL_GetKeyName(rotateCounterClockwiseKey));

    std::string bindMessage = "Press a key or button to rebind...";

    optionsTitleTexture.loadFromRenderedText("Game", {255,255,255,255});
    optionsTitleTexture2.loadFromRenderedText("Video", {255,255,255,255});
    optionsTitleTexture3.loadFromRenderedText("Input", {255,255,255,255});
    inputConfigKeyDirectionLabel.loadFromRenderedText((invalidRebindAttempt) ? "*key/button is reserved, try again*"
                                                    : (waitingForKeyRebind) ? "*Press ESC or Start to Cancel*" 
                                                    : "*Select an option below to change binding*", {255,255,255,255});
    inputConfigHardDropLabel.loadFromRenderedText((waitingForKeyRebind && keyToRebind == 1 ) ? bindMessage : hardDropLine, {255,255,255,255});
    inputConfigHoldLabel.loadFromRenderedText((waitingForKeyRebind && keyToRebind == 2 ) ? bindMessage : holdLine, {255,255,255,255});
    inputConfigRotateCWLabel.loadFromRenderedText((waitingForKeyRebind && keyToRebind == 3 ) ? bindMessage : rotateCWLine, {255,255,255,255});
    inputConfigRotateCCWLabel.loadFromRenderedText((waitingForKeyRebind && keyToRebind == 4 ) ? bindMessage : rotateCCWLine, {255,255,255,255});
    backTexture.loadFromRenderedText("Return", {255,255,255,255});

    // Selection rectangle around the chosen option
    const LTexture* selTex = (InputOptionsMenuSelection == 0) ? &optionsTitleTexture3
                           : (InputOptionsMenuSelection == 1) ? &inputConfigHardDropLabel
                           : (InputOptionsMenuSelection == 2) ? &inputConfigHoldLabel
                           : (InputOptionsMenuSelection == 3) ? &inputConfigRotateCWLabel
                           : (InputOptionsMenuSelection == 4) ? &inputConfigRotateCCWLabel
                           : &backTexture;
    const int selX = (InputOptionsMenuSelection == 0) ? xInput
                   : (InputOptionsMenuSelection == 1) ? xHardDrop
                   : (InputOptionsMenuSelection == 2) ? xHold
                   : (InputOptionsMenuSelection == 3) ? xRCW
                   : (InputOptionsMenuSelection == 4) ? xRCCW
                   : xBack;
    const int selY = (InputOptionsMenuSelection == 0) ? yInput
                   : (InputOptionsMenuSelection == 1) ? yHardDrop
                   : (InputOptionsMenuSelection == 2) ? yHold
                   : (InputOptionsMenuSelection == 3) ? yRCW
                   : (InputOptionsMenuSelection == 4) ? yRCCW
                   : yBack;

    const int padX = 18;
    const int padY = 10;
    

    //rect to show current tab
    SDL_SetRenderDrawColor(gRenderer, 128, 128, 128, 70); 
    SDL_FRect tabRect{482, 10, 89, 32};
    SDL_RenderFillRect(gRenderer, &tabRect);
    
    SDL_SetRenderDrawColor(gRenderer, 49, 117, 73, 70);
    SDL_FRect selectRect{
        static_cast<float>(selX - padX),
        static_cast<float>(selY - padY),
        static_cast<float>(selTex->getWidth() + padX * 2 - 2),
        static_cast<float>(selTex->getHeight() + padY * 2 - 2)
    };
    SDL_RenderFillRect(gRenderer, &selectRect); 

    //draw text
    optionsTitleTexture.render(xGame, yGame);
    optionsTitleTexture2.render(xVideo, yVideo);
    optionsTitleTexture3.render(xInput, yInput);
    inputConfigKeyDirectionLabel.render(xKeyDir, yKeyDir);
    inputConfigHardDropLabel.render(xHardDrop, yHardDrop);
    inputConfigHoldLabel.render(xHold, yHold);
    inputConfigRotateCWLabel.render(xRCW, yRCW);
    inputConfigRotateCCWLabel.render(xRCCW, yRCCW);
    backTexture.render(xBack, yBack);
}



int handleInputOptionsMenuEvent(const SDL_Event& e) {

    if (waitingForKeyRebind) {
        invalidRebindAttempt = false;
        handleRebindCapture(e);
        return -1; // Stay on this screen
    }

    //handke keyboard input for menu navigation
    if (e.type == SDL_EVENT_KEY_DOWN) {
        if (e.key.key == SDLK_UP) {
            moveInputOptionsMenuSelection(-1);
        } else if (e.key.key == SDLK_DOWN) {
            moveInputOptionsMenuSelection(1);
        }else if (e.key.key == SDLK_LEFT) {
            if (InputOptionsMenuSelection == 0) { // Game tab
                optionsTab = 1;
            }
        } else if (e.key.key == SDLK_RIGHT) {
            if (InputOptionsMenuSelection == 0) { // Game tab
                optionsTab = 0;
            } 
        } else if (e.key.key == SDLK_ESCAPE) {
            InputOptionsMenuSelection = 0;
            return 5; // Return to main menu
        } else if (e.key.key == SDLK_RETURN || e.key.key == SDLK_KP_ENTER) {
            if (InputOptionsMenuSelection == 5) {
                InputOptionsMenuSelection = 0;
                return 5; // Return to main menu
            } else if (InputOptionsMenuSelection >= 1 && InputOptionsMenuSelection <=4) {
                // Start rebind process
                
                waitingForKeyRebind = true;
                keyToRebind = InputOptionsMenuSelection;
                
            }
            
        }
    }

    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP) {
            moveInputOptionsMenuSelection(-1);
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
            moveInputOptionsMenuSelection(1);
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
            if (InputOptionsMenuSelection == 0) { // Game tab
                optionsTab = 0;
            } 
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_LEFT) {
            if (InputOptionsMenuSelection == 0) { // Game tab
                optionsTab = 1;
            }
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_EAST) {
            InputOptionsMenuSelection = 0;
            return 5;
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
            if (InputOptionsMenuSelection == 5) { // Back
                InputOptionsMenuSelection = 0;
                return 5; // Return to main menu
            } else if (InputOptionsMenuSelection >= 1 && InputOptionsMenuSelection <=4) {
                // Start rebind process
                
                waitingForKeyRebind = true;
                keyToRebind = InputOptionsMenuSelection;
            }
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER) {
            invalidRebindAttempt = false;
            InputOptionsMenuSelection = 0;
            optionsTab = 1;
        } else if (e.gbutton.button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) {
            invalidRebindAttempt = false;
            InputOptionsMenuSelection = 0;
            optionsTab = 0;
        }
    }

    // Analog stick up/down for menu
    if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
        const int v = e.gaxis.value;
        if (v <= -kAxisPress) {
            if (!menuAxisUpHeld) {
                moveInputOptionsMenuSelection(-1);
                menuAxisUpHeld = true;
                menuAxisDownHeld = false;
            }
        } else if (v >= kAxisPress) {
            if (!menuAxisDownHeld) {
                moveInputOptionsMenuSelection(1);
                menuAxisDownHeld = true;
                menuAxisUpHeld = false;
            }
        } else if (std::abs(v) < kAxisRelease) {
            menuAxisUpHeld = false;
            menuAxisDownHeld = false;
        }
    } 
    
    if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
        const int v = e.gaxis.value;
        if (v <= -kAxisPress) {
            if (!pauseAxisLeftHeld) {
                if (InputOptionsMenuSelection == 0) { // Game tab
                    optionsTab = 1;
                } 
                pauseAxisLeftHeld = true;
                pauseAxisRightHeld = false;
            }
        } else if (v >= kAxisPress) {
            if (!pauseAxisRightHeld) {
                if (InputOptionsMenuSelection == 0) { // Game tab
                    optionsTab = 0;
                }
                pauseAxisRightHeld = true;
                pauseAxisLeftHeld = false;
            }
        } else if (std::abs(v) < kAxisRelease) {
            pauseAxisLeftHeld = false;
            pauseAxisRightHeld = false;
        }
    }

    return -1;
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
    pauseMenuSelection = 0;
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

void close()
{
    if (gActiveGamepad) {
        SDL_CloseGamepad(gActiveGamepad);
        gActiveGamepad = nullptr;
    }

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

    destroyMenuLogoTexture();

    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    gRenderer = nullptr;
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;

    //Quit SDL subsystems
    TTF_Quit();
    SDL_Quit();
}