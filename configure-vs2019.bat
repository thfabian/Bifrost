@echo off
setlocal EnableDelayedExpansion
pushd %~dp0

set interactive=1
echo %cmdcmdline% | find /i "%~0" >nul
if not errorlevel 1 set interactive=0

:: ===================

:: Get all the thirdpary env variables
call "%~dp0\thirdparty\init.bat"

:: Call preamke
set BIFROST_BUILD_DIR=build-vs2019
"%BIFROST_PREMAKE_DIR%\premake5.exe" vs2019

:: ===================

set EXIT_CODE=%errorlevel%
if _%interactive%_==_0_ pause

popd
endlocal && set EXIT_CODE=%EXIT_CODE%
exit /B %EXIT_CODE%