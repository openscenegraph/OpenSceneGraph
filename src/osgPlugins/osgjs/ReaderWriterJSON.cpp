//    copyright: 'Cedric Pinson cedric@plopbyte.com'
#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>
#include <osg/Version>
#include <osg/Endian>
#include <osg/Projection>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>

#include <osgUtil/UpdateVisitor>
#include <osgDB/ReaderWriter>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgAnimation/UpdateMatrixTransform>
#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/BasicAnimationManager>

#include <vector>

#include "json_stream"
#include "JSON_Objects"
#include "Animation"
#include "CompactBufferVisitor"
#include "WriteVisitor"



using namespace osg;


class ReaderWriterJSON : public osgDB::ReaderWriter
{
public:

     struct OptionsStruct {
         int resizeTextureUpToPowerOf2;
         bool useExternalBinaryArray;
         bool mergeAllBinaryFiles;
         bool disableCompactBuffer;
         bool inlineImages;
         bool varint;
         bool strictJson;
         std::vector<std::string> useSpecificBuffer;
         std::string baseLodURL;
         OptionsStruct() {
             resizeTextureUpToPowerOf2 = 0;
             useExternalBinaryArray = false;
             mergeAllBinaryFiles = false;
             disableCompactBuffer = false;
             inlineImages = false;
             varint = false;
             strictJson = true;
         }
    };


    ReaderWriterJSON()
    {
        supportsExtension("osgjs","OpenSceneGraph Javascript implementation format");
        supportsOption("resizeTextureUpToPowerOf2=<int>","Specify the maximum power of 2 allowed dimension for texture. Using 0 will disable the functionality and no image resizing will occur.");
        supportsOption("useExternalBinaryArray","create binary files for vertex arrays");
        supportsOption("mergeAllBinaryFiles","merge all binary files into one to avoid multi request on a server");
        supportsOption("inlineImages","insert base64 encoded images instead of referring to them");
        supportsOption("varint","Use varint encoding to serialize integer buffers");
        supportsOption("useSpecificBuffer=userkey1[=uservalue1][:buffername1],userkey2[=uservalue2][:buffername2]","uses specific buffers for unshared buffers attached to geometries having a specified user key/value. Buffer name *may* be specified after ':' and will be set to uservalue by default. If no value is set then only the existence of a uservalue with key string is performed.");
        supportsOption("disableCompactBuffer","keep source types and do not try to optimize buffers size");
        supportsOption("disableStrictJson","do not clean string (to utf8) or floating point (should be finite) values");
    }

    virtual const char* className() const { return "OSGJS json Writer"; }

