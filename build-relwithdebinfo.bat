@echo off
setlocal
echo %1

cmake -S . -B cmake-build-relwithdebinfo -DCMAKE_BUILD_TYPE=RelWithDebInfo -DFMOD_LIBS_PATH=%1
if errorlevel 1 exit /b 1

cmake --build cmake-build-relwithdebinfo --target game -j 10
if errorlevel 1 exit /b 1

echo RelWithDebInfo build finished successfully.
endlocal
pause