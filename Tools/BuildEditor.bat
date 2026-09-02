@echo off
setlocal
if "%UE_ROOT%"=="" (
  echo ERROR: Set UE_ROOT to your Unreal Engine folder, for example:
  echo   set UE_ROOT=C:\Program Files\Epic Games\UE_5.4
  exit /b 1
)
call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" CricketGameEditor Win64 Development "%~dp0..\CricketGame.uproject" -WaitMutex
