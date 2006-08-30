# Microsoft Developer Studio Project File - Name="osgWrapper osg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=osgWrapper osg - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wrapper_osg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wrapper_osg.mak" CFG="osgWrapper osg - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "osgWrapper osg - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "osgWrapper osg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "osgWrapper osg - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /I "../../../../3rdParty/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib OpenThreadsWin32.lib  /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBC" /opt:ref /opt:icf /out:"$(OutDir)/osgwrapper_osg.dll" /implib:"../../../lib/$(PlatformName)/osgwrapper_osg.lib" /libpath:"../../../lib/$(PlatformName)" /libpath:"../../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../../Producer/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "osgWrapper osg - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /vmg /GR /GX /Zi /Od /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /I "../../../../3rdParty/include" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WIN32" /D "_DEBUG" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib OpenThreadsWin32d.lib  /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBC" /out:"$(OutDir)/osgwrapper_osgd.dll" /pdbtype:sept /implib:"../../../lib/$(PlatformName)/osgwrapper_osgd.lib" /libpath:"../../../lib/$(PlatformName)" /libpath:"../../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../../Producer/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "osgWrapper osg - Win32 Release"
# Name "osgWrapper osg - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\AlphaFunc.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\AnimationPath.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ApplicationUsage.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ArgumentParser.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Array.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\AutoTransform.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Billboard.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\BlendColor.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\BlendEquation.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\BlendFunc.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\BoundingBox.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\BoundingSphere.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\BufferObject.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\CameraNode.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\CameraView.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ClampColor.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ClearNode.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ClipNode.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ClipPlane.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ClusterCullingCallback.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\CollectOccludersVisitor.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ColorMask.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ColorMatrix.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ConvexPlanarOccluder.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ConvexPlanarPolygon.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\CoordinateSystemNode.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\CopyOp.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\CullFace.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\CullSettings.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\CullStack.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\CullingSet.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Depth.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\DisplaySettings.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\DrawPixels.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Drawable.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Endian.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Fog.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\FragmentProgram.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\FrameBufferObject.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\FrameStamp.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\FrontFace.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\GL2Extensions.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Geode.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Geometry.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\GraphicsContext.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\GraphicsThread.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Group.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Image.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ImageStream.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\LOD.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Light.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\LightModel.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\LightSource.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\LineSegment.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\LineStipple.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\LineWidth.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\LogicOp.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Material.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Matrix.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\MatrixTransform.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Matrixd.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Matrixf.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Multisample.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Node.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\NodeCallback.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\NodeTrackerCallback.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\NodeVisitor.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Notify.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Object.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\OccluderNode.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\PagedLOD.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Plane.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Point.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\PointSprite.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\PolygonMode.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\PolygonOffset.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\PolygonStipple.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Polytope.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\PositionAttitudeTransform.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\PrimitiveSet.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Program.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Projection.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ProxyNode.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Quat.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Referenced.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Scissor.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Sequence.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ShadeModel.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Shader.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ShadowVolumeOccluder.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Shape.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\ShapeDrawable.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\State.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\StateAttribute.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\StateSet.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Stencil.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Switch.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\TexEnv.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\TexEnvCombine.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\TexEnvFilter.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\TexGen.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\TexGenNode.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\TexMat.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Texture.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Texture1D.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Texture2D.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Texture3D.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\TextureCubeMap.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\TextureRectangle.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Timer.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Transform.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Uniform.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec2.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec2b.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec2d.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec2f.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec2s.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec3.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec3b.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec3d.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec3f.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec3s.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec4.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec4b.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec4d.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec4f.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec4s.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Vec4ub.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\VertexProgram.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\Viewport.cpp
# End Source File

# Begin Source File
SOURCE=..\..\..\src\osgWrappers\osg\observer_ptr.cpp
# End Source File

# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

