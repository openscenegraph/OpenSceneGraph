#ifdef WIN32
#include <Io.h>
#include <Windows.h>
#include <Winbase.h>
#elif !defined(macintosh)
#include <unistd.h>
#include <dlfcn.h>
#endif

#include <osg/Notify>

#include <osgDB/DynamicLibrary>
#include <osgDB/FileUtils>

using namespace osg;
using namespace osgDB;

DynamicLibrary::DynamicLibrary(const std::string& name,HANDLE handle)
{
    _name = name;
    _handle = handle;
}

DynamicLibrary::~DynamicLibrary()
{
    if (_handle)
    {
#ifdef WIN32
        FreeLibrary((HMODULE)_handle);
#elif !defined(macintosh)
        dlclose(_handle);
#endif    
    }
}

DynamicLibrary* DynamicLibrary::loadLibrary(const std::string& libraryName)
{

    char* fullLibraryName = osgDB::findDSO( libraryName.c_str() );
    if (fullLibraryName==NULL) return NULL;

#ifdef WIN32
    HANDLE handle = LoadLibrary( fullLibraryName );
    if (handle) return new DynamicLibrary(libraryName,handle);
    notify(WARN) << "DynamicLibrary::failed loading "<<fullLibraryName<<endl;
#elif !defined(macintosh)
    HANDLE handle = dlopen( fullLibraryName, RTLD_LAZY );
    if (handle) return new DynamicLibrary(libraryName,handle);
    notify(WARN) << "DynamicLibrary::failed loading "<<fullLibraryName<<endl;
    notify(WARN) << "DynamicLibrary::error "<<dlerror()<<endl;
#endif
    return NULL;
}

DynamicLibrary::PROC_ADDRESS DynamicLibrary::getProcAddress(const std::string& procName)
{
    if (_handle==NULL) return NULL;
#ifdef WIN32
    return GetProcAddress( (HMODULE)_handle,  procName.c_str() );
#elif !defined(macintosh)
    return dlsym( _handle,  procName.c_str() );
#endif
    return NULL;
}
