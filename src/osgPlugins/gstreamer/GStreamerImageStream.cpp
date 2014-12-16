#include "GStreamerImageStream.hpp"

#if 0
    #define OSGGST_INFO OSG_NOTICE
#else
    #define OSGGST_INFO OSG_INFO
#endif

namespace osgGStreamer {

GStreamerImageStream::GStreamerImageStream():
    _loop(0),
    _pipeline(0),
    _internal_buffer(0),
    _width(0),
    _height(0)
{
    setOrigin(osg::Image::TOP_LEFT);

    _loop = g_main_loop_new(NULL, FALSE);
}

GStreamerImageStream::GStreamerImageStream(const GStreamerImageStream & image, const osg::CopyOp & copyop) :
    osg::ImageStream(image, copyop),
    _loop(0),
    _pipeline(0),
    _internal_buffer(0),
    _width(0),
    _height(0)
{
    setOrigin(osg::Image::TOP_LEFT);

    _loop = g_main_loop_new(NULL, FALSE);

    if (!getFileName().empty())
    {
        open(getFileName());
    }
}

GStreamerImageStream::~GStreamerImageStream()
{
    gst_element_set_state(_pipeline, GST_STATE_NULL);
    gst_element_get_state(_pipeline, NULL, NULL, GST_CLOCK_TIME_NONE); //wait until the state changed

    g_main_loop_quit(_loop);
    g_main_loop_unref(_loop);

    free(_internal_buffer);
}

bool GStreamerImageStream::open(const std::string& filename)
{
    setFileName(filename);

    GError *error = NULL;

    // get stream info

    bool has_audio_stream = false;

    gchar *uri = g_filename_to_uri(filename.c_str(), NULL, NULL);

    if( uri!=0 && gst_uri_is_valid(uri) )
    {
        GstDiscoverer *item = gst_discoverer_new(1*GST_SECOND, &error);
        GstDiscovererInfo *info = gst_discoverer_discover_uri(item, uri, &error);
        GList *audio_list = gst_discoverer_info_get_audio_streams(info);

        if( g_list_length(audio_list) > 0 )
            has_audio_stream = true;

        gst_discoverer_info_unref(info);
        g_free(uri);
    }

    // build pipeline
    const gchar *audio_pipe = "";
    if( has_audio_stream )
    {
        audio_pipe = "deco. ! queue ! audioconvert ! autoaudiosink";
    }

    gchar *string = g_strdup_printf("filesrc location=%s ! \
        decodebin name=deco \
        deco. ! queue ! videoconvert ! video/x-raw,format=RGB ! appsink name=sink emit-signals=true \
        %s", filename.c_str(), audio_pipe);

    _pipeline = gst_parse_launch(string, &error);

    g_free(string);

    if (error)
    {
        g_printerr("Error: %s\n", error->message);
        g_error_free(error);
    }

    if( _pipeline == NULL )
    {
        return false;
    }


    // bus
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline));

    gst_bus_add_watch(bus, (GstBusFunc)on_message, this);

    gst_object_unref(bus);


    // sink

    GstElement *sink = gst_bin_get_by_name(GST_BIN(_pipeline), "sink");

    g_signal_connect(sink, "new-sample", G_CALLBACK(on_new_sample), this);
    g_signal_connect(sink, "new-preroll", G_CALLBACK(on_new_preroll), this);

    gst_object_unref(sink);

    gst_element_set_state(_pipeline, GST_STATE_PAUSED);
    gst_element_get_state(_pipeline, NULL, NULL, GST_CLOCK_TIME_NONE); // wait until the state changed

    if (_width==0 || _height==0)
    {
        // no valid image has been setup by a on_new_preroll() call.
        return false;
    }

    // setLoopingMode(osg::ImageStream::NO_LOOPING);

    // start the thread to run gstreamer main loop
    start();

    return true;
}

//** Controls **

void GStreamerImageStream::play()
{
    OSGGST_INFO<<"GStreamerImageStream::play()"<<std::endl;
    gst_element_set_state(_pipeline, GST_STATE_PLAYING);
}

void GStreamerImageStream::pause()
{
    OSGGST_INFO<<"GStreamerImageStream::pause()"<<std::endl;
    gst_element_set_state(_pipeline, GST_STATE_PAUSED);
}

void GStreamerImageStream::rewind()
{
    OSGGST_INFO<<"GStreamerImageStream::rewind()"<<std::endl;
    gst_element_seek_simple(_pipeline, GST_FORMAT_TIME, GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), 0);
}

void GStreamerImageStream::seek(double time)
{
    OSGGST_INFO<<"GStreamerImageStream::seek("<<time<<")"<<std::endl;
    gst_element_seek_simple(_pipeline, GST_FORMAT_TIME, GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), time * GST_MSECOND);
}

//** Callback implementations **

GstFlowReturn GStreamerImageStream::on_new_sample(GstAppSink *appsink, GStreamerImageStream *user_data)
{
    // get the buffer from appsink

    GstSample *sample = gst_app_sink_pull_sample(appsink);
    GstBuffer *buffer = gst_sample_get_buffer(sample);

    // upload data

    GstMapInfo info;
    gst_buffer_map(buffer, &info, GST_MAP_READ);
    gst_buffer_extract(buffer, 0, user_data->_internal_buffer, info.size);

    // data has been modified so dirty the image so the texture will updated
    user_data->dirty();

    // clean resources

    gst_buffer_unmap(buffer, &info);
    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

GstFlowReturn GStreamerImageStream::on_new_preroll(GstAppSink *appsink, GStreamerImageStream *user_data)
{
    // get the sample from appsink

    GstSample *sample = gst_app_sink_pull_preroll(appsink);

    // get sample info

    GstCaps *caps = gst_sample_get_caps(sample);
    GstStructure *structure = gst_caps_get_structure(caps, 0);

    int width;
    int height;

    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);

    if (width<=0 || height<=0)
    {
        OSG_NOTICE<<"Error: video size invalid width="<<width<<", height="<<height<<std::endl;
        return GST_FLOW_ERROR;
    }

    if (user_data->_width != width || user_data->_height != height)
    {
        user_data->_width = width;
        user_data->_height = height;


        int row_width = width*3;
        if ((row_width%4)!=0)
        {
            row_width += (4-(row_width%4));
        }

        // if buffer previously assigned free it before allocating new buffer.
        if (user_data->_internal_buffer) free(user_data->_internal_buffer);

        // allocate buffer
        user_data->_internal_buffer = (unsigned char*)malloc(sizeof(unsigned char)*row_width*height);

        // assign buffer to image
        user_data->setImage(user_data->_width, user_data->_height, 1, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, user_data->_internal_buffer, osg::Image::NO_DELETE, 4);
    }

    // clean resources
    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

gboolean GStreamerImageStream::on_message(GstBus *bus, GstMessage *message, GStreamerImageStream *user_data)
{
    if( GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS)
    {
        OSGGST_INFO<<"Video '"<<user_data->getFileName()<<"' finished."<<std::endl;
        if (user_data->getLoopingMode()==osg::ImageStream::LOOPING)
        {
            user_data->rewind();
        }
    }

    return TRUE;
}

void GStreamerImageStream::run()
{
    g_main_loop_run(_loop);
}


} // namespace osgGStreamer
