
#ifndef HEADER_GUARD_OSGFFMPEG_FFMPEG_DECODER_AUDIO_H
#define HEADER_GUARD_OSGFFMPEG_FFMPEG_DECODER_AUDIO_H

#include <OpenThreads/Thread>

#include "FFmpegClocks.hpp"
#include "FFmpegPacket.hpp"

#include <osg/AudioStream>

#include "BoundedMessageQueue.hpp"




namespace osgFFmpeg {



class FFmpegDecoderAudio : public OpenThreads::Thread
{
public:

    typedef BoundedMessageQueue<FFmpegPacket> PacketQueue;
    typedef void (* PublishFunc) (const FFmpegDecoderAudio & decoder, void * user_data);

    FFmpegDecoderAudio(PacketQueue & packets, FFmpegClocks & clocks);
    ~FFmpegDecoderAudio();

    void open(AVStream * stream);
    void close(bool waitForThreadToExit);
    
    virtual void run();

    void setAudioSink(osg::ref_ptr<osg::AudioSinkInterface> audio_sink);
    void fillBuffer(void * buffer, size_t size);

    bool validContext() const;
    int frequency() const;
    int nbChannels() const;
    osg::AudioStream::SampleFormat sampleFormat() const;

private:

    typedef osg::ref_ptr<osg::AudioSinkInterface> SinkPtr;
    typedef std::vector<uint8_t> Buffer;

    void decodeLoop();
    void adjustBufferEndTps(size_t buffer_size);
    size_t decodeFrame(void * buffer, size_t size);


    PacketQueue &        m_packets;
    FFmpegClocks &        m_clocks;
    AVStream *            m_stream;
    AVCodecContext *    m_context;
    FFmpegPacket        m_packet;
    const uint8_t *        m_packet_data;
    int                    m_bytes_remaining;

    Buffer                m_audio_buffer;
    size_t                m_audio_buf_size;
    size_t                m_audio_buf_index;

    int                    m_frequency;
    int                    m_nb_channels;
    osg::AudioStream::SampleFormat    m_sample_format;

    SinkPtr                m_audio_sink;

    bool                m_end_of_stream;
    volatile bool        m_exit;
};





inline bool FFmpegDecoderAudio::validContext() const
{
    return m_context != 0;
}


inline int FFmpegDecoderAudio::frequency() const
{
    return m_frequency;
}


inline int FFmpegDecoderAudio::nbChannels() const
{
    return m_nb_channels;
}


inline osg::AudioStream::SampleFormat FFmpegDecoderAudio::sampleFormat() const
{
    return m_sample_format;
}



} // namespace osgFFmpeg



#endif // HEADER_GUARD_OSGFFMPEG_FFMPEG_DECODER_AUDIO_H
