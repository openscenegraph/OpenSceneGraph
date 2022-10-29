# Find Wayland development libraries & headers for wayland-egl & wayland-cursor & EGL
#
# WAYLAND_FOUND - system has a libwayland-[egl|client|cursor] + libEGL
# WAYLAND_INCLUDE_DIR - where to find header files
# WAYLAND_LIBRARIES - the libraries to link against Wayland
#
# copyright (c) 2022 Phil Ashby <phil.github@ashbysoft.com>
# Redistribution and use of this file is allowed according to the terms of the BSD license.
#

# Use PkgConfig to find includes and libs
find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
	PKG_CHECK_MODULES(WAYLAND QUIET wayland-egl wayland-cursor egl)
endif ()
