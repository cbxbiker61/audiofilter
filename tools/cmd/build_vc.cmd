@echo off

set platform=Win32
set config=Release
set dir=%~dp0

rem -------------------------------------------------------
rem Determine the project name

if exist %1.sln goto set_project
if exist %1.dsw goto set_project

for %%f in (*.dsw) do set workspace=%%~nf
for %%f in (*.sln) do set solution=%%~nf
goto parse2

:set_project
set workspace=%1
set solution=%1
shift

rem -------------------------------------------------------
rem Parse the command line

:parse2
if /i "%1"=="vc6"     set compiler=vc6& shift
if /i "%1"=="vc9"     set compiler=vc9& shift
if /i "%1"=="Win32"   set platform=Win32& shift
if /i "%1"=="x64"     set platform=x64& shift
if /i "%1"=="Debug"   set config=Debug& shift
if /i "%1"=="Release" set config=Release& shift
if not "%1"=="" goto usage
call "%dir%config.cmd"

rem -------------------------------------------------------
rem Detect the compiler

if not defined compiler goto vc9
goto %compiler%

:no_vc9
if not defined compiler goto vc6
goto err_vcbuild

:no_vc6
if not defined compiler goto err_detect
goto err_msdev

rem -------------------------------------------------------
rem Visual Studio 97

:vc6
if defined vc6vars call "%vc6vars%" 2>nul
if exist "%MSDevDir%\bin\msdev.exe" set msdev=%MSDevDir%\bin\msdev.exe
if not defined msdev for %%f in (msdev.exe) do set msdev=%%~$PATH:f
if not defined msdev (
  call vcvars32 2>nul
  for %%v in (msdev.exe) do if exist "%%~$PATH:v" set msdev=%%~$PATH:v
)
if not defined msdev goto no_vc6
if "%workspace%"=="" goto err_workspace

%msdev% %workspace%.dsw /make "%workspace% - Win32 %config%"
goto end

rem -------------------------------------------------------
rem Visual Studio 2008

:vc9
if defined vc9vars call "%vc9vars%" 2>nul
for %%f in (vcbuild.exe) do set vcbuild=%%~$PATH:f
if not defined vcbuild (
  call vcvars32 2>nul
  for %%v in (vcbuild.exe) do if exist "%%~$PATH:v" set vcbuild=%%~$PATH:v
)
if not defined vcbuild goto no_vc9
if "%solution%"=="" goto err_solution

vcbuild %solution%.sln "%config%|%platform%"
goto end

rem -------------------------------------------------------
rem Error messages and usage

:err_detect
echo Error: compiler not found
goto fail

:err_solution
echo Error: no solution (*.sln) file found.
goto fail

:err_workspace
echo Error: no workspace (*.dsw) file found.
goto fail

:err_msdev
echo Error: msdev.exe not found.
goto fail

:err_vcbuild
echo Error: vcbuild.exe not found.
goto fail

:usage
echo Usage: %~n0 [project] [compiler] [platform] [configuration]
echo Where
echp   project: project to compile (without .sln or .dsw extension)
echo   compiler: vc6 (Visual Studio 97) or vc9 (VisualStudio 2008)
echo   paltform: Win32 or x64
echo   configuration: Debug or Release
goto fail

:fail
error 2>nul
:end
