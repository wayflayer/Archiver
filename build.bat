@echo off
setlocal

rmdir /s /q build 2>nul

cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

if %errorlevel% equ 0 (
    echo Сборка завершена. Запуск: build\Release\myapp.exe
    build\Release\myapp.exe
) else (
    echo Ошибка сборки
)
pause