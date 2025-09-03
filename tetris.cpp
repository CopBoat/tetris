/* Headers */
//Using SDL, SDL_image, and STL string
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <iostream>
#include <vector>
#include <ctime>

/* Constants */
//Screen dimension constants
constexpr int kScreenWidth{ 640 };
constexpr int kScreenHeight{ 640 };
constexpr int kScreenFps{ 60 };
constexpr int blockSize{ 32 }; // Size of each block in pixels
constexpr int boardWidth{ 15 }; // Width of the board in blocks
constexpr int boardHeight{ 20 }; // Height of the board in blocks

class Board
{
public:
    int current[boardWidth][boardHeight];
    // bool checkLeft();
    // bool checkRight();
    // bool checkDown();

    // Initialize the board with empty blocks
    Board() {
        for (int x = 0; x < boardWidth; ++x)
            for (int y = 0; y < boardHeight; ++y)
                current[x][y] = 0; 
    }
};

class Piece
{
public:
    // Piece properties
    int x{ boardWidth / 2 }; // X position on the board
    int y{ 0 }; // Y position on the board
    int width{ 0 }; // Width of the piece in blocks
    int height{ 0 }; // Height of the piece in blocks
    std::vector<std::vector<int>> shape; // 2D vector representing the piece shape
    int rotation{ 0 }; // Current rotation state of the piece
    int color; // Color for each block in the piece
    // std::vector<SDL_FRect> blocks; // Blocks that make up the piece
    // SDL_FRect getBounds() const {
    //     return { static_cast<float>(x * blockSize), static_cast<float>(y * blockSize), static_cast<float>(width * blockSize), static_cast<float>(height * blockSize) };
    // }
};

class LTimer
{
    public:
        //Initializes variables
        LTimer();

        //The various clock actions
        void start();
        void stop();
        void pause();
        void unpause();

        //Gets the timer's time
        Uint64 getTicksNS();

        //Checks the status of the timer
        bool isStarted();
        bool isPaused();

    private:
        //The clock time when the timer started
        Uint64 mStartTicks;

        //The ticks stored when the timer was paused
        Uint64 mPausedTicks;

        //The timer status
        bool mPaused;
        bool mStarted;
};

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
    int getWidth();
    int getHeight();
    bool isLoaded();

private:
    //Contains texture data
    SDL_Texture* mTexture;

    //Texture dimensions
    int mWidth;
    int mHeight;
};

/* Function Prototypes */
//Starts up SDL and creates window
bool init();

//Frees media and shuts down SDL
void close();

/* Global Variables */
//The window we'll be rendering to
SDL_Window* gWindow{ nullptr };

//The renderer used to draw to the window
SDL_Renderer* gRenderer{ nullptr };

//Global font
TTF_Font* gFont{ nullptr };

//UI TEXT
LTexture scoreLabel; 
LTexture levelLabel; 
LTexture nextLabel; 
LTexture holdLabel;
LTexture highScoreLabel; // Added for high score display
LTexture gameOverLabel;

LTexture score;
LTexture level;
LTexture highScore;

int scoreValue{ 0 };
int levelValue{ 0 };
int highScoreValue{ 0 };

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

int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
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

//LTimer Implementation
LTimer::LTimer():
    mStartTicks{ 0 },
    mPausedTicks{ 0 },

    mPaused{ false },
    mStarted{ false }
{

}

void LTimer::start()
{
    //Start the timer
    mStarted = true;

    //Unpause the timer
    mPaused = false;

    //Get the current clock time
    mStartTicks = SDL_GetTicksNS();
    mPausedTicks = 0;
}

void LTimer::stop()
{
    //Stop the timer
    mStarted = false;

    //Unpause the timer
    mPaused = false;

    //Clear tick variables
    mStartTicks = 0;
    mPausedTicks = 0;
}

