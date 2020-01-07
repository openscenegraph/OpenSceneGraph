/* -*-c++-*- OpenSceneGraph - Copyright (C) 2009 Tharsis Software
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
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
*/


#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "DirectShowTexture"


class ReaderWriterDirectShow : public osgDB::ReaderWriter
{
public:

    ReaderWriterDirectShow()
    {
        supportsExtension("directshow", "");
        supportsExtension("avi",    "");
        supportsExtension("wmv",    "Windows Media Video format");
        supportsExtension("mpg",    "Mpeg movie format");
        supportsExtension("mpeg",   "Mpeg movie format");
    }

    virtual ~ReaderWriterDirectShow()
    {
    }

    virtual const char * className() const
    {
        return "ReaderWriterDirectShow";
    }
    
    virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        return readImage(file, options);
    }

    virtual ReadResult readImage(const std::string & filename, const osgDB::ReaderWriter::Options * options) const
    {
        const std::string ext = osgDB::getLowerCaseFileExtension(filename);
        if (ext=="directshow") return readImageStream(osgDB::getNameLessExtension(filename),options);
        if (! acceptsExtension(ext))
            return ReadResult::FILE_NOT_HANDLED;
        return readImageStream(filename, options);
    }

    ReadResult readImageStream(const std::string& filename, const osgDB::ReaderWriter::Options * options) const
    {
        OSG_INFO << "ReaderWriterDirectShow::readImage " << filename << std::endl;
        const std::string path = osgDB::containsServerAddress(filename) ?
            filename :
            osgDB::findDataFile(filename, options);

        osg::ref_ptr<DirectShowImageStream> image_stream(new DirectShowImageStream);

        if (path.empty()) // try with capture
        {
            std::map<std::string,std::string> map;
            if (options)
            {
                map["captureWantedWidth"] = options->getPluginStringData("captureWantedWidth");
                map["captureWantedHeight"] = options->getPluginStringData("captureWantedHeight");
                map["captureWantedFps"] = options->getPluginStringData("captureWantedFps");
                map["captureVideoDevice"] = options->getPluginStringData("captureVideoDevice");
                map["captureSoundDevice"] = options->getPluginStringData("captureSoundDevice");
                map["captureSoundDeviceNbChannels"] = options->getPluginStringData("captureSoundDeviceNbChannels");
            }
            if (filename != "capture")
            {
                if (!options || (options && options->getPluginStringData("captureVideoDevice").empty()))
                     map["captureVideoDevice"] = filename;
            }
            image_stream->setOptions(map);

            if (! image_stream->openCaptureDevices())
                return ReadResult::FILE_NOT_HANDLED;
            return image_stream.release();
        }

        if (! image_stream->openFile(filename))
            return ReadResult::FILE_NOT_HANDLED;

        return image_stream.release();
    }

private:

};



REGISTER_OSGPLUGIN(directshow, ReaderWriterDirectShow)
