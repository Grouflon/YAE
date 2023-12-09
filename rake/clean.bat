@REM 
@echo Cleaning %1...
@set BIN_DIR=bin\%1\
@set BUILD_DIR=intermediate\build\%1\
@if exist %BIN_DIR% ( rmdir /s /q %BIN_DIR% )
@if exist %BUILD_DIR% ( rmdir /s /q %BUILD_DIR% )
