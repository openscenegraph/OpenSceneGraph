#pragma once

#include <memory>
#include <wrl.h>

#define GL_GLEXT_PROTOTYPES 1 // why do we need this when vs template example didn't?
// OpenGL ES includes
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// EGL includes
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

// ANGLE include for Windows Store
#include <angle_windowsstore.h>