#ifndef GLOBALS_H
#define GLOBALS_H

#include "ltexture.h"
#include <SDL3/SDL.h>
#include <string>
#include <vector>


bool init(std::string title = "Tetris (CopBoat's Version)");

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

inline const std::vector<std::string> windowTitles = {
    "Heck is a Tspin?",
    "Kirkland SignatureTM Block Game",
    "FROM TACOMA WITH LOVE",
    "IPiece when?!",
    "can I please get a longboi",
    "VLC - HighScore_World-Record.mp4",
    "Thou shalt make complete rows - Tetraviticus 2:15",
    "bring the pieces back together (rediscover communication)"
};

#endif