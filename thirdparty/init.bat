@echo off
pushd %~dp0

for /f "delims=" %%D in ('dir /a:d /b') do (
  if exist "%%~fD\init.bat" (
    call "%%~fD\init.bat"
  )
)

popd
