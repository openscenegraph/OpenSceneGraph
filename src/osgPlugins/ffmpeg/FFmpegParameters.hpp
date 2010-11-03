
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
    AVFormatParameters* getFormatParameter() { return &m_parameters; }
    
    void parse(const std::string& name, const std::string& value);

protected:

    AVInputFormat* m_format;
    AVFormatParameters m_parameters;
};



} // namespace osgFFmpeg



#endif // HEADER_GUARD_OSGFFMPEG_FFMPEG_PARAMETERS_H
