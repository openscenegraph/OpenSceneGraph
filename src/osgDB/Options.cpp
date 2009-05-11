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

#include <osgDB/Options>
#include <osgDB/Registry>

using namespace osgDB;

Options::Options(const Options& options,const osg::CopyOp& copyop):
    osg::Object(options,copyop),
    _str(options._str),
    _databasePaths(options._databasePaths),
    _objectCacheHint(options._objectCacheHint),
    _buildKdTreesHint(options._buildKdTreesHint),
    _pluginData(options._pluginData),
    _pluginStringData(options._pluginStringData),
    _findFileCallback(options._findFileCallback),
    _readFileCallback(options._readFileCallback),
    _writeFileCallback(options._writeFileCallback),
    _fileLocationCallback(options._fileLocationCallback),
    _fileCache(options._fileCache) {}
