#include "globals.h"
#include "tPieceIcon.h"
#include "Pixeboy_ttf.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>

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

bool init()
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
        if( SDL_CreateWindowAndRenderer( "Tetris (CopBoat's Version)", kScreenWidth, kScreenHeight, 0, &gWindow, &gRenderer ) == false )
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

    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    gRenderer = nullptr;
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;

    //Quit SDL subsystems
    TTF_Quit();
    SDL_Quit();
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