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

#include "FFmpegImageStream.hpp"

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>


/** Implementation heavily inspired by http://www.dranger.com/ffmpeg/ */

class ReaderWriterFFmpeg : public osgDB::ReaderWriter
{
public:

    ReaderWriterFFmpeg()
    {
        supportsProtocol("http","Read video/audio from http using ffmpeg.");
        supportsProtocol("rtsp","Read video/audio from rtsp using ffmpeg.");

        supportsExtension("ffmpeg", "");
        supportsExtension("avi",    "");
        supportsExtension("flv",    "Flash video");
        supportsExtension("mov",    "Quicktime");
        supportsExtension("ogg",    "Theora movie format");
        supportsExtension("mpg",    "Mpeg movie format");
        supportsExtension("mpv",    "Mpeg movie format");
        supportsExtension("wmv",    "Windows Media Video format");
        supportsExtension("mkv",    "Matroska");
        supportsExtension("mjpeg",  "Motion JPEG");
        supportsExtension("mp4",    "MPEG-4");
        supportsExtension("sav",    "MPEG-4");
        supportsExtension("3gp",    "MPEG-4");
        supportsExtension("sdp",    "MPEG-4");

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
        const std::string ext = osgDB::getLowerCaseFileExtension(filename);
        if (ext=="ffmpeg") return readImage(osgDB::getNameLessExtension(filename),options);

        if (filename.compare(0, 5, "/dev/")==0)
        {
            return readImageStream(filename, options);
        }
    
        if (! acceptsExtension(ext))
            return ReadResult::FILE_NOT_HANDLED;

        const std::string path = osgDB::containsServerAddress(filename) ?
            filename :
            osgDB::findDataFile(filename, options);

        if (path.empty())
            return ReadResult::FILE_NOT_FOUND;

        return readImageStream(path, options);
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
