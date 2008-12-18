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
        XineImageStream():
            _xine(0),
            _vo(0),
            _ao(0),
            _visual(0),
            _stream(0),
            _event_queue(0),
            _ready(false),
            _volume(-1.0)
        {
            setOrigin(osg::Image::TOP_LEFT);
        }

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        XineImageStream(const XineImageStream& image,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
            ImageStream(image,copyop) {}

        META_Object(osgXine,XineImageStream);
        
        void setVolume(float volume)
        {
            _volume = osg::minimum(osg::maximum(volume,0.0f),1.0f);
            if (_stream) 
            {
                xine_set_param(_stream, XINE_PARAM_AUDIO_VOLUME, static_cast<int>(_volume*100.0f));
                osg::notify(osg::NOTICE)<<"Setting volume "<<_volume<<std::endl;
            }
        }
        
        float getVolume() const
        {
            return _volume;
        }

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
            _ao = audio_driver ? xine_open_audio_driver(_xine, audio_driver, NULL) : xine_open_audio_driver(_xine, "auto", NULL);

            if (!_vo)
            {
                osg::notify(osg::NOTICE)<<"XineImageStream::open() : Failed to create video driver"<<std::endl;
                return false;
            }
            

            // set up stream
            _stream = xine_stream_new(_xine, _ao, _vo);
            
            if (_stream)
            {
                if (_volume < 0.0)
                {
                    _volume = static_cast<float>(xine_get_param(_stream, XINE_PARAM_AUDIO_VOLUME))/100.0f;
                }
                else
                {
                    setVolume(_volume);
                }
            }

            _event_queue = xine_event_new_queue(_stream);
            xine_event_create_listener_thread(_event_queue, event_listener, this);

            int result = xine_open(_stream, filename.c_str());
            
            if (result==0)
            {
                osg::notify(osg::INFO)<<"XineImageStream::open() : Could not ready movie file."<<std::endl;
                close();
                return false;
            }
            
            
            _ready = false;
            
            int width = xine_get_stream_info(_stream,XINE_STREAM_INFO_VIDEO_WIDTH);
            int height = xine_get_stream_info(_stream,XINE_STREAM_INFO_VIDEO_HEIGHT);
            allocateImage(width,height,1,GL_RGB,GL_UNSIGNED_BYTE,1);

            osg::notify(osg::INFO)<<"XineImageStream::open() size "<<width<<" "<<height<<std::endl;

            // play();
            
            return true;

        }

        virtual void play()
        {
            if (_status!=PLAYING && _stream)
            {
                if (_status==PAUSED)
                {
                    xine_set_param (_stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
                    _status=PLAYING;
                }
                else
                {
                    osg::notify(osg::INFO)<<"XineImageStream::play()"<<std::endl;
                    if (xine_play(_stream, 0, 0))
                    {
                        while (!_ready)
                        {
                            osg::notify(osg::INFO)<<"   waiting..."<<std::endl;
                            OpenThreads::Thread::microSleep(10000);
                        }

                        _status=PLAYING;

                    }
                    else
                    {
                        osg::notify(osg::NOTICE)<<"Error!!!"<<std::endl;
                    }
                }
            }
        }

        virtual void pause()
        {
            if (_status==PAUSED || _status==INVALID) return;

            _status=PAUSED;
            
            if (_stream)
            {
                xine_set_param (_stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
            }
        }

        virtual void rewind()
        {
            if (_status==INVALID) return;

            _status=REWINDING;
            if (_stream)
            {
                osg::notify(osg::INFO)<<"Warning::XineImageStream::rewind() - rewind disabled at present."<<std::endl;
                //xine_trick_mode(_stream,XINE_TRICK_MODE_FAST_REWIND,0);
            }
        }

        virtual void quit(bool /*waitForThreadToExit*/ = true)
        {
            close();
        }

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

            osg::notify(osg::INFO)<<"image memcpy size="<<imageStream->getTotalSizeInBytes()<<" time="<<osg::Timer::instance()->delta_m(start_tick,osg::Timer::instance()->tick())<<"ms"<<std::endl;


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
        xine_event_queue_t*     _event_queue;
        bool                    _ready;
        float                   _volume;

    protected:


        virtual ~XineImageStream()
        {
            osg::notify(osg::INFO)<<"Killing XineImageStream"<<std::endl;
            close();
            osg::notify(osg::INFO)<<"Closed XineImageStream"<<std::endl;
        }

        void close()
        {

            osg::notify(osg::INFO)<<"XineImageStream::close()"<<std::endl;

            if (_stream)
            {
                  osg::notify(osg::INFO)<<"  Closing stream"<<std::endl;
                
                  xine_close(_stream);

                  osg::notify(osg::INFO)<<"  Disposing stream"<<std::endl;

                  xine_dispose(_stream);
                  _stream = 0;
            }


            if (_event_queue)
            {
                _event_queue = 0;
            }

            if (_ao)
            {
               osg::notify(osg::INFO)<<"  Closing audio driver"<<std::endl;

                xine_close_audio_driver(_xine, _ao);  
                
                _ao = 0;
            }
            
            if (_vo)
            {
               osg::notify(osg::INFO)<<"  Closing video driver"<<std::endl;

                xine_close_video_driver(_xine, _vo);  
                
                _vo = 0;
            }

           osg::notify(osg::INFO)<<"closed XineImageStream "<<std::endl;

        }


        static void event_listener(void *user_data, const xine_event_t *event)
        {
            XineImageStream* xis = reinterpret_cast<XineImageStream*>(user_data);
            switch(event->type)
            {
            case XINE_EVENT_UI_PLAYBACK_FINISHED:
                if (xis->getLoopingMode()==LOOPING)
                {
                    //rewind();
                    xine_play(xis->_stream, 0, 0);
                }
                break;
            }
        }

};

}

