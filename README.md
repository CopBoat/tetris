# tetris (CopBoat's Version)
(Place holder for gif)

Tetris built with C++ and SDL3! (sdl3 3.2.20-1, sdl3_image 3.2.4-1, sdl3_ttf 3.2.2-1)

## Features
- Implements the [Super Rotation System](https://tetris.wiki/Super_Rotation_System)
- Lock Delay of 30 frames (0.5 seconds) with 10 resets allocated for horizontal movement and 5 resets allocated for rotations
- Next imediate piece is displayed upon placement
- The current piece may be swaped for the hold piece once per placement
- Visual modification options for the board, pieces, and placement preivew
- Fullscreen toggle, small, standard, and large presets, as well as conventional click and drag can be used to resize the game window 
- Highscores and settings are preserved between play sessions (not yet implemented)
- Controller support for Xinput gamepads
  (Other gamepads such as switch pro, daulshock, etc. will most likely work as well but are untested)

## Controls
Non-directional inputs can be rebound in the options menu

| Action | Keyboard | Controller |
| ------ | -------- | ---------- |
| Move Left | Left Arrow | DPad Left & Analog Stick Left|
| Move Right | Right Arrow | DPad Right & Analog Stick Right |
| Soft Drop | Down Arrow | DPad Down & Analog Stick Down |
| Rotate Clockwise | Up Arrow | X Button |
| Rotate Counter Clockwise | Left  CTRL | B Button |
| Hard Drop | Spacebar | A Button |
| Hold | h | LB Button |
| Pause | Escape | Start Button |

## Installation
~~Grab one of the releases or compile it yourself with the instructions below!~~
No releases yet, for now use the instructions below
### 1. Clone the repository and navigate to the cloned folder:
```bash
git clone https://github.com/CopBoat/tetris.git
cd tetris
```

### 2. **For Arch/Manjaro or Ubuntu/Debian users**, simply run the automated build script:
```bash
./build.sh
```
### 3. **For other Linux distributions or Windows users** (via WSL/MSYS2):
- In the project root, create a build folder:
  ```bash
  mkdir build
  cd build
  ```

- Use CMake to generate the build system for your platform (Linux, Windows, etc.):
  ```bash
  cmake ..
   ```
- Compile the project using the generated build system:
  ```bash
  cmake --build .
  ```

### 4. Enjoy your executable! All assets are embedded, so you can move the executable wherever you like 😁

## Resources
- [The Tetris Wiki](https://tetris.wiki/Tetris.wiki): Guidlines and general information
- [Lazy Foo' Productions SDL3 Tutorial Series](https://lazyfoo.net/tutorials/SDL3/index.php): A great series of lessons, to which he suggested creating tetris after completing. This project utilizes the LTexture and LTimer classes from his true type/animation lessons.