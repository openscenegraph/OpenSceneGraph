# Microsoft Developer Studio Project File - Name="Core osgParticle" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Core osgParticle - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "osgParticle.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "osgParticle.mak" CFG="Core osgParticle - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Core osgParticle - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Core osgParticle - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Core osgParticle - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE "Core osgParticle - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Core osgParticle - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "../../lib"
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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../include" /I "../../../OpenThreads/include" /I "../../../Producer/include" /I "../../../3rdParty/include" /D "NDEBUG" /D "_MBCS" /D "_USRDLL" /D "OSGPARTICLE_LIBRARY" /D "WIN32" /D "_WINDOWS" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32.lib opengl32.lib glu32.lib /nologo /dll /debug /machine:I386 /opt:ref /opt:icf /out:"$(OutDir)/osgParticle.dll" /implib:"../../lib/$(PlatformName)/osgParticle.lib" /libpath:"../../lib/$(PlatformName)" /libpath:"../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../Producer/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib"

!ELSEIF  "$(CFG)" == "Core osgParticle - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /vmg /GR /GX /Zi /Od /I "../../include" /I "../../../OpenThreads/include" /I "../../../Producer/include" /I "../../../3rdParty/include" /D "OSGPARTICLE_LIBRARY" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /GZ /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32d.lib opengl32.lib glu32.lib /nologo /dll /debug /machine:I386 /out:"$(OutDir)/osgParticled.dll" /pdbtype:sept /implib:"../../lib/$(PlatformName)/osgParticled.lib" /libpath:"../../lib/$(PlatformName)" /libpath:"../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../Producer/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Core osgParticle - Win32 Release Static"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "../../include" /I "../../../OpenThreads/include" /I "../../../Producer/include" /I "../../../3rdParty/include" /D "NDEBUG" /D "_MBCS" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "WIN32" /D "_WINDOWS" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nodefaultlib /nologo /out:"$(OutDir)/osgParticle_s.lib"

!ELSEIF  "$(CFG)" == "Core osgParticle - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../lib/$(PlatformName)"
# PROP Intermediate_Dir "$(PlatformName)/$(ConfigurationName)_Static"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /vmg /GR /GX /Z7 /Od /I "../../include" /I "../../../OpenThreads/include" /I "../../../Producer/include" /I "../../../3rdParty/include" /D "OSG_LIBRARY_STATIC" /D "OT_LIBRARY_STATIC" /D "PR_LIBRARY_STATIC" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /GZ /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nodefaultlib /nologo /out:"$(OutDir)/osgParticled_s.lib"
# SUBTRACT LIB32 /pdb:none

!ENDIF 

# Begin Target

# Name "Core osgParticle - Win32 Release"
# Name "Core osgParticle - Win32 Debug"
# Name "Core osgParticle - Win32 Release Static"
# Name "Core osgParticle - Win32 Debug Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\osgParticle\Emitter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\ExplosionEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\ExplosionDebrisEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\FireEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\FluidFrictionOperator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\FluidProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\ModularEmitter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\ModularProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\MultiSegmentPlacer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\Particle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\ParticleEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\ParticleProcessor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\ParticleSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\PrecipitationEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\ConnectedParticleSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\ParticleSystemUpdater.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\Program.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\SmokeEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\SmokeTrailEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgParticle\Version.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ";h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\osgParticle\AccelOperator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\AngularAccelOperator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\BoxPlacer
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\CenteredPlacer
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\Counter
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\ConstantRateCounter
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\Emitter
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\ExplosionEffect
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\ExplosionDebrisEffect
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\Export
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\FireEffect
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\FluidFrictionOperator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\FluidProgram
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\ForceOperator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\Interpolator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\LinearInterpolator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\ModularEmitter
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\ModularProgram
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\MultiSegmentPlacer
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\Operator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\Particle
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\ParticleEffect
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\ParticleProcessor
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\ParticleSystem
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\ConnectedParticleSystem
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\ParticleSystemUpdater
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\Placer
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\PointPlacer
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\Program
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\RadialShooter
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\RandomRateCounter
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\range
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\SectorPlacer
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\SegmentPlacer
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\Shooter
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\SmokeEffect
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\SmokeTrailEffect
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\VariableRateCounter
# End Source File
# Begin Source File

SOURCE=..\..\include\osgParticle\Version
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
