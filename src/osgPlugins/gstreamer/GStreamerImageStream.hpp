#ifndef HEADER_GUARD_OSGGSTREAMER_GSTREAMER_IMAGE_STREAM_H
#define HEADER_GUARD_OSGGSTREAMER_GSTREAMER_IMAGE_STREAM_H

#include <gst/app/gstappsink.h>
#include <gst/pbutils/pbutils.h>

#include <osg/ImageStream>

#include <OpenThreads/Thread>

namespace osgGStreamer {

    class GStreamerImageStream : public osg::ImageStream, public OpenThreads::Thread
    {
    public:

        GStreamerImageStream();
        GStreamerImageStream(const GStreamerImageStream & image, const osg::CopyOp & copyop = osg::CopyOp::SHALLOW_COPY);

        META_Object(osgGStreamer, GStreamerImageStream);

        bool open(const std::string &filename);
        
        virtual void play();
        virtual void pause();
        virtual void rewind();
        virtual void seek(double time);

        virtual int s() const;
        virtual int t() const;

    private:

        virtual ~GStreamerImageStream();

        static gboolean on_message(GstBus *bus, GstMessage *message, GStreamerImageStream *user_data);

        static GstFlowReturn on_new_sample(GstAppSink *appsink, GStreamerImageStream *user_data);
        static GstFlowReturn on_new_preroll(GstAppSink *appsink, GStreamerImageStream *user_data);

        virtual void run();

        GMainLoop *loop;
        GstElement *pipeline;

        unsigned char *internal_buffer;

        int width;
        int height;
    };
}

#endif // HEADER_GUARD_OSGGSTREAMER_GSTREAMER_IMAGE_STREAM_H
