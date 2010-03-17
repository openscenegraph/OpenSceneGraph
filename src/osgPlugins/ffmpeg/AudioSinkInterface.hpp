
#ifndef HEADER_GUARD_OSGFFMPEG_AUDIO_SINK_INTERFACE_H
#define HEADER_GUARD_OSGFFMPEG_AUDIO_SINK_INTERFACE_H

#include <osg/Object>



namespace osgFFmpeg
{

    class AudioSinkInterface : public osg::Object
    {
    public:

        AudioSinkInterface() :
            m_delay(0.0) { }

        virtual void startPlaying() = 0;
        virtual bool playing() const = 0;

        virtual double getDelay() const { return m_delay; }
        virtual void setDelay(const double delay) { m_delay = delay; }

        virtual const char * libraryName() const { return "osgFFmpeg"; }
        virtual const char * className() const { return "AudioSinkInterface"; }

    private:

        virtual AudioSinkInterface * cloneType() const { return 0; }
        virtual AudioSinkInterface * clone(const osg::CopyOp &) const { return 0; }

        double    m_delay;
    };

}



#endif // HEADER_GUARD_OSGFFMPEG_AUDIO_SINK_INTERFACE_H
