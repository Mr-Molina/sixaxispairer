@echo off
echo Building sixaxispairer directly with Visual Studio compiler...

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

REM Create build directory
if not exist build-vs mkdir build-vs

REM Create a temporary directory for modified source files
if exist temp rmdir /s /q temp
mkdir temp
echo Creating temporary source files with fixed include paths...

REM Fix include paths in all source files
powershell -Command "(Get-Content main.c) -replace '#include <hidapi/hidapi.h>', '#include \"hidapi-win/include/hidapi.h\"' | Set-Content temp\main.c"
powershell -Command "(Get-Content controller_info.h) -replace '#include <hidapi/hidapi.h>', '#include \"hidapi-win/include/hidapi.h\"' | Set-Content temp\controller_info.h"
powershell -Command "(Get-Content controller_connection.h) -replace '#include <hidapi/hidapi.h>', '#include \"hidapi-win/include/hidapi.h\"' | Set-Content temp\controller_connection.h"
powershell -Command "(Get-Content ui.c) -replace '#include <hidapi/hidapi.h>', '#include \"hidapi-win/include/hidapi.h\"' | Set-Content temp\ui.c"

REM Copy other files to temp directory
copy mac_utils.c temp\
copy mac_utils.h temp\
copy controller_info.c temp\
copy controller_connection.c temp\
copy ui.h temp\

cd temp

REM Compile source files
echo Compiling source files...
cl.exe /c /EHsc /I.. /DWIN32 /D_WINDOWS main.c mac_utils.c controller_info.c controller_connection.c ui.c

if %ERRORLEVEL% neq 0 (
    echo Compilation failed!
    cd ..
    exit /b %ERRORLEVEL%
)

REM Link executable
echo Linking executable...
link.exe /OUT:..\build-vs\sixaxispairer.exe main.obj mac_utils.obj controller_info.obj controller_connection.obj ui.obj ..\hidapi-win\x64\hidapi.lib

if %ERRORLEVEL% neq 0 (
    echo Linking failed!
    cd ..
    exit /b %ERRORLEVEL%
)

cd ..

REM Copy DLL
echo Copying DLL files...
copy hidapi-win\x64\hidapi.dll build-vs\

echo Build completed successfully!
echo You can run the program with: .\build-vs\sixaxispairer.exe