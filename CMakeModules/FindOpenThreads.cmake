# OpenThreads is a C++ based threading library. Its largest userbase 
# seems to OpenSceneGraph so you might notice I accept OSGDIR as an
# environment path.
# I consider this part of the Findosg* suite used to find OpenSceneGraph 
# components.
# Each component is separate and you must opt in to each module.
# 
# Locate OpenThreads
# This module defines
# OPENTHREADS_LIBRARY
# OPENTHREADS_FOUND, if false, do not try to link to OpenThreads
# OPENTHREADS_INCLUDE_DIR, where to find the headers
#
# $OPENTHREADS_DIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENTHREADS_DIR
# used in building osg.
#
# Created by Eric Wing.

# Header files are presumed to be included like
# #include <OpenThreads/Thread>

# Try the user's environment request before anything else.

FIND_PATH(OPENTHREADS_INCLUDE_DIR OpenThreads/Thread
  ${CMAKE_INSTALL_PREFIX}/include
	$ENV{OPENTHREADS_DIR}/include
	$ENV{OSG_DIR}/include
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local/include
	/usr/include
	/sw/include # Fink
	/opt/local/include # DarwinPorts
	/opt/csw/include # Blastwave
	/opt/include
	[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
)


FIND_LIBRARY(OPENTHREADS_LIBRARY 
	NAMES OpenThreads OpenThreadsd OpenThreadsWin32 OpenThreadsWin32d
	PATHS
	${CMAKE_INSTALL_PREFIX}/lib
	$ENV{OPENTHREADS_DIR}/lib
	$ENV{OSG_DIR}/lib
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local/lib
	/usr/lib
	/sw/lib
	/opt/local/lib
	/opt/csw/lib
	/opt/lib
	[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
)

SET(OPENTHREADS_FOUND "NO")
IF(OPENTHREADS_INCLUDE_DIR AND OPENTHREADS_LIBRARY)
	SET(OPENTHREADS_FOUND "YES")
ENDIF(OPENTHREADS_INCLUDE_DIR AND OPENTHREADS_LIBRARY)


