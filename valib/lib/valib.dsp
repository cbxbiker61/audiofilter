# Microsoft Developer Studio Project File - Name="valib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=valib - Win32 Debug Libc
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "valib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "valib.mak" CFG="valib - Win32 Debug Libc"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "valib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "valib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "valib - Win32 Perf" (based on "Win32 (x86) Static Library")
!MESSAGE "valib - Win32 Debug Libc" (based on "Win32 (x86) Static Library")
!MESSAGE "valib - Win32 Release Libc" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "valib"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "valib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "valib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG.DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x417 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "valib - Win32 Perf"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Perf"
# PROP BASE Intermediate_Dir "Perf"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Perf"
# PROP Intermediate_Dir "Perf"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O2 /Ob2 /I "..\valib" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "DOUBLE_SAMPLE" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "valib - Win32 Debug Libc"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "valib___Win32_Debug_Libc"
# PROP BASE Intermediate_Dir "valib___Win32_Debug_Libc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Libc"
# PROP Intermediate_Dir "Debug_Libc"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MDd /W3 /Gm /Zi /Od /I "..\valib" /D "WIN32" /D "_DEBUG.DEBUG" /D "_MBCS" /D "_LIB" /D "TIME_WIN32" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG.DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x417 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "valib - Win32 Release Libc"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "valib___Win32_Release_Libc"
# PROP BASE Intermediate_Dir "valib___Win32_Release_Libc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Libc"
# PROP Intermediate_Dir "Release_Libc"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /O2 /Ob2 /I "..\valib" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "TIME_WIN32" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "valib - Win32 Release"
# Name "valib - Win32 Debug"
# Name "valib - Win32 Perf"
# Name "valib - Win32 Debug Libc"
# Name "valib - Win32 Release Libc"
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\valib\auto_file.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\auto_file.h
# End Source File
# Begin Source File

SOURCE=..\valib\bitstream.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\bitstream.h
# End Source File
# Begin Source File

SOURCE=..\valib\crc.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\crc.h
# End Source File
# Begin Source File

SOURCE=..\valib\data.h
# End Source File
# Begin Source File

SOURCE=..\valib\defs.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\defs.h
# End Source File
# Begin Source File

SOURCE=..\valib\filter.h
# End Source File
# Begin Source File

SOURCE=..\valib\filter_graph.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filter_graph.h
# End Source File
# Begin Source File

SOURCE=..\valib\filter_tester.h
# End Source File
# Begin Source File

SOURCE=..\valib\fir.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\fir.h
# End Source File
# Begin Source File

SOURCE=..\valib\log.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\log.h
# End Source File
# Begin Source File

SOURCE=..\valib\mpeg_demux.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\mpeg_demux.h
# End Source File
# Begin Source File

SOURCE=..\valib\parser.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parser.h
# End Source File
# Begin Source File

SOURCE=..\valib\renderer.h
# End Source File
# Begin Source File

SOURCE=..\valib\rng.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\rng.h
# End Source File
# Begin Source File

SOURCE=..\valib\spk.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\spk.h
# End Source File
# Begin Source File

SOURCE=..\valib\sync.h
# End Source File
# Begin Source File

SOURCE=..\valib\syncscan.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\syncscan.h
# End Source File
# Begin Source File

SOURCE=..\valib\vargs.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\vargs.h
# End Source File
# Begin Source File

SOURCE=..\valib\vtime.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\vtime.h
# End Source File
# End Group
# Begin Group "filters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\valib\filters\agc.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\agc.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\bass_redir.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\bass_redir.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\cache.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\cache.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\convert.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\convert.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\convert_func.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\convert_func.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\convert_linear2pcm.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\convert_pcm2linear.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\convolver.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\convolver.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\convolver_mch.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\convolver_mch.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\counter.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\decoder.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\decoder.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\decoder_graph.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\decoder_graph.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\dejitter.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\dejitter.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\delay.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\delay.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\demux.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\demux.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\detector.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\detector.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\dither.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\dither.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\dvd_graph.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\dvd_graph.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\equalizer.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\equalizer.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\equalizer_mch.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\equalizer_mch.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\gain.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\gain.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\levels.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\levels.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\linear_filter.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\linear_filter.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\mixer.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\mixer.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\parser_filter.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\parser_filter.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\proc.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\proc.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\proc_state.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\proc_state.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\resample.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\resample.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\slice.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\slice.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\spdifer.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\spdifer.h
# End Source File
# Begin Source File

