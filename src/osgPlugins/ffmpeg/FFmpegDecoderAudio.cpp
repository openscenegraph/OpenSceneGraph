#include "FFmpegDecoderAudio.hpp"

#include <osg/Notify>

#include <stdexcept>
#include <string.h>

//DEBUG
//#include <iostream>


#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#define av_frame_free  avcodec_free_frame
#endif

#if LIBAVCODEC_VERSION_MAJOR < 56
   #define AV_CODEC_ID_NONE CODEC_ID_NONE
#endif

namespace osgFFmpeg {

static int decode_audio(AVCodecContext *avctx, int16_t *samples,
                         int *frame_size_ptr,
                         const uint8_t *buf, int buf_size,
                         SwrContext *swr_context,
                         int out_sample_rate,
                         int out_nb_channels,
                         AVSampleFormat out_sample_format)
{
#if LIBAVCODEC_VERSION_MAJOR >= 53 || (LIBAVCODEC_VERSION_MAJOR==52 && LIBAVCODEC_VERSION_MINOR>=32)

    AVPacket avpkt;
    av_init_packet(&avpkt);
    avpkt.data = const_cast<uint8_t *>(buf);
    avpkt.size = buf_size;

    AVFrame *frame = av_frame_alloc();
    int ret, got_frame = 0;

    if (!frame)
        return AVERROR(ENOMEM);

    ret = avcodec_decode_audio4(avctx, frame, &got_frame, &avpkt);

#ifdef USE_AVRESAMPLE    // libav's AVFrame structure does not contain a 'channels' field
    if (ret >= 0 && got_frame) {
#else
    if (ret >= 0 && got_frame && av_frame_get_channels(frame)>0) {
#endif
        int ch, plane_size;
        int planar = av_sample_fmt_is_planar(avctx->sample_fmt);

        int out_samples;
        // if sample rate changes, number of samples is different
        if ( out_sample_rate !=  avctx->sample_rate ) {
//            out_samples = av_rescale_rnd(swr_get_delay(swr_context, avctx->sample_rate) +
//                                 frame->nb_samples, out_sample_rate, avctx->sample_rate, AV_ROUND_UP);
            out_samples = av_rescale_rnd(frame->nb_samples, out_sample_rate, avctx->sample_rate, AV_ROUND_UP);
        }
        else {
            out_samples = frame->nb_samples;
        }

        int output_data_size = av_samples_get_buffer_size(&plane_size, out_nb_channels,
                                                    out_samples,
                                                    out_sample_format, 1);

        if (*frame_size_ptr < output_data_size) {
            av_log(avctx, AV_LOG_ERROR, "output buffer size is too small for "
                                        "the current frame (%d < %d)\n", *frame_size_ptr, output_data_size);
            av_frame_free(&frame);
            return AVERROR(EINVAL);
        }

        // if resampling is needed, call swr_convert
        if ( swr_context != NULL ) {

            out_samples = swr_convert(swr_context, (uint8_t **)&samples, out_samples,
                        (const uint8_t **)frame->extended_data, frame->nb_samples);

            // recompute output_data_size following swr_convert result (number of samples actually converted)
            output_data_size = av_samples_get_buffer_size(&plane_size, out_nb_channels,
                                                    out_samples,
                                                    out_sample_format, 1);
        }
        else {

            memcpy(samples, frame->extended_data[0], plane_size);

            if (planar && avctx->channels > 1) {
                uint8_t *out = ((uint8_t *)samples) + plane_size;
                for (ch = 1; ch < avctx->channels; ch++) {
                    memcpy(out, frame->extended_data[ch], plane_size);
                    out += plane_size;
                }
            }
        }

        *frame_size_ptr = output_data_size;

    } else {
        *frame_size_ptr = 0;
    }
    av_frame_free(&frame);
    return ret;

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
    m_exit(false),
    m_swr_context(NULL)
{
}



FFmpegDecoderAudio::~FFmpegDecoderAudio()
{
    this->close(true);
}



void FFmpegDecoderAudio::open(AVStream * const stream, FFmpegParameters* parameters)
{
    try
    {
        // Sound can be optional (i.e. no audio stream is present)
        if (stream == 0)
            return;

        m_stream = stream;
        m_context = stream->codec;

        m_in_sample_rate = m_context->sample_rate;
        m_in_nb_channels = m_context->channels;
        m_in_sample_format = m_context->sample_fmt;

        AVDictionaryEntry *opt_out_sample_rate = av_dict_get( *parameters->getOptions(), "out_sample_rate", NULL, 0 );
        if ( opt_out_sample_rate )
            m_out_sample_rate = atoi(opt_out_sample_rate->value);
        else
            m_out_sample_rate = m_in_sample_rate;

        AVDictionaryEntry *opt_out_sample_format = av_dict_get( *parameters->getOptions(), "out_sample_format", NULL, 0 );
        if ( opt_out_sample_format )
            m_out_sample_format = (AVSampleFormat) atoi(opt_out_sample_format->value);
        else
            // always packed, planar formats are evil!
            m_out_sample_format = av_get_packed_sample_fmt( m_in_sample_format );

        AVDictionaryEntry *opt_out_nb_channels = av_dict_get( *parameters->getOptions(), "out_nb_channels", NULL, 0 );
        if ( opt_out_nb_channels )
            m_out_nb_channels = atoi(opt_out_nb_channels->value);
        else
            m_out_nb_channels = m_in_nb_channels;

        if ( m_in_sample_rate != m_out_sample_rate
            || m_in_nb_channels != m_out_nb_channels
            || m_in_sample_format != m_out_sample_format )
        {
#if 0
printf("### CONVERTING from sample format %s TO %s\n\t\tFROM %d TO %d channels\n\t\tFROM %d Hz to %d Hz\n",
            av_get_sample_fmt_name(m_in_sample_format),
            av_get_sample_fmt_name(m_out_sample_format),
            m_in_nb_channels,
            m_out_nb_channels,
            m_in_sample_rate,
            m_out_sample_rate);
#endif
            m_swr_context = swr_alloc_set_opts(NULL,
                    av_get_default_channel_layout(m_out_nb_channels),
                    m_out_sample_format,
                    m_out_sample_rate,
                    av_get_default_channel_layout(m_in_nb_channels),
                    m_in_sample_format,
                    m_in_sample_rate,
                    0, NULL );

            int err = swr_init(m_swr_context);

            if ( err ) {
                char error_string[512];
                av_strerror(err, error_string, 512);
                OSG_WARN << "FFmpegDecoderAudio - WARNING: Error initializing resampling context : " << error_string << std::endl;
                swr_free(&m_swr_context);
                throw std::runtime_error("swr_init() failed");;
            }
        }

        // Check stream sanity
        if (m_context->codec_id == AV_CODEC_ID_NONE)
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

        m_context->get_buffer = avcodec_default_get_buffer;
        m_context->release_buffer = avcodec_default_release_buffer;

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
    swr_free(&m_swr_context);
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
    // The FFmpegDecoderAudio object takes the responsibility of destroying the audio_sink.
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

        adjustBufferEndPts(fill_size);
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


void FFmpegDecoderAudio::adjustBufferEndPts(const size_t buffer_size)
{
    int bytes_per_second = nbChannels() * frequency();

    switch (sampleFormat())
    {
    case osg::AudioStream::SAMPLE_FORMAT_U8:
        bytes_per_second *= 1;
        break;

    case osg::AudioStream::SAMPLE_FORMAT_S16:
        bytes_per_second *= 2;
        break;

    case osg::AudioStream::SAMPLE_FORMAT_S24:
        bytes_per_second *= 3;
        break;

    case osg::AudioStream::SAMPLE_FORMAT_S32:
        bytes_per_second *= 4;
        break;

    case osg::AudioStream::SAMPLE_FORMAT_F32:
        bytes_per_second *= 4;
        break;

    default:
        throw std::runtime_error("unsupported audio sample format");
    }

    m_clocks.audioAdjustBufferEndPts(double(buffer_size) / double(bytes_per_second));
}



size_t FFmpegDecoderAudio::decodeFrame(void * const buffer, const size_t size)
{
    for (;;)
    {
        // Decode current packet

        while (m_bytes_remaining > 0)
        {
            int data_size = size;

            const int bytes_decoded = decode_audio(m_context, reinterpret_cast<int16_t*>(buffer), &data_size, m_packet_data, m_bytes_remaining, m_swr_context, m_out_sample_rate, m_out_nb_channels, m_out_sample_format);

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


/**
 *
 */
osg::AudioStream::SampleFormat FFmpegDecoderAudio::sampleFormat() const
{
    switch (m_out_sample_format)
    {
    case AV_SAMPLE_FMT_NONE:
        throw std::runtime_error("invalid audio format AV_SAMPLE_FMT_NONE");
    case AV_SAMPLE_FMT_U8:
        return osg::AudioStream::SAMPLE_FORMAT_U8;
        break;
    case AV_SAMPLE_FMT_S16:
        return osg::AudioStream::SAMPLE_FORMAT_S16;
        break;
    case AV_SAMPLE_FMT_S32:
        return osg::AudioStream::SAMPLE_FORMAT_S32;
        break;
    case AV_SAMPLE_FMT_FLT:
        return osg::AudioStream::SAMPLE_FORMAT_F32;
        break;
    case AV_SAMPLE_FMT_DBL:
        throw std::runtime_error("unhandled audio format AV_SAMPLE_FMT_DBL");

    default:
        throw std::runtime_error("unknown audio format");
    }
}

} // namespace osgFFmpeg
