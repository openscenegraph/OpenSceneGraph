#if defined(WIN32) && !defined(__CYGWIN__)
#include <Io.h>
#include <Windows.h>
#include <Winbase.h>
#elif defined(__DARWIN_OSX__)
#include <mach-o/dyld.h>
#else // all other unix
#include <unistd.h>
#ifdef __hpux__
// Although HP-UX has dlopen() it is broken! We therefore need to stick
// to shl_load()/shl_unload()/shl_findsym()
#include <dl.h>
#include <errno.h>
#else
#include <dlfcn.h>
#endif
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
#if defined(WIN32) && !defined(__CYGWIN__)
        FreeLibrary((HMODULE)_handle);
#elif defined(__DARWIN_OSX__)
        NSUnLinkModule(_handle, FALSE);
#elif defined(__hpux__)
        // fortunately, shl_t is a pointer
        shl_unload (static_cast<shl_t>(_handle));
#else // other unix
        dlclose(_handle);
#endif    
    }
}

DynamicLibrary* DynamicLibrary::loadLibrary(const std::string& libraryName)
{

    std::string fullLibraryName = osgDB::findLibraryFile(libraryName);
    if (fullLibraryName.empty()) return NULL;

#if defined(WIN32) && !defined(__CYGWIN__)
    HANDLE handle = LoadLibrary( fullLibraryName.c_str() );
    if (handle) return osgNew DynamicLibrary(libraryName,handle);
    notify(WARN) << "DynamicLibrary::failed loading "<<fullLibraryName<<std::endl;
#elif defined(__DARWIN_OSX__)
    NSObjectFileImage image;
    // NSModule os_handle = NULL;
    if (NSCreateObjectFileImageFromFile(fullLibraryName.c_str(), &image) == NSObjectFileImageSuccess) {
        // os_handle = NSLinkModule(image, fullLibraryName.c_str(), TRUE);
        HANDLE handle = NSLinkModule(image, fullLibraryName.c_str(), TRUE);
        NSDestroyObjectFileImage(image);
        if (handle) return osgNew DynamicLibrary(libraryName,handle);
    }
    // if (os_handle) return osgNew DynamicLibrary(libraryName,os_handle);
    notify(WARN) << "DynamicLibrary::failed loading "<<fullLibraryName<<std::endl;
#elif defined(__hpux__)
    // BIND_FIRST is neccessary for some reason
    HANDLE handle = shl_load ( fullLibraryName.c_str(), BIND_DEFERRED|BIND_FIRST|BIND_VERBOSE, 0);
    if (handle) return osgNew DynamicLibrary(libraryName,handle);
    notify(WARN) << "DynamicLibrary::failed loading "<<fullLibraryName<<std::endl;
    notify(WARN) << "DynamicLibrary::error "<<strerror(errno)<<std::endl;
#else // other unix
    HANDLE handle = dlopen( fullLibraryName.c_str(), RTLD_LAZY );
    if (handle) return osgNew DynamicLibrary(libraryName,handle);
    notify(WARN) << "DynamicLibrary::failed loading "<<fullLibraryName<<std::endl;
    notify(WARN) << "DynamicLibrary::error "<<dlerror()<<std::endl;
#endif
    return NULL;
}

DynamicLibrary::PROC_ADDRESS DynamicLibrary::getProcAddress(const std::string& procName)
{
    if (_handle==NULL) return NULL;
#if defined(WIN32) && !defined(__CYGWIN__)
    return (DynamicLibrary::PROC_ADDRESS)GetProcAddress( (HMODULE)_handle,
                                                         procName.c_str() );
#elif defined(__DARWIN_OSX__)
    std::string temp("_");
    NSSymbol symbol;
    temp += procName;	// Mac OS X prepends an underscore on function names
    symbol = NSLookupAndBindSymbol(temp.c_str());
    return NSAddressOfSymbol(symbol);
#elif defined(__hpux__)
    void* result = NULL;
    if (shl_findsym (reinterpret_cast<shl_t*>(&_handle), procName.c_str(), TYPE_PROCEDURE, result) == 0)
    {
        return result;
    }
    else
    {
        notify(WARN) << "DynamicLibrary::failed looking up " << procName << std::endl;
        notify(WARN) << "DynamicLibrary::error " << strerror(errno) << std::endl;
        return NULL;
    }
#else // other unix
    void* sym = dlsym( _handle,  procName.c_str() );
    if (!sym) {
        notify(WARN) << "DynamicLibrary::failed looking up " << procName << std::endl;
        notify(WARN) << "DynamicLibrary::error " << dlerror() << std::endl;
    }
    return sym;
#endif
    return NULL;
}
