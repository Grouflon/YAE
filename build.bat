@echo off

set BIN_PATH=.\bin\
set OBJ_PATH=.\obj\
set EXE_NAME=yae.exe

if [%1] == [] goto BUILD
if [%1] == [clean] goto CLEAN
if [%1] == [build] goto BUILD
IF [%1] == [run] GOTO RUN

:BUILD
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

set SOURCE=^
.\src\main.cpp ^
.\src\Logger.cpp ^
.\src\HashTools.cpp ^
.\src\StringHash.cpp

set LIBS=^
.\extern\GLFW\lib-vc2019\glfw3.lib ^
User32.lib ^
Gdi32.lib ^
Shell32.lib

set INCLUDE_PATHS=^
/I.\src\ ^
/I.\extern\GLFW\include\ ^
/I.\extern\glm\ ^
/I %VK_SDK_PATH%\include\

if not exist %BIN_PATH% (mkdir %BIN_PATH%)
if not exist %OBJ_PATH% (mkdir %OBJ_PATH%)

:: /MD - causes the application to use the multithread-specific and DLL-specific version of the run-time library.
:: /Zi - produces a separate PDB file that contains all the symbolic debugging information for use with the debugger.
cl /MD /Zi /Fo%OBJ_PATH% /Fd%BIN_PATH% /Fe%BIN_PATH%%EXE_NAME% %INCLUDE_PATHS% %SOURCE% %LIBS%
echo -Build done-
goto:EOF

:CLEAN
if exist %BIN_PATH% rd /s /q %BIN_PATH%
if exist %OBJ_PATH% rd /s /q %OBJ_PATH%
echo -Clean done-
goto:EOF

:RUN
call "%BIN_PATH%%EXE_NAME%"
echo -Done running-
goto:EOF
