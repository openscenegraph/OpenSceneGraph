
#include "FFmpegParameters.hpp"

#include <string>
#include <iostream>
#include <sstream>



namespace osgFFmpeg {



FFmpegParameters::FFmpegParameters() :
    m_format(0)
{
    memset(&m_parameters, 0, sizeof(m_parameters));
}


FFmpegParameters::~FFmpegParameters()
{}


void FFmpegParameters::parse(const std::string& name, const std::string& value)
{
    if (value.empty())
    {
        return;
    }
    else if (name == "format")
    {
        avdevice_register_all();
        m_format = av_find_input_format(value.c_str());
        if (!m_format)
            OSG_NOTICE<<"Failed to apply input video format: "<<value.c_str()<<std::endl;
    }
    else if (name == "pixel_format")
    {
        m_parameters.pix_fmt = avcodec_get_pix_fmt(value.c_str());
    }
    else if (name == "frame_size")
    {
        int frame_width = 0, frame_height = 0;
        if (av_parse_video_frame_size(&frame_width, &frame_height, value.c_str()) < 0)
        {
            OSG_NOTICE<<"Failed to apply frame size: "<<value.c_str()<<std::endl;
            return;
        }
        if ((frame_width % 2) != 0 || (frame_height % 2) != 0)
        {
            OSG_NOTICE<<"Frame size must be a multiple of 2: "<<frame_width<<"x"<<frame_height<<std::endl;
            return;
        }
        m_parameters.width = frame_width;
        m_parameters.height = frame_height;
    }
    else if (name == "frame_rate")
    {
        AVRational frame_rate;
        if (av_parse_video_frame_rate(&frame_rate, value.c_str()) < 0)
        {
            OSG_NOTICE<<"Failed to apply frame rate: "<<value.c_str()<<std::endl;
            return;
        }
        m_parameters.time_base.den = frame_rate.num;
        m_parameters.time_base.num = frame_rate.den;
    }
    else if (name == "audio_sample_rate")
    {
        int audio_sample_rate = 44100;
        std::stringstream ss(value); ss >> audio_sample_rate;
        m_parameters.sample_rate = audio_sample_rate;
    }
}



} // namespace osgFFmpeg
