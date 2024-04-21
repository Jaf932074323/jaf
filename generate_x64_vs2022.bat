@echo off
cd /d %~dp0
set curpath=%~dp0

if not exist build md build
cd build
if not exist x64 md x64
cd x64
if not exist windows md windows
cd windows

if exist %curpath%setenv.gitignore.bat call %curpath%setenv.gitignore.bat

cmake -G"Visual Studio 17 2022" -DHOST_PLATFORM=PLATFORM_X64 -DHOST_OS=WINDOWS ../../../src
@pause
