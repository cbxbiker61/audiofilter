@echo off

rem Visual Studio temp files

del *.aps 2> nul
del *.ncb 2> nul
del *.plg 2> nul
del *.dep 2> nul
del *.opt 2> nul
del /a:h *.suo 2> nul
del *.user 2> nul

rem Build files

rmdir /s /q Debug 2> nul
rmdir /s /q Release 2> nul
rmdir /s /q x64 2> nul
