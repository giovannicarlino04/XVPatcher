# XVPatcher - Dragon Ball Xenoverse 1 Patching DLL

XVPatcher is a modding tool for Dragon Ball Xenoverse 1 that enables various game modifications through DLL injection.

## Features

- Custom character slot patches
- CPK file redirection and patching
- Game file format support (CPK, HCA, etc.)
- Memory and hook-based game modifications
- Flash/Iggy UI element modifications

## Build Requirements

- CMake 3.10 or higher
- A C++11 compatible compiler (GCC, MSVC, etc.)
- MinHook library
- Windows development tools

## Building with CMake

### Windows with MSYS2 (Recommended)

1. Install MSYS2 from https://www.msys2.org/
2. Open MSYS2 MinGW 32-bit shell and install required packages:
   ```
   pacman -S mingw-w64-i686-gcc mingw-w64-i686-cmake mingw-w64-i686-make
   ```
3. Use the provided batch file to build:
   ```
   build.bat
   ```

### Cross-compiling from Linux

1. Install MinGW cross-compiler:
   ```
   sudo apt install mingw-w64
   ```
2. Configure using the toolchain file:
   ```
   mkdir build
   cd build
   cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-toolchain.cmake ..
   ```
3. Build:
   ```
   cmake --build .
   ```

### Visual Studio

1. Generate Visual Studio project files:
   ```
   mkdir build
   cd build
   cmake -G "Visual Studio 16 2019" -A Win32 ..
   ```
2. Open the generated solution file and build

## Configuration

The patcher looks for configuration files in:
- `./XVPATCHER/xvpatcher.ini` - Main configuration file
- `./XVPATCHER/EPatches/` - External patch files

## Usage

1. Place the compiled `xinput1_3.dll` in your game directory
2. Create the `XVPATCHER` folder in your game directory
3. Add configuration files as needed
4. Run the game

## Debugging

Debug logs are written to `./XVPATCHER/xv1_log.txt`

## Project Structure

The project has been consolidated to reduce file count:

- `Utils.cpp/h` - Consolidated utility functions (file, string, crypto, etc.)
- `Stream.cpp/h` - Consolidated stream handling (memory, file, bit streams)
- Core files remain separate for maintainability

## License

This project is available for personal and non-commercial use.

## Credits

- Created by [Your Name] 