#include "FFmpegDecoderVideo.hpp"

#include <osg/Notify>
#include <osg/Timer>

#include <stdexcept>
#include <string.h>

namespace osgFFmpeg {

// TODO - add support for using RGB or RGBA pixel format.
// Note from Jason Daly in a osg-submissions thread, "The pix_fmt field of AVCodecContext will indicate the pixel format of the decoded video"


FFmpegDecoderVideo::FFmpegDecoderVideo(PacketQueue & packets, FFmpegClocks & clocks) :
    m_packets(packets),
    m_clocks(clocks),
    m_stream(0),
    m_context(0),
    m_codec(0),
    m_packet_data(0),
    m_bytes_remaining(0),
    m_packet_pts(AV_NOPTS_VALUE),
    m_writeBuffer(0),
    m_user_data(0),
    m_publish_func(0),
    m_paused(true),
    m_exit(false)
#ifdef USE_SWSCALE
    ,m_swscale_ctx(0)
#endif
{

}



FFmpegDecoderVideo::~FFmpegDecoderVideo()
{
    OSG_INFO<<"Destructing FFmpegDecoderVideo..."<<std::endl;

    this->close(true);

#ifdef USE_SWSCALE
    if (m_swscale_ctx)
    {
        sws_freeContext(m_swscale_ctx);
        m_swscale_ctx = 0;
    }
#endif

    if (m_context)
    {
        avcodec_close(m_context);
    }

    OSG_INFO<<"Destructed FFmpegDecoderVideo"<<std::endl;
}



void FFmpegDecoderVideo::open(AVStream * const stream)
{
    m_stream = stream;
    m_context = stream->codec;

    // Trust the video size given at this point
    // (avcodec_open seems to sometimes return a 0x0 size)
    m_width = m_context->width;
    m_height = m_context->height;
    findAspectRatio();

    // Find out whether we support Alpha channel
    m_alpha_channel = (m_context->pix_fmt == PIX_FMT_YUVA420P);

    // Find out the framerate
    #if LIBAVCODEC_VERSION_MAJOR >= 56
    m_frame_rate = av_q2d(stream->avg_frame_rate);
    #else
    m_frame_rate = av_q2d(stream->r_frame_rate);
    #endif

    // Find the decoder for the video stream
    m_codec = avcodec_find_decoder(m_context->codec_id);

    if (m_codec == 0)
        throw std::runtime_error("avcodec_find_decoder() failed");

    // Inform the codec that we can handle truncated bitstreams
    //if (p_codec->capabilities & CODEC_CAP_TRUNCATED)
    //    m_context->flags |= CODEC_FLAG_TRUNCATED;

    // Open codec
    if (avcodec_open2(m_context, m_codec, NULL) < 0)
        throw std::runtime_error("avcodec_open() failed");

    // Allocate video frame
    m_frame.reset(avcodec_alloc_frame());

    // Allocate converted RGB frame
    m_frame_rgba.reset(avcodec_alloc_frame());
    m_buffer_rgba[0].resize(avpicture_get_size(PIX_FMT_RGB24, width(), height()));
    m_buffer_rgba[1].resize(m_buffer_rgba[0].size());

    // Assign appropriate parts of the buffer to image planes in m_frame_rgba
    avpicture_fill((AVPicture *) (m_frame_rgba).get(), &(m_buffer_rgba[0])[0], PIX_FMT_RGB24, width(), height());

    // Override get_buffer()/release_buffer() from codec context in order to retrieve the PTS of each frame.
    m_context->opaque = this;
    m_context->get_buffer = getBuffer;
    m_context->release_buffer = releaseBuffer;
}


void FFmpegDecoderVideo::close(bool waitForThreadToExit)
{
    if (isRunning())
    {
        m_exit = true;
        if (waitForThreadToExit)
            join();
    }
}

void FFmpegDecoderVideo::pause(bool pause)
{
    if(pause)
        m_paused = true;
    else
        m_paused = false;
}

void FFmpegDecoderVideo::run()
{
    try
    {
        decodeLoop();
    }

    catch (const std::exception & error)
    {
        OSG_WARN << "FFmpegDecoderVideo::run : " << error.what() << std::endl;
    }

    catch (...)
    {
        OSG_WARN << "FFmpegDecoderVideo::run : unhandled exception" << std::endl;
    }
}



void FFmpegDecoderVideo::decodeLoop()
{
    FFmpegPacket packet;
    double pts;

    while (! m_exit)
    {
        // Work on the current packet until we have decoded all of it

        while (m_bytes_remaining > 0)
        {
            // Save global PTS to be stored in m_frame via getBuffer()

            m_packet_pts = packet.packet.pts;

            // Decode video frame

            int frame_finished = 0;

            // We want to use the entire packet since some codecs will require extra information for decoding
            const int bytes_decoded = avcodec_decode_video2(m_context, m_frame.get(), &frame_finished, &(packet.packet));

            if (bytes_decoded < 0)
                throw std::runtime_error("avcodec_decode_video failed()");

            m_bytes_remaining -= bytes_decoded;
            m_packet_data += bytes_decoded;

            // Publish the frame if we have decoded a complete frame
            if (frame_finished)
            {
                AVRational timebase;
                // Find out the frame pts
                if (m_frame->pts != int64_t(AV_NOPTS_VALUE))
                {
                    pts = m_frame->pts;
                    timebase = m_context->time_base;
                }
                else if (packet.packet.dts == int64_t(AV_NOPTS_VALUE) &&
                        m_frame->opaque != 0 &&
                        *reinterpret_cast<const int64_t*>(m_frame->opaque) != int64_t(AV_NOPTS_VALUE))
                {
                    pts = *reinterpret_cast<const int64_t*>(m_frame->opaque);
                    timebase = m_stream->time_base;
                }
                else if (packet.packet.dts != int64_t(AV_NOPTS_VALUE))
                {
                    pts = packet.packet.dts;
                    timebase = m_stream->time_base;
                }
                else
                {
                    pts = 0;
                    timebase = m_context->time_base;
                }

                pts *= av_q2d(timebase);

                const double synched_pts = m_clocks.videoSynchClock(m_frame.get(), av_q2d(timebase), pts);
                const double frame_delay = m_clocks.videoRefreshSchedule(synched_pts);

                publishFrame(frame_delay, m_clocks.audioDisabled());
            }
        }

        while(m_paused && !m_exit)
        {
            microSleep(10000);
        }

        // Get the next packet

        pts = 0;

        if (packet.valid())
            packet.clear();

        bool is_empty = true;
        packet = m_packets.timedPop(is_empty, 10);

        if (! is_empty)
        {
            if (packet.type == FFmpegPacket::PACKET_DATA)
            {
                m_bytes_remaining = packet.packet.size;
                m_packet_data = packet.packet.data;
            }
            else if (packet.type == FFmpegPacket::PACKET_FLUSH)
            {
                avcodec_flush_buffers(m_context);
            }
        }
    }
}



void FFmpegDecoderVideo::findAspectRatio()
{
    float ratio = 0.0f;

    if (m_context->sample_aspect_ratio.num != 0)
        ratio = float(av_q2d(m_context->sample_aspect_ratio));

    if (ratio <= 0.0f)
        ratio = 1.0f;

    m_pixel_aspect_ratio = ratio;
}

int FFmpegDecoderVideo::convert(AVPicture *dst, int dst_pix_fmt, AVPicture *src,
            int src_pix_fmt, int src_width, int src_height)
{
    osg::Timer_t startTick = osg::Timer::instance()->tick();
#ifdef USE_SWSCALE
    if (m_swscale_ctx==0)
    {
        m_swscale_ctx = sws_getContext(src_width, src_height, (PixelFormat) src_pix_fmt,
                                      src_width, src_height, (PixelFormat) dst_pix_fmt,
                                      /*SWS_BILINEAR*/ SWS_BICUBIC, NULL, NULL, NULL);
    }


    OSG_DEBUG<<"Using sws_scale ";

    int result =  sws_scale(m_swscale_ctx,
                            (src->data), (src->linesize), 0, src_height,
                            (dst->data), (dst->linesize));
#else

    OSG_DEBUG<<"Using img_convert ";

    int result = img_convert(dst, dst_pix_fmt, src,
                             src_pix_fmt, src_width, src_height);

#endif
    osg::Timer_t endTick = osg::Timer::instance()->tick();
    OSG_DEBUG<<" time = "<<osg::Timer::instance()->delta_m(startTick,endTick)<<"ms"<<std::endl;

    return result;
}


void FFmpegDecoderVideo::publishFrame(const double delay, bool audio_disabled)
{
    // If no publishing function, just ignore the frame
    if (m_publish_func == 0)
        return;

#if 1
    // new code from Jean-Sebasiten Guay - needs testing as we're unclear on the best solution
    // If the display delay is too small, we better skip the frame.
    if (!audio_disabled && delay < -0.010)
        return;
#else
    // original solution that hung on video stream over web.
    // If the display delay is too small, we better skip the frame.
    if (delay < -0.010)
        return;
#endif

    AVPicture * const src = (AVPicture *) m_frame.get();
    AVPicture * const dst = (AVPicture *) m_frame_rgba.get();

    // Assign appropriate parts of the buffer to image planes in m_frame_rgba
    avpicture_fill((AVPicture *) (m_frame_rgba).get(), &(m_buffer_rgba[m_writeBuffer])[0], PIX_FMT_RGB24, width(), height());

    // Convert YUVA420p (i.e. YUV420p plus alpha channel) using our own routine

    if (m_context->pix_fmt == PIX_FMT_YUVA420P)
        yuva420pToRgba(dst, src, width(), height());
    else
        convert(dst, PIX_FMT_RGB24, src, m_context->pix_fmt, width(), height());

    // Wait 'delay' seconds before publishing the picture.
    int i_delay = static_cast<int>(delay * 1000000 + 0.5);

    while (i_delay > 1000)
    {
        // Avoid infinite/very long loops
        if (m_exit)
            return;

        const int micro_delay = (std::min)(1000000, i_delay);

        OpenThreads::Thread::microSleep(micro_delay);

        i_delay -= micro_delay;
    }

    m_writeBuffer = 1-m_writeBuffer;

    m_publish_func(* this, m_user_data);
}



void FFmpegDecoderVideo::yuva420pToRgba(AVPicture * const dst, AVPicture * const src, int width, int height)
{
    convert(dst, PIX_FMT_RGB24, src, m_context->pix_fmt, width, height);

    const size_t bpp = 4;

    uint8_t * a_dst = dst->data[0] + 3;

    for (int h = 0; h < height; ++h) {

        const uint8_t * a_src = src->data[3] + h * src->linesize[3];

        for (int w = 0; w < width; ++w) {
            *a_dst = *a_src;
            a_dst += bpp;
            a_src += 1;
        }
    }
}



int FFmpegDecoderVideo::getBuffer(AVCodecContext * const context, AVFrame * const picture)
{
    const FFmpegDecoderVideo * const this_ = reinterpret_cast<const FFmpegDecoderVideo*>(context->opaque);

    const int result = avcodec_default_get_buffer(context, picture);
    int64_t * p_pts = reinterpret_cast<int64_t*>( av_malloc(sizeof(int64_t)) );

    *p_pts = this_->m_packet_pts;
    picture->opaque = p_pts;

    return result;
}



void FFmpegDecoderVideo::releaseBuffer(AVCodecContext * const context, AVFrame * const picture)
{
    if (picture != 0)
        av_freep(&picture->opaque);

    avcodec_default_release_buffer(context, picture);
}



} // namespace osgFFmpeg
