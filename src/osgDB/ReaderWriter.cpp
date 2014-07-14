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
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Archive>

using namespace osgDB;

osg::Object* ReaderWriter::ReadResult::getObject() { return _object.get(); }
osg::Image* ReaderWriter::ReadResult::getImage() { return dynamic_cast<osg::Image*>(_object.get()); }
osg::HeightField* ReaderWriter::ReadResult::getHeightField() { return dynamic_cast<osg::HeightField*>(_object.get()); }
osg::Node* ReaderWriter::ReadResult::getNode() { return dynamic_cast<osg::Node*>(_object.get()); }
osgDB::Archive* ReaderWriter::ReadResult::getArchive() { return dynamic_cast<osgDB::Archive*>(_object.get()); }
osg::Shader* ReaderWriter::ReadResult::getShader() { return dynamic_cast<osg::Shader*>(_object.get()); }
osg::Script* ReaderWriter::ReadResult::getScript() { return dynamic_cast<osg::Script*>(_object.get()); }

osg::Object* ReaderWriter::ReadResult::takeObject() { osg::Object* obj = _object.get(); if (obj) { obj->ref(); _object=NULL; obj->unref_nodelete(); } return obj; }
osg::Image* ReaderWriter::ReadResult::takeImage() { osg::Image* image=dynamic_cast<osg::Image*>(_object.get()); if (image) { image->ref(); _object=NULL; image->unref_nodelete(); } return image; }
osg::HeightField* ReaderWriter::ReadResult::takeHeightField() { osg::HeightField* hf=dynamic_cast<osg::HeightField*>(_object.get()); if (hf) { hf->ref(); _object=NULL; hf->unref_nodelete(); } return hf; }
osg::Node* ReaderWriter::ReadResult::takeNode() { osg::Node* node=dynamic_cast<osg::Node*>(_object.get()); if (node) { node->ref(); _object=NULL; node->unref_nodelete(); } return node; }
osgDB::Archive* ReaderWriter::ReadResult::takeArchive() { osgDB::Archive* archive=dynamic_cast<osgDB::Archive*>(_object.get()); if (archive) { archive->ref(); _object=NULL; archive->unref_nodelete(); } return archive; }
osg::Shader* ReaderWriter::ReadResult::takeShader() { osg::Shader* shader=dynamic_cast<osg::Shader*>(_object.get()); if (shader) { shader->ref(); _object=NULL; shader->unref_nodelete(); } return shader; }
osg::Script* ReaderWriter::ReadResult::takeScript() { osg::Script* script=dynamic_cast<osg::Script*>(_object.get()); if (script) { script->ref(); _object=NULL; script->unref_nodelete(); } return script; }

ReaderWriter::~ReaderWriter()
{
}

bool ReaderWriter::acceptsExtension(const std::string& extension) const
{
    // check for an exact match
    std::string lowercase_ext = convertToLowerCase(extension);
    return (_supportedExtensions.count(lowercase_ext)!=0);
}

bool ReaderWriter::acceptsProtocol(const std::string& protocol) const
{
    std::string lowercase_protocol = convertToLowerCase(protocol);
    return (_supportedProtocols.count(lowercase_protocol)!=0);
}

void ReaderWriter::supportsProtocol(const std::string& fmt, const std::string& description)
{
    Registry::instance()->registerProtocol(fmt);
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

ReaderWriter::Features ReaderWriter::supportedFeatures() const
{
    int features = FEATURE_NONE;
    std::string dummyFilename;

    if (readObject(dummyFilename,0).status()!=ReadResult::NOT_IMPLEMENTED) features |= FEATURE_READ_OBJECT;
    if (readImage(dummyFilename,0).status()!=ReadResult::NOT_IMPLEMENTED) features |= FEATURE_READ_IMAGE;
    if (readHeightField(dummyFilename,0).status()!=ReadResult::NOT_IMPLEMENTED) features |= FEATURE_READ_HEIGHT_FIELD;
    if (readShader(dummyFilename,0).status()!=ReadResult::NOT_IMPLEMENTED) features |= FEATURE_READ_SHADER;
    if (readNode(dummyFilename,0).status()!=ReadResult::NOT_IMPLEMENTED) features |= FEATURE_READ_NODE;

    osg::ref_ptr<osg::Image> image = new osg::Image;
    osg::ref_ptr<osg::HeightField> hf = new osg::HeightField;
    osg::ref_ptr<osg::Shader> shader = new osg::Shader;
    osg::ref_ptr<osg::Node> node = new osg::Node;

    if (writeObject(*image, dummyFilename,0).status()!=WriteResult::NOT_IMPLEMENTED) features |= FEATURE_WRITE_OBJECT;
    if (writeImage(*image,dummyFilename,0).status()!=WriteResult::NOT_IMPLEMENTED) features |= FEATURE_WRITE_IMAGE;
    if (writeHeightField(*hf,dummyFilename,0).status()!=WriteResult::NOT_IMPLEMENTED) features |= FEATURE_WRITE_HEIGHT_FIELD;
    if (writeShader(*shader,dummyFilename,0).status()!=WriteResult::NOT_IMPLEMENTED) features |= FEATURE_WRITE_SHADER;
    if (writeNode(*node, dummyFilename,0).status()!=WriteResult::NOT_IMPLEMENTED) features |= FEATURE_WRITE_NODE;

    return Features(features);
}

ReaderWriter::FeatureList ReaderWriter::featureAsString(ReaderWriter::Features feature)
{
    typedef struct {
        ReaderWriter::Features feature;
        const char *s;
    } FeatureStringList;

    FeatureStringList list[] = {
        { FEATURE_READ_OBJECT, "readObject" },
        { FEATURE_READ_IMAGE,  "readImage" },
        { FEATURE_READ_HEIGHT_FIELD, "readHeightField" },
        { FEATURE_READ_NODE, "readNode" },
        { FEATURE_READ_SHADER, "readShader" },
        { FEATURE_WRITE_OBJECT, "writeObject" },
        { FEATURE_WRITE_IMAGE, "writeImage" },
        { FEATURE_WRITE_HEIGHT_FIELD, "writeHeightField" },
        { FEATURE_WRITE_NODE, "writeNode" },
        { FEATURE_WRITE_SHADER, "writeShader" },
        { FEATURE_NONE,0 }
    };

    FeatureList result;

    for(FeatureStringList *p=list; p->feature != 0; p++)
    {
        if ((feature & p->feature) != 0)
            result.push_back(p->s);
    }
    return result;
}

bool ReaderWriter::fileExists(const std::string& filename, const Options* /*options*/) const
{
    return ::osgDB::fileExists(filename);
}
