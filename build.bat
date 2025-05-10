@echo off
echo Building sixaxispairer...

REM Check for Visual Studio installation
where cl.exe >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Visual Studio compiler (cl.exe) not found in PATH.
    echo Checking for Visual Studio installation...
    
    REM Try to find and use Visual Studio Developer Command Prompt
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
        echo Found Visual Studio 2022 Community, setting up environment...
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" (
        echo Found Visual Studio 2019 Community, setting up environment...
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
    ) else (
        echo No Visual Studio installation found.
        echo Please install Visual Studio with C++ development tools or MinGW-w64.
        echo For Visual Studio: https://visualstudio.microsoft.com/downloads/
        echo For MinGW-w64: https://winlibs.com/
        exit /b 1
    )
)

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo CMake not found in PATH.
    echo Please install CMake from https://cmake.org/download/
    exit /b 1
)

if not exist build mkdir build
cd build

echo Configuring with CMake...
cmake -DHIDAPI_ROOT_DIR=../hidapi-win ..

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    echo.
    echo This might be due to:
    echo 1. Missing C compiler - Install Visual Studio with C++ workload
    echo 2. Missing HIDAPI - Check that hidapi-win directory is correct
    echo.
    echo For detailed error information, check CMakeFiles\CMakeError.log
    exit /b %ERRORLEVEL%
)

echo Building project...
cmake --build .

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

echo Copying DLL files...
copy ..\hidapi-win\x64\hidapi.dll .

echo Build completed successfully!
echo You can run the program with: .\sixaxispairer.exe
cd ..