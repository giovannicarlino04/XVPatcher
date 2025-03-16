# Building XVPatcher with Visual Studio

This document provides detailed instructions for building XVPatcher using Visual Studio.

## Prerequisites

- **Visual Studio**: 2019 or 2022 with C++ Desktop Development workload installed
- **CMake**: Version 3.10 or higher (included with Visual Studio or can be installed separately)

## Build Methods

### Method 1: Using the build_vs.bat Script (Recommended)

1. Open a command prompt and navigate to the XVPatcher directory
2. Run the `build_vs.bat` script:
   ```
   build_vs.bat
   ```
3. The script will automatically:
   - Create a build_vs directory
   - Configure the project with CMake
   - Build the solution
   - Copy the DLL to your game directory (if configured)

### Method 2: Using Visual Studio with CMake

1. Open Visual Studio
2. Select "Open a local folder" and navigate to the XVPatcher directory
3. Visual Studio will automatically detect the CMakeLists.txt file
4. Select "x86-Release" from the configuration dropdown
5. Build the project using the "Build" menu or press F7

### Method 3: Manual Configuration

1. Create a build directory:
   ```
   mkdir build_vs
   cd build_vs
   ```
   
2. Configure with CMake:
   ```
   cmake -G "Visual Studio 17 2022" -A Win32 ..
   ```
   
3. Open the generated solution file:
   ```
   XVPatcher.sln
   ```
   
4. Build the solution in Visual Studio

## Troubleshooting

### Missing Libraries

If you encounter errors about missing libraries, make sure to:

1. Install the correct VC++ Redistributable package
2. Check that the library paths in the CMakeLists.txt file are correct

### Fixing Link Errors

If you encounter link errors related to xinput1_3.lib:

1. Open the Visual Studio solution
2. Right-click on the project and select "Properties"
3. Go to "Linker" > "Input"
4. Add "xinput.lib" to the "Additional Dependencies" field

### Version Compatibility Issues

Visual Studio 2022 uses the "Visual Studio 17 2022" generator in CMake.
Visual Studio 2019 uses the "Visual Studio 16 2019" generator.

If you're using a different version, modify the generator in the build_vs.bat script:

```
cmake -G "Visual Studio 15 2017" -A Win32 ..
```

## DLL Location

After a successful build, the DLL can be found at:

```
build_vs\bin\Release\xinput1_3.dll
```

Copy this file to your Dragon Ball Xenoverse game directory to use it. 