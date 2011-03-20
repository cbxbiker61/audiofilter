# Microsoft Developer Studio Project File - Name="test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=test - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "test.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "test.mak" CFG="test - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "test - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "test"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "test - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /Zi /O2 /Oy- /Ob2 /I "..\valib" /I "..\liba52" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "LIBA52_DOUBLE" /D "AC3_DEBUG" /D "AC3_DEBUG_NODITHER" /D "AC3_DEBUG_NOIMDCT" /D "TIME_WIN32" /FAs /Fr /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /map /debug /debugtype:both /machine:I386
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "test - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /GX /ZI /Od /I "..\valib" /I "..\liba52" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "LIBA52_DOUBLE" /D "AC3_DEBUG" /D "AC3_DEBUG_NODITHER" /D "AC3_DEBUG_NOIMDCT" /D "TIME_WIN32" /FAs /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /debug /machine:I386

!ENDIF 

# Begin Target

# Name "test - Win32 Release"
# Name "test - Win32 Debug"
# Begin Group "tests"

# PROP Default_Filter ""
# Begin Group "common_tests"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tests\test_base.cpp
# End Source File
# Begin Source File

SOURCE=.\tests\test_fir.cpp
# End Source File
# Begin Source File

SOURCE=.\tests\test_general.cpp
# End Source File
# Begin Source File

SOURCE=.\tests\test_rng.cpp
# End Source File
# End Group
# Begin Group "filter_tests"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tests\test_bitstream.cpp
# End Source File
# Begin Source File

SOURCE=.\tests\filters\test_convolver.cpp
# End Source File
# Begin Source File

SOURCE=.\tests\filters\test_convolver_mch.cpp
# End Source File
# Begin Source File

SOURCE=.\tests\filters\test_linear_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\tests\filters\test_proc.cpp
# End Source File
# Begin Source File

SOURCE=.\tests\filters\test_resample.cpp
# End Source File
# Begin Source File

SOURCE=.\tests\filters\test_slice.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\test_bs_convert.cpp
# End Source File
# Begin Source File

SOURCE=.\test_crash.cpp
# End Source File
# Begin Source File

SOURCE=.\test_crc.cpp
# End Source File
# Begin Source File

SOURCE=.\test_decodergraph.cpp
# End Source File
# Begin Source File

SOURCE=.\test_demux.cpp
# End Source File
# Begin Source File

SOURCE=.\test_despdifer.cpp
# End Source File
# Begin Source File

SOURCE=.\test_detector.cpp
# End Source File
# Begin Source File

SOURCE=.\test_dvdgraph.cpp
# End Source File
# Begin Source File

SOURCE=.\test_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\test_filtergraph.cpp
# End Source File
# Begin Source File

SOURCE=.\test_old_style.cpp
# End Source File
# Begin Source File

SOURCE=.\test_parser_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\test_spdifer.cpp
# End Source File
# Begin Source File

SOURCE=.\test_streambuf.cpp
# End Source File
# Begin Source File

SOURCE=.\test_suite.cpp
# End Source File
# Begin Source File

SOURCE=.\test_syncer.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\all_filters.h
# End Source File
# Begin Source File

SOURCE=.\suite.cpp
# End Source File
# Begin Source File

SOURCE=.\suite.h
# End Source File
# Begin Source File

SOURCE=.\test.cpp
# End Source File
# Begin Source File

SOURCE=.\test_source.h
# End Source File
# End Target
# End Project
