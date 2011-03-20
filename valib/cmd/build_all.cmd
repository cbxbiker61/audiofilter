@echo off

set platform=Win32
set config=Release

if /i "%1"=="vc6"     set compiler=vc6& shift
if /i "%1"=="vc9"     set compiler=vc9& shift
if /i "%1"=="Win32"   set platform=Win32& shift
if /i "%1"=="x64"     set platform=x64& shift
if /i "%1"=="Debug"   set config=Debug& shift
if /i "%1"=="Release" set config=Release& shift

set OLD_DIR=%CD%
set DIRS=%*
if "%DIRS%"=="" set DIRS=*

for /d %%d in (%DIRS%) do if exist %%d\build.cmd (
  cd %%d
  echo Building %%d...
  call build.cmd %compiler% %platform% %config%
  if errorlevel 1 echo Build error at %cd%\%%d& goto err
  cd %OLD_DIR%
)
goto end

:err
cd %OLD_DIR%
error 2>nul

:end
