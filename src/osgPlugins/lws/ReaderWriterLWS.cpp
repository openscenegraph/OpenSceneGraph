/*******************************************************
      Lightwave Scene Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <string>
#include <sstream>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "SceneLoader.h"


class ReaderWriterLWS : public osgDB::ReaderWriter
{
public:
    ReaderWriterLWS() {}

    virtual const char* className() { return "ReaderWriterLWS"; }

    virtual bool acceptsExtension(const std::string &extension) {
        return osgDB::equalCaseInsensitive(extension, "lws");
    }

    virtual ReadResult readNode(const std::string &file, const osgDB::ReaderWriter::Options *options)
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);        
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile(file);
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        lwosg::SceneLoader::Options conv_options = parse_options(options);

        lwosg::SceneLoader scene_loader(conv_options);
        osg::ref_ptr<osg::Node> node = scene_loader.load(fileName);
        if (node.valid()) {
            return node.take();
        }

        return ReadResult::FILE_NOT_HANDLED;
    }

    lwosg::SceneLoader::Options parse_options(const Options *options) const;

protected:

    

};

lwosg::SceneLoader::Options ReaderWriterLWS::parse_options(const Options *options) const
{
    lwosg::SceneLoader::Options conv_options;

    if (options) {
        std::istringstream iss(options->getOptionString());
        std::string opt;
        while (iss >> opt) {
            // no options yet!
        }
    }

    return conv_options;
}


// register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterLWS> g_lwsReaderWriterProxy;