void LTimer::pause()
{
    //If the timer is running and isn't already paused
    if( mStarted && !mPaused )
    {
        //Pause the timer
        mPaused = true;

        //Calculate the paused ticks
        mPausedTicks = SDL_GetTicksNS() - mStartTicks;
        mStartTicks = 0;
    }
}

void LTimer::unpause()
{
    //If the timer is running and paused
    if( mStarted && mPaused )
    {
        //Unpause the timer
        mPaused = false;

        //Reset the starting ticks
        mStartTicks = SDL_GetTicksNS() - mPausedTicks;

        //Reset the paused ticks
        mPausedTicks = 0;
    }
}

Uint64 LTimer::getTicksNS()
{
    //The actual timer time
    Uint64 time{ 0 };

    //If the timer is running
    if( mStarted )
    {
        //If the timer is paused
        if( mPaused )
        {
            //Return the number of ticks when the timer was paused
            time = mPausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            time = SDL_GetTicksNS() - mStartTicks;
        }
    }

    return time;
}

bool LTimer::isStarted()
{
    //Timer is running and paused or unpaused
    return mStarted;
}

bool LTimer::isPaused()
{
    //Timer is running and paused
    return mPaused && mStarted;
}

bool loadMedia()
{
    //File loading flag
    bool success{ true };

    //Load scene font
    std::string fontPath{ "Pixeboy-z8XGD.ttf" };
    if( gFont = TTF_OpenFont( fontPath.c_str(), 24 ); gFont == nullptr )
    {
        SDL_Log( "Could not load %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError() );
        success = false;
    }
    else
    {
        //Load text
        SDL_Color textColor{ 0xFF, 0xFF, 0xFF, 0xFF }; // White text
        if( scoreLabel.loadFromRenderedText( "SCORE", textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError() );
            success = false;
        }
        if( levelLabel.loadFromRenderedText( "LEVEL", textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError() );
            success = false;
        }
        if( nextLabel.loadFromRenderedText( "NEXT", textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError() );
            success = false;
        }
        if( score.loadFromRenderedText( std::to_string(scoreValue), textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError() );
            success = false;
        }
        if( level.loadFromRenderedText( std::to_string(levelValue+1), textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError() );
            success = false;
        }
        if( holdLabel.loadFromRenderedText( "HOLD", textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError() );
            success = false;
        }
        if( highScoreLabel.loadFromRenderedText( "HIGH SCORE", textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError() );
            success = false;
        }
        if( highScore.loadFromRenderedText( std::to_string(highScoreValue), textColor ) == false )
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError() );
            success = false;
        }
    }

    return success;
}

