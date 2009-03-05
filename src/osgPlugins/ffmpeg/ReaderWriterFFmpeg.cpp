
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "FFmpegHeaders.hpp"
#include "FFmpegImageStream.hpp"



/** Implementation heavily inspired by http://www.dranger.com/ffmpeg/ */

class ReaderWriterFFmpeg : public osgDB::ReaderWriter
{
public:

    ReaderWriterFFmpeg()
    {
        supportsExtension("avi", "");
        supportsExtension("flv", "");
        supportsExtension("mov", "");
        supportsExtension("ogg", "Theora movie format");
        supportsExtension("mpg", "Mpeg movie format");
        supportsExtension("mpv", "Mpeg movie format");
        supportsExtension("wmv", "");

        // Register all FFmpeg formats/codecs
        av_register_all();
    }

    virtual ~ReaderWriterFFmpeg()
    {

    }

    virtual const char * className() const
    {
        return "ReaderWriterFFmpeg";
    }

    virtual ReadResult readImage(const std::string & filename, const osgDB::ReaderWriter::Options * options) const
    {
        if (filename.compare(0, 5, "/dev/")==0)
        {
            return readImageStream(filename, options);
        }
    
        const std::string ext = osgDB::getLowerCaseFileExtension(filename);
        if (! acceptsExtension(ext))
            return ReadResult::FILE_NOT_HANDLED;

        const std::string path = osgDB::containsServerAddress(filename) ?
            filename :
            osgDB::findDataFile(filename, options);

        if (path.empty())
            return ReadResult::FILE_NOT_FOUND;

        return readImageStream(filename, options);
    }
    
    ReadResult readImageStream(const std::string& filename, const osgDB::ReaderWriter::Options * options) const
    {
        osg::notify(osg::INFO) << "ReaderWriterFFmpeg::readImage " << filename << std::endl;

        osg::ref_ptr<osgFFmpeg::FFmpegImageStream> image_stream(new osgFFmpeg::FFmpegImageStream);

        if (! image_stream->open(filename))
            return ReadResult::FILE_NOT_HANDLED;

        return image_stream.release();
    }

private:

};



REGISTER_OSGPLUGIN(ffmpeg, ReaderWriterFFmpeg)
