// (C) Robert Osfield, Feb 2004.
// GPL'd.

#include <osg/ImageStream>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>
#include <osg/Timer>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <xine.h>
#include <xine/xineutils.h>
#include <xine/video_out.h>

#include "video_out_rgb.h"

static int ready = 0;

static void my_render_frame(uint32_t width, uint32_t height, void* data, void* userData)
{
    osg::Image* imageStream = (osg::Image*) userData;
    
    GLenum pixelFormat = GL_BGRA;

#if 0    
    if (!ready)
    {
        imageStream->allocateImage(width,height,1,pixelFormat,GL_UNSIGNED_BYTE,1);
        imageStream->setInternalTextureFormat(GL_RGBA);
    }

    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    
    memcpy(imageStream->data(),data,imageStream->getTotalSizeInBytes());
    
    osg::notify(osg::NOTICE)<<"image memcpy size="<<imageStream->getTotalSizeInBytes()<<" time="<<osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick())<<"ms"<<std::endl;


    imageStream->dirty();
#else
    imageStream->setImage(width,height,1,
                      GL_RGB,
                      pixelFormat,GL_UNSIGNED_BYTE,
                      (unsigned char *)data,
                      osg::Image::NO_DELETE,
                      1);
#endif
    ready = 1;
}

class ReaderWriterXine : public osgDB::ReaderWriter
{
    public:

        ReaderWriterXine()
        {
            _xine = xine_new();

            const char* user_home = xine_get_homedir();
            if(user_home)
            {
                char* cfgfile = NULL;
	        asprintf(&(cfgfile), "%s/.xine/config", user_home);
	        xine_config_load(_xine, cfgfile);
            }

            xine_init(_xine);
            
           register_plugin(_xine, "/usr/local/lib/osgPlugins", "osgdb_xine.so");      

        }
     
        virtual ~ReaderWriterXine()
        {
            if (_xine) xine_exit(_xine);
            _xine = NULL;
        }
        
        virtual const char* className() const { return "Xine ImageStream Reader"; }

        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"mpg") ||
                   osgDB::equalCaseInsensitive(extension,"mov");
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            //std::string ext = osgDB::getLowerCaseFileExtension(file);
            //if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            //std::string fileName = osgDB::findDataFile( file, options );
            //if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osg::notify(osg::NOTICE)<<"ReaderWriterXine::readImage "<< file<< std::endl;

            //XineImageStream* imageStream = new XineImageStream(file.c_str());
            osg::Image* imageStream = new osg::ImageStream;

            // create visual
            rgbout_visual_info_t* visual = new rgbout_visual_info_t;
	    visual->levels = PXLEVEL_ALL;
            visual->format = PX_RGB32;
            visual->user_data = imageStream;
	    visual->callback = my_render_frame;


            

            // set up video driver
            xine_video_port_t* vo = xine_open_video_driver(_xine, "rgb", XINE_VISUAL_TYPE_RGBOUT, (void*)visual);

            // set up audio driver
            char* audio_driver = getenv("OSG_XINE_AUDIO_DRIVER");
            xine_audio_port_t* ao = audio_driver ? xine_open_audio_driver(_xine, audio_driver, NULL) : xine_open_audio_driver(_xine, "none", NULL);

            if (!vo)
            {
                osg::notify(osg::NOTICE)<<"Failed to create video driver"<<std::endl;
                return 0;
            }
            

            // set up stream
            xine_stream_t* stream = xine_stream_new(_xine, ao, vo);

            // set up queue
            // xine_event_queue_t* queue = xine_event_new_queue(stream);

            int result = xine_open(stream, file.c_str());
            osg::notify(osg::NOTICE)<<"ReaderWriterXine::readImage - xine_open"<<result<<std::endl;
             
            xine_play(stream, 0, 0);

            // imageStream->play();

            while (!ready)
            {
                osg::notify(osg::NOTICE)<<"waiting..."<<std::endl;
                usleep(10000);
            }

            return imageStream;
        }

    protected:
        xine_t*             _xine;


};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterXine> g_readerWriter_Xine_Proxy;
