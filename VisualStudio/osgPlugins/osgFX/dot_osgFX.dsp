# Microsoft Developer Studio Project File - Name="osgPlugin osgFX" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=osgPlugin osgFX - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dot_osgFX.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dot_osgFX.mak" CFG="osgPlugin osgFX - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "osgPlugin osgFX - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "osgPlugin osgFX - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "osgPlugin osgFX - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "../../../bin"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../bin"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../../../include" /I "../../../../OpenThreads/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DOT_OSGNV_EXPORTS" /D "_MBCS" /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../../../include" /I "../../../../OpenThreads/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DOT_OSGNV_EXPORTS" /D "_MBCS" /GZ /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib osgd.lib osgDBd.lib /nologo /subsystem:windows /dll /pdb:"..\..\..\bin\dot_osgFXd.pdb" /debug /machine:IX86 /out:"..\..\..\bin\osgdb_osgFXd.dll" /implib:"../../../lib/osgdb_osgFXd.lib" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:"..\..\..\bin\dot_osgFXd.pdb" /debug /machine:IX86 /out:"..\..\..\bin\osgdb_osgFXd.dll" /implib:"../../../lib/osgdb_osgFXd.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "osgPlugin osgFX - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "../../../bin"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../bin"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "../../../include" /I "../../../../OpenThreads/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DOT_OSGNV_EXPORTS" /D "_MBCS" /GF /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "../../../include" /I "../../../../OpenThreads/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DOT_OSGNV_EXPORTS" /D "_MBCS" /GF /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib osg.lib osgDB.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"..\..\..\bin\osgdb_osgFX.dll" /implib:"../../../lib/osgdb_osgFX.lib" /pdbtype:sept /opt:ref /opt:icf
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"..\..\..\bin\osgdb_osgFX.dll" /implib:"../../../lib/osgdb_osgFX.lib" /pdbtype:sept /opt:ref /opt:icf

!ENDIF 

# Begin Target

# Name "osgPlugin osgFX - Win32 Debug"
# Name "osgPlugin osgFX - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm"
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgFX\IO_AnisotropicLighting.cpp
DEP_CPP_IO_AN=\
	"..\..\..\Include\osg\ArgumentParser"\
	"..\..\..\Include\Osg\Array"\
	"..\..\..\Include\Osg\BoundingBox"\
	"..\..\..\Include\Osg\BoundingSphere"\
	"..\..\..\include\osg\buffered_value"\
	"..\..\..\Include\Osg\ConvexPlanarOccluder"\
	"..\..\..\Include\Osg\ConvexPlanarPolygon"\
	"..\..\..\Include\Osg\CopyOp"\
	"..\..\..\include\osg\DisplaySettings"\
	"..\..\..\Include\Osg\Drawable"\
	"..\..\..\Include\Osg\Export"\
	"..\..\..\include\osg\fast_back_stack"\
	"..\..\..\include\osg\FrameStamp"\
	"..\..\..\Include\Osg\Geode"\
	"..\..\..\Include\Osg\Gl"\
	"..\..\..\Include\Osg\Group"\
	"..\..\..\Include\Osg\Image"\
	"..\..\..\include\osg\Math"\
	"..\..\..\Include\Osg\Matrix"\
	"..\..\..\Include\Osg\Node"\
	"..\..\..\include\osg\NodeCallback"\
	"..\..\..\Include\Osg\NodeVisitor"\
	"..\..\..\Include\Osg\Object"\
	"..\..\..\Include\Osg\OccluderNode"\
	"..\..\..\Include\Osg\Plane"\
	"..\..\..\Include\Osg\Polytope"\
	"..\..\..\Include\Osg\Quat"\
	"..\..\..\include\osg\ref_ptr"\
	"..\..\..\Include\Osg\Referenced"\
	"..\..\..\Include\Osg\Shape"\
	"..\..\..\Include\Osg\State"\
	"..\..\..\Include\Osg\StateAttribute"\
	"..\..\..\Include\Osg\StateSet"\
	"..\..\..\Include\Osg\Texture"\
	"..\..\..\Include\Osg\Texture2D"\
	"..\..\..\include\osg\UByte4"\
	"..\..\..\Include\Osg\Vec2"\
	"..\..\..\Include\Osg\Vec3"\
	"..\..\..\Include\Osg\Vec4"\
	"..\..\..\include\osg\Viewport"\
	"..\..\..\Include\osgDB\DotOsgWrapper"\
	"..\..\..\Include\osgDB\DynamicLibrary"\
	"..\..\..\Include\osgDB\Export"\
	"..\..\..\Include\osgDB\Field"\
	"..\..\..\Include\osgDB\FieldReader"\
	"..\..\..\Include\osgDB\FieldReaderIterator"\
	"..\..\..\Include\osgDB\Input"\
	"..\..\..\Include\osgDB\Output"\
	"..\..\..\include\osgDB\ReaderWriter"\
	"..\..\..\Include\osgDB\Registry"\
	"..\..\..\include\osgFX\AnisotropicLighting"\
	"..\..\..\include\osgFX\Effect"\
	"..\..\..\include\osgFX\Export"\
	"..\..\..\include\osgFX\Technique"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgFX\IO_BumpMapping.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgFX\IO_Cartoon.cpp
