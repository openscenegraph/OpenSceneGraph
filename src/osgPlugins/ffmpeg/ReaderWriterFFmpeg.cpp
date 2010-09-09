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

#include "FFmpegHeaders.hpp"
#include "FFmpegImageStream.hpp"

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#if LIBAVCODEC_VERSION_MAJOR >= 53 || (LIBAVCODEC_VERSION_MAJOR==52 && LIBAVCODEC_VERSION_MINOR>=30)
    #define USE_AV_LOCK_MANAGER
#endif



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

#ifdef USE_AV_LOCK_MANAGER
        // enable thread locking
        av_lockmgr_register(&lockMgr);
#endif
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
        OSG_INFO << "ReaderWriterFFmpeg::readImage " << filename << std::endl;

        osg::ref_ptr<osgFFmpeg::FFmpegImageStream> image_stream(new osgFFmpeg::FFmpegImageStream);

        if (! image_stream->open(filename))
            return ReadResult::FILE_NOT_HANDLED;

        return image_stream.release();
    }

private:

#ifdef USE_AV_LOCK_MANAGER
    static int lockMgr(void **mutex, enum AVLockOp op)
    {
        // returns are 0 success
        OpenThreads::Mutex **m=(OpenThreads::Mutex**)mutex;
        if (op==AV_LOCK_CREATE)
        {
            *m=new OpenThreads::Mutex;
            return !*m;
        }
        else if (op==AV_LOCK_DESTROY)
        {
            delete *m;
            return 0;
        }
        else if (op==AV_LOCK_OBTAIN)
        {
            (*m)->lock();
            return 0;
        }
        else if (op==AV_LOCK_RELEASE)
        {
            (*m)->unlock();
            return 0;
        }
        else
        {
            return -1;
        }
    }
#endif

};



REGISTER_OSGPLUGIN(ffmpeg, ReaderWriterFFmpeg)
