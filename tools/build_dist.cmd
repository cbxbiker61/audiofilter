rem @echo off

set ver_text=%1
set ver_file=%2
set timestamp=%date:~8,2%%date:~3,2%%date:~0,2%%time:~0,2%%time:~3,2%
if "%time:~0,1%" == " " set timestamp=%date:~8,2%%date:~3,2%%date:~0,2%0%time:~1,1%%time:~3,2%

if "%ver_text%"=="" set ver_text=test%timestamp%
if "%ver_file%"=="" set ver_file=test%timestamp%

set src_arc=ac3filter_tools_%ver_file%_src.zip
set src_files=tools\*.* valib\*.*

set bin_arc=ac3filter_tools_%ver_file%.zip
set bin_files=tools\Release\*.exe tools\*.txt

set bin64_arc=ac3filter_tools_%ver_file%_x64.zip
set bin64_files=tools\x64\Release\*.exe tools\*.txt

set make_src=pkzip25 -add -rec -dir -excl=CVS -excl=.hg -excl=Debug -excl=Release -excl=x64 -excl=*.ncb -lev=9 %src_arc% %src_files%
set make_bin=pkzip25 -add -lev=9 %bin_arc% %bin_files%
set make_bin64=pkzip25 -add -lev=9 %bin64_arc% %bin64_files%

rem -------------------------------------------------------
rem Clean all and make the source distribution
:build_source

cd ../valib
call clean.cmd
cd ../tools
call clean.cmd

del ..\%bin_arc%
del ..\%bin64_arc%
del ..\%src_arc%

cd ..
%make_src%
if errorlevel 1 goto fail
cd tools

rem -------------------------------------------------------
rem Build projects
:build_projects

call build.cmd win32
if errorlevel 1 goto fail

call build.cmd x64
if errorlevel 1 goto fail

rem -------------------------------------------------------
rem Build the distribution
:build

cd ..
%make_bin%
if errorlevel 1 goto fail

%make_bin64%
if errorlevel 1 goto fail
cd tools

rem -------------------------------------------------------
rem All OK

echo All OK!
goto end

rem -------------------------------------------------------
rem Error messages

:fail
echo Build failed!
error 2>nul
:end
