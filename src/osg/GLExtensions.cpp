#ifdef _WIN32
#include <windows.h>
#elif !defined macintosh
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
        
        osg::notify(INFO)<<"OpenGL extensions supported by installed OpenGL drivers are:"<<endl;
        for(ExtensionSet::iterator itr=s_extensionSet.begin();
            itr!=s_extensionSet.end();
            ++itr)
        {
            osg::notify(INFO)<<"    "<<*itr<<endl;
        }
            
    }

    // true if extension found in extensionSet.
    bool result = s_extensionSet.find(extension)!=s_extensionSet.end();

    if (result) osg::notify(INFO)<<"OpenGL extension '"<<extension<<"' is supported."<<endl;
    else osg::notify(INFO)<<"OpenGL extension '"<<extension<<"' is not supported."<<endl;

    return result;
}


void* osg::getGLExtensionFuncPtr(const char *funcName)
{
#if defined(_WIN32)
   return wglGetProcAddress(funcName);
#elif defined(macintosh)
   return NULL;
#else
   static void *lib = dlopen("libGL.so", RTLD_LAZY);
   if (lib)
      return dlsym(lib, funcName);
   else
      return NULL;
#endif
}
