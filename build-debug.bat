@echo off
setlocal

IF NOT DEFINED FMOD_LIBS_PATH (
    SET "FMOD_LIBS_PATH=%~1"
) ELSE (
    echo "PATH TO FMOD LIBS NOT SET"
)
echo  FMOD_LIBS_PATH=%FMOD_LIBS_PATH%

cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DFMOD_LIBS_PATH=%1 -G Ninja
if errorlevel 1 exit /b 1

cmake --build cmake-build-debug --target game -j 10
if errorlevel 1 exit /b 1

echo Debug build finished successfully.
endlocal
pause