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

namespace osgXine
{

class XineImageStream : public osg::ImageStream
{
    public:
        XineImageStream() {}

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        XineImageStream(const XineImageStream& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            ImageStream(image,copyop) {}

        META_Object(osgXine,XineImageStream);
        
        bool open(xine_t* xine, const std::string& filename)
        {
            if (filename==getFileName()) return true;
            
            _xine = xine;
        
            // create visual
            rgbout_visual_info_t* visual = new rgbout_visual_info_t;
	    visual->levels = PXLEVEL_ALL;
            visual->format = PX_RGB32;
            visual->user_data = this;
	    visual->callback = my_render_frame;

            // set up video driver
            _vo = xine_open_video_driver(_xine, "rgb", XINE_VISUAL_TYPE_RGBOUT, (void*)visual);

            // set up audio driver
            char* audio_driver = getenv("OSG_XINE_AUDIO_DRIVER");
            _ao = audio_driver ? xine_open_audio_driver(_xine, audio_driver, NULL) : xine_open_audio_driver(_xine, "none", NULL);

            if (!_vo)
            {
                osg::notify(osg::NOTICE)<<"Failed to create video driver"<<std::endl;
                return false;
            }
            

            // set up stream
            _stream = xine_stream_new(_xine, _ao, _vo);

            // set up queue
            // xine_event_queue_t* queue = xine_event_new_queue(stream);

            int result = xine_open(_stream, filename.c_str());
            osg::notify(osg::NOTICE)<<"XineImageStream::open - xine_open"<<result<<std::endl;
             
            _ready = false;

            xine_play(_stream, 0, 0);

            // imageStream->play();

            while (!_ready)
            {
                osg::notify(osg::NOTICE)<<"waiting..."<<std::endl;
                usleep(10000);
            }
        }

        virtual void play() { _status=PLAYING; }

        virtual void pause() { _status=PAUSED; }

        virtual void rewind() { _status=REWINDING; }

        virtual void quit(bool /*waitForThreadToExit*/ = true) {}

        static void my_render_frame(uint32_t width, uint32_t height, void* data, void* userData)
        {
            XineImageStream* imageStream = (XineImageStream*) userData;

            GLenum pixelFormat = GL_BGRA;

        #if 0    
            if (!imageStream->_ready)
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
            imageStream->_ready = true;
        }


        xine_t*                 _xine;

        xine_video_port_t*      _vo;
        xine_audio_port_t*      _ao;

        rgbout_visual_info_t*   _visual;
        xine_stream_t*          _stream;
        
        bool                    _ready;

    protected:


        virtual ~XineImageStream()
        {
            close();
        }

        void close()
        {
            if (_stream)
            {
                  xine_close(_stream);
                  xine_dispose(_stream);
                  _stream = 0;
            }


            if (_ao)
            {
                xine_close_audio_driver(_xine, _ao);  
            }
            
            if (_vo)
            {
                xine_close_video_driver(_xine, _vo);  
            }

        }

};

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
            
            register_rgbout_plugin(_xine);

            // register_plugin(_xine, "/usr/local/lib/osgPlugins", "osgdb_xine.so");      

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

            osg::ref_ptr<osgXine::XineImageStream> imageStream = new osgXine::XineImageStream();

            if (!imageStream->open(_xine, file)) return ReadResult::FILE_NOT_HANDLED;

            return imageStream.release();
        }

    protected:
        xine_t*             _xine;


};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterXine> g_readerWriter_Xine_Proxy;
