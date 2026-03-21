@echo off
setlocal

cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
if errorlevel 1 exit /b 1

cmake --build cmake-build-debug --target game -j 10
if errorlevel 1 exit /b 1

echo Debug build finished successfully.
endlocal
