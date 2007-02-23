# Microsoft Developer Studio Project File - Name="osgPlugin flt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=osgPlugin flt - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "flt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "flt.mak" CFG="osgPlugin flt - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "osgPlugin flt - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "osgPlugin flt - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "osgPlugin flt - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE "osgPlugin flt - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "osgPlugin flt - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../bin/$(PlatformName)"
# PROP Intermediate_Dir "$(PlatformName)/$(ConfigurationName)"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /I "../../../../3rdParty/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FLT_LIBRARY" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x414 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32.lib /nologo /dll /debug /machine:I386 /opt:ref /opt:icf /out:"$(OutDir)/osgdb_flt.dll" /implib:"../../../lib/$(PlatformName)/osgdb_flt.lib" /libpath:"../../../lib/$(PlatformName)" /libpath:"../../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../../Producer/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib"

!ELSEIF  "$(CFG)" == "osgPlugin flt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../bin/$(PlatformName)"
# PROP Intermediate_Dir "$(PlatformName)/$(ConfigurationName)"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /vmg /GR /GX /Zi /Od /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /I "../../../../3rdParty/include" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FLT_LIBRARY" /D "WIN32" /D "_DEBUG" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /GZ /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x414 /d "_DEBUG"
# ADD RSC /l 0x417 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32d.lib /nologo /dll /debug /machine:I386 /out:"$(OutDir)/osgdb_fltd.dll" /pdbtype:sept /implib:"../../../lib/$(PlatformName)/osgdb_fltd.lib" /libpath:"../../../lib/$(PlatformName)" /libpath:"../../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../../Producer/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "osgPlugin flt - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "../../../lib"
# PROP BASE Intermediate_Dir "Release_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../lib/$(PlatformName)"
# PROP Intermediate_Dir "$(PlatformName)/$(ConfigurationName)_Static"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /I "../../../../3rdParty/include" /D "WIN32" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nodefaultlib /nologo /out:"$(OutDir)/osgdb_flt_s.lib"
# SUBTRACT LIB32 /nodefaultlib

!ELSEIF  "$(CFG)" == "osgPlugin flt - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "../../../lib"
# PROP BASE Intermediate_Dir "Debug_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../lib/$(PlatformName)"
# PROP Intermediate_Dir "$(PlatformName)/$(ConfigurationName)_Static"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /vmg /GR /GX /Z7 /Od /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /I "../../../../3rdParty/include" /D "_WINDOWS" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "_MBCS" /D "WIN32" /D "_DEBUG" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nodefaultlib /nologo /out:"$(OutDir)/osgdb_fltd_s.lib"
# SUBTRACT LIB32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "osgPlugin flt - Win32 Release"
# Name "osgPlugin flt - Win32 Debug"
# Name "osgPlugin flt - Win32 Release Static"
# Name "osgPlugin flt - Win32 Debug Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\BoundingVolumeRecords.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\BSPRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ColorPaletteRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\CommentRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ControlRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\DofRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ExtensionRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ExternalRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\FaceRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\FindExternalModelVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\flt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\flt2osg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\FltFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\GeoSetBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\GroupRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\HeaderRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\Input.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\InstanceRecords.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LightPointPaletteRecords.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LightPointRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LightPointSystemRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LightSourcePaletteRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LightSourceRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LocalVertexPoolRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LodRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LongIDRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\MaterialPaletteRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\MeshPrimitiveRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\MeshRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\MultiTextureRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ObjectRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\OldMaterialPaletteRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\OldVertexRecords.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\Pool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ReaderWriterATTR.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ReaderWriterFLT.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\Record.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\RecordVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\Registry.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\RoadRecords.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\SwitchRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\TextureMappingPaletteRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\TexturePaletteRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\TransformationRecords.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\UnknownRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\UVListRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\VertexPoolRecords.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;"
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\AttrData.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\BoundingVolumeRecords.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\BSPRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ColorPaletteRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\CommentRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ControlRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\DofRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ExtensionRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ExternalRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\FaceRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\FindExternalModelVisitor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\flt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\flt2osg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\FltFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\FltRecords.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\GeoSetBuilder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\GroupRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\HeaderRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\Input.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\InstanceRecords.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LightPointPaletteRecords.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LightPointRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LightPointSystemRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LightSourcePaletteRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LightSourceRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LocalVertexPoolRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LodRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\LongIDRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\MaterialPaletteRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\MeshPrimitiveRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\MeshRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\MultiTextureRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ObjectRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\OldMaterialPaletteRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\OldVertexRecords.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\opcodes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\Pool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\ReaderWriterFLT.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\Record.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\RecordVisitor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\Registry.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\RoadRecords.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\SwitchRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\TextureMappingPaletteRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\TexturePaletteRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\TransformationRecords.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\UnknownRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\UVListRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\VertexPoolRecords.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\flt\license.txt
# End Source File
# End Group
# End Target
# End Project

