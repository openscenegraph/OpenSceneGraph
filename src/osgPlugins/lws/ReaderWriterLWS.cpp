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
    ReaderWriterLWS()
    {
        supportsExtension("lws","Lightwave scene format");
    }

    virtual const char* className() const { return "ReaderWriterLWS"; }

    virtual bool acceptsExtension(const std::string &extension) const {
        return osgDB::equalCaseInsensitive(extension, "lws");
    }

    virtual ReadResult readNode(const std::string &file, const osgDB::ReaderWriter::Options *options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile(file, options);
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        // code for setting up the database path so that internally referenced file are searched for on relative paths.
        osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
        local_opt->setDatabasePath(osgDB::getFilePath(fileName));

        lwosg::SceneLoader::Options conv_options = parse_options(local_opt.get());

        lwosg::SceneLoader scene_loader(conv_options);
        osg::ref_ptr<osg::Node> node = scene_loader.load(fileName, local_opt.get());
        if (node.valid()) {
            return node.release();
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
REGISTER_OSGPLUGIN(lws, ReaderWriterLWS)
