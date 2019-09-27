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

#include "GStreamerImageStream.hpp"

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>


class ReaderWriterGStreamer : public osgDB::ReaderWriter
{
public:

    ReaderWriterGStreamer()
    {
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

        gst_init(NULL, NULL);
    }

    virtual ~ReaderWriterGStreamer()
    {
    }

    virtual const char * className() const
    {
        return "ReaderWriterGStreamer";
    }

    virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        return readImage(file, options);
    }

    virtual ReadResult readImage(const std::string & filename, const osgDB::ReaderWriter::Options* options) const
    {
        const std::string ext = osgDB::getLowerCaseFileExtension(filename);

        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        const std::string path = osgDB::containsServerAddress(filename) ?
            filename :
            osgDB::findDataFile(filename, options);

        if (path.empty()) return ReadResult::FILE_NOT_FOUND;

        osg::ref_ptr<osgGStreamer::GStreamerImageStream> imageStream = new osgGStreamer::GStreamerImageStream();

        if (!imageStream->open(filename)) return ReadResult::FILE_NOT_HANDLED;

        return imageStream.release();
    }
};



REGISTER_OSGPLUGIN(gstreamer, ReaderWriterGStreamer)
