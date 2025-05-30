@echo off
echo Building sixaxispairer...

REM Find Visual Studio installation
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_PATH=%%i"
    )
)

if not defined VS_PATH (
    echo Visual Studio with C++ tools not found.
    echo Please install Visual Studio with "Desktop development with C++" workload.
    exit /b 1
)

echo Found Visual Studio at: %VS_PATH%

REM Set up Visual Studio environment
if exist "%VS_PATH%\Common7\Tools\VsDevCmd.bat" (
    call "%VS_PATH%\Common7\Tools\VsDevCmd.bat" -arch=x64
) else (
    echo Could not find VsDevCmd.bat
    exit /b 1
)

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo CMake not found in PATH.
    echo Using direct compilation method instead.
    call build-direct-final.bat
    exit /b %ERRORLEVEL%
)

if not exist build mkdir build
cd build

echo Configuring with CMake...
cmake -DHIDAPI_ROOT_DIR=../hidapi-win ..

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    echo Falling back to direct compilation method...
    cd ..
    call build-direct-final.bat
    exit /b %ERRORLEVEL%
)

echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

echo Build completed successfully!
echo You can run the program with: .\Release\sixaxispairer.exe
cd ..