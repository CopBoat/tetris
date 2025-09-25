#ifndef LTEXTURE_H
#define LTEXTURE_H

#include <SDL3/SDL.h>
#include <string>
#include <SDL3_ttf/SDL_ttf.h>


//credit to lazyfoo.net for this class
class LTexture
{
    public:
        //Symbolic constant
        static constexpr float kOriginalSize = -1.f;

        //Initializes texture variables
        LTexture();

        //Cleans up texture variables
        ~LTexture();

        //Loads texture from disk
        bool loadFromFile( std::string path );

        #if defined(SDL_TTF_MAJOR_VERSION)
        //Creates texture from text
        bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
        #endif

        //Cleans up texture
        void destroy();

        //Sets color modulation
        void setColor( Uint8 r, Uint8 g, Uint8 b);

        //Sets opacity
        void setAlpha( Uint8 alpha );

        //Sets blend mode
        void setBlending( SDL_BlendMode blendMode );

        //Draws texture
        void render( float x, float y, SDL_FRect* clip = nullptr, float width = kOriginalSize, float height = kOriginalSize, double degrees = 0.0, SDL_FPoint* center = nullptr, SDL_FlipMode flipMode = SDL_FLIP_NONE );

        //Gets texture attributes
        int getWidth() const;
        int getHeight() const;
        bool isLoaded();

    private:
        //Contains texture data
        SDL_Texture* mTexture;

        //Texture dimensions
        int mWidth;
        int mHeight;
};

#endif