# Microsoft Developer Studio Project File - Name="wxsgv" - Package Owner=<4>

# Microsoft Developer Studio Generated Build File, Format Version 6.00

# ** DO NOT EDIT **



# TARGTYPE "Win32 (x86) Application" 0x0101



CFG=wxsgv - Win32 Debug

!MESSAGE This is not a valid makefile. To build this project using NMAKE,

!MESSAGE use the Export Makefile command and run

!MESSAGE 

!MESSAGE NMAKE /f "wxsgv.mak".

!MESSAGE 

!MESSAGE You can specify a configuration when running NMAKE

!MESSAGE by defining the macro CFG on the command line. For example:

!MESSAGE 

!MESSAGE NMAKE /f "wxsgv.mak" CFG="wxsgv - Win32 Debug"

!MESSAGE 

!MESSAGE Possible choices for configuration are:

!MESSAGE 

!MESSAGE "wxsgv - Win32 Release" (based on "Win32 (x86) Application")

!MESSAGE "wxsgv - Win32 Debug" (based on "Win32 (x86) Application")

!MESSAGE 



# Begin Project

# PROP AllowPerConfigDependencies 0

# PROP Scc_ProjName ""

# PROP Scc_LocalPath ""

CPP=cl.exe

MTL=midl.exe

RSC=rc.exe



!IF  "$(CFG)" == "wxsgv - Win32 Release"



# PROP BASE Use_MFC 0

# PROP BASE Use_Debug_Libraries 0

# PROP BASE Output_Dir "Release"

# PROP BASE Intermediate_Dir "Release"

# PROP BASE Target_Dir ""

# PROP Use_MFC 0

# PROP Use_Debug_Libraries 0

# PROP Output_Dir "Release"

# PROP Intermediate_Dir "Release"

# PROP Ignore_Export_Lib 0

# PROP Target_Dir ""

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c

# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "\APIs\wx2\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__WINDOWS__" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /YX"wx/wxprec.h" /FD /c

# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32

# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32

# ADD BASE RSC /l 0x409 /d "NDEBUG"

# ADD RSC /l 0x409 /d "NDEBUG"

BSC32=bscmake.exe

# ADD BASE BSC32 /nologo

# ADD BSC32 /nologo

LINK32=link.exe

# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

# ADD LINK32 wx22_7.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib netcdf.lib libpng.lib zlib.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /libpath:"\APIs\libpng-1.0.8" /libpath:"\APIs\netcdf-3.5.0.win32bin\lib"



!ELSEIF  "$(CFG)" == "wxsgv - Win32 Debug"



# PROP BASE Use_MFC 0

# PROP BASE Use_Debug_Libraries 1

# PROP BASE Output_Dir "Debug"

# PROP BASE Intermediate_Dir "Debug"

# PROP BASE Target_Dir ""

# PROP Use_MFC 0

# PROP Use_Debug_Libraries 1

# PROP Output_Dir "Debug"

# PROP Intermediate_Dir "Debug"

# PROP Ignore_Export_Lib 0

# PROP Target_Dir ""

# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c

# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "\APIs\wx2\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__WINDOWS__" /D "__WXMSW__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /D WXUSINGDLL=1 /FR /YX"wx/wxprec.h" /FD /GZ /c

# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32

# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32

# ADD BASE RSC /l 0x409 /d "_DEBUG"

# ADD RSC /l 0x409 /d "_DEBUG"

BSC32=bscmake.exe

# ADD BASE BSC32 /nologo

# ADD BSC32 /nologo

LINK32=link.exe

# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

# ADD LINK32 wx22_7d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib netcdf.lib libpng.lib zlib.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /out:"../../../bin/wxsgv.exe" /pdbtype:sept /libpath:"\APIs\libpng-1.0.8" /libpath:"\APIs\netcdf-3.5.0.win32bin\lib"



!ENDIF 



# Begin Target



# Name "wxsgv - Win32 Release"

# Name "wxsgv - Win32 Debug"

# Begin Group "Source Files"



# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"

# Begin Source File



SOURCE=..\..\..\src\Demos\wxsgv\app.cpp

# End Source File

# Begin Source File



SOURCE=..\..\..\src\Demos\wxsgv\canvas.cpp

# End Source File

# Begin Source File



SOURCE=..\..\..\src\Demos\wxsgv\frame.cpp

# End Source File

# Begin Source File



SOURCE=..\..\..\src\Demos\wxsgv\SceneGraphDlg.cpp

# End Source File

# Begin Source File



SOURCE=..\..\..\src\Demos\wxsgv\wxsgv_wdr.cpp

# End Source File

# End Group

# Begin Group "Header Files"



# PROP Default_Filter "h;hpp;hxx;hm;inl"

# Begin Source File



SOURCE=..\..\..\src\Demos\wxsgv\app.h

# End Source File

# Begin Source File



SOURCE=..\..\..\src\Demos\wxsgv\canvas.h

# End Source File

# Begin Source File



SOURCE=..\..\..\src\Demos\wxsgv\frame.h

# End Source File

# Begin Source File



SOURCE=..\..\..\src\Demos\wxsgv\SceneGraphDlg.h

# End Source File

# Begin Source File



SOURCE=..\..\..\src\Demos\wxsgv\wxsgv_wdr.h

# End Source File

# End Group

# Begin Group "Resource Files"



# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"

# Begin Source File



SOURCE=..\..\..\src\Demos\wxsgv\wxsgv.rc

# End Source File

# End Group

# End Target

# End Project

