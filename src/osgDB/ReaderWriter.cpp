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

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Archive>

using namespace osgDB;

osg::Object* ReaderWriter::ReadResult::getObject() { return _object.get(); }
osg::Image* ReaderWriter::ReadResult::getImage() { return dynamic_cast<osg::Image*>(_object.get()); }
osg::HeightField* ReaderWriter::ReadResult::getHeightField() { return dynamic_cast<osg::HeightField*>(_object.get()); }
osg::Node* ReaderWriter::ReadResult::getNode() { return dynamic_cast<osg::Node*>(_object.get()); }
osgDB::Archive* ReaderWriter::ReadResult::getArchive() { return dynamic_cast<osgDB::Archive*>(_object.get()); }
osg::Shader* ReaderWriter::ReadResult::getShader() { return dynamic_cast<osg::Shader*>(_object.get()); }

osg::Object* ReaderWriter::ReadResult::takeObject() { osg::Object* obj = _object.get(); if (obj) { obj->ref(); _object=NULL; obj->unref_nodelete(); } return obj; }
osg::Image* ReaderWriter::ReadResult::takeImage() { osg::Image* image=dynamic_cast<osg::Image*>(_object.get()); if (image) { image->ref(); _object=NULL; image->unref_nodelete(); } return image; }
osg::HeightField* ReaderWriter::ReadResult::takeHeightField() { osg::HeightField* hf=dynamic_cast<osg::HeightField*>(_object.get()); if (hf) { hf->ref(); _object=NULL; hf->unref_nodelete(); } return hf; }
osg::Node* ReaderWriter::ReadResult::takeNode() { osg::Node* node=dynamic_cast<osg::Node*>(_object.get()); if (node) { node->ref(); _object=NULL; node->unref_nodelete(); } return node; }
osgDB::Archive* ReaderWriter::ReadResult::takeArchive() { osgDB::Archive* archive=dynamic_cast<osgDB::Archive*>(_object.get()); if (archive) { archive->ref(); _object=NULL; archive->unref_nodelete(); } return archive; }
osg::Shader* ReaderWriter::ReadResult::takeShader() { osg::Shader* shader=dynamic_cast<osg::Shader*>(_object.get()); if (shader) { shader->ref(); _object=NULL; shader->unref_nodelete(); } return shader; }

ReaderWriter::~ReaderWriter()
{
}

bool ReaderWriter::acceptsExtension(const std::string& extension) const
{
    // check for an exact match
    std::string lowercase_ext = convertToLowerCase(extension);
    if (_supportedExtensions.count(lowercase_ext)!=0) return true;
    
    // if plugin supports wildcard extension then passthrough all types
    return (_supportedExtensions.count("*")!=0);
}

void ReaderWriter::supportsProtocol(const std::string& fmt, const std::string& description)
{
    _supportedProtocols[convertToLowerCase(fmt)] = description;
}

void ReaderWriter::supportsExtension(const std::string& fmt, const std::string& description)
{
    _supportedExtensions[convertToLowerCase(fmt)] = description;
}

void ReaderWriter::supportsOption(const std::string& fmt, const std::string& description)
{
    _supportedOptions[fmt] = description;
}
