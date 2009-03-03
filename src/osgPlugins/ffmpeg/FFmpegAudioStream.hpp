
#ifndef HEADER_GUARD_OSGFFMPEG_FFMPEG_AUDIO_STREAM_H
#define HEADER_GUARD_OSGFFMPEG_FFMPEG_AUDIO_STREAM_H

#include <osg/AudioStream>
#include "FFmpegDecoder.hpp"

namespace osgFFmpeg
{

    class FFmpegAudioStream : public osg::AudioStream
    {
    public:

        FFmpegAudioStream(FFmpegDecoder* decoder=0);
        FFmpegAudioStream(const FFmpegAudioStream & audio, const osg::CopyOp & copyop = osg::CopyOp::SHALLOW_COPY);

        META_Object(osgFFmpeg, FFmpegAudioStream);

        virtual void setAudioSink(osg::AudioSink* audio_sink);
        
        void consumeAudioBuffer(void * const buffer, const size_t size);
        
        bool audioStream() const;
        int audioFrequency() const;
        int audioNbChannels() const;
        osg::AudioStream::SampleFormat audioSampleFormat() const;

        double duration() const;

    private:

        virtual ~FFmpegAudioStream();

        osg::ref_ptr<FFmpegDecoder>    m_decoder;

    };

}



#endif // HEADER_GUARD_OSGFFMPEG_FFMPEG_IMAGE_STREAM_H
