@echo off
REM XVPatcher build script for Windows using MSYS2

setlocal enabledelayedexpansion

REM Set paths - update these as needed
set MSYS2_PATH=C:\msys64
set MINGW32_PATH=%MSYS2_PATH%\mingw32\bin
set PATH=%MINGW32_PATH%;%PATH%

REM Clean up build directory if it exists
if exist build (
  echo Cleaning up previous build...
  rmdir /s /q build
)

REM Create and enter build directory
mkdir build
cd build

REM Configure with CMake
echo Configuring project with CMake...
cmake -G "MinGW Makefiles" ..

REM Build the project
echo Building XVPatcher...
cmake --build . -- -j %NUMBER_OF_PROCESSORS%

REM Check if build was successful
if %ERRORLEVEL% NEQ 0 (
  echo Build failed with error %ERRORLEVEL%
  exit /b %ERRORLEVEL%
)

echo Build completed successfully!

REM Copy DLL to game directory if path exists
if exist "%GAME_DIR%" (
  echo Copying xinput1_3.dll to game directory...
  copy /Y bin\xinput1_3.dll "%GAME_DIR%\"
)

echo Done!
endlocal