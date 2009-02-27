
#ifndef HEADER_GUARD_OSGFFMPEG_FFMPEG_IMAGE_STREAM_H
#define HEADER_GUARD_OSGFFMPEG_FFMPEG_IMAGE_STREAM_H

#include <osg/ImageStream>

#include <OpenThreads/Condition>
#include <OpenThreads/Thread>


#ifdef _WIN32
    #if defined OSG_LIBRARY_STATIC
    #define OSGFFMPEG_EXPORT_API
    #elif defined OSG_LIBRARY || defined osgFFmpeg_EXPORTS
    #define OSGFFMPEG_EXPORT_API  __declspec(dllexport)
    #else
    #define OSGFFMPEG_EXPORT_API  __declspec(dllimport);
    #endif
#else
    #define OSGFFMPEG_EXPORT_API
#endif



namespace osgFFmpeg
{

    class FFmpegDecoder;
    class FFmpegDecoderAudio;
    class FFmpegDecoderVideo;

    template <class T>
    class MessageQueue;


    class OSGFFMPEG_EXPORT_API FFmpegImageStream : public osg::ImageStream, public OpenThreads::Thread
    {
    public:

        FFmpegImageStream();
        FFmpegImageStream(const FFmpegImageStream & image, const osg::CopyOp & copyop = osg::CopyOp::SHALLOW_COPY);

        META_Object(osgFFmpeg, FFmpegImageStream);

        bool open(const std::string & filename);

        virtual void play();
        virtual void pause();
        virtual void rewind();
        virtual void quit(bool waitForThreadToExit = true);

        virtual void setAudioSink(osg::AudioSinkInterface* audio_sink);
        
        void consumeAudioBuffer(void * const buffer, const size_t size);
        
        bool audioStream() const;
        int audioFrequency() const;
        int audioNbChannels() const;
        osg::AudioStream::SampleFormat audioSampleFormat() const;

        double duration() const;

        bool videoAlphaChannel() const;
        double videoAspectRatio() const;
        double videoFrameRate() const;


    private:

        enum Command
        {
            CMD_PLAY,
            CMD_PAUSE,
            CMD_STOP,
            CMD_REWIND
        };

        typedef MessageQueue<Command> CommandQueue;
        typedef OpenThreads::Mutex Mutex;
        typedef OpenThreads::Condition Condition;

        virtual ~FFmpegImageStream();
        virtual void run();
        virtual void applyLoopingMode();

        bool handleCommand(Command cmd);

        void cmdPlay();
        void cmdPause();
        void cmdRewind();

        static void publishNewFrame(const FFmpegDecoderVideo &, void * user_data);

        FFmpegDecoder *    m_decoder;
        CommandQueue *    m_commands;

        Mutex            m_mutex;
        Condition        m_frame_published_cond;
        bool            m_frame_published_flag;
    };

}



#endif // HEADER_GUARD_OSGFFMPEG_FFMPEG_IMAGE_STREAM_H
