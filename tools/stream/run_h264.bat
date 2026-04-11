@echo off
setlocal
set "SCRIPT=%~dp0stream_h264.ps1"
powershell -NoProfile -ExecutionPolicy Bypass -NoExit -File "%SCRIPT%"
