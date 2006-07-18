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

#include <osg/Notify>
#include <osg/Endian>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/Archive>

#include <streambuf>

using namespace osgDB;

osgDB::Archive* osgDB::openArchive(const std::string& filename, Archive::ArchiveStatus status, unsigned int indexBlockSizeHint)
{
    return openArchive(filename, status, indexBlockSizeHint, Registry::instance()->getOptions());
}

osgDB::Archive* osgDB::openArchive(const std::string& filename, Archive::ArchiveStatus status, unsigned int indexBlockSizeHint,ReaderWriter::Options* options)
{
    ReaderWriter::ReadResult result = osgDB::Registry::instance()->openArchive(filename, status, indexBlockSizeHint, options);
    return result.takeArchive();
}

Archive::Archive()
{
    osg::notify(osg::INFO)<<"Archive::Archive() open"<<std::endl;
}

Archive::~Archive()
{
    osg::notify(osg::INFO)<<"Archive::~Archive() closed"<<std::endl;
}