DEP_CPP_IO_CA=\
	"..\..\..\Include\osg\ArgumentParser"\
	"..\..\..\Include\Osg\Array"\
	"..\..\..\Include\Osg\BoundingBox"\
	"..\..\..\Include\Osg\BoundingSphere"\
	"..\..\..\include\osg\buffered_value"\
	"..\..\..\Include\Osg\ConvexPlanarOccluder"\
	"..\..\..\Include\Osg\ConvexPlanarPolygon"\
	"..\..\..\Include\Osg\CopyOp"\
	"..\..\..\include\osg\DisplaySettings"\
	"..\..\..\Include\Osg\Drawable"\
	"..\..\..\Include\Osg\Export"\
	"..\..\..\include\osg\fast_back_stack"\
	"..\..\..\include\osg\FrameStamp"\
	"..\..\..\Include\Osg\Geode"\
	"..\..\..\Include\Osg\Gl"\
	"..\..\..\Include\Osg\Group"\
	"..\..\..\Include\Osg\Image"\
	"..\..\..\Include\Osg\LineWidth"\
	"..\..\..\Include\Osg\Material"\
	"..\..\..\include\osg\Math"\
	"..\..\..\Include\Osg\Matrix"\
	"..\..\..\Include\Osg\Node"\
	"..\..\..\include\osg\NodeCallback"\
	"..\..\..\Include\Osg\NodeVisitor"\
	"..\..\..\Include\Osg\Object"\
	"..\..\..\Include\Osg\OccluderNode"\
	"..\..\..\Include\Osg\Plane"\
	"..\..\..\Include\Osg\Polytope"\
	"..\..\..\Include\Osg\Quat"\
	"..\..\..\include\osg\ref_ptr"\
	"..\..\..\Include\Osg\Referenced"\
	"..\..\..\Include\Osg\Shape"\
	"..\..\..\Include\Osg\State"\
	"..\..\..\Include\Osg\StateAttribute"\
	"..\..\..\Include\Osg\StateSet"\
	"..\..\..\include\osg\UByte4"\
	"..\..\..\Include\Osg\Vec2"\
	"..\..\..\Include\Osg\Vec3"\
	"..\..\..\Include\Osg\Vec4"\
	"..\..\..\include\osg\Viewport"\
	"..\..\..\Include\osgDB\DotOsgWrapper"\
	"..\..\..\Include\osgDB\DynamicLibrary"\
	"..\..\..\Include\osgDB\Export"\
	"..\..\..\Include\osgDB\Field"\
	"..\..\..\Include\osgDB\FieldReader"\
	"..\..\..\Include\osgDB\FieldReaderIterator"\
	"..\..\..\Include\osgDB\Input"\
	"..\..\..\Include\osgDB\Output"\
	"..\..\..\include\osgDB\ReaderWriter"\
	"..\..\..\Include\osgDB\Registry"\
	"..\..\..\include\osgFX\Cartoon"\
	"..\..\..\include\osgFX\Effect"\
	"..\..\..\include\osgFX\Export"\
	"..\..\..\include\osgFX\Technique"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgFX\IO_Effect.cpp
