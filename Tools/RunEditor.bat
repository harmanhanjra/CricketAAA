@echo off
setlocal
if "%UE_ROOT%"=="" (
  echo ERROR: Set UE_ROOT to your Unreal Engine folder.
  exit /b 1
)
start "CricketGame" "%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0..\CricketGame.uproject"