SOURCE=..\valib\filters\spectrum.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\filters\spectrum.h
# End Source File
# End Group
# Begin Group "parsers"

# PROP Default_Filter ""
# Begin Group "ac3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_bitalloc.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_bitalloc.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_defs.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_dither.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_enc.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_enc.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_header.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_header.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_imdct.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_imdct.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_mdct.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_mdct.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_parser.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\ac3\ac3_tables.h
# End Source File
# End Group
# Begin Group "mpa"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\valib\parsers\mpa\mpa_defs.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\mpa\mpa_header.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\mpa\mpa_header.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\mpa\mpa_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\mpa\mpa_parser.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\mpa\mpa_synth.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\mpa\mpa_synth.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\mpa\mpa_synth_filter.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\mpa\mpa_tables.h
# End Source File
# End Group
# Begin Group "dts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\valib\parsers\dts\dts_defs.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\dts\dts_header.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\dts\dts_header.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\dts\dts_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\dts\dts_parser.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\dts\dts_tables.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\dts\dts_tables_adpcm.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\dts\dts_tables_fir.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\dts\dts_tables_huffman.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\dts\dts_tables_quantization.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\dts\dts_tables_vq.h
# End Source File
# End Group
# Begin Group "spdif"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\valib\parsers\spdif\spdif_header.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\spdif\spdif_header.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\spdif\spdif_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\spdif\spdif_parser.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\spdif\spdif_wrapper.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\spdif\spdif_wrapper.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\valib\parsers\file_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\file_parser.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\multi_frame.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\multi_frame.h
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\multi_header.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\parsers\multi_header.h
# End Source File
# End Group
# Begin Group "sink"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\valib\sink\sink_dshow.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\sink\sink_dshow.h
# End Source File
# Begin Source File

SOURCE=..\valib\sink\sink_dsound.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\sink\sink_dsound.h
# End Source File
# Begin Source File

SOURCE=..\valib\sink\sink_raw.h
# End Source File
# Begin Source File

SOURCE=..\valib\sink\sink_wav.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\sink\sink_wav.h
# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\valib\win32\cpu.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\win32\cpu.h
# End Source File
# Begin Source File

SOURCE=..\valib\win32\thread.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\win32\thread.h
# End Source File
# Begin Source File

SOURCE=..\valib\win32\winspk.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\win32\winspk.h
# End Source File
# End Group
# Begin Group "source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\valib\source\dsound_source.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\source\dsound_source.h
# End Source File
# Begin Source File

SOURCE=..\valib\source\generator.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\source\generator.h
# End Source File
# Begin Source File

SOURCE=..\valib\source\raw_source.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\source\raw_source.h
# End Source File
# Begin Source File

SOURCE=..\valib\source\wav_source.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\source\wav_source.h
# End Source File
# End Group
# Begin Group "dsp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\valib\dsp\dbesi0.c
# End Source File
# Begin Source File

SOURCE=..\valib\dsp\dbesi0.h
# End Source File
# Begin Source File

SOURCE=..\valib\dsp\fft.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\dsp\fft.h
# End Source File
# Begin Source File

SOURCE=..\valib\dsp\fftsg.c
# End Source File
# Begin Source File

SOURCE=..\valib\dsp\fftsg.h
# End Source File
# Begin Source File

SOURCE=..\valib\dsp\kaiser.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\dsp\kaiser.h
# End Source File
# End Group
# Begin Group "fir"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\valib\fir\delay_fir.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\fir\delay_fir.h
# End Source File
# Begin Source File

SOURCE=..\valib\fir\echo_fir.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\fir\echo_fir.h
# End Source File
# Begin Source File

SOURCE=..\valib\fir\eq_fir.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\fir\eq_fir.h
# End Source File
# Begin Source File

SOURCE=..\valib\fir\multi_fir.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\fir\multi_fir.h
# End Source File
# Begin Source File

SOURCE=..\valib\fir\parallel_fir.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\fir\parallel_fir.h
# End Source File
# Begin Source File

SOURCE=..\valib\fir\param_fir.cpp
# End Source File
# Begin Source File

SOURCE=..\valib\fir\param_fir.h
# End Source File
# End Group
# End Target
# End Project