DEP_CPP_IO_EF=\
	"..\..\..\Include\osg\ArgumentParser"\
	"..\..\..\Include\Osg\Array"\
	"..\..\..\Include\Osg\BoundingBox"\
	"..\..\..\Include\Osg\BoundingSphere"\
	"..\..\..\include\osg\buffered_value"\
	"..\..\..\Include\Osg\ConvexPlanarOccluder"\
	"..\..\..\Include\Osg\ConvexPlanarPolygon"\
	"..\..\..\Include\Osg\CopyOp"\
	"..\..\..\include\osg\DisplaySettings"\
	"..\..\..\Include\Osg\Drawable"\
	"..\..\..\Include\Osg\Export"\
	"..\..\..\include\osg\fast_back_stack"\
	"..\..\..\include\osg\FrameStamp"\
	"..\..\..\Include\Osg\Geode"\
	"..\..\..\Include\Osg\Gl"\
	"..\..\..\Include\Osg\Group"\
	"..\..\..\Include\Osg\Image"\
	"..\..\..\include\osg\Math"\
	"..\..\..\Include\Osg\Matrix"\
	"..\..\..\Include\Osg\Node"\
	"..\..\..\include\osg\NodeCallback"\
	"..\..\..\Include\Osg\NodeVisitor"\
	"..\..\..\Include\Osg\Object"\
	"..\..\..\Include\Osg\OccluderNode"\
	"..\..\..\Include\Osg\Plane"\
	"..\..\..\Include\Osg\Polytope"\
	"..\..\..\Include\Osg\Quat"\
	"..\..\..\include\osg\ref_ptr"\
	"..\..\..\Include\Osg\Referenced"\
	"..\..\..\Include\Osg\Shape"\
	"..\..\..\Include\Osg\State"\
	"..\..\..\Include\Osg\StateAttribute"\
	"..\..\..\Include\Osg\StateSet"\
	"..\..\..\include\osg\UByte4"\
	"..\..\..\Include\Osg\Vec2"\
	"..\..\..\Include\Osg\Vec3"\
	"..\..\..\Include\Osg\Vec4"\
	"..\..\..\include\osg\Viewport"\
	"..\..\..\Include\osgDB\DotOsgWrapper"\
	"..\..\..\Include\osgDB\DynamicLibrary"\
	"..\..\..\Include\osgDB\Export"\
	"..\..\..\Include\osgDB\Field"\
	"..\..\..\Include\osgDB\FieldReader"\
	"..\..\..\Include\osgDB\FieldReaderIterator"\
	"..\..\..\Include\osgDB\Input"\
	"..\..\..\Include\osgDB\Output"\
	"..\..\..\include\osgDB\ReaderWriter"\
	"..\..\..\Include\osgDB\Registry"\
	"..\..\..\include\osgFX\Effect"\
	"..\..\..\include\osgFX\Export"\
	"..\..\..\include\osgFX\Technique"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgFX\IO_Scribe.cpp
DEP_CPP_IO_SC=\
	"..\..\..\Include\osg\ArgumentParser"\
	"..\..\..\Include\Osg\Array"\
	"..\..\..\Include\Osg\BoundingBox"\
	"..\..\..\Include\Osg\BoundingSphere"\
	"..\..\..\include\osg\buffered_value"\
	"..\..\..\Include\Osg\ConvexPlanarOccluder"\
	"..\..\..\Include\Osg\ConvexPlanarPolygon"\
	"..\..\..\Include\Osg\CopyOp"\
	"..\..\..\include\osg\DisplaySettings"\
	"..\..\..\Include\Osg\Drawable"\
	"..\..\..\Include\Osg\Export"\
	"..\..\..\include\osg\fast_back_stack"\
	"..\..\..\include\osg\FrameStamp"\
	"..\..\..\Include\Osg\Geode"\
	"..\..\..\Include\Osg\Gl"\
	"..\..\..\Include\Osg\Group"\
	"..\..\..\Include\Osg\Image"\
	"..\..\..\Include\Osg\LineWidth"\
	"..\..\..\Include\Osg\Material"\
	"..\..\..\include\osg\Math"\
	"..\..\..\Include\Osg\Matrix"\
	"..\..\..\Include\Osg\Node"\
	"..\..\..\include\osg\NodeCallback"\
	"..\..\..\Include\Osg\NodeVisitor"\
	"..\..\..\Include\Osg\Object"\
	"..\..\..\Include\Osg\OccluderNode"\
	"..\..\..\Include\Osg\Plane"\
	"..\..\..\Include\Osg\Polytope"\
	"..\..\..\Include\Osg\Quat"\
	"..\..\..\include\osg\ref_ptr"\
	"..\..\..\Include\Osg\Referenced"\
	"..\..\..\Include\Osg\Shape"\
	"..\..\..\Include\Osg\State"\
	"..\..\..\Include\Osg\StateAttribute"\
	"..\..\..\Include\Osg\StateSet"\
	"..\..\..\include\osg\UByte4"\
	"..\..\..\Include\Osg\Vec2"\
	"..\..\..\Include\Osg\Vec3"\
	"..\..\..\Include\Osg\Vec4"\
	"..\..\..\include\osg\Viewport"\
	"..\..\..\Include\osgDB\DotOsgWrapper"\
	"..\..\..\Include\osgDB\DynamicLibrary"\
	"..\..\..\Include\osgDB\Export"\
	"..\..\..\Include\osgDB\Field"\
	"..\..\..\Include\osgDB\FieldReader"\
	"..\..\..\Include\osgDB\FieldReaderIterator"\
	"..\..\..\Include\osgDB\Input"\
	"..\..\..\Include\osgDB\Output"\
	"..\..\..\include\osgDB\ReaderWriter"\
	"..\..\..\Include\osgDB\Registry"\
	"..\..\..\include\osgFX\Effect"\
	"..\..\..\include\osgFX\Export"\
	"..\..\..\include\osgFX\Scribe"\
	"..\..\..\include\osgFX\Technique"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgFX\IO_SpecularHighlights.cpp