class ReaderWriterXine : public osgDB::ReaderWriter
{
    public:

        ReaderWriterXine()
        {
            supportsExtension("avi","");
            supportsExtension("db","");
            supportsExtension("ogv","");
            supportsExtension("flv","");
            supportsExtension("mov","");
            supportsExtension("mpg","Mpeg movie format");
            supportsExtension("mpv","Mpeg movie format");
            supportsExtension("wmv","");
            supportsExtension("xine","Xine plugin Pseduo plugin");
        
            _xine = xine_new();

            const char* user_home = xine_get_homedir();
            if(user_home)
            {
                char* cfgfile = NULL;
                int result = asprintf(&(cfgfile), "%s/.xine/config", user_home);                
                if (result>0) xine_config_load(_xine, cfgfile);
            }

            xine_init(_xine);
            
            register_rgbout_plugin(_xine);
        }
     
        virtual ~ReaderWriterXine()
        {
            osg::notify(osg::INFO)<<"~ReaderWriterXine()"<<std::endl;
        
            if (_xine) xine_exit(_xine);
            _xine = NULL;
        }
        
        virtual const char* className() const { return "Xine ImageStream Reader"; }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
            
            std::string fileName;
            if (ext=="xine")
            {
                fileName = osgDB::findDataFile( osgDB::getNameLessExtension(file), options);
                osg::notify(osg::INFO)<<"Xine stipped filename = "<<fileName<<std::endl;
            }
            else
            {
                fileName = osgDB::findDataFile( file, options );
                if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;
            }

            osg::notify(osg::INFO)<<"ReaderWriterXine::readImage "<< file<< std::endl;

            osg::ref_ptr<osgXine::XineImageStream> imageStream = new osgXine::XineImageStream();

            if (!imageStream->open(_xine, fileName)) return ReadResult::FILE_NOT_HANDLED;

            return imageStream.release();
        }

    protected:
        xine_t*             _xine;


};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(xine, ReaderWriterXine)
