// (C) Robert Osfield, Feb 2004.
// GPL'd.

#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <xine.h>
#include <xine/xineutils.h>
#include <xine/video_out.h>


typedef enum
{
/*	PX_RGB_PLANAR = 1, ? */
	PX_ARGB      = 2,  /* 32 bits [a:8@24, r:8@26, g:8@8, b:8@0] */
	PX_ARGB1555  = 3,  /* 16 bits [a:1@15, r:5@10, g:5@5, b:5@0] */
	PX_RGB32     = 4,  /* 32 bits [r:8@16, g:8@8, b:8@0] */
	PX_RGB24     = 5,  /* 24 bits [r:8@16, g:8@8, b:8@0] */
	PX_RGB16     = 6,  /* 16 bits [r:5@11, g:6@5, b:5@0] */
	PX_BGRA      = 7,  /* 32 bits [a:8@0, r:8@8, g:8@16, b:8@24] */
	PX_BGRA5551  = 8,  /* 16 bits [a:1@0, r:5@1, g:5@6, b:5@11] */
	PX_BGR32     = 9,  /* 32 bits [r:8@0, g:8@8, b:8@16] */
	PX_BGR24     = 10, /* 24 bits [r:8@0, g:8@8, b:8@16] */
	PX_BGR16     = 11  /* 16 bits [r:5@0, g:6@5, b:5@11] */

} rgb_pixel_format_t;


typedef enum
{
	PXLEVEL_NONE = 0,
	PXLEVEL_R    = (1 << 0),
	PXLEVEL_G    = (1 << 1),
	PXLEVEL_B    = (1 << 2),
	PXLEVEL_ALL  = 7 /* PX_LEVEL_R | PX_LEVEL_G | PX_LEVEL_B */

} rgb_pixel_levels_t;



/*
 * Applications that want to use this driver must provide a
 * callabck function (eg. for rendering frames);
 * RGBout will pass it a buffer containing pixels in the format
 * specified by "format" (generally you have only to BLIT
 * the buffer if you want to display the frame).
 * "levels" selects which RGB level is visible (if you dont't
 * need this feature, set it to PXLEVEL_ALL).
 *
 * N.B.: DO NOT FREE THE BUFFER
 *
 */

typedef struct
{
	rgb_pixel_format_t format;
	rgb_pixel_levels_t levels;
        void* user_data;
	void (*callback) (uint32_t width, uint32_t height, void* imageData, void* userData);

} rgbout_visual_info_t;


static int ready = 0;

static void my_render_frame(uint32_t width, uint32_t height, void* data, void* userData)
{
    osg::Image* imageStream = (osg::Image*) userData;
    
    GLenum pixelFormat = GL_RGB;
    
    imageStream->setImage(width,height,1,
                      GL_RGB,
                      pixelFormat,GL_UNSIGNED_BYTE,
                      (unsigned char *)data,
                      osg::Image::NO_DELETE,
                      1);

    ready = 1;
}

class ReaderWriterXine : public osgDB::ReaderWriter
{
    public:

        ReaderWriterXine()
        {
            osg::notify(osg::NOTICE)<<"I'm here"<<std::endl;

            _xine = xine_new();

            const char* user_home = xine_get_homedir();
            if(user_home)
            {
                char* cfgfile = NULL;
	        asprintf(&(cfgfile), "%s/.xine/config", user_home);
	        xine_config_load(_xine, cfgfile);
            }

            xine_init(_xine);
        
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
            osg::Image* imageStream = new osg::Image;

            // create visual
            rgbout_visual_info_t* visual = new rgbout_visual_info_t;
	    visual->levels = PXLEVEL_ALL;
            //visual->format = PX_BGRA;
            visual->format = PX_BGR24;
            visual->user_data = imageStream;
	    visual->callback = my_render_frame;

            // set up drivers
            xine_video_port_t* vo = xine_open_video_driver(_xine, "rgb", XINE_VISUAL_TYPE_FB, (void*)visual);
            xine_audio_port_t* ao = xine_open_audio_driver(_xine, "none", NULL);

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
                usleep(1000);
            }

            return imageStream;
        }

    protected:
        xine_t*             _xine;


};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterXine> g_readerWriter_Xine_Proxy;
