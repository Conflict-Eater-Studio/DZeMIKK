@echo off
setlocal

cmake -S . -B cmake-build-relwithdebinfo -DCMAKE_BUILD_TYPE=RelWithDebInfo
if errorlevel 1 exit /b 1

cmake --build cmake-build-relwithdebinfo --target game -j 10
if errorlevel 1 exit /b 1

echo RelWithDebInfo build finished successfully.
endlocal
