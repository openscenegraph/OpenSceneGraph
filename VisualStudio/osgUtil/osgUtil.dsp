# Microsoft Developer Studio Project File - Name="Core osgUtil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Core osgUtil - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "osgUtil.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "osgUtil.mak" CFG="Core osgUtil - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Core osgUtil - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Core osgUtil - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Core osgUtil - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "../../lib"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../lib"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../include" /D "NDEBUG" /D "_MBCS" /D "_USRDLL" /D "OSGUTIL_LIBRARY" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 glu32.lib opengl32.lib /nologo /dll /pdb:none /machine:I386 /out:"../../bin/osgUtil.dll" /libpath:"../../lib"

!ELSEIF  "$(CFG)" == "Core osgUtil - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../lib"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /vmg /vd0 /GR /GX /Zi /Od /I "../../include" /D "OSGUTIL_LIBRARY" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 glu32.lib opengl32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/osgUtild.dll" /pdbtype:sept /libpath:"../../lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Core osgUtil - Win32 Release"
# Name "Core osgUtil - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\osgUtil\CubeMapGenerator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\HalfWayMapGenerator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\HighlightMapGenerator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\UpdateVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\CullVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\DelaunayTriangulator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\DisplayListVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\DisplayRequirementsVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\InsertImpostorsVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\IntersectVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\Optimizer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\RenderBin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\RenderGraph.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\RenderLeaf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\RenderStage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\RenderStageLighting.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\RenderToTextureStage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\SceneView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\SmoothingVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\Tesselator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\TransformCallback.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\TransformAttributeFunctor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\TriStripVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\TriStrip_tri_stripper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgUtil\Version.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ";h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\osgUtil\CubeMapGenerator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\HalfWayMapGenerator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\HighlightMapGenerator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\ReflectionMapGenerator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\UpdateVisitor
# End Source File
# Begin Source File

SOURCE=..\..\Include\osgUtil\CullVisitor
# End Source File
# Begin Source File

SOURCE=..\..\Include\osgUtil\DelaunayTriangulator
# End Source File
# Begin Source File

SOURCE=..\..\Include\osgUtil\DisplayListVisitor
# End Source File
# Begin Source File

SOURCE=..\..\Include\osgUtil\DisplayRequirementsVisitor
# End Source File
# Begin Source File

SOURCE=..\..\Include\osgUtil\Export
# End Source File
# Begin Source File

SOURCE=..\..\Include\osgUtil\InsertImpostorsVisitor
# End Source File
# Begin Source File

SOURCE=..\..\Include\osgUtil\IntersectVisitor
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\Optimizer
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\RenderBin
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\RenderGraph
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\RenderLeaf
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\RenderStage
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\RenderStageLighting
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\RenderToTextureStage
# End Source File
# Begin Source File

SOURCE=..\..\Include\osgUtil\SceneView
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\SmoothingVisitor
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\Tesselator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\TransformCallback
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\TransformAttributeFunctor
# End Source File
# Begin Source File

SOURCE=..\..\include\osgUtil\TriStripVisitor
# End Source File
# Begin Source File

SOURCE=..\..\Src\osgUtil\TriStrip_graph_array.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\osgUtil\TriStrip_heap_array.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\osgUtil\TriStrip_tri_stripper.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\osgUtil\Version
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
