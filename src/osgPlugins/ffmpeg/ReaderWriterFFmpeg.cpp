/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
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
#include "FFmpegParameters.hpp"

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>


#if LIBAVCODEC_VERSION_MAJOR >= 53 || \
    (LIBAVCODEC_VERSION_MAJOR==52 && LIBAVCODEC_VERSION_MINOR>=30) || \
    (LIBAVCODEC_VERSION_MAJOR==52 && LIBAVCODEC_VERSION_MINOR==20 && LIBAVCODEC_VERSION_MICRO >= 1)
    #define USE_AV_LOCK_MANAGER
#endif

extern "C" {

static void log_to_osg(void *ptr, int level, const char *fmt, va_list vl)
{
    char logbuf[256];
    vsnprintf(logbuf, sizeof(logbuf), fmt, vl);
    logbuf[sizeof(logbuf) - 1] = '\0';

    osg::NotifySeverity severity = osg::DEBUG_FP;

    switch (level) {
    case AV_LOG_PANIC:
        severity = osg::ALWAYS;
        break;
    case AV_LOG_FATAL:
        severity = osg::FATAL;
        break;
    case AV_LOG_ERROR:
        severity = osg::WARN;
        break;
    case AV_LOG_WARNING:
        severity = osg::NOTICE;
        break;
    case AV_LOG_INFO:
        severity = osg::INFO;
        break;
    case AV_LOG_VERBOSE:
        severity = osg::DEBUG_INFO;
        break;
    default:
    case AV_LOG_DEBUG:
        severity = osg::DEBUG_FP;
        break;
    }

    // Most av_logs have a trailing newline already
    osg::notify(severity) << logbuf;
}

} // extern "C"

/** Implementation heavily inspired by http://www.dranger.com/ffmpeg/ */

class ReaderWriterFFmpeg : public osgDB::ReaderWriter
{
public:

    ReaderWriterFFmpeg()
    {
        supportsProtocol("http","Read video/audio from http using ffmpeg.");
        supportsProtocol("rtsp","Read video/audio from rtsp using ffmpeg.");
        supportsProtocol("rtp","Read video/audio from rtp using ffmpeg.");
        supportsProtocol("tcp","Read video/audio from tcp using ffmpeg.");

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
        supportsExtension("m4v",    "MPEG-4");
        supportsExtension("sav",    "Unknown");
        supportsExtension("3gp",    "3G multi-media format");
        supportsExtension("sdp",    "Session Description Protocol");
        supportsExtension("m2ts",   "MPEG-2 Transport Stream");

        supportsOption("format",            "Force setting input format (e.g. vfwcap for Windows webcam)");
        supportsOption("pixel_format",      "Set pixel format");
        supportsOption("frame_size",              "Set frame size (e.g. 320x240)");
        supportsOption("frame_rate",        "Set frame rate (e.g. 25)");
        supportsOption("audio_sample_rate", "Set audio sampling rate (e.g. 44100)");
        supportsOption("context",            "AVIOContext* for custom IO");

        av_log_set_callback(log_to_osg);

#ifdef USE_AV_LOCK_MANAGER
        // enable thread locking
        av_lockmgr_register(&lockMgr);
#endif
        // Register all FFmpeg formats/codecs
        av_register_all();

        avformat_network_init();
    }

    virtual ~ReaderWriterFFmpeg()
    {
    }

    virtual const char * className() const
    {
        return "ReaderWriterFFmpeg";
    }

    virtual ReadResult readImage(const std::string & filename, const osgDB::ReaderWriter::Options* options) const
    {
        const std::string ext = osgDB::getLowerCaseFileExtension(filename);
        if (ext=="ffmpeg") return readImage(osgDB::getNameLessExtension(filename),options);

        if (filename.compare(0, 5, "/dev/")==0)
        {
            return readImageStream(filename, NULL);
        }

        osg::ref_ptr<osgFFmpeg::FFmpegParameters> parameters(new osgFFmpeg::FFmpegParameters);
        parseOptions(parameters.get(), options);
        if (parameters->isFormatAvailable())
        {
            return readImageStream(filename, parameters.get());
        }

        if (! acceptsExtension(ext))
            return ReadResult::FILE_NOT_HANDLED;

        const std::string path = osgDB::containsServerAddress(filename) ?
            filename :
            osgDB::findDataFile(filename, options);

        if (path.empty())
            return ReadResult::FILE_NOT_FOUND;

        return readImageStream(path, parameters.get());
    }

    ReadResult readImageStream(const std::string& filename, osgFFmpeg::FFmpegParameters* parameters) const
    {
        OSG_INFO << "ReaderWriterFFmpeg::readImage " << filename << std::endl;

        osg::ref_ptr<osgFFmpeg::FFmpegImageStream> image_stream(new osgFFmpeg::FFmpegImageStream);

        if (! image_stream->open(filename, parameters))
            return ReadResult::FILE_NOT_HANDLED;

        return image_stream.release();
    }

private:

    void parseOptions(osgFFmpeg::FFmpegParameters* parameters, const osgDB::ReaderWriter::Options * options) const
    {
        if (options && options->getNumPluginStringData()>0)
        {
            const FormatDescriptionMap& supportedOptList = supportedOptions();
            for (FormatDescriptionMap::const_iterator itr = supportedOptList.begin();
                 itr != supportedOptList.end(); ++itr)
            {
                const std::string& name = itr->first;
                parameters->parse(name, options->getPluginStringData(name));
            }
        }
        if (options && options->getNumPluginData()>0)
        {
            AVIOContext* context = (AVIOContext*)options->getPluginData("context");
            if (context != NULL)
            {
                parameters->setContext(context);
            }
        }
    }

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
