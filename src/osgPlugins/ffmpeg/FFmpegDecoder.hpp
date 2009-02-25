
#ifndef HEADER_GUARD_OSGFFMPEG_FFMPEG_DECODER_H
#define HEADER_GUARD_OSGFFMPEG_FFMPEG_DECODER_H

#include <boost/shared_ptr.hpp>

#include "FFmpegDecoderAudio.hpp"
#include "FFmpegDecoderVideo.hpp"



namespace osgFFmpeg {



class FFmpegDecoder
{
public:

    FFmpegDecoder();
    ~FFmpegDecoder();

    bool open(const std::string & filename);
    void close();

    bool readNextPacket();
    void rewind();

    void loop(bool loop);
    bool loop() const;

    double duration() const;

    FFmpegDecoderAudio & audio_decoder();
    FFmpegDecoderVideo & video_decoder();
    FFmpegDecoderAudio const & audio_decoder() const;
    FFmpegDecoderVideo const & video_decoder() const;

protected:

    enum State
    {
        NORMAL,
        END_OF_STREAM,
        REWINDING
    };

    typedef boost::shared_ptr<AVFormatContext> FormatContextPtr;
    typedef BoundedMessageQueue<FFmpegPacket> PacketQueue;

    void findAudioStream();
    void findVideoStream();
    void flushAudioQueue();
    void flushVideoQueue();
    bool readNextPacketNormal();
    bool readNextPacketEndOfStream();
    bool readNextPacketRewinding();
    void rewindButDontFlushQueues();

    FormatContextPtr    m_format_context;
    AVStream *            m_audio_stream;
    AVStream *            m_video_stream;

    unsigned int        m_audio_index;
    unsigned int        m_video_index;

    FFmpegClocks        m_clocks;
    FFmpegPacket        m_pending_packet;
    PacketQueue            m_audio_queue;
    PacketQueue            m_video_queue;
    
    FFmpegDecoderAudio    m_audio_decoder;
    FFmpegDecoderVideo    m_video_decoder;

    double                m_duration;
    double                m_start;

    State                m_state;
    bool                m_loop;
};





inline void FFmpegDecoder::loop(const bool loop)
{
    m_loop = loop;
}


inline bool FFmpegDecoder::loop() const
{
    return m_loop;
}


inline double FFmpegDecoder::duration() const
{
    return double(m_format_context->duration) / AV_TIME_BASE;    
}


inline FFmpegDecoderAudio & FFmpegDecoder::audio_decoder()
{
    return m_audio_decoder;
}


inline FFmpegDecoderVideo & FFmpegDecoder::video_decoder()
{
    return m_video_decoder;
}


inline FFmpegDecoderAudio const & FFmpegDecoder::audio_decoder() const
{
    return m_audio_decoder;
}


inline FFmpegDecoderVideo const & FFmpegDecoder::video_decoder() const
{
    return m_video_decoder;
}



} // namespace osgFFmpeg



#endif // HEADER_GUARD_OSGFFMPEG_FFMPEG_DECODER_H
