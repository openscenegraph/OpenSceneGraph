
#include "FFmpegClocks.hpp"

#include <cmath>
#include <algorithm>

// DEBUG
//#include <iostream>


namespace osgFFmpeg {



namespace
{

    inline double clamp(const double value, const double min, const double max)
    {
        return (std::min)((std::max)(value, min), max);
    }

}



FFmpegClocks::FFmpegClocks() :
    m_video_clock(0),
    m_start_time(0),
    m_pause_time(0),
    m_seek_time(0),
    m_last_frame_delay(0.040),
    m_last_frame_pts(0),
    m_last_actual_delay(0),
    m_frame_time(0),
    m_audio_buffer_end_pts(0),
    m_audio_delay(0.0),
    m_audio_disabled(false),
    m_paused(false),
    m_last_current_time(0.0)
{

}



void FFmpegClocks::reset(const double start_time)
{
    ScopedLock lock(m_mutex);

    m_video_clock = start_time;

    m_start_time = start_time;
    m_last_frame_delay = 0.040;
    m_last_frame_pts = start_time - m_last_frame_delay;
    m_frame_time = start_time;

    m_pause_time = 0;
    m_seek_time = 0;

    m_audio_buffer_end_pts = start_time;
    m_audio_timer.setStartTick();
}

void FFmpegClocks::pause(bool pause)
{
    if(pause)
        m_paused = true;
    else
    {
        m_paused = false;
        if(!m_audio_disabled) m_audio_timer.setStartTick();
    }
}



void FFmpegClocks::rewind()
{
    ScopedLock lock(m_mutex);

    m_pause_time = 0;
    m_seek_time = 0;

    m_audio_buffer_end_pts = m_start_time;
    m_audio_timer.setStartTick();

    m_last_frame_delay = 0.040;
    m_frame_time = m_start_time;

    if (m_audio_disabled)
        return;

    m_video_clock = m_start_time;
}

void FFmpegClocks::seek(double seek_time)
{
    ScopedLock lock(m_mutex);

    m_video_clock = seek_time;
    m_last_frame_delay = 0.040;
    m_frame_time = seek_time;
}


void FFmpegClocks::audioSetBufferEndPts(const double pts)
{
    ScopedLock lock(m_mutex);

    m_audio_buffer_end_pts = pts;
    m_audio_timer.setStartTick();
}



void FFmpegClocks::audioAdjustBufferEndPts(double increment)
{
    ScopedLock lock(m_mutex);

    m_audio_buffer_end_pts += increment;
    m_audio_timer.setStartTick();
}



void FFmpegClocks::audioSetDelay(const double delay)
{
    m_audio_delay = delay;
}



void FFmpegClocks::audioDisable()
{
    ScopedLock lock(m_mutex);

    m_audio_disabled = true;
}



double FFmpegClocks::videoSynchClock(const AVFrame * const frame, const double time_base, double pts)
{
    if (pts != 0)
    {
        // If we have a PTS, set the video clock to it.
        m_video_clock = pts;
    }
    else
    {
        // Else, if we don't, use the video clock value.
        pts = m_video_clock;
    }

    // Update the video clock to take into account the frame delay

    double frame_delay = time_base * (1 + frame->repeat_pict);

    m_video_clock += frame_delay;

    return pts;
}



double FFmpegClocks::videoRefreshSchedule(const double pts)
{
    ScopedLock lock(m_mutex);

    // DEBUG
    //std::cerr << "ftime / dpts / delay / audio_time / adelay:  ";

    double delay = pts - m_last_frame_pts;


    //std::cerr << m_frame_time << "  /  ";
    //std::cerr << delay << "  /  ";


    // If incorrect delay, use previous one

    if (delay <= 0.0 || delay >= 1.0)
    {
        delay = m_last_frame_delay;
        if(!m_audio_disabled) m_frame_time = pts - delay;
    }


    // Save for next time
    m_last_frame_delay = delay;
    m_last_frame_pts = pts;

    // Update the delay to synch to the audio stream

    // Ideally the frame time should be incremented after the actual delay is computed.
    // But because of the sound latency, it seems better to keep some latency in the video too.
    m_frame_time += delay;

    const double audio_time = getAudioTime();
    const double actual_delay = clamp(m_frame_time - audio_time, -0.5*delay, 2.5*delay);

    //m_frame_time += delay;


    // DEBUG
    //std::cerr << delay << "  /  ";
    //std::cerr << audio_time << "  /  ";
    //std::cerr << actual_delay << std::endl;

    m_last_actual_delay = actual_delay;

    return actual_delay;
}



double FFmpegClocks::getStartTime() const
{
    return m_start_time;
}

void FFmpegClocks::setPauseTime(double pause_time)
{
    m_pause_time += pause_time;
}

void FFmpegClocks::setSeekTime(double seek_time)
{
    m_seek_time += getAudioTime() - seek_time;
}



double FFmpegClocks::getAudioTime() const
{
    if(m_audio_disabled)
        return m_audio_buffer_end_pts + m_audio_timer.time_s() - m_pause_time - m_audio_delay - m_seek_time;
    else
        return m_audio_buffer_end_pts + m_audio_timer.time_s() - m_audio_delay;
}


double FFmpegClocks::getCurrentTime()
{
    if(!m_paused)
        m_last_current_time = getAudioTime();

    return m_last_current_time;
}

} // namespace osgFFmpeg
