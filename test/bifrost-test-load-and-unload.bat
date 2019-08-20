@echo off
setlocal EnableDelayedExpansion
pushd %~dp0

set interactive=1
echo %cmdcmdline% | find /i "%~0" >nul
if not errorlevel 1 set interactive=0

:: ===================

set build_mode=Debug
if _%1_ neq __ set build_mode=%1 
set bin_path="%~dp0..\build-vs2017\bin\%build_mode%"

call "%~dp0\..\thirdparty\init.bat"
"%BIFROST_PYTHON_DIR%\python.exe" bifrost-test.py test_and_load "%bin_path%"

:: ===================

set EXIT_CODE=%errorlevel%
if _%interactive%_==_0_ pause

popd
endlocal && set EXIT_CODE=%EXIT_CODE%
exit /B %EXIT_CODE%