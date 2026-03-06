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

## Windows Release Build (for fresh Windows installs)
Build a distributable folder that contains `tetris.exe` and all required SDL/MSVC runtime DLLs:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --config Release --prefix dist
```

After this, share the contents of the `dist` folder (or zip it). A user can extract and run `tetris.exe` directly.

### Single-EXE mode on Windows (default)
This project now defaults to static SDL linking on Windows (`-DTETRIS_STATIC_SDL=ON`), so the install output can be a single `tetris.exe` file:

```bash
cmake -S . -B build-static -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 -DTETRIS_STATIC_SDL=ON
cmake --build build-static
cmake --install build-static --config Release --prefix dist-static
```