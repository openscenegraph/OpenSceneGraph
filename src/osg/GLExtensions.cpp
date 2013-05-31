/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#include <osg/GLExtensions>
#include <osg/GL>
#include <osg/Notify>
#include <osg/Math>
#include <osg/buffered_value>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

#include <string>
#include <vector>
#include <set>

#if defined(WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif // WIN32_LEAN_AND_MEAN
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif // NOMINMAX
    #include <windows.h>
#elif defined(__APPLE__)
    // The NS*Symbol* stuff found in <mach-o/dyld.h> is deprecated.
    // Since 10.3 (Panther) OS X has provided the dlopen/dlsym/dlclose
    // family of functions under <dlfcn.h>. Since 10.4 (Tiger), Apple claimed
    // the dlfcn family was significantly faster than the NS*Symbol* family.
    // Since 'deprecated' needs to be taken very seriously with the
    // coming of 10.5 (Leopard), it makes sense to use the dlfcn family when possible.
    #include <AvailabilityMacros.h>
    #if !defined(MAC_OS_X_VERSION_10_3) || (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_3)
        #define USE_APPLE_LEGACY_NSSYMBOL
        #include <mach-o/dyld.h>
    #else
        #include <dlfcn.h>
    #endif
#else
    #include <dlfcn.h>
#endif

typedef std::set<std::string>  ExtensionSet;
static osg::buffered_object<ExtensionSet> s_glExtensionSetList;
static osg::buffered_object<std::string> s_glRendererList;
static osg::buffered_value<int> s_glInitializedList;

static osg::buffered_object<ExtensionSet> s_gluExtensionSetList;
static osg::buffered_object<std::string> s_gluRendererList;
static osg::buffered_value<int> s_gluInitializedList;

float osg::getGLVersionNumber()
{
    // needs to be extended to do proper things with subversions like 1.5.1, etc.
    char *versionstring   = (char*) glGetString( GL_VERSION );
    if (!versionstring) return 0.0;

    return (findAsciiToFloat(versionstring));
}

bool osg::isExtensionInExtensionString(const char *extension, const char *extensionString)
{
    const char *startOfWord = extensionString;
    const char *endOfWord;
    while ((endOfWord = strchr(startOfWord,' ')) != 0)
    {
        if (strncmp(extension, startOfWord, endOfWord - startOfWord) == 0)
            return true;
        startOfWord = endOfWord+1;
    }
    if (*startOfWord && strcmp(extension, startOfWord) == 0)
        return true;

   return false;
}

bool osg::isGLExtensionSupported(unsigned int contextID, const char *extension)
{
    return osg::isGLExtensionOrVersionSupported(contextID, extension, FLT_MAX);
}

