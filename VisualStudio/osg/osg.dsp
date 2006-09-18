# Microsoft Developer Studio Project File - Name="Core osg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Core osg - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "osg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "osg.mak" CFG="Core osg - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Core osg - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Core osg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Core osg - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE "Core osg - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Core osg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../bin/$(PlatformName)"
# PROP Intermediate_Dir "$(PlatformName)/$(ConfigurationName)"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../include" /I "../../../OpenThreads/include" /I "../../../Producer/include" /I "../../../3rdParty/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OSG_LIBRARY" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32.lib opengl32.lib glu32.lib /nologo /dll /debug /machine:I386 /opt:ref /opt:icf /out:"$(OutDir)/osg.dll" /implib:"../../lib/$(PlatformName)/osg.lib" /libpath:"../../lib/$(PlatformName)" /libpath:"../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../Producer/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib"

!ELSEIF  "$(CFG)" == "Core osg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../bin/$(PlatformName)"
# PROP Intermediate_Dir "$(PlatformName)/$(ConfigurationName)"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /vmg /GR /GX /Zi /Od /I "../../include" /I "../../../OpenThreads/include" /I "../../../Producer/include" /I "../../../3rdParty/include" /D "_WINDOWS" /D "OSG_LIBRARY" /D "WIN32" /D "_DEBUG" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /GZ /Zm200 /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32d.lib opengl32.lib glu32.lib /nologo /dll /debug /machine:I386 /out:"$(OutDir)/osgd.dll" /pdbtype:sept /implib:"../../lib/$(PlatformName)/osgd.lib" /libpath:"../../lib/$(PlatformName)" /libpath:"../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../Producer/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "Core osg - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "../../lib"
# PROP BASE Intermediate_Dir "Release_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../lib/$(PlatformName)"
# PROP Intermediate_Dir "$(PlatformName)/$(ConfigurationName)_Static"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "../../include" /I "../../../OpenThreads/include" /I "../../../Producer/include" /I "../../../3rdParty/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nodefaultlib /nologo /out:"$(OutDir)/osg_s.lib"

!ELSEIF  "$(CFG)" == "Core osg - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "../../lib"
# PROP BASE Intermediate_Dir "Debug_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../lib/$(PlatformName)"
# PROP Intermediate_Dir "$(PlatformName)/$(ConfigurationName)_Static"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /vmg /GR /GX /Z7 /Od /I "../../include" /I "../../../OpenThreads/include" /I "../../../Producer/include" /I "../../../3rdParty/include" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /GZ /Zm200 /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nodefaultlib /nologo /out:"$(OutDir)/osgd_s.lib"

!ENDIF 

# Begin Target

# Name "Core osg - Win32 Release"
# Name "Core osg - Win32 Debug"
# Name "Core osg - Win32 Release Static"
# Name "Core osg - Win32 Debug Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\osg\AlphaFunc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\AnimationPath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ApplicationUsage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ArgumentParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Array.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\AutoTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Billboard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\BlendColor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\BlendEquation.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\BlendFunc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\BoundingBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\BoundingSphere.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\BufferObject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\CameraNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\CameraView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ClearNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ClipNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ClipPlane.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ClampColor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ClusterCullingCallback.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\CollectOccludersVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ColorMask.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ColorMatrix.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ConvexPlanarOccluder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ConvexPlanarPolygon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\CoordinateSystemNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\CopyOp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\CullFace.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\CullingSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\CullSettings.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\CullStack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Depth.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\DisplaySettings.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Drawable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\DrawPixels.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Fog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\FragmentProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\FrameBufferObject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\FrameStamp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\FrontFace.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Geode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Geometry.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\GLExtensions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\GraphicsContext.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\GraphicsThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Group.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\dxtctool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Image.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ImageStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Light.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\LightModel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\LightSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\LineSegment.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\LineStipple.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\LineWidth.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\LOD.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\LogicOp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Material.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Matrixd.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Matrixf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\MatrixTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Multisample.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Node.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\NodeCallback.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\NodeTrackerCallback.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\NodeVisitor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Notify.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Object.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\OccluderNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\PagedLOD.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Point.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\PointSprite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\PolygonMode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\PolygonOffset.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\PolygonStipple.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\PositionAttitudeTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\PrimitiveSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Program.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Projection.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ProxyNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Quat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Referenced.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Scissor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Sequence.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ShadeModel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Shader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ShadowVolumeOccluder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Shape.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\ShapeDrawable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\State.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\StateAttribute.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\StateSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Stencil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Switch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\TexEnv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\TexEnvCombine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\TexEnvFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\TexGen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\TexGenNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\TexMat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Texture.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Texture1D.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Texture2D.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Texture3D.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\TextureCubeMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\TextureRectangle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Timer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Transform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Uniform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\UnitTestFramework.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Vec3.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Version.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\VertexProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\View.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\Viewport.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ";h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Include\Osg\AlphaFunc
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\AnimationPath
# End Source File
# Begin Source File

