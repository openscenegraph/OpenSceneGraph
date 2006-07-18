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

//The dlopen calls were not adding to OS X until 10.3 
#ifdef __APPLE__
#include <AvailabilityMacros.h>
#if !defined(MAC_OS_X_VERSION_10_3) || (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_3)
#define APPLE_PRE_10_3
#endif
#endif

#if defined(WIN32) && !defined(__CYGWIN__)
#include <Io.h>
#include <Windows.h>
#include <Winbase.h>
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
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
#include <osgDB/FileNameUtils>

using namespace osg;
using namespace osgDB;

DynamicLibrary::DynamicLibrary(const std::string& name,HANDLE handle)
{
    _name = name;
    _handle = handle;
    osg::notify(osg::INFO)<<"Opened DynamicLibrary "<<_name<<std::endl;
}

DynamicLibrary::~DynamicLibrary()
{
    if (_handle)
    {
        osg::notify(osg::INFO)<<"Closing DynamicLibrary "<<_name<<std::endl;
#if defined(WIN32) && !defined(__CYGWIN__)
        FreeLibrary((HMODULE)_handle);
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
        NSUnLinkModule(static_cast<NSModule>(_handle), FALSE);
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

    std::string fullLibraryName = osgDB::findLibraryFile(libraryName);
    if (!fullLibraryName.empty()) handle = getLibraryHandle( fullLibraryName ); // try the lib we have found
    else handle = getLibraryHandle( libraryName ); // havn't found a lib ourselves, see if the OS can find it simply from the library name.
    
    if (handle) return new DynamicLibrary(libraryName,handle);

    // else no lib found so report errors.
    notify(INFO) << "DynamicLibrary::failed loading \""<<libraryName<<"\""<<std::endl;

    return NULL;
}

DynamicLibrary::HANDLE DynamicLibrary::getLibraryHandle( const std::string& libraryName)
{
    HANDLE handle = NULL;

#if defined(WIN32) && !defined(__CYGWIN__)
    handle = LoadLibrary( libraryName.c_str() );
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
    NSObjectFileImage image;
    // NSModule os_handle = NULL;
    if (NSCreateObjectFileImageFromFile(libraryName.c_str(), &image) == NSObjectFileImageSuccess) {
        // os_handle = NSLinkModule(image, libraryName.c_str(), TRUE);
        handle = NSLinkModule(image, libraryName.c_str(), TRUE);
        NSDestroyObjectFileImage(image);
    }
#elif defined(__hpux__)
    // BIND_FIRST is neccessary for some reason
    handle = shl_load ( libraryName.c_str(), BIND_DEFERRED|BIND_FIRST|BIND_VERBOSE, 0);
    return handle;
#else // other unix

    // dlopen will not work with files in the current directory unless
    // they are prefaced with './'  (DB - Nov 5, 2003).
    std::string localLibraryName;
    if( libraryName == osgDB::getSimpleFileName( libraryName ) )
        localLibraryName = "./" + libraryName;
    else
        localLibraryName = libraryName;
    handle = dlopen( localLibraryName.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if( handle == NULL )
        notify(INFO) << "DynamicLibrary::getLibraryHandle( "<< libraryName << ") - dlopen(): " << dlerror() << std::endl;
#endif
    return handle;
}

DynamicLibrary::PROC_ADDRESS DynamicLibrary::getProcAddress(const std::string& procName)
{
    if (_handle==NULL) return NULL;
#if defined(WIN32) && !defined(__CYGWIN__)
    return (DynamicLibrary::PROC_ADDRESS)GetProcAddress( (HMODULE)_handle,
                                                         procName.c_str() );
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
    std::string temp("_");
    NSSymbol symbol;
    temp += procName;   // Mac OS X prepends an underscore on function names
    symbol = NSLookupSymbolInModule(static_cast<NSModule>(_handle), temp.c_str());
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
