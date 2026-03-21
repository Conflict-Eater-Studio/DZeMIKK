@echo off
setlocal

cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1

cmake --build cmake-build-release --target game -j 10
if errorlevel 1 exit /b 1

echo Release build finished successfully.
endlocal
