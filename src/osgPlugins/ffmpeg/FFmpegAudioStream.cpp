
#include "FFmpegAudioStream.hpp"

#include <OpenThreads/ScopedLock>
#include <osg/Notify>

#include "FFmpegDecoder.hpp"
#include "MessageQueue.hpp"

#include <memory>



namespace osgFFmpeg {



FFmpegAudioStream::FFmpegAudioStream(FFmpegDecoder* decoder):
    m_decoder(decoder)
{
}



FFmpegAudioStream::FFmpegAudioStream(const FFmpegAudioStream & audio, const osg::CopyOp & copyop) :
    osg::AudioStream(audio, copyop)
{
}

FFmpegAudioStream::~FFmpegAudioStream()
{
    // detact the audio sink first to avoid destrction order issues.
    setAudioSink(0);
}

void FFmpegAudioStream::setAudioSink(osg::AudioSink* audio_sink)
{
    OSG_NOTICE<<"FFmpegAudioStream::setAudioSink( "<<audio_sink<<")"<<std::endl;
    m_decoder->audio_decoder().setAudioSink(audio_sink);
}


void FFmpegAudioStream::consumeAudioBuffer(void * const buffer, const size_t size)
{
    m_decoder->audio_decoder().fillBuffer(buffer, size);
}

double FFmpegAudioStream::duration() const
{
    return m_decoder->duration();
}



int FFmpegAudioStream::audioFrequency() const
{
    return m_decoder->audio_decoder().frequency();
}



int FFmpegAudioStream::audioNbChannels() const
{
    return m_decoder->audio_decoder().nbChannels();
}



osg::AudioStream::SampleFormat FFmpegAudioStream::audioSampleFormat() const
{
    return m_decoder->audio_decoder().sampleFormat();
}


} // namespace osgFFmpeg
