@echo off
REM Check if VCPKG_ROOT is set
if "%VCPKG_ROOT%"=="" (
    echo Error: VCPKG_ROOT environment variable is not set.
    exit /b 1
)

REM Check if the vcpkg path exists
if not exist "%VCPKG_ROOT%" (
    echo Error: VCPKG_ROOT directory does not exist: %VCPKG_ROOT%
    exit /b 1
)

REM Run the CMake command with the specified toolchain file and target triplet
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows

REM Check if CMake configuration succeeded
if errorlevel 1 (
    echo Error: CMake configuration failed.
    exit /b 1
)

REM Change to the build directory
cd build

REM Build all targets
cmake --build .

REM Check if the build succeeded
if errorlevel 1 (
    echo Error: Build failed.
    exit /b 1
)

echo Build succeeded.
