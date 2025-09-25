#include "ltexture.h"
#include "globals.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

LTexture::LTexture():
    //Initialize texture variables
    mTexture{ nullptr },
    mWidth{ 0 },
    mHeight{ 0 }
{

}

LTexture::~LTexture()
{
    //Clean up texture
    destroy();
}

bool LTexture::loadFromFile( std::string path )
{
    //Clean up texture if it already exists
    destroy();

    //Load surface
    if( SDL_Surface* loadedSurface = IMG_Load( path.c_str() ); loadedSurface == nullptr )
    {
        SDL_Log( "Unable to load image %s! SDL_image error: %s\n", path.c_str(), SDL_GetError() );
    }
    else
    {
        //Create texture from surface
        if( mTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface ); mTexture == nullptr )
        {
            SDL_Log( "Unable to create texture from loaded pixels! SDL error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        //Clean up loaded surface
        SDL_DestroySurface( loadedSurface );
    }

    //Return success if texture loaded
    return mTexture != nullptr;
}

void LTexture::destroy()
{
    //Clean up texture
    SDL_DestroyTexture( mTexture );
    mTexture = nullptr;
    mWidth = 0;
    mHeight = 0;
}

void LTexture::render( float x, float y, SDL_FRect* clip, float width, float height, double degrees, SDL_FPoint* center, SDL_FlipMode flipMode )
{
    //Set texture position and size
    float renderWidth = (width == kOriginalSize) ? static_cast<float>(mWidth) : width;
    float renderHeight = (height == kOriginalSize) ? static_cast<float>(mHeight) : height;
    SDL_FRect dstRect{ x, y, renderWidth, renderHeight };

    //Render texture with optional clip, rotation, and flipping
    SDL_RenderTextureRotated(gRenderer, mTexture, clip, &dstRect, degrees, center, flipMode);
}

int LTexture::getWidth() const
{
    return mWidth;
}

int LTexture::getHeight() const
{
    return mHeight;
}

bool LTexture::isLoaded()
{
    return mTexture != nullptr;
}

#if defined(SDL_TTF_MAJOR_VERSION)
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
    //Clean up existing texture
    destroy();

    //Load text surface
    if( SDL_Surface* textSurface = TTF_RenderText_Blended( gFont, textureText.c_str(), 0, textColor ); textSurface == nullptr )
    {
        SDL_Log( "Unable to render text surface! SDL_ttf Error: %s\n", SDL_GetError() );
    }
    else
    {
        //Create texture from surface
        if( mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface ); mTexture == nullptr )
        {
            SDL_Log( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        //Free temp surface
        SDL_DestroySurface( textSurface );
    }
    
    //Return success if texture loaded
    return mTexture != nullptr;
}
#endif