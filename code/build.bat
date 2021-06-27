@echo off

mkdir ..\build
pushd ..\build


cl -Zi ..\code\win32_snake.cpp User32.lib Gdi32.lib Kernel32.lib

popd