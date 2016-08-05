/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield
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

#ifndef OSGDB_FSTREAM
#define OSGDB_FSTREAM 1

#include <osgDB/Export>
#include <osg/Export>

#include <fstream>


namespace osgDB
{

/**
* Convenience function for fstream open , std::ifstream, and std::ofstream to
* automatically handle UTF-8 to UTF-16 filename conversion. Always use one
* of these classes in any OpenSceneGraph code instead of the STL equivalent.
*/

void OSGDB_EXPORT open(std::fstream& fs, const char* filename,std::ios_base::openmode mode);

class ifstream : public std::ifstream
{
public:
    OSGDB_EXPORT ifstream();
    OSGDB_EXPORT explicit ifstream(const char* filename,
        std::ios_base::openmode mode = std::ios_base::in);
    OSGDB_EXPORT ~ifstream();

    void OSGDB_EXPORT open(const char* filename,
        std::ios_base::openmode mode = std::ios_base::in);
};

class ofstream : public std::ofstream
{
public:
    OSGDB_EXPORT ofstream();
    OSGDB_EXPORT explicit ofstream(const char* filename,
        std::ios_base::openmode mode = std::ios_base::out);
    OSGDB_EXPORT ~ofstream();

    void OSGDB_EXPORT open(const char* filename,
        std::ios_base::openmode mode = std::ios_base::out);
};

}

#endif
