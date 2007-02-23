# Microsoft Developer Studio Project File - Name="osgPlugin osgParticle" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=osgPlugin osgParticle - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dot_osgParticle.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dot_osgParticle.mak" CFG="osgPlugin osgParticle - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "osgPlugin osgParticle - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "osgPlugin osgParticle - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "osgPlugin osgParticle - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE "osgPlugin osgParticle - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "osgPlugin osgParticle - Win32 Release"

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
# ADD LINK32 OpenThreadsWin32.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBC" /opt:ref /opt:icf /out:"$(OutDir)/osgdb_osgParticle.dll" /implib:"../../../lib/$(PlatformName)/osgdb_osgParticle.lib" /libpath:"../../../lib/$(PlatformName)" /libpath:"../../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../../Producer/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "osgPlugin osgParticle - Win32 Debug"

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
# ADD LINK32 OpenThreadsWin32d.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBC" /out:"$(OutDir)/osgdb_osgParticled.dll" /pdbtype:sept /implib:"../../../lib/$(PlatformName)/osgdb_osgParticled.lib" /libpath:"../../../lib/$(PlatformName)" /libpath:"../../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../../Producer/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../../3rdParty/lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "osgPlugin osgParticle - Win32 Release Static"

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
# ADD LIB32 /nodefaultlib /nologo /out:"$(OutDir)/osgdb_osgParticle_s.lib"
# SUBTRACT LIB32 /nodefaultlib

!ELSEIF  "$(CFG)" == "osgPlugin osgParticle - Win32 Debug Static"

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
# ADD LIB32 /nodefaultlib /nologo /out:"$(OutDir)/osgdb_osgParticled_s.lib"
# SUBTRACT LIB32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "osgPlugin osgParticle - Win32 Release"
# Name "osgPlugin osgParticle - Win32 Debug"
# Name "osgPlugin osgParticle - Win32 Release Static"
# Name "osgPlugin osgParticle - Win32 Debug Static"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_AccelOperator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_AngularAccelOperator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_BoxPlacer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_CenteredPlacer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_ConnectedParticleSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_ConstantRateCounter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_Emitter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_FluidFrictionOperator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_ForceOperator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_LinearInterpolator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_ModularEmitter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_ModularProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_MultiSegmentPlacer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_Particle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_ParticleEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_ExplosionEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_ExplosionDebrisEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_SmokeEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_SmokeTrailEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_FireEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_ParticleProcessor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_ParticleSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_ParticleSystemUpdater.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_PointPlacer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_Program.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_RadialShooter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_RandomRateCounter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_SectorPlacer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_SegmentPlacer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\osgPlugins\osgParticle\IO_VariableRateCounter.cpp
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

