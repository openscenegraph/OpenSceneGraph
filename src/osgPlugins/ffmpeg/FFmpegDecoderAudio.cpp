#include "FFmpegDecoderAudio.hpp"

#include <osg/Notify>

#include <stdexcept>
#include <string.h>

//DEBUG
//#include <iostream>


#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
#endif



namespace osgFFmpeg {

static int decode_audio(AVCodecContext *avctx, int16_t *samples,
                         int *frame_size_ptr,
                         const uint8_t *buf, int buf_size)
{
#if LIBAVCODEC_VERSION_MAJOR >= 53 || (LIBAVCODEC_VERSION_MAJOR==52 && LIBAVCODEC_VERSION_MINOR>=32)

    // following code segment copied from ffmpeg's avcodec_decode_audio2()
    // implementation to avoid warnings about deprecated function usage.
    AVPacket avpkt;
    av_init_packet(&avpkt);
    avpkt.data = const_cast<uint8_t *>(buf);
    avpkt.size = buf_size;

    return avcodec_decode_audio3(avctx, samples, frame_size_ptr, &avpkt);
#else
    // fallback for older versions of ffmpeg that don't have avcodec_decode_audio3.
    return avcodec_decode_audio2(avctx, samples, frame_size_ptr, buf, buf_size);
#endif
}


FFmpegDecoderAudio::FFmpegDecoderAudio(PacketQueue & packets, FFmpegClocks & clocks) :
    m_packets(packets),
    m_clocks(clocks),
    m_stream(0),
    m_context(0),
    m_packet_data(0),
    m_bytes_remaining(0),
    m_audio_buffer((AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2),
    m_audio_buf_size(0),
    m_audio_buf_index(0),
    m_end_of_stream(false),
    m_paused(true),
    m_exit(false)
{

}



FFmpegDecoderAudio::~FFmpegDecoderAudio()
{
    this->close(true);
}



void FFmpegDecoderAudio::open(AVStream * const stream)
{
    try
    {
        // Sound can be optional (i.e. no audio stream is present)
        if (stream == 0)
            return;

        m_stream = stream;
        m_context = stream->codec;

        m_frequency = m_context->sample_rate;
        m_nb_channels = m_context->channels;
        switch (m_context->sample_fmt)
        {
        case AV_SAMPLE_FMT_NONE:
            throw std::runtime_error("invalid audio format AV_SAMPLE_FMT_NONE");
        case AV_SAMPLE_FMT_U8:
            m_sample_format = osg::AudioStream::SAMPLE_FORMAT_U8;
            break;
        case AV_SAMPLE_FMT_S16:
            m_sample_format = osg::AudioStream::SAMPLE_FORMAT_S16;
            break;
        case AV_SAMPLE_FMT_S32:
            m_sample_format = osg::AudioStream::SAMPLE_FORMAT_S32;
            break;
        case AV_SAMPLE_FMT_FLT:
            m_sample_format = osg::AudioStream::SAMPLE_FORMAT_F32;
            break;
        case AV_SAMPLE_FMT_DBL:
            throw std::runtime_error("unhandled audio format AV_SAMPLE_FMT_DBL");
        default:
            throw std::runtime_error("unknown audio format");
        }

        // Check stream sanity
        if (m_context->codec_id == CODEC_ID_NONE)
            throw std::runtime_error("invalid audio codec");;

        // Find the decoder for the audio stream
        AVCodec * const p_codec = avcodec_find_decoder(m_context->codec_id);

        if (p_codec == 0)
            throw std::runtime_error("avcodec_find_decoder() failed");

        // Inform the codec that we can handle truncated bitstreams
        //if (p_codec->capabilities & CODEC_CAP_TRUNCATED)
        //    m_context->flags |= CODEC_FLAG_TRUNCATED;

        // Open codec
        if (avcodec_open2(m_context, p_codec, NULL) < 0)
            throw std::runtime_error("avcodec_open() failed");
    }

    catch (...)
    {
        m_context = 0;
        throw;
    }
}

void FFmpegDecoderAudio::pause(bool pause)
{
    if (pause != m_paused)
    {
        m_paused = pause;
        if (m_audio_sink.valid())
        {
            if (m_paused) m_audio_sink->pause();
            else m_audio_sink->play();
        }
    }
}

void FFmpegDecoderAudio::close(bool waitForThreadToExit)
{
    if (isRunning())
    {
        m_exit = true;
        if (waitForThreadToExit)
            join();
    }
}

void FFmpegDecoderAudio::setVolume(float volume)
{
    if (m_audio_sink.valid())
    {
        m_audio_sink->setVolume(volume);
    }
}

float FFmpegDecoderAudio::getVolume() const
{
    if (m_audio_sink.valid())
    {
        return m_audio_sink->getVolume();
    }
    return 0.0f;
}

void FFmpegDecoderAudio::run()
{
    try
    {
        decodeLoop();
    }

    catch (const std::exception & error)
    {
        OSG_WARN << "FFmpegDecoderAudio::run : " << error.what() << std::endl;
    }

    catch (...)
    {
        OSG_WARN << "FFmpegDecoderAudio::run : unhandled exception" << std::endl;
    }
}


void FFmpegDecoderAudio::setAudioSink(osg::ref_ptr<osg::AudioSink> audio_sink)
{
    // The FFmpegDecoderAudio object takes the responsability of destroying the audio_sink.
    OSG_NOTICE<<"Assigning "<<audio_sink<<std::endl;
    m_audio_sink = audio_sink;
}



void FFmpegDecoderAudio::fillBuffer(void * const buffer, size_t size)
{
    uint8_t * dst_buffer = reinterpret_cast<uint8_t*>(buffer);

    while (size != 0)
    {
        if (m_audio_buf_index == m_audio_buf_size)
        {
            m_audio_buf_index = 0;

            // Pre-fetch audio buffer is empty, refill it.
            const size_t bytes_decoded = decodeFrame(&m_audio_buffer[0], m_audio_buffer.size());

            // If nothing could be decoded (e.g. error or no packet available), output a bit of silence
            if (bytes_decoded == 0)
            {
                m_audio_buf_size = std::min(Buffer::size_type(1024), m_audio_buffer.size());
                memset(&m_audio_buffer[0], 0, m_audio_buf_size);
            }
            else
            {
                m_audio_buf_size = bytes_decoded;
            }
        }

        const size_t fill_size = std::min(m_audio_buf_size - m_audio_buf_index, size);

        memcpy(dst_buffer, &m_audio_buffer[m_audio_buf_index], fill_size);

        size -= fill_size;
        dst_buffer += fill_size;

        m_audio_buf_index += fill_size;

        adjustBufferEndTps(fill_size);
    }
}



void FFmpegDecoderAudio::decodeLoop()
{
    const bool skip_audio = ! validContext() || ! m_audio_sink.valid();

    if (! skip_audio && ! m_audio_sink->playing())
    {
        m_clocks.audioSetDelay(m_audio_sink->getDelay());
        m_audio_sink->play();
    }
    else
    {
        m_clocks.audioDisable();
    }

    while (! m_exit)
    {

        if(m_paused)
        {
            m_clocks.pause(true);
            m_pause_timer.setStartTick();

            while(m_paused && !m_exit)
            {
                microSleep(10000);
            }

            m_clocks.setPauseTime(m_pause_timer.time_s());
            m_clocks.pause(false);
        }

        // If skipping audio, make sure the audio stream is still consumed.
        if (skip_audio)
        {
            bool is_empty;
            FFmpegPacket packet = m_packets.timedPop(is_empty, 10);

            if (packet.valid())
                packet.clear();
        }
        // Else, just idle in this thread.
        // Note: If m_audio_sink has an audio callback, this thread will still be awaken
        // from time to time to refill the audio buffer.
        else
        {
            OpenThreads::Thread::microSleep(10000);
        }
    }
}



void FFmpegDecoderAudio::adjustBufferEndTps(const size_t buffer_size)
{
    int sample_size = nbChannels() * frequency();

    switch (sampleFormat())
    {
    case osg::AudioStream::SAMPLE_FORMAT_U8:
        sample_size *= 1;
        break;

    case osg::AudioStream::SAMPLE_FORMAT_S16:
        sample_size *= 2;
        break;

    case osg::AudioStream::SAMPLE_FORMAT_S24:
        sample_size *= 3;
        break;

    case osg::AudioStream::SAMPLE_FORMAT_S32:
        sample_size *= 4;
        break;

    case osg::AudioStream::SAMPLE_FORMAT_F32:
        sample_size *= 4;
        break;

    default:
        throw std::runtime_error("unsupported audio sample format");
    }

    m_clocks.audioAdjustBufferEndPts(double(buffer_size) / double(sample_size));
}



size_t FFmpegDecoderAudio::decodeFrame(void * const buffer, const size_t size)
{
    for (;;)
    {
        // Decode current packet

        while (m_bytes_remaining > 0)
        {
            int data_size = size;

            const int bytes_decoded = decode_audio(m_context, reinterpret_cast<int16_t*>(buffer), &data_size, m_packet_data, m_bytes_remaining);

            if (bytes_decoded < 0)
            {
                // if error, skip frame
                m_bytes_remaining = 0;
                break;
            }

            m_bytes_remaining -= bytes_decoded;
            m_packet_data += bytes_decoded;

            // If we have some data, return it and come back for more later.
            if (data_size > 0)
                return data_size;
        }

        // Get next packet

        if (m_packet.valid())
            m_packet.clear();

        if (m_exit)
            return 0;

        bool is_empty = true;
        m_packet = m_packets.tryPop(is_empty);

        if (is_empty)
            return 0;

        if (m_packet.type == FFmpegPacket::PACKET_DATA)
        {
            if (m_packet.packet.pts != int64_t(AV_NOPTS_VALUE))
            {
                const double pts = av_q2d(m_stream->time_base) * m_packet.packet.pts;
                m_clocks.audioSetBufferEndPts(pts);
            }

            m_bytes_remaining = m_packet.packet.size;
            m_packet_data = m_packet.packet.data;
        }
        else if (m_packet.type == FFmpegPacket::PACKET_END_OF_STREAM)
        {
            m_end_of_stream = true;
        }
        else if (m_packet.type == FFmpegPacket::PACKET_FLUSH)
        {
            avcodec_flush_buffers(m_context);
        }

        // just output silence when we reached the end of stream
        if (m_end_of_stream)
        {
            memset(buffer, 0, size);
            return size;
        }
    }
}



} // namespace osgFFmpeg
