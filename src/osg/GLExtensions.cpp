#if defined(WIN32)
#include <windows.h>
#elif defined(__DARWIN_OSX__)
#include <mach-o/dyld.h>
#else
#include <dlfcn.h>
#endif

#include <osg/GL>
#include <osg/GLExtensions>
#include <osg/Notify>


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <set>

const bool osg::isGLExtensionSupported(const char *extension)
{
    typedef std::set<std::string>  ExtensionSet;
    static ExtensionSet s_extensionSet;
    static const char* s_extensions = NULL;
    if (s_extensions==NULL)
    {
        // get the extension list from OpenGL.
        s_extensions = (const char*)glGetString(GL_EXTENSIONS);
        if (s_extensions==NULL) return false;

        // insert the ' ' delimiated extensions words into the extensionSet.
        const char *startOfWord = s_extensions;
        const char *endOfWord;
        while ((endOfWord = strchr(startOfWord,' '))!=NULL)
        {
            s_extensionSet.insert(std::string(startOfWord,endOfWord));
            startOfWord = endOfWord+1;
        }
        if (*startOfWord!=0) s_extensionSet.insert(std::string(startOfWord));
        
        osg::notify(INFO)<<"OpenGL extensions supported by installed OpenGL drivers are:"<<std::endl;
        for(ExtensionSet::iterator itr=s_extensionSet.begin();
            itr!=s_extensionSet.end();
            ++itr)
        {
            osg::notify(INFO)<<"    "<<*itr<<std::endl;
        }
            
    }

    // true if extension found in extensionSet.
    bool result = s_extensionSet.find(extension)!=s_extensionSet.end();

    if (result) osg::notify(INFO)<<"OpenGL extension '"<<extension<<"' is supported."<<std::endl;
    else osg::notify(INFO)<<"OpenGL extension '"<<extension<<"' is not supported."<<std::endl;

    return result;
}


void* osg::getGLExtensionFuncPtr(const char *funcName)
{
#if defined(WIN32)
   return wglGetProcAddress(funcName);
#elif defined(__DARWIN_OSX__)
    std::string temp( "_" );
    NSSymbol symbol;
    temp += funcName;	// Mac OS X prepends an underscore on function names
    symbol = NSLookupAndBindSymbol( temp.c_str() );
    return NSAddressOfSymbol( symbol );
#else // all other unixes
   // Note: although we use shl_load() etc. for Plugins on HP-UX, it's
   // not neccessary here since we only used them because library
   // intialization was not taking place with dlopen() which renders
   // Plugins useless on HP-UX.
   static void *lib = dlopen("libGL.so", RTLD_LAZY);
   if (lib)
      return dlsym(lib, funcName);
   else
      return NULL;
#endif
}
