/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

    HANDLE handle = NULL;


/*
// Daniel's proposed version.
    // try letting the OS find the library first
    handle = getLibraryHandle( libraryName );
    if (handle) return new DynamicLibrary(libraryName,handle);

    // else try to locate it ourselves
    std::string fullLibraryName = osgDB::findLibraryFile(libraryName);
    if (fullLibraryName.empty()) return NULL;
    handle = getLibraryHandle( fullLibraryName );
    if (handle) return new DynamicLibrary(libraryName,handle);

    notify(WARN)
        << "DynamicLibrary::failed loading "<<fullLibraryName<<std::endl;
*/

// Robert's version of above reordering of findLibraryFile path.

    std::string fullLibraryName = osgDB::findLibraryFile(libraryName);
    if (!fullLibraryName.empty()) handle = getLibraryHandle( fullLibraryName ); // try the lib we have found
    else handle = getLibraryHandle( libraryName ); // havn't found a lib ourselves, see if the OS can find it simply from the library name.
    
    if (handle) return new DynamicLibrary(libraryName,handle);

    // else no lib found so report errors.
    notify(WARN) << "DynamicLibrary::failed loading "<<fullLibraryName<<std::endl;

#if defined(WIN32) && !defined(__CYGWIN__)
    // nothing more
#elif defined(__DARWIN_OSX__)
    // nothing more
#elif defined(__hpux__)
    notify(WARN) << "DynamicLibrary::error "<<strerror(errno)<<std::endl;
#else // other unix
    notify(WARN) << "DynamicLibrary::error "<<dlerror()<<std::endl;
#endif
    return NULL;
}

DynamicLibrary::HANDLE
DynamicLibrary::getLibraryHandle( const std::string& libraryName)
{
    HANDLE handle = NULL;

#if defined(WIN32) && !defined(__CYGWIN__)
    handle = LoadLibrary( libraryName.c_str() );
#elif defined(__DARWIN_OSX__)
    NSObjectFileImage image;
    // NSModule os_handle = NULL;
    if (NSCreateObjectFileImageFromFile(libraryName.c_str(), &image) == NSObjectFileImageSuccess) {
        // os_handle = NSLinkModule(image, libraryName.c_str(), TRUE);
        handle = NSLinkModule(image, libraryName.c_str(), TRUE);
        NSDestroyObjectFileImage(image);
    }
#elif defined(__hpux__)
    // BIND_FIRST is neccessary for some reason
    handle = shl_load ( libraryName.c_str(),
            BIND_DEFERRED|BIND_FIRST|BIND_VERBOSE, 0);
    return handle;
#else // other unix
    handle = dlopen( libraryName.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if( handle == NULL )
        printf( "dlopen: %s\n", dlerror() );
#endif
    return handle;
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
