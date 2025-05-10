@echo off
echo Building sixaxispairer...

if not exist build mkdir build
cd build

echo Configuring with CMake...
cmake -DHIDAPI_ROOT_DIR=../hidapi-win ..

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
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