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
- Hold: h
- Pause: Escape
### Controller
- Move Left: Dpad Left
- Move Right: Dpad Right
- Soft Drop (Move Down): Dpad Down
- Rotate Clockwise: X Button
- Rotate Counter Clockwise: B Button
- Hard Drop: A Button
- Hold: Left Bumper
- Pause: Start Button

## Installation
Grab one of the releases or compile it yourself with the instructions below!
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
- ```
  mkdir build
  cd build
  ```

- Use CMake to generate the build system for your platform (Linux, Windows, etc.):
- ```bash
  cmake ..
```
- Compile the project using the generated build system:
```bash
  cmake --build .
```


### 4. Enjoy your executable! All dependencies are embedded, so you can move the executable wherever you like 😁

## Resources
- [The Tetris Wiki](https://tetris.wiki/Tetris.wiki): Guidlines and general information
- [Lazy Foo' Productions SDL3 Tutorial Series](https://lazyfoo.net/tutorials/SDL3/index.php): A great series of lessons, to which he suggested creating tetris after completing. This project utilizes the LTexture and LTimer classes from his true type/animation lessons.
