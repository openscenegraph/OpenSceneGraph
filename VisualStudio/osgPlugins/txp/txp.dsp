# Microsoft Developer Studio Project File - Name="osgPlugin txp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=osgPlugin txp - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "txp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "txp.mak" CFG="osgPlugin txp - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "osgPlugin txp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "osgPlugin txp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "osgPlugin txp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../lib"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../../include" /I "../../../src/osgPlugins/txp" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OSGTXP_LIBRARY" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41a /d "NDEBUG"
# ADD RSC /l 0x41a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 OpenThreadsWin32.lib /nologo /dll /pdb:none /machine:I386 /nodefaultlib:"LIBC" /out:"../../../bin/osgdb_txp.dll" /libpath:"../../../lib"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "osgPlugin txp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../lib"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /vmg /vd0 /GR /GX /Zi /Od /I "../../../include" /I "../../../src/osgPlugins/txp" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D __COMP_DLL=7931 /D "WIN32" /D "_DEBUG" /D "OSGTXP_LIBRARY" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41a /d "_DEBUG"
# ADD RSC /l 0x41a /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32d.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBC" /out:"../../../bin/osgdb_txpd.dll" /pdbtype:sept /libpath:"../../../lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "osgPlugin txp - Win32 Release"
# Name "osgPlugin txp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\ReaderWriterTXP.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPArchive.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPPagedLOD.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPPageManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPSeamLOD.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPTileNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TileMapper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_basic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_compat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_geom.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_header.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_label.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_light.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_managers.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_material.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_model.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_nodes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_parse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_pparse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_print.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_range.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_rarchive.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_readbuf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_scene.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_swap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_tile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_warchive.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_writebuf.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\ReaderWriterTXP.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPArchive.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPNode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPPagedLOD.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPPageManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPParser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPSeamLOD.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TXPTileNode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\TileMapper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trdll.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_compat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_geom.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_ident.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_io.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_managers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_print.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_read.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_scene.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_swap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_sys.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_util.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\txp\trpage_write.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
