# Microsoft Developer Studio Project File - Name="Core osgIntrospection" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Core osgIntrospection - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "osgIntrospection.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "osgIntrospection.mak" CFG="Core osgIntrospection - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Core osgIntrospection - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Core osgIntrospection - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Core osgIntrospection - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../include" /I "../../../OpenThreads/include" /I "../../../Producer/include" /I "../../../3rdParty/include" /D "NDEBUG" /D "_MBCS" /D "_USRDLL" /D "OSGINTROSPECTION_LIBRARY" /D "WIN32" /D "_WINDOWS" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32.lib opengl32.lib glu32.lib /nologo /dll /debug /machine:I386 /opt:ref /opt:icf /out:"$(OutDir)/osgIntrospection.dll" /implib:"../../lib/$(PlatformName)/osgIntrospection.lib" /libpath:"../../lib/$(PlatformName)" /libpath:"../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../Producer/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib"

!ELSEIF  "$(CFG)" == "Core osgIntrospection - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /vmg /GR /GX /Zi /Od /I "../../include" /I "../../../OpenThreads/include" /I "../../../Producer/include" /I "../../../3rdParty/include" /D "OSGINTROSPECTION_LIBRARY" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_CRT_SECURE_NO_DEPRECATE" /YX /FD /GZ /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 OpenThreadsWin32d.lib opengl32.lib glu32.lib /nologo /dll /debug /machine:I386 /out:"$(OutDir)/osgIntrospectiond.dll" /pdbtype:sept /implib:"../../lib/$(PlatformName)/osgIntrospectiond.lib" /libpath:"../../lib/$(PlatformName)" /libpath:"../../../OpenThreads/lib/$(PlatformName)" /libpath:"../../../Producer/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib/$(PlatformName)" /libpath:"../../../3rdParty/lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Core osgIntrospection - Win32 Release"
# Name "Core osgIntrospection - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\osgIntrospection\ConstructorInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgIntrospection\CustomAttributeProvider.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgIntrospection\DefaultReflectors.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgIntrospection\MethodInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgIntrospection\PropertyInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgIntrospection\Reflection.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgIntrospection\Type.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgIntrospection\Utility.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\osgIntrospection\Value.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ";h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\osgIntrospection\Attributes
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\Comparator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\ConstructorInfo
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\Converter
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\ConverterProxy
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\CustomAttribute
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\CustomAttributeProvider
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\Exceptions
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\Export
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\InstanceCreator
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\MethodInfo
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\ParameterInfo
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\PropertyInfo
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\PublicMemberAccessor
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\ReaderWriter
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\Reflection
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\ReflectionMacros
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\Reflector
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\StaticMethodInfo
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\Type
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\TypedConstructorInfo
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\TypedMethodInfo
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\TypeNameAliasProxy
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\Utility
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\Value
# End Source File
# Begin Source File

SOURCE=..\..\include\osgIntrospection\variant_cast
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project


