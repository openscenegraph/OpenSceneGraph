/* -*-c++-*- OpenSceneGraph - Copyright (C) Cedric Pinson
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

#include <osg/Notify>
#include <osg/Geode>
#include <osg/Version>
#include <osg/Endian>

#include <osgDB/ReaderWriter>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <string>
#include "GLTFReader"

using namespace osg;

class ReaderWriterGLTF : public osgDB::ReaderWriter
{
public:

    struct OptionsStruct {
        std::string filepath;
        std::string materialJSON;
    };

    ReaderWriterGLTF()
    {
        supportsExtension("gltf","ASCII glTF  format");
        supportsExtension("glb","binary gltf format");
    }

    virtual const char* className() const { return "ReaderWriterGLTF"; }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        std::string filepath(fileName);

        std::string ext = osgDB::getLowerCaseFileExtension(filepath);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        filepath = osgDB::findDataFile(filepath, options);
        if (filepath.empty()) return ReadResult::FILE_NOT_FOUND;

        GLTFParser parser(filepath);
        parser.readFile();
        if (!parser.get())
            return ReadResult::ERROR_IN_READING_FILE;

        return parser.get();
    }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(glb, ReaderWriterGLTF)