@echo off
REM XVPatcher build script for Visual Studio

setlocal enabledelayedexpansion

REM Set up Visual Studio environment if needed
REM Uncomment and modify the line below to match your Visual Studio installation if the script can't find Visual Studio
REM call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"

REM Clean up build directory if it exists
if exist build_vs (
  echo Cleaning up previous build...
  rmdir /s /q build_vs
)

REM Create and enter build directory
mkdir build_vs
cd build_vs

REM Configure with CMake for Visual Studio
echo Configuring project with CMake for Visual Studio...
cmake -G "Visual Studio 17 2022" -A Win32 -DCMAKE_CONFIGURATION_TYPES="Release" ..

REM Check if generation was successful
if %ERRORLEVEL% NEQ 0 (
  echo CMake configuration failed with error %ERRORLEVEL%
  exit /b %ERRORLEVEL%
)

REM Build the project
echo Building XVPatcher...
cmake --build . --config Release

REM Check if build was successful
if %ERRORLEVEL% NEQ 0 (
  echo Build failed with error %ERRORLEVEL%
  exit /b %ERRORLEVEL%
)

echo Build completed successfully!

REM Copy DLL to game directory if path exists
if exist "%GAME_DIR%" (
  echo Copying xinput1_3.dll to game directory...
  copy /Y Release\xinput1_3.dll "%GAME_DIR%\"
) else (
  echo Game directory not found. Copy Release\xinput1_3.dll to your game directory manually.
)

echo Done!
endlocal 