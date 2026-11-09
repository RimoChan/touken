@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cd /d "%~dp0"
chcp 65001 >nul
cl.exe /utf-8 /EHsc /O2 /std:c++20 *.cpp /link /subsystem:windows /ENTRY:wWinMainCRTStartup /out:"启动器.exe"
del /q *.obj 2>nul
