#ifndef GLOBALS_H
#define GLOBALS_H

#include "ltexture.h"
#include <SDL3/SDL.h>


bool init();

bool loadMedia();

void close();

constexpr int kScreenWidth{ 640 };
constexpr int kScreenHeight{ 640 };
constexpr int kScreenFps{ 60 };
constexpr int blockSize{ 32 }; // Size of each block in pixels

extern SDL_Window* gWindow;

//The renderer used to draw to the window
extern SDL_Renderer* gRenderer;

//Global font
extern TTF_Font* gFont;

//UI TEXT
extern LTexture scoreLabel; 
extern LTexture levelLabel; 
extern LTexture nextLabel; 
extern LTexture holdLabel;
extern LTexture highScoreLabel; // Added for high score display
extern LTexture gameOverLabel;

extern LTexture score;
extern LTexture level;
extern LTexture highScore;

extern int scoreValue;
extern int levelValue;
extern int highScoreValue;


#endif