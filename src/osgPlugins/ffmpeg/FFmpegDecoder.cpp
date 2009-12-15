
#include "FFmpegDecoder.hpp"

#include <osg/Notify>
#include <osgDB/FileNameUtils>

#include <cassert>
#include <limits>
#include <stdexcept>
#include <string.h>
#include <iostream>


namespace osgFFmpeg {



FFmpegDecoder::FFmpegDecoder() :
    m_audio_stream(0),
    m_video_stream(0),
    m_audio_queue(100),
    m_video_queue(100),
    m_audio_decoder(m_audio_queue, m_clocks),
    m_video_decoder(m_video_queue, m_clocks),
    m_state(NORMAL),
    m_loop(false)
{

}



FFmpegDecoder::~FFmpegDecoder()
{
    close(true);
}


bool FFmpegDecoder::open(const std::string & filename)
{
    try
    {
        // Open video file
        AVFormatContext * p_format_context = 0;

        if (filename.compare(0, 5, "/dev/")==0)
        {
            avdevice_register_all();
        
            osg::notify(osg::NOTICE)<<"Attempting to stream "<<filename<<std::endl;

            AVFormatParameters formatParams;
            memset(&formatParams, 0, sizeof(AVFormatParameters));
            AVInputFormat *iformat;

            formatParams.channel = 0;
            formatParams.standard = 0;
#if 1
            formatParams.width = 320;
            formatParams.height = 240;
#else
            formatParams.width = 640;
            formatParams.height = 480;
#endif            
            formatParams.time_base.num = 1;
            formatParams.time_base.den = 30;

            std::string format = "video4linux2";
            iformat = av_find_input_format(format.c_str());
            
            if (iformat)
            {
                osg::notify(osg::NOTICE)<<"Found input format: "<<format<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Failed to find input format: "<<format<<std::endl;
            }

            int error = av_open_input_file(&p_format_context, filename.c_str(), iformat, 0, &formatParams);
            if (error != 0)
            {
                std::string error_str;
                switch (error)
                {
                    //case AVERROR_UNKNOWN: error_str = "AVERROR_UNKNOWN"; break;   // same value as AVERROR_INVALIDDATA
                    case AVERROR_IO: error_str = "AVERROR_IO"; break;
                    case AVERROR_NUMEXPECTED: error_str = "AVERROR_NUMEXPECTED"; break;
                    case AVERROR_INVALIDDATA: error_str = "AVERROR_INVALIDDATA"; break;
                    case AVERROR_NOMEM: error_str = "AVERROR_NOMEM"; break;
                    case AVERROR_NOFMT: error_str = "AVERROR_NOFMT"; break;
                    case AVERROR_NOTSUPP: error_str = "AVERROR_NOTSUPP"; break;
                    case AVERROR_NOENT: error_str = "AVERROR_NOENT"; break;
                    case AVERROR_PATCHWELCOME: error_str = "AVERROR_PATCHWELCOME"; break;
                    default: error_str = "Unknown error"; break;
                }

                throw std::runtime_error("av_open_input_file() failed : " + error_str);
            }
        }
        else
        {
            if (av_open_input_file(&p_format_context, filename.c_str(), 0, 0, 0) !=0 )
                throw std::runtime_error("av_open_input_file() failed");
        }
        
        m_format_context.reset(p_format_context);

        // Retrieve stream info
        if (av_find_stream_info(p_format_context) < 0)
            throw std::runtime_error("av_find_stream_info() failed");

        m_duration = double(m_format_context->duration) / AV_TIME_BASE;
        m_start = double(m_format_context->start_time) / AV_TIME_BASE;

        // TODO move this elsewhere
        m_clocks.reset(m_start);

        // Dump info to stderr
        dump_format(p_format_context, 0, filename.c_str(), false);

        // Find and open the first video and audio streams (note that audio stream is optional and only opened if possible)

        findVideoStream();
        findAudioStream();

        m_video_decoder.open(m_video_stream);

        try
        {
            m_audio_decoder.open(m_audio_stream);
        }

        catch (const std::runtime_error & error)
        {
            osg::notify(osg::WARN) << "FFmpegImageStream::open audio failed, audio stream will be disabled: " << error.what() << std::endl;
        }
    }

    catch (const std::runtime_error & error)
    {
        osg::notify(osg::WARN) << "FFmpegImageStream::open : " << error.what() << std::endl;
        return false;
    }
    
    return true;
}



void FFmpegDecoder::close(bool waitForThreadToExit)
{
    flushAudioQueue();
    flushVideoQueue();
    
    m_audio_decoder.close(waitForThreadToExit);
    m_video_decoder.close(waitForThreadToExit);
}



bool FFmpegDecoder::readNextPacket()
{
    switch (m_state)
    {
    case NORMAL:
        return readNextPacketNormal();

    case PAUSE:
        return false;

    case END_OF_STREAM:
        return readNextPacketEndOfStream();

    case REWINDING:
        return readNextPacketRewinding();

    case SEEKING:
        return readNextPacketSeeking();

    default:
        assert(false);
        return false;
    }
}



void FFmpegDecoder::rewind()
{
    m_pending_packet.clear();

    flushAudioQueue();
    flushVideoQueue();
    rewindButDontFlushQueues();
}

void FFmpegDecoder::seek(double time) 
{
    m_pending_packet.clear();

    flushAudioQueue();
    flushVideoQueue();
    seekButDontFlushQueues(time);
}

void FFmpegDecoder::pause() 
{
    m_pending_packet.clear();

    flushAudioQueue();
    flushVideoQueue();
    m_state = PAUSE;
}

void FFmpegDecoder::findAudioStream()
{
    for (unsigned int i = 0; i < m_format_context->nb_streams; ++i)
    {
        if (m_format_context->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
        {
            m_audio_stream = m_format_context->streams[i];
            m_audio_index = i;
            return;
        }
    }

    m_audio_stream = 0;
    m_audio_index = std::numeric_limits<unsigned int>::max();
}



void FFmpegDecoder::findVideoStream()
{
    for (unsigned int i = 0; i < m_format_context->nb_streams; ++i)
    {
        if (m_format_context->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
        {
            m_video_stream = m_format_context->streams[i];
            m_video_index = i;
            return;
        }
    }

    throw std::runtime_error("could not find a video stream");
}



inline void FFmpegDecoder::flushAudioQueue()
{
    FFmpegPacketClear pc;
    m_audio_queue.flush(pc);
}



inline void FFmpegDecoder::flushVideoQueue()
{
    FFmpegPacketClear pc;
    m_video_queue.flush(pc);
}



bool FFmpegDecoder::readNextPacketNormal()
{
    AVPacket packet;

    if (! m_pending_packet)
    {
        bool end_of_stream = false;

        // Read the next frame packet
        if (av_read_frame(m_format_context.get(), &packet) < 0)
        {
            if (url_ferror(m_format_context->pb) == 0)
                end_of_stream = true;
            else
                throw std::runtime_error("av_read_frame() failed");
        }

        if (end_of_stream)
        {
            // If we reach the end of the stream, change the decoder state
            if (loop())
            {
                m_clocks.reset(m_start);
                rewindButDontFlushQueues();
            }
            else
                m_state = END_OF_STREAM;

            return false;
        }
        else
        {
            // Make the packet data available beyond av_read_frame() logical scope.
            if (av_dup_packet(&packet) < 0)
                throw std::runtime_error("av_dup_packet() failed");

            m_pending_packet = FFmpegPacket(packet);            
        }
    }

    // Send data packet
    if (m_pending_packet.type == FFmpegPacket::PACKET_DATA)
    {
        if (m_pending_packet.packet.stream_index == m_audio_index)
        {
            if (m_audio_queue.timedPush(m_pending_packet, 10)) {
                m_pending_packet.release();
                return true;
            }
        }
        else if (m_pending_packet.packet.stream_index == m_video_index)
        {
            if (m_video_queue.timedPush(m_pending_packet, 10)) {
                m_pending_packet.release();
                return true;
            }
        }
        else
        {
            m_pending_packet.clear();
            return true;
        }
    }

    return false;
}



bool FFmpegDecoder::readNextPacketEndOfStream()
{
    const FFmpegPacket packet(FFmpegPacket::PACKET_END_OF_STREAM);

    m_audio_queue.timedPush(packet, 10);
    m_video_queue.timedPush(packet, 10);

    return false;
}
    


bool FFmpegDecoder::readNextPacketRewinding()
{
    const FFmpegPacket packet(FFmpegPacket::PACKET_FLUSH);

    if (m_audio_queue.timedPush(packet, 10) && m_video_queue.timedPush(packet, 10))
        m_state = NORMAL;

    return false;
}



void FFmpegDecoder::rewindButDontFlushQueues()
{
    const AVRational AvTimeBaseQ = { 1, AV_TIME_BASE }; // = AV_TIME_BASE_Q

    const int64_t pos = int64_t(m_clocks.getStartTime() * double(AV_TIME_BASE));
    const int64_t seek_target = av_rescale_q(pos, AvTimeBaseQ, m_video_stream->time_base);

    if (av_seek_frame(m_format_context.get(), m_video_index, seek_target, 0/*AVSEEK_FLAG_BYTE |*/ /*AVSEEK_FLAG_BACKWARD*/) < 0)
        throw std::runtime_error("av_seek_frame failed()");

    m_clocks.rewind();
    m_state = REWINDING;
}

bool FFmpegDecoder::readNextPacketSeeking() 
{
    const FFmpegPacket packet(FFmpegPacket::PACKET_FLUSH);

    if (m_audio_queue.timedPush(packet, 10) && m_video_queue.timedPush(packet, 10))
        m_state = NORMAL;

    return false;    
}

void FFmpegDecoder::seekButDontFlushQueues(double time)
{
    const AVRational AvTimeBaseQ = { 1, AV_TIME_BASE }; // = AV_TIME_BASE_Q

    const int64_t pos = int64_t(m_clocks.getStartTime()+time * double(AV_TIME_BASE));
    const int64_t seek_target = av_rescale_q(pos, AvTimeBaseQ, m_video_stream->time_base);

    m_clocks.setSeekTime(time);

    if (av_seek_frame(m_format_context.get(), m_video_index, seek_target, 0/*AVSEEK_FLAG_BYTE |*/ /*AVSEEK_FLAG_BACKWARD*/) < 0)
        throw std::runtime_error("av_seek_frame failed()");

    m_clocks.seek(time);
    m_state = SEEKING;    
}



} // namespace osgFFmpeg
