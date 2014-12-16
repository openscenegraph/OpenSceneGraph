#include "GStreamerImageStream.hpp"

namespace osgGStreamer {

GStreamerImageStream::GStreamerImageStream()
{
    setOrigin(osg::Image::TOP_LEFT);

    loop = g_main_loop_new(NULL, FALSE);
}

GStreamerImageStream::GStreamerImageStream(const GStreamerImageStream & image, const osg::CopyOp & copyop) :
    osg::ImageStream(image, copyop)
{
    // TODO: probably incorrect or incomplete
}

GStreamerImageStream::~GStreamerImageStream()
{
    OSG_INFO<<"Destructing GStreamerImageStream..."<<std::endl;

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_element_get_state(pipeline, NULL, NULL, GST_CLOCK_TIME_NONE); //wait until the state changed

    g_main_loop_quit(loop);
    g_main_loop_unref(loop);

    free(internal_buffer);
}

bool GStreamerImageStream::open(const std::string &filename)
{
    setFileName(filename);

    GError *error = NULL;

    // get stream info

    bool has_audio_stream = false;

    GstDiscoverer *item = gst_discoverer_new(1*GST_SECOND, &error);
    gchar *uri = g_filename_to_uri(filename.c_str(), NULL, NULL);

    if( gst_uri_is_valid(uri) )
    {
        GstDiscovererInfo *info = gst_discoverer_discover_uri(item, uri, &error);
        GList *audio_list = gst_discoverer_info_get_audio_streams(info);

        if( g_list_length(audio_list) > 0 )
            has_audio_stream = true;

        gst_discoverer_info_unref(info);
        g_free(uri);
    }

    // build pipeline

    gchar *audio_pipe = "";

    if( has_audio_stream )
        audio_pipe = "deco. ! queue ! audioconvert ! autoaudiosink";

    gchar *string = g_strdup_printf("filesrc location=%s ! \
        decodebin name=deco \
        deco. ! queue ! videoconvert ! video/x-raw,format=RGB ! appsink name=sink emit-signals=true \
        %s", filename.c_str(), audio_pipe);

    pipeline = gst_parse_launch(string, &error);

    if( pipeline == NULL )
    {
        g_printerr("Error: %s\n", error->message);
        return false;
    }

    g_free(string);
    g_error_free(error);

    // bus

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));

    gst_bus_add_watch(bus, (GstBusFunc)on_message, this);

    gst_object_unref(bus);


    // sink

    GstElement *sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

    g_signal_connect(sink, "new-sample", G_CALLBACK(on_new_sample), this);
    g_signal_connect(sink, "new-preroll", G_CALLBACK(on_new_preroll), this);

    gst_object_unref(sink);

    gst_element_set_state(pipeline, GST_STATE_PAUSED);
    gst_element_get_state(pipeline, NULL, NULL, GST_CLOCK_TIME_NONE); // wait until the state changed

    //setPixelBufferObject(new osg::PixelBufferObject(this)); // can help with the performance
    setImage(width, height, 1, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, internal_buffer, osg::Image::NO_DELETE);

    start();

    return true;
}

//** Controls **

void GStreamerImageStream::play()
{
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void GStreamerImageStream::pause()
{
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
}

void GStreamerImageStream::rewind()
{
    gst_element_seek_simple(pipeline, GST_FORMAT_TIME, GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), 0);
}

void GStreamerImageStream::seek(double time)
{
    gst_element_seek_simple(pipeline, GST_FORMAT_TIME, GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), time * GST_MSECOND);
}

int GStreamerImageStream::s() const
{
    return width;
}

int GStreamerImageStream::t() const
{
    return height;
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
    gst_buffer_extract(buffer, 0, user_data->internal_buffer, info.size);

    OSG_NOTICE<<"on_new_sample("<<(user_data->width)<<", "<<(user_data->height)<<")"<<std::endl;

    user_data->setImage(user_data->width, user_data->height, 1, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, user_data->internal_buffer, osg::Image::NO_DELETE, 4);

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

    user_data->width = width;
    user_data->height = height;

    int row_width = width*3;
    if ((row_width%4)!=0)
    {
        OSG_NOTICE<<"Rounding up row width from "<<row_width<<" to ";
        row_width += (4-(row_width%4));
        OSG_NOTICE<<row_width<<std::endl;;
    }

    user_data->internal_buffer = (unsigned char*)malloc(sizeof(unsigned char)*row_width*height);

    OSG_NOTICE<<"on_new_preroll("<<(user_data->width)<<", "<<(user_data->height)<<")"<<std::endl;

    // clean resources

    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

gboolean GStreamerImageStream::on_message(GstBus *bus, GstMessage *message, GStreamerImageStream *user_data)
{
    if( GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS)
    {
        user_data->rewind();
    }

    return TRUE;
}

void GStreamerImageStream::run()
{
    g_main_loop_run(loop);
}

} // namespace osgGStreamer
