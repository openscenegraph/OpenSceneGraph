# Microsoft Developer Studio Project File - Name="Example osgfxbrowser" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Example osgfxbrowser - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "osgfxbrowser.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "osgfxbrowser.mak" CFG="Example osgfxbrowser - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Example osgfxbrowser - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "Example osgfxbrowser - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Example osgfxbrowser - Win32 Debug"

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
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /GZ /Zm200 /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib osgd.lib Producer.lib osgProducerd.lib osgUtild.lib osgDBd.lib osgUtild.lib osgGAd.lib osgTextd.lib /nologo /subsystem:console /pdb:"..\..\..\bin\osgfxbrowser.pdb" /debug /machine:IX86 /out:"..\..\..\bin\osgfxbrowserd.exe" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib /nologo /subsystem:console /pdb:"..\..\..\bin\osgfxbrowser.pdb" /debug /machine:IX86 /out:"..\..\..\bin\osgfxbrowserd.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Example osgfxbrowser - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "../../../bin"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../bin"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /GF /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "../../../include" /I "../../../../OpenThreads/include" /I "../../../../Producer/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /GF /Zm200 /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib osg.lib Producer.lib osgProducer.lib osgUtil.lib osgDB.lib osgUtil.lib osgGA.lib osgText.lib /nologo /subsystem:console /debug /machine:IX86 /pdbtype:sept /opt:ref /opt:icf
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib /nologo /subsystem:console /debug /machine:IX86 /pdbtype:sept /opt:ref /opt:icf

!ENDIF 

# Begin Target

# Name "Example osgfxbrowser - Win32 Debug"
# Name "Example osgfxbrowser - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm"
# Begin Source File

SOURCE=..\..\..\examples\osgfxbrowser\Frame.cpp
DEP_CPP_FRAME=\
	"..\..\..\Include\Osg\Array"\
	"..\..\..\Include\Osg\BoundingBox"\
	"..\..\..\Include\Osg\BoundingSphere"\
	"..\..\..\include\osg\buffered_value"\
	"..\..\..\Include\Osg\CopyOp"\
	"..\..\..\include\osg\DisplaySettings"\
	"..\..\..\Include\Osg\Drawable"\
	"..\..\..\Include\Osg\Export"\
	"..\..\..\include\osg\fast_back_stack"\
	"..\..\..\include\osg\FrameStamp"\
	"..\..\..\Include\Osg\Gl"\
	"..\..\..\Include\Osg\Image"\
	"..\..\..\include\osg\Math"\
	"..\..\..\Include\Osg\Matrix"\
	"..\..\..\Include\Osg\Node"\
	"..\..\..\include\osg\NodeCallback"\
	"..\..\..\Include\Osg\NodeVisitor"\
	"..\..\..\Include\Osg\Object"\
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
	"..\..\..\include\osgText\Export"\
	"..\..\..\include\osgText\Font"\
	"..\..\..\include\osgText\String"\
	"..\..\..\include\osgText\Text"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\examples\osgfxbrowser\Frame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\examples\osgfxbrowser\osgfxbrowser.cpp
