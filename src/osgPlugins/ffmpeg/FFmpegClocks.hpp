
#ifndef HEADER_GUARD_OSGFFMPEG_FFMPEG_CLOCKS_H
#define HEADER_GUARD_OSGFFMPEG_FFMPEG_CLOCKS_H

#include <osg/Timer>

#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#include "FFmpegHeaders.hpp"



namespace osgFFmpeg {



class FFmpegClocks
{
public:

    FFmpegClocks();

    void reset(double start_time);
    void pause(bool pause);
    void seek(double seek_time);
    void rewind();

    void audioSetBufferEndPts(double pts);
    void audioAdjustBufferEndPts(double increment);
    void audioSetDelay(double delay);
    void audioDisable();
    bool audioDisabled() const { return m_audio_disabled; }

    double videoSynchClock(const AVFrame * frame, double time_base, double pts);
    double videoRefreshSchedule(double pts);

    double getStartTime() const;
    double getCurrentTime();
    void setPauseTime(double pause_time);
    void setSeekTime(double seek_time);

private:

    double getAudioTime() const;

    typedef osg::Timer Timer;
    typedef OpenThreads::Mutex Mutex;
    typedef OpenThreads::ScopedLock<Mutex> ScopedLock;

    mutable Mutex m_mutex;

    double    m_video_clock;

    double    m_start_time;
    double    m_pause_time;
    double    m_seek_time;
    double    m_last_frame_delay;
    double    m_last_frame_pts;
    double    m_last_actual_delay;
    double    m_frame_time;
    double    m_audio_buffer_end_pts;
    double    m_audio_delay;
    Timer     m_audio_timer;
    bool      m_audio_disabled;
    bool      m_paused;
    double    m_last_current_time;

    
};



} // namespace osgFFmpeg



#endif // HEADER_GUARD_OSGFFMPEG_FFMPEG_CLOCKS_H
