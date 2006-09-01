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
//osgIntrospection - Copyright (C) 2005 Marco Jez

#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/ReaderWriter>

#include <string>

// Built-in types

ABSTRACT_OBJECT_REFLECTOR(void)

ATOMIC_VALUE_REFLECTOR(char)
WATOMIC_VALUE_REFLECTOR(wchar_t)
ATOMIC_VALUE_REFLECTOR(signed char)
ATOMIC_VALUE_REFLECTOR(unsigned char)

ATOMIC_VALUE_REFLECTOR(int)
ATOMIC_VALUE_REFLECTOR(unsigned int)
ATOMIC_VALUE_REFLECTOR(long int)
ATOMIC_VALUE_REFLECTOR(long long int)
ATOMIC_VALUE_REFLECTOR(unsigned long int)
ATOMIC_VALUE_REFLECTOR(unsigned long long int)
ATOMIC_VALUE_REFLECTOR(short int)
ATOMIC_VALUE_REFLECTOR(unsigned short int)

ATOMIC_VALUE_REFLECTOR(bool)

ATOMIC_VALUE_REFLECTOR(float)

ATOMIC_VALUE_REFLECTOR(double)
ATOMIC_VALUE_REFLECTOR(long double)


// STL types

ATOMIC_VALUE_REFLECTOR(std::string)
WATOMIC_VALUE_REFLECTOR(std::wstring)

