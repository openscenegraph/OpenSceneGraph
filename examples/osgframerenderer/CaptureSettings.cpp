#include "CaptureSettings.h"

using namespace gsc;

CaptureSettings::CaptureSettings():
    _stereoMode(OFF),
    _offscreen(false),
    _outputImageFlip(false),
    _width(1024),
    _height(512),
    _screenWidth(0.0),
    _screenHeight(0.0),
    _screenDistance(0.0),
    _samples(0),
    _sampleBuffers(0),
    _frameRate(60.0),
    _numberOfFrames(0.0)
{
}

CaptureSettings::CaptureSettings(const CaptureSettings& cs, const osg::CopyOp& copyop):
    osg::Object(cs, copyop),
    _inputFileName(cs._inputFileName),
    _outputFileName(cs._outputFileName),
    _outputDirectoryName(cs._outputDirectoryName),
    _outputBaseFileName(cs._outputBaseFileName),
    _outputExtension(cs._outputExtension),
    _stereoMode(cs._stereoMode),
    _offscreen(cs._offscreen),
    _outputImageFlip(cs._outputImageFlip),
    _width(cs._width),
    _height(cs._height),
    _screenWidth(cs._screenWidth),
    _screenHeight(cs._screenHeight),
    _screenDistance(cs._screenDistance),
    _samples(cs._samples),
    _sampleBuffers(cs._sampleBuffers),
    _frameRate(cs._frameRate),
    _numberOfFrames(cs._numberOfFrames),
    _eventHandlers(cs._eventHandlers),
    _properties(cs._properties)
{
}

void CaptureSettings::setOutputFileName(const std::string& filename)
{
    _outputFileName = filename;
    
    _outputDirectoryName = osgDB::getFilePath(filename);
    if (!_outputDirectoryName.empty()) _outputDirectoryName += osgDB::getNativePathSeparator();
    
    _outputBaseFileName = osgDB::getStrippedName(filename);
    
    _outputExtension = osgDB::getFileExtensionIncludingDot(filename);
}

const std::string& CaptureSettings::getOutputFileName() const
{
    return _outputFileName;
}

std::string CaptureSettings::getOutputFileName(unsigned int frameNumber) const
{
    std::stringstream str;
    str<<_outputDirectoryName<<_outputBaseFileName<<"_"<<frameNumber<<_outputExtension;
    return str.str();
}
std::string CaptureSettings::getOutputFileName(unsigned int cameraNum, unsigned int frameNumber) const
{
    std::stringstream str;
    str<<_outputDirectoryName<<_outputBaseFileName<<"_"<<cameraNum<<"_"<<frameNumber<<_outputExtension;
    return str.str();
}

bool CaptureSettings::valid() const
{
    return _numberOfFrames>0 && !_outputBaseFileName.empty() && !_outputExtension.empty() && !_inputFileName.empty();
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// Serialization support
//
static bool checkEventHandlers( const gsc::CaptureSettings& cs )
{
    return !cs.getEventHandlers().empty();
}

static bool readEventHandlers( osgDB::InputStream& is, gsc::CaptureSettings& cs )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::ref_ptr<osg::Object> obj = is.readObject();
        gsc::UpdateProperty* up = dynamic_cast<gsc::UpdateProperty*>( obj.get() );
        if ( up ) cs.addUpdateProperty( up );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeEventHandlers( osgDB::OutputStream& os, const gsc::CaptureSettings& cs )
{
    const gsc::CaptureSettings::EventHandlers& pl = cs.getEventHandlers();
    unsigned int size = pl.size();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << pl[i].get();
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

static bool checkProperties( const gsc::CaptureSettings& cs )
{
    return !cs.getProperties().empty();
}

static bool readProperties( osgDB::InputStream& is, gsc::CaptureSettings& cs )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::ref_ptr<osg::Object> obj = is.readObject();
        gsc::UpdateProperty* up = dynamic_cast<gsc::UpdateProperty*>( obj.get() );
        if ( up ) cs.addUpdateProperty( up );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeProperties( osgDB::OutputStream& os, const gsc::CaptureSettings& cs )
{
    const gsc::CaptureSettings::Properties& pl = cs.getProperties();
    unsigned int size = pl.size();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << pl[i].get();
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( gsc_CaptureSettings,
                         new gsc::CaptureSettings,
                         gsc::CaptureSettings,
                         "osg::Object gsc::CaptureSettings" )
{
    ADD_STRING_SERIALIZER( InputFileName, "" );
    ADD_STRING_SERIALIZER( OutputFileName, "" );
    ADD_DOUBLE_SERIALIZER( FrameRate, 60.0 );

    BEGIN_ENUM_SERIALIZER( StereoMode, OFF );
        ADD_ENUM_VALUE( OFF );
        ADD_ENUM_VALUE( HORIZONTAL_SPLIT );
        ADD_ENUM_VALUE( VERTICAL_SPLIT );
    END_ENUM_SERIALIZER();

    ADD_BOOL_SERIALIZER( Offscreen, false );
    ADD_BOOL_SERIALIZER( OutputImageFlip, false );

    ADD_UINT_SERIALIZER( Width, 1024 );
    ADD_UINT_SERIALIZER( Height, 512 );

    ADD_FLOAT_SERIALIZER( ScreenWidth, 0.0 );
    ADD_FLOAT_SERIALIZER( ScreenHeight, 0.0 );
    ADD_FLOAT_SERIALIZER( ScreenDistance, 0.0 );

    BEGIN_ENUM_SERIALIZER( PixelFormat, RGB );
        ADD_ENUM_VALUE( RGB );
        ADD_ENUM_VALUE( RGBA );
    END_ENUM_SERIALIZER();
    
    ADD_UINT_SERIALIZER( Samples, 0 );
    ADD_UINT_SERIALIZER( SampleBuffers, 0 );
    
    ADD_UINT_SERIALIZER( NumberOfFrames, 0 );
    ADD_USER_SERIALIZER( EventHandlers );
    ADD_USER_SERIALIZER( Properties );


    
}

