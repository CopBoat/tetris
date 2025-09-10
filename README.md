# tetris (CopBoat's Version)
(Place holder for gif)

Tetris built with C++ and SDL3! (sdl3 3.2.20-1, sdl3_image 3.2.4-1, sdl3_ttf 3.2.2-1)

## Features
- Implements the [Super Rotation System](https://tetris.wiki/Super_Rotation_System)
- Lock Delay TBD
- Controller support for Xinput gamepads
  (Other gamepads such as switch pro, daulshock, etc. will most likely work as well but are untested)

## Controls
### Keyboard
- Move Left: Left Arrow
- Move Right: Right Arrow
- Soft Drop (Move Down): Down Arrow
- Rotate Clockwise: Up Arrow
- Rotate Counter Clockwise: 
- Hard Drop: Spacebar
- Pause: Escape
### Controller
- Move Left: Dpad Left
- Move Right: Dpad Right
- Soft Drop (Move Down): Dpad Down
- Rotate Clockwise: X Button
- Rotate Counter Clockwise: B Button
- Hard Drop: A Button
- Pause: Start Button

## Installation
Grab one of the releases or compile it yourself with the instructions below!

### Build Instructions
1. Clone the repo: `git clone https://github.com/CopBoat/tetris.git`
2. In the project root, create a build folder: `mkdir build`
3. Enter the build folder: `cd build`
4. Use CMake to generate a Makefile for your system (Windows, Linux, etc.): `cmake ..`
5. Use the Makefile: `cmake --build .`
6. Enjoy your executable. Dependencies are embeded so you can move it wherever you like 😁

## Resources
- [The Tetris Wiki](https://tetris.wiki/Tetris.wiki): Guidlines and general information
- [Lazy Foo' Productions SDL3 Tutorial Series](https://lazyfoo.net/tutorials/SDL3/index.php): A great series of lessons, to which he suggested creating tetris after completing. This project was built from the true type/animation lesson as a base. 