/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield
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


/* README:
 * 
 * This code is loosely based on the QTKit implementation of Eric Wing, I removed
 * some parts and added other parts. 
 * 
 * What's new:
 * - it can handle URLs currently http and rtsp
 * - it supports OS X's CoreVideo-technology, this will render the movie-frames
 *   into a bunch of textures. If you load your movie via readImageFile you'll
 *   get the standard behaviour, an ImageStream, where the data gets updated on 
 *   every new video-frame. This may be slow.
 *   To get CoreVideo, you'll need to use readObjectFile and cast the result (if any)
 *   to an osg::Texture and use that as your video-texture. If you need access to the
 *   imagestream, just cast getImage to an image-stream. Please note, the data-
 *   property of the image-stream does NOT store the current frame, instead it's empty.
 *
 */

#include <osg/ImageStream>
#include <osg/Notify>
#include <osg/Geode>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "OSXQTKitVideo.h"
#include "OSXCoreVideoTexture.h"
#include "VideoFrameDispatcher.h"




class ReaderWriterQTKit : public osgDB::ReaderWriter
{
    public:

        ReaderWriterQTKit()
        {
            supportsExtension("mov","Quicktime movie format");
            supportsExtension("mpg","Mpeg movie format");
            supportsExtension("mp4","Mpeg movie format");
            supportsExtension("m4v","Mpeg movie format");
            supportsExtension("flv","Flash video file (if Perian is installed)");
            supportsExtension("dv","dv movie format");
            supportsExtension("avi","avi movie format (if Perian/WMV is installed)");
            supportsExtension("sdp","sdp movie format");
            supportsExtension("swf","swf movie format (if Perian is installed)");
            supportsExtension("3gp","3gp movie format");
            
            supportsProtocol("http", "streaming media per http");
            supportsProtocol("rtsp", "streaming media per rtsp");
            
            supportsOption("disableCoreVideo", "disable the usage of coreVideo when using readObjectFile, returns an ImageStream instead");
            supportsOption("disableMultiThreadedFrameDispatching", "disable the usage of the multithreade VideoFrameDispatcher to decode video frames");
            
        }
        
    
        virtual ~ReaderWriterQTKit()
        {
            OSG_INFO<<"~ReaderWriterQTKit()"<<std::endl;
        }
        
        virtual const char* className() const { return "QTKit ImageStream Reader"; }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
            
            std::string fileName(file);
            if (ext=="QTKit")
            {
                fileName = osgDB::getNameLessExtension(fileName);
                OSG_INFO<<"ReaderWriterQTKit stipped filename = "<<fileName<<std::endl;
            }
            if (!osgDB::containsServerAddress(fileName))
            {
                fileName = osgDB::findDataFile( fileName, options );
                if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;
            }
            
            static OpenThreads::Mutex mutex;
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);

            OSG_INFO<<"ReaderWriterQTKit::readImage "<< fileName<< std::endl;

            osg::ref_ptr<OSXQTKitVideo> video = new OSXQTKitVideo();
            bool disable_multi_threaded_frame_dispatching = options ? (options->getPluginStringData("disableMultiThreadedFrameDispatching") == "true"): false;
            bool disable_core_video                       = options ? (options->getPluginStringData("disableCoreVideo") == "true") : false;
            OSG_INFO << "disableMultiThreadedFrameDispatching: " << disable_multi_threaded_frame_dispatching << std::endl;
            OSG_INFO << "disableCoreVideo                    : " << disable_core_video << std::endl;
            
            if (!options
                || (!disable_multi_threaded_frame_dispatching
                && disable_core_video))
            {
                static osg::ref_ptr<osgVideo::VideoFrameDispatcher> video_frame_dispatcher(NULL);
                if (!video_frame_dispatcher) {
                    std::string num_threads_str = options ? options->getPluginStringData("numFrameDispatchThreads") : "0";
                    video_frame_dispatcher = new osgVideo::VideoFrameDispatcher(atoi(num_threads_str.c_str()));
                }
                video_frame_dispatcher->addVideo(video);
            }
            
            video->open(fileName);
            
            return video->valid() ? video.release() : NULL;
        }
    
        virtual ReadResult readObject (const std::string &file, const osgDB::ReaderWriter::Options* options) const
        {
            ReadResult rr = readImage(file, options);
            if (!rr.validImage())
                return rr;
            bool use_core_video = true;
            
            if (options && !options->getPluginStringData("disableCoreVideo").empty())
                use_core_video = false;
            
            osg::ref_ptr<OSXQTKitVideo> video = dynamic_cast<OSXQTKitVideo*>(rr.getImage());
            if (!video || !use_core_video)
                return rr;
            
            osg::ref_ptr<OSXCoreVideoTexture> texture = new OSXCoreVideoTexture(video);
            return texture.release();
        }

    protected:


};



// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(QTKit, ReaderWriterQTKit)
