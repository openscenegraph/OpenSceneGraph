#include <osg/ImageStream>
#include <osg/Notify>
#include <osg/Geode>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "OSXAVFoundationVideo.h"
#include "OSXAVFoundationCoreVideoTexture.h"

class ReaderWriterAVFoundation : public osgDB::ReaderWriter
{
    public:

        ReaderWriterAVFoundation()
        {
            supportsExtension("mov","Quicktime movie format");
            supportsExtension("mpg","Mpeg movie format");
            supportsExtension("mp4","Mpeg movie format");
            supportsExtension("m4v","Mpeg movie format");
            supportsExtension("mpeg","Mpeg movie format");
            supportsExtension("avfoundation","AVFoundation movie format");

            supportsProtocol("http", "streaming media per http");
            supportsProtocol("rtsp", "streaming media per rtsp");     
        }
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return
                osgDB::equalCaseInsensitive(extension,"mov") ||
                osgDB::equalCaseInsensitive(extension,"mpg") ||
                osgDB::equalCaseInsensitive(extension,"mp4") ||
                osgDB::equalCaseInsensitive(extension,"mpv") ||
                osgDB::equalCaseInsensitive(extension,"mpeg")||
                osgDB::equalCaseInsensitive(extension,"avfoundation");
        }
    
        virtual ~ReaderWriterAVFoundation()
        {
            OSG_INFO<<"~ReaderWriterAVFoundation()"<<std::endl;
        }
        
        virtual const char* className() const { return "AVFoundation ImageStream Reader"; }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
            
            std::string fileName(file);
            if (ext=="avfoundation")
            {
                fileName = osgDB::getNameLessExtension(fileName);
                OSG_INFO<<"AVFoundation stipped filename = "<<fileName<<std::endl;
            }
            if (!osgDB::containsServerAddress(fileName))
            {
                fileName = osgDB::findDataFile( fileName, options );
                if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;
            }
            
            static OpenThreads::Mutex mutex;
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);
            
            OSG_INFO<<"ReaderWriterAVFoundation::readImage "<< fileName<< std::endl;

            osg::ref_ptr<OSXAVFoundationVideo> video = new OSXAVFoundationVideo();
            
            bool disable_multi_threaded_frame_dispatching = options ? (options->getPluginStringData("disableMultiThreadedFrameDispatching") == "true") : false;
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
            
            osg::ref_ptr<OSXAVFoundationVideo> video = dynamic_cast<OSXAVFoundationVideo*>(rr.getImage());
            if (!video || !use_core_video)
                return rr;
            
            osg::ref_ptr<osg::Texture> texture = video->createSuitableTexture();
            if (texture.valid())
                return texture.release();
            
            return video.release();
        }

    protected:


};



// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(avfoundation, ReaderWriterAVFoundation)
