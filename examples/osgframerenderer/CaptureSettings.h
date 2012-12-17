#ifndef CAPTURESETTINGS_H
#define CAPTURESETTINGS_H

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osg/AnimationPath>

#include "UpdateProperty.h"

namespace gsc
{

class CaptureSettings : public osg::Object
{
public:
    CaptureSettings();
    CaptureSettings(const CaptureSettings& cs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

    META_Object(gsc, CaptureSettings);

    void setInputFileName(const std::string& filename) { _inputFileName = filename; }
    const std::string& getInputFileName() const { return _inputFileName; }
    
    void setOutputFileName(const std::string& filename);
    const std::string& getOutputFileName() const;
    
    std::string getOutputFileName(unsigned int frameNumber) const;
    std::string getOutputFileName(unsigned int cameraNumber, unsigned int frameNumber) const;

    enum StereoMode
    {
        OFF,
        HORIZONTAL_SPLIT,
        VERTICAL_SPLIT
    };

    void setStereoMode(StereoMode mode) { _stereoMode = mode; }
    StereoMode getStereoMode() const { return _stereoMode; }

    void setOffscreen(bool o) { _offscreen = o; }
    bool getOffscreen() const { return _offscreen; }

    void setOutputImageFlip(bool flip) { _outputImageFlip = flip; }
    bool getOutputImageFlip() const { return _outputImageFlip; }

    void setWidth(unsigned int width) { _width = width; }
    unsigned int getWidth() const { return _width; }

    void setHeight(unsigned int height) { _height = height; }
    unsigned int getHeight() const { return _height; }

    void setScreenWidth(float width) { _screenWidth = width; }
    float getScreenWidth() const { return _screenWidth; }

    void setScreenHeight(float height) { _screenHeight = height; }
    float getScreenHeight() const { return _screenHeight; }

    void setScreenDistance(float distance) { _screenDistance = distance; }
    float getScreenDistance() const { return _screenDistance; }


    enum PixelFormat
    {
        RGB,
        RGBA
    };
    
    void setPixelFormat(PixelFormat format) { _pixelFormat = format; }
    PixelFormat getPixelFormat() const { return _pixelFormat; }

    void setSamples(unsigned int s) { _samples = s; }
    unsigned int getSamples() const { return _samples; }

    void setSampleBuffers(unsigned int s) { _sampleBuffers = s; }
    unsigned int getSampleBuffers() const { return _sampleBuffers; }

    void setFrameRate(double fr) { _frameRate = fr; }
    double getFrameRate() const { return _frameRate; }

    void setNumberOfFrames(unsigned int nf) { _numberOfFrames = nf; }
    unsigned int getNumberOfFrames() const { return _numberOfFrames; }

    typedef std::vector< osg::ref_ptr<osgGA::GUIEventHandler> > EventHandlers;
    void setEventHandlers(const EventHandlers& eh);
    EventHandlers& getEventHandlers() { return _eventHandlers; }
    const EventHandlers& getEventHandlers() const { return _eventHandlers; }

    typedef std::vector< osg::ref_ptr<UpdateProperty> > Properties;

    void addUpdateProperty(UpdateProperty* up) { _properties.push_back(up); }
    
    void setProperties(const Properties& pl) { _properties = pl; }
    Properties& getProperties() { return _properties; }
    const Properties& getProperties() const { return _properties; }

    template<typename T>
    T* getPropertyOfType()
    {
        for(Properties::iterator itr = _properties.begin();
            itr != _properties.end();
            ++itr)
        {
            T* p = dynamic_cast<T*>(itr->get());
            if (p) return p;
        }
        return 0;
    }
    
    bool valid() const;

protected:
    virtual ~CaptureSettings() {}

    std::string _inputFileName;

    std::string _outputFileName;
    std::string _outputDirectoryName;
    std::string _outputBaseFileName;
    std::string _outputExtension;

    StereoMode                          _stereoMode;
    bool                                _offscreen;
    bool                                _outputImageFlip;
    
    unsigned int                        _width;
    unsigned int                        _height;
    
    float                               _screenWidth;
    float                               _screenHeight;
    float                               _screenDistance;

    PixelFormat                         _pixelFormat;
    unsigned int                        _samples;
    unsigned int                        _sampleBuffers;
    
    double                              _frameRate;
    unsigned int                        _numberOfFrames;

    EventHandlers                       _eventHandlers;
    Properties                          _properties;
    
};
    
}

#endif
