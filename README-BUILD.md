# Building SixAxisPairer

This document provides instructions for building the SixAxisPairer application on Windows.

## Prerequisites

1. **C Compiler** - You need one of the following:
   - Visual Studio with C++ development tools (recommended)
   - MinGW-w64

2. **CMake** - Version 3.18 or higher
   - Download from: https://cmake.org/download/

3. **HIDAPI Library** - Already included in the `hidapi-win` directory

## Build Options

### Option 1: Using Visual Studio Developer Command Prompt

1. Open a Visual Studio Developer Command Prompt
   - Start menu → Visual Studio → Developer Command Prompt

2. Navigate to the project directory:
   ```
   cd path\to\sixaxispairer
   ```

3. Run the build script:
   ```
   build.bat
   ```

### Option 2: Using Direct Compilation (No CMake)

If you're having trouble with CMake, you can build directly with the Visual Studio compiler:

1. Open a Visual Studio Developer Command Prompt

2. Navigate to the project directory:
   ```
   cd path\to\sixaxispairer
   ```

3. Run the direct build script:
   ```
   direct-build.bat
   ```

### Option 3: Using Visual Studio Code

1. Install Visual Studio Code
2. Install the "C/C++" and "CMake Tools" extensions
3. Open the project folder in VS Code
4. When prompted, select a compiler kit (Visual Studio or MinGW)
5. Click the "Build" button in the status bar

## Troubleshooting

### "CMAKE_C_COMPILER not set" Error

This error occurs when CMake cannot find a C compiler. Solutions:

1. **Install Visual Studio** with C++ development tools
   - Download from: https://visualstudio.microsoft.com/downloads/
   - During installation, select "Desktop development with C++"

2. **Run from Developer Command Prompt**
   - Open "Developer Command Prompt for VS" from the Start menu
   - Navigate to your project directory and run `build.bat`

3. **Install MinGW-w64**
   - Download from: https://winlibs.com/
   - Add the bin directory to your PATH environment variable

### "Cannot find hidapi" Error

1. Verify the `hidapi-win` directory exists and contains:
   - `include/hidapi.h`
   - `x64/hidapi.dll` and `x64/hidapi.lib`

2. Try the direct build script:
   ```
   direct-build.bat
   ```

## Running the Application

After building, you can run the application from the build directory:

```
cd build
sixaxispairer.exe
```

Make sure `hidapi.dll` is in the same directory as the executable.