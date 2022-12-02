
# Be sure to modify the definitions in this file to agree with your
# systems installation.
#  NOTE: be sure that the install directories use '\' not '/' for paths.


# MSVC install directories
LIBINSTALL     = "F:\\workspace\\osg-3rdParty-install\\glut\\lib"
INCLUDEINSTALL = "F:\\workspace\\osg-3rdParty-install\\glut\\include\\GL"

# Win95 dll directory
#DLLINSTALL     = "F:\\workspace\\osg-3rdParty-install\\glut\\bin"

# WinNT dll directory
DLLINSTALL     = "F:\\workspace\\osg-3rdParty-install\\glut\\bin"

# Microsoft OpenGL libraries
#
GLU    = glu32.lib
OPENGL = opengl32.lib
GLUT   = $(TOP)/lib/glut/glut.lib
GLUTLIB = glut.lib
GLUTDLL = glut.dll

# SGI OpenGL for Windows libraries (formerly Cosmo OpenGL)
# >> To use, uncomment lines below and comment out the similiar
# >> lines above.  You can download SGI OpenGL for Windows for
# >> free from http://www.meer.net/~gold/OpenGL/opengl2.exe
#
#GLU     = \oglsdk\lib\glu.lib
#OPENGL  = \oglsdk\lib\opengl.lib
#GLUT    = $(TOP)/lib/glut/glut.lib
#GLUTLIB = glut.lib
#GLUTDLL = glut.dll

# The Micro UI lib
MUI     = $(TOP)/lib/mui/mui.lib

# The OpenGL Extrusion and Tubing lib
GLE     = $(TOP)/lib/gle/gle.lib

# The OpenGL Sphere Mapping lib
GLSMAP  = $(TOP)/lib/glsmap/glsmap.lib

# common definitions used by all makefiles
CFLAGS	= $(cflags) $(cdebug) $(EXTRACFLAGS) -DWIN32 -I$(TOP)/include
LIBS	= $(lflags) $(ldebug) $(EXTRALIBS) $(GLUT) $(GLU) $(OPENGL) $(guilibs)
EXES	= $(SRCS:.c=.exe) $(CPPSRCS:.cpp=.exe)

!IFNDEF NODEBUG
lcommon = /NODEFAULTLIB /INCREMENTAL:NO /DEBUG /NOLOGO
!ENDIF

# default rule
default	: $(EXES)

# cleanup rules
clean	::
	@del /f *.obj
	@del /f *.pdb
	@del /f *.ilk
	@del /f *.ncb
	@del /f *~
	@del /f *.exp

clobber	:: clean
	@del /f *.exe
	@del /f *.dll
	@del /f *.lib
	-@del /f $(LDIRT)

# inference rules
$(EXES)	: $*.obj $(DEPLIBS)
	echo $@
        $(link) -out:$@ $** $(LIBS)
.c.obj	: 
	$(CC) $(CFLAGS) $<
.cpp.obj : 
	$(CC) $(CFLAGS) $<
