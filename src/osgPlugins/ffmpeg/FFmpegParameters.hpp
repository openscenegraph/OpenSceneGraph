
#ifndef HEADER_GUARD_OSGFFMPEG_FFMPEG_PARAMETERS_H
#define HEADER_GUARD_OSGFFMPEG_FFMPEG_PARAMETERS_H

#include "FFmpegHeaders.hpp"

#include <osg/Notify>


namespace osgFFmpeg {



class FFmpegParameters : public osg::Referenced
{
public:

    FFmpegParameters();
    ~FFmpegParameters();

    bool isFormatAvailable() const { return m_format!=NULL; }
    
    AVInputFormat* getFormat() { return m_format; }
    AVDictionary** getOptions() { return &m_options; }
    void setContext(AVIOContext* context) { m_context = context; }
    AVIOContext* getContext() { return m_context; }
    
    void parse(const std::string& name, const std::string& value);

protected:

    AVInputFormat* m_format;
    AVIOContext* m_context;
    AVDictionary* m_options;
};



} // namespace osgFFmpeg



#endif // HEADER_GUARD_OSGFFMPEG_FFMPEG_PARAMETERS_H
