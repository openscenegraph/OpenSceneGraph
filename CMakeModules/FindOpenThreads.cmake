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
	NAMES OpenThreads  OpenThreadsWin32 
	PATHS
	${CMAKE_INSTALL_PREFIX}/lib
	$ENV{OPENTHREADS_DIR}/lib
	$ENV{OSG_DIR}/lib
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local/lib64
	/usr/local/lib
	/usr/lib64
	/usr/lib
	/sw/lib
	/opt/local/lib
	/opt/csw/lib
	/opt/lib
	[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
)
FIND_LIBRARY(OPENTHREADS_LIBRARY_DEBUG 
	NAMES  OpenThreadsd  OpenThreadsWin32d
	PATHS
	${CMAKE_INSTALL_PREFIX}/lib
	$ENV{OPENTHREADS_DIR}/lib
	$ENV{OSG_DIR}/lib
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local/lib64
	/usr/local/lib
	/usr/lib64
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
  MESSAGE("-- Found OpenThreads: "${OPENTHREADS_LIBRARY})
  IF(NOT OPENTHREADS_LIBRARY_DEBUG)
  	MESSAGE("-- Warning Debug OpenThreads not found, using: ${OPENTHREADS_LIBRARY}")
  	SET(OPENTHREADS_LIBRARY_DEBUG "${OPENTHREADS_LIBRARY}")
  ENDIF(NOT OPENTHREADS_LIBRARY_DEBUG)
ENDIF(OPENTHREADS_INCLUDE_DIR AND OPENTHREADS_LIBRARY)

