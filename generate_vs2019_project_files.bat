@echo off

tools\premake5.exe --file=premake5.lua vs2019

::IF "%1"=="--no-prompt" goto :done
:: pause so the user can see the output if they double clicked the configure script
::pause

:done
