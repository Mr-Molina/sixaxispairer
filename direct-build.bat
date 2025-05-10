@echo off
echo Building sixaxispairer with direct compiler invocation...

REM Check for Visual Studio installation
where cl.exe >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Visual Studio compiler (cl.exe) not found in PATH.
    echo Please run this script from a Visual Studio Developer Command Prompt
    echo or install Visual Studio with C++ development tools.
    exit /b 1
)

REM Create build directory
if not exist build-direct mkdir build-direct
cd build-direct

REM Compile source files
echo Compiling source files...
cl.exe /c /EHsc /I..\hidapi-win\include ..\main.c ..\mac_utils.c ..\controller_info.c ..\controller_connection.c ..\ui.c

if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    exit /b %ERRORLEVEL%
)

REM Link executable
echo Linking executable...
link.exe /OUT:sixaxispairer.exe main.obj mac_utils.obj controller_info.obj controller_connection.obj ui.obj ..\hidapi-win\x64\hidapi.lib

if %ERRORLEVEL% neq 0 (
    echo Linking failed!
    exit /b %ERRORLEVEL%
)

REM Copy DLL
echo Copying DLL files...
copy ..\hidapi-win\x64\hidapi.dll .

echo Build completed successfully!
echo You can run the program with: .\sixaxispairer.exe
cd ..