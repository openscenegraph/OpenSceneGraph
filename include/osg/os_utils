/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2017 Robert Osfield
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

#ifndef OSG_os_utils
#define OSG_os_utils 1

#include <osg/Export>

#ifdef __cplusplus
extern "C" {
#endif

/** Cross platform version of C system() function. */
extern OSG_EXPORT int osg_system(const char* str);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#include <string>

#if defined(OSG_ENVVAR_SUPPORTED)
    #include <stdlib.h>
    #include <sstream>
#endif

namespace osg {


inline unsigned int getClampedLength(const char* str, unsigned int maxNumChars=4096)
{
    unsigned int i = 0;
    while(i<maxNumChars && str[i]!=0) { ++i; }
    return i;
}

inline std::string getEnvVar(const char* name)
{
#ifdef OSG_ENVVAR_SUPPORTED
    std::string value;
    const char* ptr = getenv(name);
    if (ptr) value.assign(ptr, getClampedLength(ptr));
    return value;
#else
    OSG_UNUSED(name);
    return std::string();
#endif
}


template<typename T>
inline bool getEnvVar(const char* name, T& value)
{
#ifdef OSG_ENVVAR_SUPPORTED
    const char* ptr = getenv(name);
    if (!ptr) return false;

    std::istringstream str(std::string(ptr, getClampedLength(ptr)));
    str >> value;
    return !str.fail();
#else
    OSG_UNUSED2(name, value);
    return false;
#endif
}

template<>
inline bool getEnvVar(const char* name, std::string& value)
{
#ifdef OSG_ENVVAR_SUPPORTED
    const char* ptr = getenv(name);
    if (!ptr) return false;

    value.assign(ptr, getClampedLength(ptr));
    return true;
#else
    OSG_UNUSED2(name, value);
    return false;
#endif
}

template<typename T1, typename T2>
inline bool getEnvVar(const char* name, T1& value1, T2& value2)
{
#ifdef OSG_ENVVAR_SUPPORTED
    const char* ptr = getenv(name);
    if (!ptr) return false;

    std::istringstream str(std::string(ptr, getClampedLength(ptr)));
    str >> value1 >> value2;
    return !str.fail();
#else
    OSG_UNUSED3(name, value1, value2);
    return false;
#endif
}

template<typename T1, typename T2, typename T3>
inline bool getEnvVar(const char* name, T1& value1, T2& value2, T3& value3)
{
#ifdef OSG_ENVVAR_SUPPORTED
    const char* ptr = getenv(name);
    if (!ptr) return false;

    std::istringstream str(std::string(ptr, getClampedLength(ptr)));
    str >> value1 >> value2 >> value3;
    return !str.fail();
#else
    OSG_UNUSED4(name, value1, value2, value3);
    return false;
#endif
}

template<typename T1, typename T2, typename T3, typename T4>
inline bool getEnvVar(const char* name, T1& value1, T2& value2, T3& value3, T4& value4)
{
#ifdef OSG_ENVVAR_SUPPORTED
    const char* ptr = getenv(name);
    if (!ptr) return false;

    std::istringstream str(std::string(ptr, getClampedLength(ptr)));
    str >> value1 >> value2 >> value3 >> value4;
    return !str.fail();
#else
    OSG_UNUSED5(name, value1, value2, value3, value4);
    return false;
#endif
}

}

#endif // _cplusplus

# endif