DEP_CPP_OSGFX=\
	"..\..\..\Include\Osg\AlphaFunc"\
	"..\..\..\Include\Osg\AnimationPath"\
	"..\..\..\Include\osg\ApplicationUsage"\
	"..\..\..\Include\osg\ArgumentParser"\
	"..\..\..\Include\Osg\Array"\
	"..\..\..\Include\Osg\BlendFunc"\
	"..\..\..\Include\Osg\BoundingBox"\
	"..\..\..\Include\Osg\BoundingSphere"\
	"..\..\..\include\osg\buffered_value"\
	"..\..\..\include\osg\ClearNode"\
	"..\..\..\Include\Osg\ColorMask"\
	"..\..\..\Include\Osg\ConvexPlanarOccluder"\
	"..\..\..\Include\Osg\ConvexPlanarPolygon"\
	"..\..\..\Include\Osg\CopyOp"\
	"..\..\..\Include\Osg\CullingSet"\
	"..\..\..\Include\Osg\CullStack"\
	"..\..\..\include\osg\DisplaySettings"\
	"..\..\..\Include\Osg\Drawable"\
	"..\..\..\Include\Osg\Export"\
	"..\..\..\include\osg\fast_back_stack"\
	"..\..\..\include\osg\FrameStamp"\
	"..\..\..\Include\Osg\Geode"\
	"..\..\..\Include\Osg\Geometry"\
	"..\..\..\Include\Osg\Gl"\
	"..\..\..\Include\Osg\Group"\
	"..\..\..\Include\Osg\Image"\
	"..\..\..\Include\Osg\Impostor"\
	"..\..\..\Include\Osg\ImpostorSprite"\
	"..\..\..\Include\Osg\Light"\
	"..\..\..\Include\Osg\LightSource"\
	"..\..\..\Include\Osg\LineSegment"\
	"..\..\..\Include\Osg\Lod"\
	"..\..\..\include\osg\Math"\
	"..\..\..\Include\Osg\Matrix"\
	"..\..\..\Include\Osg\MatrixTransform"\
	"..\..\..\Include\Osg\Node"\
	"..\..\..\include\osg\NodeCallback"\
	"..\..\..\Include\Osg\NodeVisitor"\
	"..\..\..\Include\Osg\Notify"\
	"..\..\..\Include\Osg\Object"\
	"..\..\..\Include\Osg\OccluderNode"\
	"..\..\..\Include\Osg\PagedLOD"\
	"..\..\..\Include\Osg\Plane"\
	"..\..\..\Include\Osg\Polytope"\
	"..\..\..\Include\Osg\PrimitiveSet"\
	"..\..\..\Include\Osg\Projection"\
	"..\..\..\Include\Osg\Quat"\
	"..\..\..\include\osg\ref_ptr"\
	"..\..\..\Include\Osg\Referenced"\
	"..\..\..\Include\Osg\ShadowVolumeOccluder"\
	"..\..\..\Include\Osg\Shape"\
	"..\..\..\Include\Osg\State"\
	"..\..\..\Include\Osg\StateAttribute"\
	"..\..\..\Include\Osg\StateSet"\
	"..\..\..\Include\Osg\TexEnv"\
	"..\..\..\Include\Osg\Texture"\
	"..\..\..\Include\Osg\Texture2D"\
	"..\..\..\Include\Osg\Timer"\
	"..\..\..\Include\Osg\Transform"\
	"..\..\..\include\osg\UByte4"\
	"..\..\..\Include\Osg\Vec2"\
	"..\..\..\Include\Osg\Vec3"\
	"..\..\..\Include\Osg\Vec4"\
	"..\..\..\include\osg\Viewport"\
	"..\..\..\Include\osgDB\DatabasePager"\
	"..\..\..\Include\osgDB\Export"\
	"..\..\..\Include\osgDB\ReadFile"\
	"..\..\..\Include\osgDB\WriteFile"\
	"..\..\..\include\osgFX\Effect"\
	"..\..\..\include\osgFX\Export"\
	"..\..\..\include\osgFX\Registry"\
	"..\..\..\include\osgFX\Technique"\
	"..\..\..\Include\osgGA\Export"\
	"..\..\..\Include\osgGA\GUIActionAdapter"\
	"..\..\..\Include\osgGA\GUIEventAdapter"\
	"..\..\..\Include\osgGA\GUIEventHandler"\
	"..\..\..\Include\osgGA\GUIEventHandlerVisitor"\
	"..\..\..\Include\osgGA\KeySwitchMatrixManipulator"\
	"..\..\..\Include\osgGA\MatrixManipulator"\
	"..\..\..\Include\osgProducer\EventAdapter"\
	"..\..\..\Include\osgProducer\Export"\
	"..\..\..\Include\osgProducer\KeyboardMouseCallback"\
	"..\..\..\include\osgProducer\OsgCameraGroup"\
	"..\..\..\Include\osgProducer\OsgSceneHandler"\
	"..\..\..\Include\osgProducer\Viewer"\
	"..\..\..\include\osgText\Export"\
	"..\..\..\include\osgText\Font"\
	"..\..\..\include\osgText\String"\
	"..\..\..\include\osgText\Text"\
	"..\..\..\Include\osgUtil\CullVisitor"\
	"..\..\..\Include\osgUtil\Export"\
	"..\..\..\Include\osgUtil\IntersectVisitor"\
	"..\..\..\include\osgUtil\Optimizer"\
	"..\..\..\include\osgUtil\RenderBin"\
	"..\..\..\include\osgUtil\RenderGraph"\
	"..\..\..\include\osgUtil\RenderLeaf"\
	"..\..\..\include\osgUtil\RenderStage"\
	"..\..\..\include\osgUtil\RenderStageLighting"\
	"..\..\..\Include\osgUtil\SceneView"\
	{$(INCLUDE)}"openthreads\barrier"\
	{$(INCLUDE)}"openthreads\condition"\
	{$(INCLUDE)}"openthreads\exports"\
	{$(INCLUDE)}"openthreads\mutex"\
	{$(INCLUDE)}"openthreads\thread"\
	{$(INCLUDE)}"producer\block"\
	{$(INCLUDE)}"producer\blockingqueue"\
	{$(INCLUDE)}"producer\camera"\
	{$(INCLUDE)}"producer\cameraconfig"\
	{$(INCLUDE)}"producer\cameragroup"\
	{$(INCLUDE)}"producer\export"\
	{$(INCLUDE)}"producer\inputarea"\
	{$(INCLUDE)}"producer\keyboard"\
	{$(INCLUDE)}"producer\keyboardmouse"\
	{$(INCLUDE)}"producer\math"\
	{$(INCLUDE)}"producer\referenced"\
	{$(INCLUDE)}"producer\refopenthread"\
	{$(INCLUDE)}"producer\rendersurface"\
	{$(INCLUDE)}"producer\timer"\
	{$(INCLUDE)}"producer\types"\
	{$(INCLUDE)}"producer\visualchooser"\
	
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
