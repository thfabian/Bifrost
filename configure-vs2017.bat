@echo off
setlocal EnableDelayedExpansion
pushd %~dp0

:: Check if we are called from within a cmd.exe or double clicked
set interactive=1
echo %cmdcmdline% | find /i "%~0" >nul
if not errorlevel 1 set interactive=0

:: Get all the thirdpary env variables
call "%~dp0\thirdparty\init.bat"

:: Call preamke
set BIFROST_BUILD_DIR=build-vs2017
@RD /S /Q "%~dp0\%BIFROST_BUILD_DIR%"
"%BIFROST_PREMAKE_DIR%\premake5.exe" vs2017
set EXIT_CODE=%errorlevel%

if _%interactive%_==_0_ pause

popd
endlocal && set EXIT_CODE=%EXIT_CODE%
exit /B %EXIT_CODE%