    virtual ReadResult readObject(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
    {
        return readNode(filename, options);
    }

    virtual ReadResult readNode(const std::string& fileName, const Options* options) const;

    virtual WriteResult writeNode(const Node& node,
                                  const std::string& fileName,
                                  const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getFileExtension(fileName);
        if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;


        OptionsStruct _options = parseOptions(options);
        json_stream fout(fileName, _options.strictJson);

        if(fout) {
            WriteResult res = writeNodeModel(node, fout, osgDB::getNameLessExtension(fileName), _options);
            return res;
        }
        return WriteResult("Unable to open file for output");
    }

    virtual WriteResult writeNode(const Node& node,
                                  json_stream& fout,
                                  const osgDB::ReaderWriter::Options* options) const
    {
        if (!fout) {
            return WriteResult("Unable to write to output stream");
        }

        OptionsStruct _options;
        _options = parseOptions(options);
        return writeNodeModel(node, fout, "stream", _options);
    }

    virtual WriteResult writeNodeModel(const Node& node, json_stream& fout, const std::string& basename, const OptionsStruct& options) const
    {
        // process regular model
        osg::ref_ptr<osg::Node> model = osg::clone(&node);

        if(!options.disableCompactBuffer) {
            CompactBufferVisitor compact;
            model->accept(compact);
        }

        WriteVisitor writer;
        try {
            //osgDB::writeNodeFile(*model, "/tmp/debug_osgjs.osg");
            writer.setBaseName(basename);
            writer.useExternalBinaryArray(options.useExternalBinaryArray);
            writer.mergeAllBinaryFiles(options.mergeAllBinaryFiles);
            writer.setInlineImages(options.inlineImages);
            writer.setMaxTextureDimension(options.resizeTextureUpToPowerOf2);
            writer.setVarint(options.varint);
            writer.setBaseLodURL(options.baseLodURL);
            for(std::vector<std::string>::const_iterator specificBuffer = options.useSpecificBuffer.begin() ;
                specificBuffer != options.useSpecificBuffer.end() ; ++ specificBuffer) {
                writer.addSpecificBuffer(*specificBuffer);
            }
            model->accept(writer);
            if (writer._root.valid()) {
                writer.write(fout);
                return WriteResult::FILE_SAVED;
            }
        } catch (...) {
            osg::notify(osg::FATAL) << "can't save osgjs file" << std::endl;
            return WriteResult("Unable to write to output stream");
        }
        return WriteResult("Unable to write to output stream");
    }

    ReaderWriterJSON::OptionsStruct parseOptions(const osgDB::ReaderWriter::Options* options) const
    {
        OptionsStruct localOptions;

        if (options)
        {
            osg::notify(NOTICE) << "options " << options->getOptionString() << std::endl;
            std::istringstream iss(options->getOptionString());
            std::string opt;
            while (iss >> opt)
            {
                // split opt into pre= and post=
                std::string pre_equals;
                std::string post_equals;

                size_t found = opt.find("=");
                if(found!=std::string::npos)
                {
                    pre_equals = opt.substr(0,found);
                    post_equals = opt.substr(found+1);
                }
                else
                {
                    pre_equals = opt;
                }

                if (pre_equals == "useExternalBinaryArray")
                {
                    localOptions.useExternalBinaryArray = true;
                }
                if (pre_equals == "mergeAllBinaryFiles")
                {
                    localOptions.mergeAllBinaryFiles = true;
                }
                if (pre_equals == "disableCompactBuffer")
                {
                    localOptions.disableCompactBuffer = true;
                }
                if (pre_equals == "disableStrictJson")
                {
                    localOptions.strictJson = false;
                }


                if (pre_equals == "inlineImages")
                {
                    localOptions.inlineImages = true;
                }
                if (pre_equals == "varint")
                {
                    localOptions.varint = true;
                }

                if (pre_equals == "resizeTextureUpToPowerOf2" && post_equals.length() > 0)
                {
                    int value = atoi(post_equals.c_str());
                    localOptions.resizeTextureUpToPowerOf2 = osg::Image::computeNearestPowerOfTwo(value);
                }

                if (pre_equals == "useSpecificBuffer" && !post_equals.empty())
                {
                    size_t stop_pos = 0, start_pos = 0;
                    while((stop_pos = post_equals.find(",", start_pos)) != std::string::npos) {
                        localOptions.useSpecificBuffer.push_back(post_equals.substr(start_pos,
                                                                                    stop_pos - start_pos));
                        start_pos = stop_pos + 1;
                        ++ stop_pos;
                    }
                    localOptions.useSpecificBuffer.push_back(post_equals.substr(start_pos,
                                                                                post_equals.length() - start_pos));
                }

            }
            if (!options->getPluginStringData( std::string ("baseLodURL" )).empty())
            {
                localOptions.baseLodURL = options->getPluginStringData( std::string ("baseLodURL" ));
            }
        }
        return localOptions;
    }
};

osgDB::ReaderWriter::ReadResult ReaderWriterJSON::readNode(const std::string& file, const Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    // strip the pseudo-loader extension
    std::string fileName = osgDB::getNameLessExtension( file );

    fileName = osgDB::findDataFile( fileName, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile( fileName, options );
    if (!node)
        return ReadResult::FILE_NOT_HANDLED;

    return ReadResult::FILE_NOT_HANDLED;
}

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(osgjs, ReaderWriterJSON)