bool osg::isGLExtensionOrVersionSupported(unsigned int contextID, const char *extension, float requiredGLVersion)
{
    ExtensionSet& extensionSet = s_glExtensionSetList[contextID];
    std::string& rendererString = s_glRendererList[contextID];

    // first check to see if GL version number of recent enough.
    bool result = requiredGLVersion <= osg::getGLVersionNumber();

    if (!result)
    {
        // if not already set up, initialize all the per graphic context values.
        if (!s_glInitializedList[contextID])
        {
            s_glInitializedList[contextID] = 1;

            // set up the renderer
            const GLubyte* renderer = glGetString(GL_RENDERER);
            rendererString = renderer ? (const char*)renderer : "";

            // get the extension list from OpenGL.
            GLint numExt = 0;
            #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
            if( osg::getGLVersionNumber() >= 3.0 )
            {
                // OpenGL 3.0 adds the concept of indexed strings and
                // deprecates calls to glGetString( GL_EXTENSIONS ), which
                // will now generate GL_INVALID_ENUM.

                // Get extensions using new indexed string interface.

                typedef const GLubyte * GL_APIENTRY PFNGLGETSTRINGIPROC( GLenum, GLuint );
                PFNGLGETSTRINGIPROC* glGetStringi = 0;
                setGLExtensionFuncPtr( glGetStringi, "glGetStringi");

                if( glGetStringi != NULL )
                {
                    #  ifndef GL_NUM_EXTENSIONS
                    #    define GL_NUM_EXTENSIONS 0x821D
                    #  endif
                    glGetIntegerv( GL_NUM_EXTENSIONS, &numExt );
                    int idx;
                    for( idx=0; idx<numExt; idx++ )
                    {
                        extensionSet.insert( std::string( (char*)( glGetStringi( GL_EXTENSIONS, idx ) ) ) );
                    }
                }
                else
                {
                    OSG_WARN << "isGLExtensionOrVersionSupported: Can't obtain glGetStringi function pointer." << std::endl;
                }
            }
            #endif

            // No extensions found so far, so try with glGetString
            if (numExt == 0)
            {
                // Get extensions using GL1/2 interface.

                const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
                if (extensions==NULL) return false;

                // insert the ' ' delimiated extensions words into the extensionSet.
                const char *startOfWord = extensions;
                const char *endOfWord;
                while ((endOfWord = strchr(startOfWord,' '))!=NULL)
                {
                    extensionSet.insert(std::string(startOfWord,endOfWord));
                    startOfWord = endOfWord+1;
                }
                if (*startOfWord!=0) extensionSet.insert(std::string(startOfWord));
            }

    #if defined(WIN32) && (defined(OSG_GL1_AVAILABLE) || defined(OSG_GL2_AVAILABLE) || defined(OSG_GL3_AVAILABLE))

            // add WGL extensions to the list

            typedef const char* WINAPI WGLGETEXTENSIONSSTRINGARB(HDC);
            WGLGETEXTENSIONSSTRINGARB* wglGetExtensionsStringARB = 0;
            setGLExtensionFuncPtr(wglGetExtensionsStringARB, "wglGetExtensionsStringARB");

            typedef const char* WINAPI WGLGETEXTENSIONSSTRINGEXT();
            WGLGETEXTENSIONSSTRINGEXT* wglGetExtensionsStringEXT = 0;
            setGLExtensionFuncPtr(wglGetExtensionsStringEXT, "wglGetExtensionsStringEXT");

            const char* wglextensions = 0;

            if (wglGetExtensionsStringARB)
            {
                HDC dc = wglGetCurrentDC();
                wglextensions = wglGetExtensionsStringARB(dc);
            }
            else if (wglGetExtensionsStringEXT)
            {
                wglextensions = wglGetExtensionsStringEXT();
            }

            if (wglextensions)
            {
                const char* startOfWord = wglextensions;
                const char* endOfWord;
                while ((endOfWord = strchr(startOfWord, ' ')))
                {
                    extensionSet.insert(std::string(startOfWord, endOfWord));
                    startOfWord = endOfWord+1;
                }
                if (*startOfWord != 0) extensionSet.insert(std::string(startOfWord));
            }

    #endif

            OSG_NOTIFY(INFO)<<"OpenGL extensions supported by installed OpenGL drivers are:"<<std::endl;
            for(ExtensionSet::iterator itr=extensionSet.begin();
                itr!=extensionSet.end();
                ++itr)
            {
                OSG_NOTIFY(INFO)<<"    "<<*itr<<std::endl;
            }

        }

        // true if extension found in extensionSet.
        result = extensionSet.find(extension)!=extensionSet.end();
    }

    // now see if extension is in the extension disabled list
    bool extensionDisabled = false;
    if (result)
    {

        const std::string& disableString = getGLExtensionDisableString();
        if (!disableString.empty())
        {

            std::string::size_type pos=0;
            while ( pos!=std::string::npos && (pos=disableString.find(extension,pos))!=std::string::npos )
            {
                std::string::size_type previousColon = disableString.find_last_of(':',pos);
                std::string::size_type previousSemiColon = disableString.find_last_of(';',pos);

                std::string renderer = "";
                if (previousColon!=std::string::npos)
                {
                    if (previousSemiColon==std::string::npos) renderer = disableString.substr(0,previousColon);
                    else if (previousSemiColon<previousColon) renderer = disableString.substr(previousSemiColon+1,previousColon-previousSemiColon-1);
                }

                if (!renderer.empty())
                {

                    // remove leading spaces if they exist.
                    std::string::size_type leadingSpaces = renderer.find_first_not_of(' ');
                    if (leadingSpaces==std::string::npos) renderer = ""; // nothing but spaces
                    else if (leadingSpaces!=0) renderer.erase(0,leadingSpaces);

                    // remove trailing spaces if they exist.
                    std::string::size_type trailingSpaces = renderer.find_last_not_of(' ');
                    if (trailingSpaces!=std::string::npos) renderer.erase(trailingSpaces+1,std::string::npos);

                }

                if (renderer.empty())
                {
                    extensionDisabled = true;
                    break;
                }

                if (rendererString.find(renderer)!=std::string::npos)
                {
                    extensionDisabled = true;
                    break;

                }

                // move the position in the disable string along so that the same extension is found multiple times
                ++pos;
            }

        }
    }

    if (result)
    {
        if (!extensionDisabled)
        {
            OSG_NOTIFY(INFO)<<"OpenGL extension '"<<extension<<"' is supported."<<std::endl;
        }
        else
        {
            OSG_NOTIFY(INFO)<<"OpenGL extension '"<<extension<<"' is supported by OpenGL\ndriver but has been disabled by osg::getGLExtensionDisableString()."<<std::endl;
        }
    }
    else
    {
        OSG_NOTIFY(INFO)<<"OpenGL extension '"<<extension<<"' is not supported."<<std::endl;
    }


    return result && !extensionDisabled;
}

