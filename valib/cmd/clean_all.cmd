@echo off
set DIRS=%*
if "%DIRS%"=="" set DIRS=*

for /d %%d in (%DIRS%) do if exist %%d\clean.cmd (
  cd %%d
  echo Cleaning %%d...
  call clean.cmd
  cd ..
)
