# Microsoft Developer Studio Project File - Name="osgPlugin ive" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=osgPlugin ive - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ive.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ive.mak" CFG="osgPlugin ive - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "osgPlugin ive - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "osgPlugin ive - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "osgPlugin ive - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /dll /pdb:none /machine:I386 /nodefaultlib:"LIBC" /out:"../../../bin/osgdb_ive.dll" /libpath:"../../../lib"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "osgPlugin ive - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /vmg /vd0 /GR /GX /Zi /Od /I "../../../include" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WIN32" /D "_DEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBC" /out:"../../../bin/osgdb_ived.dll" /pdbtype:sept /libpath:"../../../lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "osgPlugin ive - Win32 Release"
# Name "osgPlugin ive - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AnimationPath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AnimationPathCallback.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Billboard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\BlendFunc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\CullFace.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DataInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DataOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Drawable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawArrayLengths.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawArrays.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawElementsUShort.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Exception.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Geode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Geometry.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Group.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Image.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Light.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LightSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LOD.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Material.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\MatrixTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Node.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Object.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PositionAttitudeTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PrimitiveSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Sequence.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\StateSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexEnv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexEnvCombine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexGen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture2D.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TextureCubeMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Transform.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AnimationPath
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AnimationPathCallback
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Billboard
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\BlendFunc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\CullFace
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DataInputStream
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DataOutputStream
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DataTypeSize
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Drawable
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawArrayLengths
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawArrays
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawElementsUShort
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Exception
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Export
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Geode
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Geometry
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Group
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Image
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Light
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LightSource
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LOD
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Material
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\MatrixTransform
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Node
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Object
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PositionAttitudeTransform
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PrimitiveSet
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ReadWrite
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Sequence
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\StateSet
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexEnv
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexEnvCombine
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexGen
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture2D
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TextureCubeMap
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Transform
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