void osg::setGLExtensionDisableString(const std::string& disableString)
{
    getGLExtensionDisableString() = disableString;
}

std::string& osg::getGLExtensionDisableString()
{
    static const char* envVar = getenv("OSG_GL_EXTENSION_DISABLE");
    static std::string s_GLExtensionDisableString(envVar?envVar:"Nothing defined");

    return s_GLExtensionDisableString;
}

OSG_INIT_SINGLETON_PROXY(GLExtensionDisableStringInitializationProxy, osg::getGLExtensionDisableString())

#ifdef OSG_GL_LIBRARY_STATIC

    #include "GLStaticLibrary.h"

    void* osg::getGLExtensionFuncPtr(const char *funcName)
    {
        return GLStaticLibrary::getProcAddress(funcName);
    }

#else

    void* osg::getGLExtensionFuncPtr(const char *funcName)
    {
        // OSG_NOTICE<<"osg::getGLExtensionFuncPtr("<<funcName<<")"<<std::endl;
    #if defined(ANDROID)
        #if defined(OSG_GLES1_AVAILABLE)
            static void *handle = dlopen("libGLESv1_CM.so", RTLD_NOW);
        #elif defined(OSG_GLES2_AVAILABLE)
            static void *handle = dlopen("libGLESv2.so", RTLD_NOW);
        #endif
        return dlsym(handle, funcName);

    #elif defined(WIN32)

        #if defined(OSG_GLES2_AVAILABLE)
            static HMODULE hmodule = GetModuleHandle(TEXT("libGLESv2.dll"));
            return convertPointerType<void*, PROC>(GetProcAddress(hmodule, funcName));
        #elif defined(OSG_GLES1_AVAILABLE)
            static HMODULE hmodule = GetModuleHandleA(TEXT("libgles_cm.dll"));
            return convertPointerType<void*, PROC>(GetProcAddress(hmodule, funcName));
        #else
            return convertPointerType<void*, PROC>(wglGetProcAddress(funcName));
        #endif

    #elif defined(__APPLE__)

        #if defined(USE_APPLE_LEGACY_NSSYMBOL)
            std::string temp( "_" );
            temp += funcName;    // Mac OS X prepends an underscore on function names
            if ( NSIsSymbolNameDefined( temp.c_str() ) )
            {
                NSSymbol symbol = NSLookupAndBindSymbol( temp.c_str() );
                return NSAddressOfSymbol( symbol );
            } else
                return NULL;
        #else
            // I am uncertain of the correct and ideal usage of dlsym here.
            // On the surface, it would seem that the FreeBSD implementation
            // would be the ideal one to copy, but ELF and Mach-o are different
            // and Apple's man page says the following about using RTLD_DEFAULT:
            // "This can be a costly search and should be avoided."
            // The documentation mentions nothing about passing in 0 so I must
            // assume the behavior is undefined.
            // So I could try copying the Sun method which I think all this
            // actually originated from.

            // return dlsym( RTLD_DEFAULT, funcName );
            static void *handle = dlopen((const char *)0L, RTLD_LAZY);
            return dlsym(handle, funcName);
        #endif

    #elif defined (__sun)

        static void *handle = dlopen((const char *)0L, RTLD_LAZY);
        return dlsym(handle, funcName);

    #elif defined (__sgi)

        static void *handle = dlopen((const char *)0L, RTLD_LAZY);
        return dlsym(handle, funcName);

    #elif defined (__FreeBSD__)

        return dlsym( RTLD_DEFAULT, funcName );

    #elif defined (__linux__)

        typedef void (*__GLXextFuncPtr)(void);
        typedef __GLXextFuncPtr (*GetProcAddressARBProc)(const char*);

        #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
        static GetProcAddressARBProc s_glXGetProcAddressARB = convertPointerType<GetProcAddressARBProc, void*>(dlsym(0, "glXGetProcAddressARB"));
        if (s_glXGetProcAddressARB)
        {
            return convertPointerType<void*, __GLXextFuncPtr>((s_glXGetProcAddressARB)(funcName));
        }
        #endif

        return dlsym(0, funcName);

    #elif defined (__QNX__)

        return dlsym(RTLD_DEFAULT, funcName);

    #else // all other unixes

        return dlsym(0, funcName);

    #endif
    }
#endif
