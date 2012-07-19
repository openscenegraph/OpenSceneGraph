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

#include <osgDB/Callbacks>
#include <osgDB/Registry>

using namespace osgDB;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FindFileCallback default implementation
//
std::string FindFileCallback::findDataFile(const std::string& filename, const Options* options, CaseSensitivity caseSensitivity)
{
    return osgDB::Registry::instance()->findDataFileImplementation(filename, options, caseSensitivity);
}

std::string FindFileCallback::findLibraryFile(const std::string& filename, const Options* options, CaseSensitivity caseSensitivity)
{
    return osgDB::Registry::instance()->findLibraryFileImplementation(filename, options, caseSensitivity);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ReadFileCallback default implementation
//
ReaderWriter::ReadResult ReadFileCallback::openArchive(const std::string& filename,ReaderWriter::ArchiveStatus status, unsigned int indexBlockSizeHint, const Options* useObjectCache)
{
    return osgDB::Registry::instance()->openArchiveImplementation(filename, status, indexBlockSizeHint, useObjectCache);
}

ReaderWriter::ReadResult ReadFileCallback::readObject(const std::string& filename, const Options* options)
{
    return osgDB::Registry::instance()->readObjectImplementation(filename,options);
}

ReaderWriter::ReadResult ReadFileCallback::readImage(const std::string& filename, const Options* options)
{
    return osgDB::Registry::instance()->readImageImplementation(filename,options);
}

ReaderWriter::ReadResult ReadFileCallback::readHeightField(const std::string& filename, const Options* options)
{
    return osgDB::Registry::instance()->readHeightFieldImplementation(filename,options);
}

ReaderWriter::ReadResult ReadFileCallback::readNode(const std::string& filename, const Options* options)
{
    return osgDB::Registry::instance()->readNodeImplementation(filename,options);
}

ReaderWriter::ReadResult ReadFileCallback::readShader(const std::string& filename, const Options* options)
{
    return osgDB::Registry::instance()->readShaderImplementation(filename,options);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WriteFileCallback default implementation
//
ReaderWriter::WriteResult WriteFileCallback::writeObject(const osg::Object& obj, const std::string& fileName,const Options* options)
{
    return osgDB::Registry::instance()->writeObjectImplementation(obj,fileName,options);
}

ReaderWriter::WriteResult WriteFileCallback::writeImage(const osg::Image& obj, const std::string& fileName,const Options* options)
{
    return osgDB::Registry::instance()->writeImageImplementation(obj,fileName,options);
}

ReaderWriter::WriteResult WriteFileCallback::writeHeightField(const osg::HeightField& obj, const std::string& fileName,const Options* options)
{
    return osgDB::Registry::instance()->writeHeightFieldImplementation(obj,fileName,options);
}

ReaderWriter::WriteResult WriteFileCallback::writeNode(const osg::Node& obj, const std::string& fileName,const Options* options)
{
    return osgDB::Registry::instance()->writeNodeImplementation(obj,fileName,options);
}

ReaderWriter::WriteResult WriteFileCallback::writeShader(const osg::Shader& obj, const std::string& fileName,const Options* options)
{
    return osgDB::Registry::instance()->writeShaderImplementation(obj,fileName,options);
}