bool init()
{
    bool success{ true };

    // if (SDL_INIT_GAMEPAD == false) {
    //     SDL_Log("SDL Gamepad subsystem could not initialize! SDL error: %s\n", SDL_GetError());
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
        if( SDL_CreateWindowAndRenderer( "FROM TACOMA WITH LOVE", kScreenWidth, kScreenHeight, 0, &gWindow, &gRenderer ) == false )
        {
            SDL_Log( "Window could not be created! SDL error: %s\n", SDL_GetError() );
            success = false;
        } else
        {
            // Add this block to set the window icon
            SDL_Surface* iconSurface = IMG_Load("SquarePiece.png"); // Use your icon file path
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

bool checkPlacement(Piece piece, Board board, int newX, int newY) {
    bool placementValid = true;
    for (int sx = 0; sx < piece.width; ++sx) {
        for (int sy = 0; sy < piece.height; ++sy) {
            if (piece.shape[sy][sx] != 0) {
                int boardX = piece.x + sx + newX;
                int boardY = piece.y + sy + newY;
                if (boardX < 0 || boardX >= boardWidth || boardY < 0 || boardY >= boardHeight || board.current[boardX][boardY] != 0) {
                    placementValid = false;
                }
            }
        }
        if (!placementValid) break;
    }
    return placementValid; 
}


enum class InputAction {
    None,
    MoveLeft,
    MoveRight,
    RotateClockwise,
    SoftDrop,
    HardDrop,
    Hold
};

int main( int argc, char* args[] )
{
    //Final exit code
    int exitCode{ 0 };

    //bool SDL_SetWindowIcon(SDL_Window *gWindow, SDL_Surface *icon);

    //Initialize
    if( init() == false )
    {
        SDL_Log( "Unable to initialize program!\n" );
        exitCode = 1;
    }
    else if( loadMedia() == false )
    {
        SDL_Log( "Unable to load media!\n" );
        exitCode = 1;
    }
    else
    {
        
        //The quit flag
        bool quit{ false };

        //The event data
        SDL_Event e;
        SDL_zero( e );
        
        LTimer capTimer;
        std::srand(static_cast<unsigned int>(time(0)));
        std::srand(static_cast<unsigned int>(std::time(0)));

        // SDL_Gamepad* gamepad = nullptr;
        // int numGamepads = SDL_GetGamepads(); // Get the number of connected gamepads
        
        // if (numGamepads > 0) {
        //     gamepad = SDL_OpenGamepad(0);
        //     std::cout << "Gamepad connected: " << SDL_GetGamepadName(gamepad) << "\n";
        // }

        int gamePadCount = 0;
        SDL_JoystickID *ids = SDL_GetGamepads(&gamePadCount);
        SDL_Gamepad* gamepad = nullptr;
        for (int i = 0; i < gamePadCount; ++i) {
            SDL_Gamepad* gamepd = SDL_OpenGamepad(ids[i]);
            if (gamepad == NULL) {
                gamepad = gamepd;
            }
            std::cout << "Gamepad connected: " << SDL_GetGamepadName(gamepd) << "\n";
    
            // Close the other gamepads
            if(i > 0) {
                SDL_CloseGamepad(gamepd);
            }
        }

        if (!gamepad) {
            //std::cerr << "Failed to open gamepad: " << SDL_GetError() << "\n";
            std::cout << "No gamepad connected.\n";
            // SDL_Quit();
            // return 1;
        }

        //float pos{ 0.0f };
        //float posX{ 320.f }; // Add this for horizontal position

        int myTickCount( 0 );
        
        Uint64 dropSpeed{ 700000000 }; // Milliseconds between drops

        // Define Tetris pieces
        Piece bigPiece;
        bigPiece.width = 4; // Width in blocks
        bigPiece.height = 1; // Height in blocks
        bigPiece.shape = { { 1, 1, 1, 1 } }; // I shape
        bigPiece.rotation = 0; // Initial rotation state
        bigPiece.color = 4; // Color identifier for the piece

        Piece squarePiece;
        squarePiece.width = 2; // Width in blocks
        squarePiece.height = 2; // Height in blocks
        squarePiece.shape = { { 1, 1 }, { 1, 1 } }; // Square shape
        squarePiece.rotation = 0; // Initial rotation state
        squarePiece.color = 3; // Color identifier for the piece

        Piece tPiece;
        tPiece.width = 3; // Width in blocks
        tPiece.height = 2; // Height in blocks
        tPiece.shape = { { 0, 1, 0 }, { 1, 1, 1 } }; // T shape
        tPiece.rotation = 0; // Initial rotation state
        tPiece.color = 2; // Color identifier for the piece 

        Piece lPiece1;
        lPiece1.width = 2; // Width in blocks
        lPiece1.height = 3; // Height in blocks
        lPiece1.shape = { { 1, 1 }, { 0, 1 }, { 0, 1 } }; // L shape
        lPiece1.rotation = 0; // Initial rotation state
        lPiece1.color = 6; // Color identifier for the piece

        Piece lPiece2;
        lPiece2.width = 2; // Width in blocks
        lPiece2.height = 3; // Height in blocks
        lPiece2.shape = { { 1, 1 }, { 1, 0 }, { 1, 0 } }; // L shape
        lPiece2.rotation = 0; // Initial rotation state
        lPiece2.color = 5; // Color identifier for the piece

        Piece sPiece1;
        sPiece1.width = 3; // Width in blocks
        sPiece1.height = 2; // Height in blocks
        sPiece1.shape = { { 0, 1, 1 }, { 1, 1, 0 } }; // S shape
        sPiece1.rotation = 0; // Initial rotation state
        sPiece1.color = 1; // Color identifier for the piece

        Piece sPiece2;
        sPiece2.width = 3; // Width in blocks
        sPiece2.height = 2; // Height in blocks
        sPiece2.shape = { { 1, 1, 0 }, { 0, 1, 1 } }; // S shape
        sPiece2.rotation = 0; // Initial rotation state
        sPiece2.color = 7; // Color identifier

        Piece pieceTypes[7] = { bigPiece, squarePiece, tPiece, lPiece1, lPiece2, sPiece1, sPiece2 }; // Array of piece types

        int pickPiece = std::rand() % 7;  // Randomly select a piece from pieceTypes 
        int nextPickPiece = std::rand() % 7; // Randomly select the next piece

        //std::cout << "Picked piece index: " << pickPiece << std::endl; // Debugging output

        Piece currentPiece = pieceTypes[pickPiece]; // Initialize current piece
        //currentPiece.rotation = 0; // Initial rotation state
        Piece nextPiece = pieceTypes[nextPickPiece]; // Initialize next piece

        

        Piece holdPiece; // Piece to hold
        
        bool newPiece{ false };

        bool holdUsed{ false }; // To track if hold was used in the current turn

        int rowsCleared = 0; // To track number of cleared rows

        int levelIncrease = 0; // To track level increase threshold

        bool hardDrop = false;

        int lockDelayFrames = 30; // Number of frames to allow after landing (adjust as desired)
        int lockDelayCounter = 0; // Counts frames since landing (reset on move/rotate)
        bool pieceLanded = false; // True if just landed, false if still falling

        Board board;

        //The main loop
        while( quit == false )
        {

            capTimer.start();

            
            InputAction action = InputAction::None;


            while( SDL_PollEvent( &e ) == true )
                {
                    //If event is quit type
                    if( e.type == SDL_EVENT_QUIT )
                    {
                        //End the main loop
                        quit = true;
                    }

                    
                    

                    

                    //Handle keyboard inputs
                    if (e.type == SDL_EVENT_KEY_DOWN)
                    {
                        switch (e.key.key)
                        {
                            case SDLK_LEFT: action = InputAction::MoveLeft;  break;
                            case SDLK_RIGHT: action = InputAction::MoveRight; break;
                            case SDLK_UP: action = InputAction::RotateClockwise; break;
                            case SDLK_DOWN: action = InputAction::SoftDrop;  break;
                                
                            case SDLK_H: action = InputAction::Hold;      break;
                                
                            case SDLK_SPACE: action = InputAction::HardDrop;  break;
                            // case SDLK_ESCAPE:
                            //     std::cout << "ESCAPE key pressed" << std::endl;
                            //     //capTimer.pause();
                            //     break;

                            default:
                                break;
                        }
                    }

                    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                        switch (e.gbutton.button) {
                            case SDL_GAMEPAD_BUTTON_DPAD_LEFT:  action = InputAction::MoveLeft;  break;
                            case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: action = InputAction::MoveRight; break;
                            case SDL_GAMEPAD_BUTTON_WEST:    action = InputAction::RotateClockwise;    break;
                            case SDL_GAMEPAD_BUTTON_DPAD_DOWN:  action = InputAction::SoftDrop;  break;
                            case SDL_GAMEPAD_BUTTON_SOUTH:    action = InputAction::HardDrop;  break;
                            case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: action = InputAction::Hold;   break;
                        }
                    }
                }




            //rotate helper
            std::vector<std::vector<int>> newShape(currentPiece.width, std::vector<int>(currentPiece.height, 0));

            // List of offsets to try for wall kick
            std::vector<std::pair<int, int>> wallKickOffsets = {{0, 0}, {-1, 0}, {1, 0}, {0, -1}, {-1, -1}, {1, -1}};

            switch (action) {
                case InputAction::MoveLeft:
                {
                    //check collision
                    // bool canMoveLeft = true;
                    // for (int sx = 0; sx < currentPiece.width; ++sx) {
                    //     for (int sy = 0; sy < currentPiece.height; ++sy) {
                    //         if (currentPiece.shape[sy][sx] != 0) {
                    //             int boardX = currentPiece.x + sx - 1;
                    //             int boardY = currentPiece.y + sy;
                    //             if (boardX < 0 || boardX >= boardWidth || boardY < 0 || boardY >= boardHeight || board.current[boardX][boardY] != 0) {
                    //                 canMoveLeft = false;
                    //             }
                    //         }
                    //     }
                    //     if (!canMoveLeft) break;
                    // }

                    if (checkPlacement(currentPiece, board, -1, 0)){
                        //clear current position
                        for (int sx = 0; sx < currentPiece.width; ++sx) {
                            for (int sy = 0; sy < currentPiece.height; ++sy) {
                                if (currentPiece.shape[sy][sx] != 0) {
                                    int boardX = currentPiece.x + sx;
                                    int boardY = currentPiece.y + sy;
                                    board.current[boardX][boardY] = 0;
                                }
                            }
                        }

                        //move left
                        currentPiece.x -= 1;
                    }

                    break;
                }
                case InputAction::MoveRight:
                {
                    //check collision
                    // bool canMoveRight = true;
                    // for (int sx = 0; sx < currentPiece.width; ++sx) {
                    //     for (int sy = 0; sy < currentPiece.height; ++sy) {
                    //         if (currentPiece.shape[sy][sx] != 0) {
                    //             int boardX = currentPiece.x + sx + 1;
                    //             int boardY = currentPiece.y + sy;
                    //             if (boardX < 0 || boardX >= boardWidth || boardY < 0 || boardY >= boardHeight || board.current[boardX][boardY] != 0) {
                    //                 canMoveRight = false;
                    //             }
                    //         }
                    //     }
                    //     if (!canMoveRight) break;
                    // }

                    if (checkPlacement(currentPiece, board, 1, 0)){
                        //clear current position
                        for (int sx = 0; sx < currentPiece.width; ++sx) {
                            for (int sy = 0; sy < currentPiece.height; ++sy) {
                                if (currentPiece.shape[sy][sx] != 0) {
                                    int boardX = currentPiece.x + sx;
                                    int boardY = currentPiece.y + sy;
                                    board.current[boardX][boardY] = 0;
                                }
                            }
                        }

                        //move left
                        currentPiece.x += 1;
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

                        // Check if rotated piece fits at (newX, newY)
                        if (checkPlacement(rotatedPiece, board, 0, 0)) {
                            // Apply rotation
                            currentPiece.shape = newShape;
                            std::swap(currentPiece.width, currentPiece.height);
                            currentPiece.rotation = (currentPiece.rotation + 1) % 4;
                            if (currentPiece.rotation % 4 == 1 || currentPiece.rotation % 4 == 3) {
                                // Shift right for vertical I piece
                                currentPiece.x += 1;
                            } 
                            else {
                                currentPiece.x -= 1;
                            }
                            
                            //currentPiece.y += 1;
                        }

                        break;
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
                        
                        for (const auto& offset : wallKickOffsets) {
                            
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
                case InputAction::SoftDrop:

                    if (checkPlacement(currentPiece, board, 0, 1)){
                        //clear current position
                        for (int sx = 0; sx < currentPiece.width; ++sx) {
                            for (int sy = 0; sy < currentPiece.height; ++sy) {
                                if (currentPiece.shape[sy][sx] != 0) {
                                    int boardX = currentPiece.x + sx;
                                    int boardY = currentPiece.y + sy;
                                    board.current[boardX][boardY] = 0;
                                }
                            }
                        }

                        //move down
                        currentPiece.y += 1;
                    }

                    break;
                case InputAction::HardDrop:
                    {
                                //hard drop
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
                                //currentPiece.y = boardHeight - currentPiece.height;
                                // Find the lowest y position where the piece can be placed without collision
                                int maxDrop = boardHeight - currentPiece.height;
                                for (int dropY = currentPiece.y; dropY <= maxDrop; ++dropY) {
                                    bool collision = false;
                                    for (int sx = 0; sx < currentPiece.width; ++sx) {
                                        for (int sy = 0; sy < currentPiece.height; ++sy) {
                                            if (currentPiece.shape[sy][sx] != 0) {
                                                int boardX = currentPiece.x + sx;
                                                int boardY = dropY + sy;
                                                if (boardY >= boardHeight || board.current[boardX][boardY] != 0) {
                                                    collision = true;
                                                    break;
                                                }
                                            }
                                        }
                                        if (collision) break;
                                    }
                                    if (collision) {
                                        currentPiece.y = dropY - 1;
                                        break;
                                    }
                                    // If we reached the last possible position, set y to maxDrop
                                    if (dropY == maxDrop) {
                                        currentPiece.y = maxDrop;
                                    }
                                }
                                

                                // while (!newPiece){
                                //     if (currentPiece.y + currentPiece.height < boardHeight && board.current[currentPiece.x][currentPiece.y + 1] == 0) {
                                //         currentPiece.y += 1;
                                //     } 
                                // }
                                hardDrop = true;
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
                            currentPiece = bigPiece; // Reset to I piece
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

            //draw line seperating the board and UI
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
                    gameOverLabel.render( 150, 300 );
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
                                    case 1: color = {255, 0, 0, 255}; break; // Red
                                    case 2: color = {0, 0, 255, 255}; break; // Blue
                                    case 3: color = {255, 255, 0, 255}; break; // Yellow
                                    case 4: color = {0, 255, 255, 255}; break; // Cyan
                                    case 5: color = {0, 255, 0, 255}; break; // Green
                                    case 6: color = {255, 0, 255, 255}; break; // Magenta
                                    case 7: color = {255, 128, 0, 255}; break; // Orange
                                    default: color = {128, 128, 128, 255}; break; // Gray
                                }
                                SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, color.a);
                                SDL_RenderFillRect(gRenderer, &rect);
                            }
                        }
                    }

                    
                }
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
            
            if (myTickCount % kScreenFps == 0 && canPlaceNext) {
                currentPiece.y += 1;
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
                        for (int y = i; y > 0; --y) {
                            for (int x = 0; x < boardWidth; ++x) {
                                //board.current[x][0] = 0; // Clear the top row
                                board.current[x][y] = board.current[x][y - 1];
                            }
                        }
                        for (int x = 0; x < boardWidth; ++x) {
                            board.current[x][0] = 0; // Clear the top row
                        }
                        // for (int x = 0; x < boardWidth; ++x) {
                        //     // for (int y = i; y > 0; --y) {
                        //     //     board.current[x][y] = board.current[x][y - 1];
                        //     // }
                        //     // board.current[x][0] = 0; // Clear the top row
                        //     board.current[x][i] = 0;
                        // }
                        
                    }

                    
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
            
            myTickCount++;


            //Cap frame rate
            //1,000,000,000
            Uint64 nsPerFrame = dropSpeed / kScreenFps; 
            Uint64 frameNs{ capTimer.getTicksNS() };
            if( frameNs < nsPerFrame )
            {
                SDL_DelayNS( nsPerFrame - frameNs );
            }
        } 

        if (gamepad) {
            SDL_CloseGamepad(gamepad);
        }
    }

    

    //Clean up
    close();

    return exitCode;
}