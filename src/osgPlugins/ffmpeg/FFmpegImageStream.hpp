
#ifndef HEADER_GUARD_OSGFFMPEG_FFMPEG_IMAGE_STREAM_H
#define HEADER_GUARD_OSGFFMPEG_FFMPEG_IMAGE_STREAM_H

#include <osg/ImageStream>

#include <OpenThreads/Condition>
#include <OpenThreads/Thread>

#include "FFmpegDecoder.hpp"
#include "MessageQueue.hpp"

namespace osgFFmpeg
{

    template <class T>
    class MessageQueue;

    class FFmpegImageStream : public osg::ImageStream, public OpenThreads::Thread
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

        osg::ref_ptr<FFmpegDecoder>    m_decoder;
        CommandQueue *    m_commands;

        Mutex            m_mutex;
        Condition        m_frame_published_cond;
        bool            m_frame_published_flag;
    };

}



#endif // HEADER_GUARD_OSGFFMPEG_FFMPEG_IMAGE_STREAM_H
