# For visual studio express 2005 and windows plateforme sdk 2003
# i have some problem with the windows sdk to build the BaseClasses, so
# here some point that will help to fix this.
# https://blogs.msdn.com/mikewasson/archive/2005/05/23/421116.aspx

# use this link to downloads vc project to build strmbase library
# http://www.microsoft.com/downloads/thankyou.aspx?familyId=8af0afa9-1383-44b4-bc8b-7d6315212323&displayLang=en

# to build strmbase.lib
# modify clutil.h line 278
#    operator=(LONG); 
#    to
#    (LONG) operator=(LONG);
    
# add in includes directory
# C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Include\atl    

# modify winutil.cpp line 2104
#    for (Count = 0;Count < Result;Count++) {
#    to
#    for (UINT Count = 0;Count < Result;Count++) {

# modify winutil.cpp line 2123
#    for (Count = PalLoCount;INT(Count) < min(PalHiStart,iColours);Count++) {
#    to
#    for (UINT Count = PalLoCount;INT(Count) < min(PalHiStart,iColours);Count++) {

# modify wxdebug.cpp line 564
#    static g_dwLastRefresh = 0;
#    to
#    static DWORD g_dwLastRefresh = 0;

# modify outputq.cpp line 6134
#    LONG iLost = 0;
#    for (long iDone = 0;
#    to
#    LONG iLost = 0;
#    long iDone = 0;
#    for (iDone = 0;

# In the properties of the project there is a fixed path to link with strmiids.lib. Replace the ../../../.. by the name of the library
# only, then the linker will find the strmiids.lib

# After those step it should compile, becareful using unicode version will result undefined symbol ... so use multibyte version instead
#
# End For visual studio express 2005 and windows plateforme sdk 2003


INCLUDE_DIRECTORIES( ${DIRECTSHOW_INLUDE_DIRS} )
SET (TARGET_EXTERNAL_LIBRARIES
    quartz comsuppw winmm d3d9 kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32
    )
    
SET(TARGET_SRC 
  ReaderWriterDirectShow.cpp
  DirectShowTexture.cpp
)

SET(TARGET_H
  DirectShowTexture
)
SET(TARGET_LIBRARIES_VARS DIRECTSHOW_LIBRARY )

#### end var setup  ###
SETUP_PLUGIN(directshow directshow)
SET_TARGET_PROPERTIES("${TARGET_DEFAULT_PREFIX}${TARGET_NAME}" PROPERTIES LINK_FLAGS "/NODEFAULTLIB:atlthunk")


