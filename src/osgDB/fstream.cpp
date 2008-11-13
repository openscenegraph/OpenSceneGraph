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

#include <osgDB/fstream>
#include <osgDB/ConvertUTF>

#include <osg/Config>

namespace osgDB
{

#ifdef OSG_USE_UTF8_FILENAME
#define OSGDB_CONVERT_UTF8_FILENAME(s) convertUTF8toUTF16(s).c_str()
#else
#define OSGDB_CONVERT_UTF8_FILENAME(s) s
#endif

    fstream::fstream(){}
    fstream::fstream(const char* filename,
        std::ios_base::openmode mode) : std::fstream(OSGDB_CONVERT_UTF8_FILENAME(filename), mode)
    {}
    fstream::~fstream(){}
    void fstream::open(const char* filename,
        std::ios_base::openmode mode)
    {
        std::fstream::open(OSGDB_CONVERT_UTF8_FILENAME(filename), mode);
    }

    ifstream::ifstream(){}
    ifstream::ifstream(const char* filename,
        std::ios_base::openmode mode) : std::ifstream(OSGDB_CONVERT_UTF8_FILENAME(filename), mode)
    {}
    ifstream::~ifstream(){}
    void ifstream::open(const char* filename,
        std::ios_base::openmode mode)
    {
        std::ifstream::open(OSGDB_CONVERT_UTF8_FILENAME(filename), mode);
    }

    ofstream::ofstream(){}
    ofstream::ofstream(const char* filename,
        std::ios_base::openmode mode) : std::ofstream(OSGDB_CONVERT_UTF8_FILENAME(filename), mode)
    {}
    ofstream::~ofstream(){}
    void ofstream::open(const char* filename,
        std::ios_base::openmode mode)
    {
        std::ofstream::open(OSGDB_CONVERT_UTF8_FILENAME(filename), mode);
    }

}
