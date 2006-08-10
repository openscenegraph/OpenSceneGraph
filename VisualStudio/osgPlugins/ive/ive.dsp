# Microsoft Developer Studio Project File - Name="osgPlugin ive" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

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
!MESSAGE "osgPlugin ive - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE "osgPlugin ive - Win32 Debug Static" (based on "Win32 (x86) Static Library")
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
# PROP Output_Dir "../../../bin/$(PlatformName)"
# PROP Intermediate_Dir "$(PlatformName)/$(ConfigurationName)"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /I "../../../../3rdParty/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBC" /out:"$(OutDir)/osgdb_ive.dll" /implib:"../../../lib/$(PlatformName)/osgdb_ive.lib" /libpath:"../../../lib/$(PlatformName)" /libpath:"../../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../../Producer/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "osgPlugin ive - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /vmg /GR /GX /Zi /Od /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /I "../../../../3rdParty/include" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WIN32" /D "_DEBUG" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /GZ /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32d.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBC" /out:"$(OutDir)/osgdb_ived.dll" /pdbtype:sept /implib:"../../../lib/$(PlatformName)/osgdb_ived.lib" /libpath:"../../../lib/$(PlatformName)" /libpath:"../../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../../Producer/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "osgPlugin ive - Win32 Release Static"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /I "../../../../3rdParty/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nodefaultlib /nologo /out:"$(OutDir)/osgdb_ive_s.lib"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "osgPlugin ive - Win32 Debug Static"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /vmg /GR /GX /Z7 /Od /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /I "../../../../3rdParty/include" /D "_WINDOWS" /D "_MBCS" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "WIN32" /D "_DEBUG" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nodefaultlib /nologo /out:"$(OutDir)/osgdb_ived_s.lib"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "osgPlugin ive - Win32 Release"
# Name "osgPlugin ive - Win32 Debug"
# Name "osgPlugin ive - Win32 Release Static"
# Name "osgPlugin ive - Win32 Debug Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AlphaFunc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AnimationPath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AnimationPathCallback.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AutoTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AzimElevationSector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AzimSector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Billboard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\BlendFunc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\BlinkSequence.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\CameraNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\CameraView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ClipPlane.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ClipNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ClusterCullingCallback.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ConeSector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ConvexPlanarOccluder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ConvexPlanarPolygon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\CoordinateSystemNode.cpp
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

SOURCE=..\..\..\src\osgPlugins\ive\Depth.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DirectionalSector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DOFTransform.cpp
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

SOURCE=..\..\..\src\osgPlugins\ive\DrawElementsUByte.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawElementsUInt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawElementsUShort.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ElevationSector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\EllipsoidModel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Exception.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\FragmentProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\FrontFace.cpp
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

SOURCE=..\..\..\src\osgPlugins\ive\Impostor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Light.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LightModel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LightPoint.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LightPointNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LightSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LineWidth.cpp
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

SOURCE=..\..\..\src\osgPlugins\ive\MultiSwitch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\MultiTextureControl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Node.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Object.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\OccluderNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PagedLOD.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Point.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PolygonMode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PolygonOffset.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PositionAttitudeTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PrimitiveSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ProxyNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ReaderWriterIVE.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Scissor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Sequence.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ShadeModel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Shape.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ShapeDrawable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\StateSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Switch.cpp
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

SOURCE=..\..\..\src\osgPlugins\ive\TexGenNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexMat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Text.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Program.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Shader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Uniform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture1D.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture2D.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture3D.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TextureCubeMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TextureRectangle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Transform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\VertexProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Viewport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\VisibilityGroup.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AlphaFunc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AnimationPath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AnimationPathCallback.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AutoTransform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AzimElevationSector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\AzimSector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Billboard.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\BlendFunc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\BlinkSequence.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\CameraNode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\CameraView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ClipPlane.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ClipNode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ClusterCullingCallback.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ConeSector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ConvexPlanarOccluder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ConvexPlanarPolygon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\CoordinateSystemNode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\CullFace.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DataInputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DataOutputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DataTypeSize.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Depth.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DirectionalSector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DOFTransform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Drawable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawArrayLengths.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawArrays.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawElementsUByte.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawElementsUInt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\DrawElementsUShort.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ElevationSector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\EllipsoidModel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\FragmentProgram.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\FrontFace.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Geode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Geometry.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Group.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Image.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Impostor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Light.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LightModel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LightPoint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LightPointNode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LightSource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LineWidth.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\LOD.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Material.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\MatrixTransform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\MultiSwitch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\MultiTextureControl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Node.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Object.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\OccluderNode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PagedLOD.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Point.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PolygonMode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PolygonOffset.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PositionAttitudeTransform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\PrimitiveSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ProxyNode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ReadWrite.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Sequence.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ShadeModel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Shape.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\ShapeDrawable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\StateSet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Switch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexEnv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexEnvCombine.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexGen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexGenNode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TexMat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Text.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Program.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Scissor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Shader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Uniform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture1D.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture2D.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Texture3D.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TextureCubeMap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\TextureRectangle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Transform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\VertexProgram.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\Viewport.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\ive\VisibilityGroup.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