DEP_CPP_IO_SP=\
	"..\..\..\Include\osg\ArgumentParser"\
	"..\..\..\Include\Osg\Array"\
	"..\..\..\Include\Osg\BoundingBox"\
	"..\..\..\Include\Osg\BoundingSphere"\
	"..\..\..\include\osg\buffered_value"\
	"..\..\..\Include\Osg\ConvexPlanarOccluder"\
	"..\..\..\Include\Osg\ConvexPlanarPolygon"\
	"..\..\..\Include\Osg\CopyOp"\
	"..\..\..\include\osg\DisplaySettings"\
	"..\..\..\Include\Osg\Drawable"\
	"..\..\..\Include\Osg\Export"\
	"..\..\..\include\osg\fast_back_stack"\
	"..\..\..\include\osg\FrameStamp"\
	"..\..\..\Include\Osg\Geode"\
	"..\..\..\Include\Osg\Gl"\
	"..\..\..\Include\Osg\Group"\
	"..\..\..\Include\Osg\Image"\
	"..\..\..\include\osg\Math"\
	"..\..\..\Include\Osg\Matrix"\
	"..\..\..\Include\Osg\Node"\
	"..\..\..\include\osg\NodeCallback"\
	"..\..\..\Include\Osg\NodeVisitor"\
	"..\..\..\Include\Osg\Object"\
	"..\..\..\Include\Osg\OccluderNode"\
	"..\..\..\Include\Osg\Plane"\
	"..\..\..\Include\Osg\Polytope"\
	"..\..\..\Include\Osg\Quat"\
	"..\..\..\include\osg\ref_ptr"\
	"..\..\..\Include\Osg\Referenced"\
	"..\..\..\Include\Osg\Shape"\
	"..\..\..\Include\Osg\State"\
	"..\..\..\Include\Osg\StateAttribute"\
	"..\..\..\Include\Osg\StateSet"\
	"..\..\..\include\osg\UByte4"\
	"..\..\..\Include\Osg\Vec2"\
	"..\..\..\Include\Osg\Vec3"\
	"..\..\..\Include\Osg\Vec4"\
	"..\..\..\include\osg\Viewport"\
	"..\..\..\Include\osgDB\DotOsgWrapper"\
	"..\..\..\Include\osgDB\DynamicLibrary"\
	"..\..\..\Include\osgDB\Export"\
	"..\..\..\Include\osgDB\Field"\
	"..\..\..\Include\osgDB\FieldReader"\
	"..\..\..\Include\osgDB\FieldReaderIterator"\
	"..\..\..\Include\osgDB\Input"\
	"..\..\..\Include\osgDB\Output"\
	"..\..\..\include\osgDB\ReaderWriter"\
	"..\..\..\Include\osgDB\Registry"\
	"..\..\..\include\osgFX\Effect"\
	"..\..\..\include\osgFX\Export"\
	"..\..\..\include\osgFX\SpecularHighlights"\
	"..\..\..\include\osgFX\Technique"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