SOURCE=..\..\Include\osg\ApplicationUsage
# End Source File
# Begin Source File

SOURCE=..\..\Include\osg\ArgumentParser
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Array
# End Source File
# Begin Source File

SOURCE=..\..\Include\osg\AutoTransform
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Billboard
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\BlendColor
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\BlendEquation
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\BlendFunc
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\BoundingBox
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\BoundingSphere
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\BoundsChecking
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\BufferObject
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\CameraNode
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\CameraView
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\ClearNode
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ClipNode
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ClipPlane
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ClampColor
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ClusterCullingCallback
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\CollectOccludersVisitor
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ColorMask
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ColorMatrix
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ConvexPlanarOccluder
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ConvexPlanarPolygon
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\CoordinateSystemNode
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\CopyOp
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\CullFace
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\CullingSet
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\CullSettings
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\CullStack
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Depth
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\DisplaySettings
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Drawable
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\DrawPixels
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Export
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\fast_back_stack
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Fog
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\FragmentProgram
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\FrameBufferObject
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\FrameStamp
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\FrontFace
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Geode
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Geometry
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Gl
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\GLExtensions
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\Glu
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Group
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\GraphicsContext
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\GraphicsThread
# End Source File
# Begin Source File

SOURCE=..\..\src\osg\dxtctool.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Image
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ImageStream
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\IndexedGeometry
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Light
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\LightModel
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\LightSource
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\LineSegment
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\LineStipple
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\LineWidth
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\LOD
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\LogicOp
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Material
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\Math
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Matrix
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Matrixd
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Matrixf
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\MatrixTransform
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\Multisample
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Node
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\NodeCallback
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\NodeTrackerCallback
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\NodeVisitor
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Notify
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Object
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\OccluderNode
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\PagedLOD
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Plane
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Point
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\PointSprite
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\PolygonMode
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\PolygonOffset
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\PolygonStipple
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Polytope
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\PositionAttitudeTransform
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\PrimitiveSet
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Projection
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Program
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ProxyNode
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Quat
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\observer_ptr
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\ref_ptr
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\io_utils
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Referenced
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\RenderInfo
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Scissor
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Sequence
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ShadeModel
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Shader
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\ShadowVolumeOccluder
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Shape
# End Source File
# Begin Source File

SOURCE=..\..\Include\osg\ShapeDrawable
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\State
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\StateAttribute
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\StateSet
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Stencil
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Switch
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\TexEnv
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\TexEnvCombine
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\TexEnvFilter
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\TexGen
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\TexGenNode
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\TexMat
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Texture
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Texture1D
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Texture2D
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Texture3D
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\TextureCubeMap
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\TextureRectangle
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Timer
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Transform
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\TriangleFunctor
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Uniform
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\UnitTestFramework
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec2b
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec3b
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec4b
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec2s
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec3s
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec4s
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec4ub
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec2
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec2d
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec2f
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec3
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec3d
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec3f
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec4
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec4d
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Vec4f
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\Version
# End Source File
# Begin Source File

SOURCE=..\..\Include\Osg\VertexProgram
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\View
# End Source File
# Begin Source File

SOURCE=..\..\include\osg\Viewport
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

