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
#include <io.h>
#include <windows.h>
#include <winbase.h>
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
#include <mach-o/dyld.h>
#else // all other unix
#include <unistd.h>
#ifdef __hpux
// Although HP-UX has dlopen() it is broken! We therefore need to stick
// to shl_load()/shl_unload()/shl_findsym()
#include <dl.h>
#include <errno.h>
#else
#include <dlfcn.h>
#endif
#endif

#include <osg/Notify>
#include <osg/GLExtensions>

#include <osgDB/DynamicLibrary>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ConvertUTF>

using namespace osgDB;

DynamicLibrary::DynamicLibrary(const std::string& name, HANDLE handle)
{
    _name = name;
    _handle = handle;
    OSG_INFO<<"Opened DynamicLibrary "<<_name<<std::endl;
}

DynamicLibrary::~DynamicLibrary()
{
    if (_handle)
    {
        OSG_INFO<<"Closing DynamicLibrary "<<_name<<std::endl;
#if defined(WIN32) && !defined(__CYGWIN__)
        FreeLibrary((HMODULE)_handle);
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
        NSUnLinkModule(static_cast<NSModule>(_handle), FALSE);
#elif defined(__hpux)
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
    OSG_INFO << "DynamicLibrary::failed loading \""<<libraryName<<"\""<<std::endl;

    return NULL;
}

DynamicLibrary::HANDLE DynamicLibrary::getLibraryHandle( const std::string& libraryName)
{
    HANDLE handle = NULL;

#if defined(WIN32) && !defined(__CYGWIN__)
#ifdef OSG_USE_UTF8_FILENAME
    handle = LoadLibraryW(  convertUTF8toUTF16(libraryName).c_str() );
#else
    handle = LoadLibrary( libraryName.c_str() );
#endif
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
    NSObjectFileImage image;
    // NSModule os_handle = NULL;
    if (NSCreateObjectFileImageFromFile(libraryName.c_str(), &image) == NSObjectFileImageSuccess) {
        // os_handle = NSLinkModule(image, libraryName.c_str(), TRUE);
        handle = NSLinkModule(image, libraryName.c_str(), TRUE);
        NSDestroyObjectFileImage(image);
    }
#elif defined(__hpux)
    // BIND_FIRST is necessary for some reason
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
    {
        if (fileExists(localLibraryName))
        {
            OSG_WARN << "Warning: dynamic library '" << libraryName << "' exists, but an error occurred while trying to open it:" << std::endl;
            OSG_WARN << dlerror() << std::endl;
        }
        else
        {
            OSG_INFO << "Warning: dynamic library '" << libraryName << "' does not exist (or isn't readable):" << std::endl;
            OSG_INFO << dlerror() << std::endl;
        }
    }
#endif
    return handle;
}

DynamicLibrary::PROC_ADDRESS DynamicLibrary::getProcAddress(const std::string& procName)
{
    if (_handle==NULL) return NULL;
#if defined(WIN32) && !defined(__CYGWIN__)
    return osg::convertPointerType<DynamicLibrary::PROC_ADDRESS, FARPROC>( GetProcAddress( (HMODULE)_handle, procName.c_str() ) );
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
    std::string temp("_");
    NSSymbol symbol;
    temp += procName;   // Mac OS X prepends an underscore on function names
    symbol = NSLookupSymbolInModule(static_cast<NSModule>(_handle), temp.c_str());
    return NSAddressOfSymbol(symbol);
#elif defined(__hpux)
    void* result = NULL;
    if (shl_findsym (reinterpret_cast<shl_t*>(&_handle), procName.c_str(), TYPE_PROCEDURE, result) == 0)
    {
        return result;
    }
    else
    {
        OSG_WARN << "DynamicLibrary::failed looking up " << procName << std::endl;
        OSG_WARN << "DynamicLibrary::error " << strerror(errno) << std::endl;
        return NULL;
    }
#else // other unix
    void* sym = dlsym( _handle,  procName.c_str() );
    if (!sym) {
        OSG_WARN << "DynamicLibrary::failed looking up " << procName << std::endl;
        OSG_WARN << "DynamicLibrary::error " << dlerror() << std::endl;
    }
    return sym;
#endif
}

