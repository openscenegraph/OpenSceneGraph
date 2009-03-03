
#ifndef HEADER_GUARD_OSGFFMPEG_FFMPEG_DECODER_VIDEO_H
#define HEADER_GUARD_OSGFFMPEG_FFMPEG_DECODER_VIDEO_H


#include "FFmpegHeaders.hpp"
#include "BoundedMessageQueue.hpp"
#include "FFmpegClocks.hpp"
#include "FFmpegPacket.hpp"

#include <boost/shared_ptr.hpp>
#include <OpenThreads/Thread>
#include <vector>

namespace osgFFmpeg {

class FramePtr
{
    public:
    
        typedef AVFrame T;
    
        explicit FramePtr() : _ptr(0) {}
        explicit FramePtr(T* ptr) : _ptr(ptr) {}
        
        ~FramePtr()
        {
            cleanup();
        }
        
        T* get() { return _ptr; }

        T * operator-> () const // never throws
        {
            return _ptr;
        }

        void reset(T* ptr) 
        {
            if (ptr==_ptr) return;
            cleanup();
            _ptr = ptr;
        }

        void cleanup()
        {
            if (_ptr) av_free(_ptr);
            _ptr = 0;
        }
        
        

    protected:
    
        T* _ptr;
};

class FFmpegDecoderVideo : public OpenThreads::Thread
{
public:

    typedef BoundedMessageQueue<FFmpegPacket> PacketQueue;
    typedef void (* PublishFunc) (const FFmpegDecoderVideo & decoder, void * user_data);

    FFmpegDecoderVideo(PacketQueue & packets, FFmpegClocks & clocks);
    ~FFmpegDecoderVideo();

    void open(AVStream * stream);
    virtual void run();

    void setUserData(void * user_data);
    void setPublishCallback(PublishFunc function);

    int width() const;
    int height() const;
    double aspectRatio() const;
    bool alphaChannel() const;
    double frameRate() const;
    const uint8_t * image() const;

private:

    typedef std::vector<uint8_t> Buffer;

    void decodeLoop();
    void findAspectRatio();
    void publishFrame(double delay);
    void swapBuffers();
    double synchronizeVideo(double pts);
    void yuva420pToRgba(AVPicture *dst, const AVPicture *src, int width, int height);

    static int getBuffer(AVCodecContext * context, AVFrame * picture);
    static void releaseBuffer(AVCodecContext * context, AVFrame * picture);

    PacketQueue &        m_packets;
    FFmpegClocks &        m_clocks;
    AVStream *            m_stream;
    AVCodecContext *    m_context;
    AVCodec *            m_codec;
    const uint8_t *        m_packet_data;
    int                    m_bytes_remaining;
    int64_t                m_packet_pts;
    
    FramePtr            m_frame;
    FramePtr            m_frame_rgba;
    Buffer                m_buffer_rgba;
    Buffer                m_buffer_rgba_public;

    void *                m_user_data;
    PublishFunc            m_publish_func;

    double                m_frame_rate;
    double                m_aspect_ratio;
    int                    m_width;
    int                    m_height;
    size_t                m_next_frame_index;
    bool                m_alpha_channel;

    volatile bool        m_exit;
};





inline void FFmpegDecoderVideo::setUserData(void * const user_data)
{
    m_user_data = user_data;
}


inline void FFmpegDecoderVideo::setPublishCallback(const PublishFunc function)
{
    m_publish_func = function;
}


inline int FFmpegDecoderVideo::width() const
{
    return m_width;
}


inline int FFmpegDecoderVideo::height() const
{
    return m_height;
}


inline double FFmpegDecoderVideo::aspectRatio() const
{
    return m_aspect_ratio;
}


inline bool FFmpegDecoderVideo::alphaChannel() const
{
    return m_alpha_channel;
}


inline double FFmpegDecoderVideo::frameRate() const
{
    return m_frame_rate;
}


inline const uint8_t * FFmpegDecoderVideo::image() const
{
    return &m_buffer_rgba_public[0];
}



} // namespace osgFFmpeg



#endif // HEADER_GUARD_OSGFFMPEG_FFMPEG_DECODER_VIDEO_H
