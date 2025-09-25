#ifndef GLOBALS_H
#define GLOBALS_H

#include "ltexture.h"
#include "ltimer.h"
#include "piece.h"
#include <SDL3/SDL.h>
#include <string>
#include <vector>


bool init(std::string title = "Tetris (CopBoat's Version)");

bool loadMedia();

void close();

void capFrameRate();

void toggleFullscreen();

void renderUI();

void renderParticles();

int drawPieceIndex();

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

extern LTexture resumeTexture;
extern LTexture quitTexture;

extern int scoreValue;
extern int levelValue;
extern int highScoreValue;

extern Piece iPiece;
extern Piece oPiece;
extern Piece tPiece;
extern Piece lPiece;
extern Piece jPiece;
extern Piece sPiece;
extern Piece zPiece;

extern float spacing;

inline const std::vector<std::string> windowTitles = {
    "Heck is a Tspin?",
    "Kirkland SignatureTM Block Game",
    "FROM TACOMA WITH LOVE",
    "IPiece when?!",
    "Can I please get a longboi?",
    "VLC - HighScore_World-Record.mp4",
    "Thou shalt make complete rows - Tetraviticus 2:15"
};


extern LTimer capTimer;
extern Uint64 lastDropTime;
extern Uint64 dropSpeed;

extern std::vector<std::pair<int, int>> wallKickOffsets0R;
extern std::vector<std::pair<int, int>> wallKickOffsetsR0;
extern std::vector<std::pair<int, int>> wallKickOffsetsR2;
extern std::vector<std::pair<int, int>> wallKickOffsets2R;
extern std::vector<std::pair<int, int>> wallKickOffsets2L;
extern std::vector<std::pair<int, int>> wallKickOffsetsL2;
extern std::vector<std::pair<int, int>> wallKickOffsetsL0;
extern std::vector<std::pair<int, int>> wallKickOffsets0L;
extern std::vector<std::pair<int, int>> wallKickOffsets[8];

extern std::vector<std::pair<int, int>> wallKickOffsetsI0R;
extern std::vector<std::pair<int, int>> wallKickOffsetsIR0;
extern std::vector<std::pair<int, int>> wallKickOffsetsIR2;
extern std::vector<std::pair<int, int>> wallKickOffsetsI2R;
extern std::vector<std::pair<int, int>> wallKickOffsetsI2L;
extern std::vector<std::pair<int, int>> wallKickOffsetsIL2;
extern std::vector<std::pair<int, int>> wallKickOffsetsIL0;
extern std::vector<std::pair<int, int>> wallKickOffsetsI0L;
extern std::vector<std::pair<int, int>> wallKickOffsetsI[8];

enum class GameState { MENU, PLAYING, OPTIONS, PUASE };
extern GameState currentState;

// Controller repeat config (ms)
extern Uint64 kDAS_MS;
extern Uint64 kARR_MS;
extern Uint64 kSoftDrop_ARR_MS;

struct RepeatState {
    bool   held{false};
    Uint64 pressedAt{0};
    Uint64 lastRepeatAt{0};
};
enum class HDir { None, Left, Right };

// Declare externs for globals defined in globals.cpp
extern RepeatState gpLeft;
extern RepeatState gpRight;
extern RepeatState gpDown;
extern HDir activeH;

// Analog stick thresholds with hysteresis
extern int kAxisPress;
extern int kAxisRelease;

// Track per-source held states
extern bool kbLeftHeld, kbRightHeld, kbDownHeld;
extern bool gpDpadLeftHeld, gpDpadRightHeld, gpDpadDownHeld;
extern bool gpAxisLeftHeld, gpAxisRightHeld, gpAxisDownHeld;
extern bool gpLeftHeld, gpRightHeld, gpDownHeld;

// Menu/pause analog navigation hysteresis flags
extern bool menuAxisUpHeld, menuAxisDownHeld;
extern bool pauseAxisLeftHeld, pauseAxisRightHeld;

//should this be extern?
extern void recomputeGamepadHeld();

extern LTexture titleTexture;
extern LTexture playTexture;
extern LTexture optionsTexture;
extern LTexture backTexture;
extern LTexture exitTexture;

extern LTexture optionsTitleTexture;
extern LTexture optionsGridLabel;
extern LTexture optionsBlockGapLabel;
extern LTexture optionsPlacementPreviewLabel;

extern LTexture optionsTitleTexture2;
extern LTexture windowSizeLabel;
extern LTexture fullscreenLabel;


extern LTexture optionsTitleTexture3;
extern LTexture inputConfigHardDropLabel;
extern LTexture inputConfigHoldLabel;
extern LTexture inputConfigRotateCWLabel;
extern LTexture inputConfigRotateCCWLabel;
extern LTexture inputConfigKeyDirectionLabel;



extern int menuSelection;

void renderMenu();
static inline void moveMenuSelection(int);
int handleMenuEvent(const SDL_Event&);

void renderGameOptions();
void renderVideoOptions();
void renderInputOptions();

extern int pauseMenuSelection;
void renderPauseMenu();
void quitToMenu();

void renderWipeIntro(SDL_Renderer*, int, int);

#endif