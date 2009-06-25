/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osg/DisplaySettings>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Math>
#include <osg/Notify>
#include <osg/ref_ptr>

#include <algorithm>
#include <string.h>

using namespace osg;
using namespace std;

DisplaySettings* DisplaySettings::instance()
{
    static ref_ptr<DisplaySettings> s_displaySettings = new DisplaySettings;
    return s_displaySettings.get();
}

DisplaySettings::DisplaySettings(const DisplaySettings& vs):Referenced(true)
{
    setDisplaySettings(vs);
}

DisplaySettings::~DisplaySettings()
{
}

 
 DisplaySettings& DisplaySettings::operator = (const DisplaySettings& vs)
{
    if (this==&vs) return *this;
    setDisplaySettings(vs);
    return *this;
}

void DisplaySettings::setDisplaySettings(const DisplaySettings& vs)
{
    _displayType = vs._displayType;
    _stereo = vs._stereo;
    _stereoMode = vs._stereoMode;
    _eyeSeparation = vs._eyeSeparation;
    _screenWidth = vs._screenWidth;
    _screenHeight = vs._screenHeight;
    _screenDistance = vs._screenDistance;

    _splitStereoHorizontalEyeMapping = vs._splitStereoHorizontalEyeMapping;
    _splitStereoHorizontalSeparation = vs._splitStereoHorizontalSeparation;

    _splitStereoVerticalEyeMapping = vs._splitStereoVerticalEyeMapping;
    _splitStereoVerticalSeparation = vs._splitStereoVerticalSeparation;
    
    _splitStereoAutoAdjustAspectRatio = vs._splitStereoAutoAdjustAspectRatio;

    _doubleBuffer = vs._doubleBuffer;
    _RGB = vs._RGB;
    _depthBuffer = vs._depthBuffer;
    _minimumNumberAlphaBits = vs._minimumNumberAlphaBits;
    _minimumNumberStencilBits = vs._minimumNumberStencilBits;

    _maxNumOfGraphicsContexts = vs._maxNumOfGraphicsContexts;
    _numMultiSamples = vs._numMultiSamples;
    
    _compileContextsHint = vs._compileContextsHint;
    _serializeDrawDispatch = vs._serializeDrawDispatch;
    
    _numDatabaseThreadsHint = vs._numDatabaseThreadsHint;
    _numHttpDatabaseThreadsHint = vs._numHttpDatabaseThreadsHint;
    
    _application = vs._application;
}

void DisplaySettings::merge(const DisplaySettings& vs)
{
    if (_stereo       || vs._stereo)        _stereo = true;
    
    // need to think what to do about merging the stereo mode.
    
    if (_doubleBuffer || vs._doubleBuffer)  _doubleBuffer = true;
    if (_RGB          || vs._RGB)           _RGB = true;
    if (_depthBuffer  || vs._depthBuffer)   _depthBuffer = true;
    
    if (vs._minimumNumberAlphaBits>_minimumNumberAlphaBits) _minimumNumberAlphaBits = vs._minimumNumberAlphaBits;
    if (vs._minimumNumberStencilBits>_minimumNumberStencilBits) _minimumNumberStencilBits = vs._minimumNumberStencilBits;
    if (vs._numMultiSamples>_numMultiSamples) _numMultiSamples = vs._numMultiSamples;

    if (vs._compileContextsHint) _compileContextsHint = vs._compileContextsHint;
    if (vs._serializeDrawDispatch) _serializeDrawDispatch = vs._serializeDrawDispatch;

    if (vs._numDatabaseThreadsHint>_numDatabaseThreadsHint) _numDatabaseThreadsHint = vs._numDatabaseThreadsHint;
    if (vs._numHttpDatabaseThreadsHint>_numHttpDatabaseThreadsHint) _numHttpDatabaseThreadsHint = vs._numHttpDatabaseThreadsHint;

    if (_application.empty()) _application = vs._application;
}

void DisplaySettings::setDefaults()
{
    _displayType = MONITOR;

    _stereo = false;
    _stereoMode = ANAGLYPHIC;
    _eyeSeparation = 0.05f;
    _screenWidth = 0.325f;
    _screenHeight = 0.26f;
    _screenDistance = 0.5f;

    _splitStereoHorizontalEyeMapping = LEFT_EYE_LEFT_VIEWPORT;
    _splitStereoHorizontalSeparation = 0;

    _splitStereoVerticalEyeMapping = LEFT_EYE_TOP_VIEWPORT;
    _splitStereoVerticalSeparation = 0;

    _splitStereoAutoAdjustAspectRatio = true;

    _doubleBuffer = true;
    _RGB = true;
    _depthBuffer = true;
    _minimumNumberAlphaBits = 0;
    _minimumNumberStencilBits = 0;
    _minimumNumberAccumRedBits = 0;
    _minimumNumberAccumGreenBits = 0;
    _minimumNumberAccumBlueBits = 0;
    _minimumNumberAccumAlphaBits = 0;
    
    _maxNumOfGraphicsContexts = 32;
    _numMultiSamples = 0;

    #ifdef __sgi
    // switch on anti-aliasing by default, just in case we have an Onyx :-)
    _numMultiSamples = 4;
    #endif
    
    _compileContextsHint = false;
    _serializeDrawDispatch = true;

    _numDatabaseThreadsHint = 2;
    _numHttpDatabaseThreadsHint = 1;
}

void DisplaySettings::setMaxNumberOfGraphicsContexts(unsigned int num)
{
    _maxNumOfGraphicsContexts = num;
}

unsigned int DisplaySettings::getMaxNumberOfGraphicsContexts() const
{
    // osg::notify(osg::NOTICE)<<"getMaxNumberOfGraphicsContexts()="<<_maxNumOfGraphicsContexts<<std::endl;
    return _maxNumOfGraphicsContexts;
}

void DisplaySettings::setMinimumNumAccumBits(unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha)
{
    _minimumNumberAccumRedBits = red;
    _minimumNumberAccumGreenBits = green;
    _minimumNumberAccumBlueBits = blue;
    _minimumNumberAccumAlphaBits = alpha;
}

static ApplicationUsageProxy DisplaySetting_e0(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_DISPLAY_TYPE <type>","MONITOR | POWERWALL | REALITY_CENTER | HEAD_MOUNTED_DISPLAY");
static ApplicationUsageProxy DisplaySetting_e1(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_STEREO_MODE <mode>","QUAD_BUFFER | ANAGLYPHIC | HORIZONTAL_SPLIT | VERTICAL_SPLIT | LEFT_EYE | RIGHT_EYE | VERTICAL_INTERLACE | HORIZONTAL_INTERLACE");
static ApplicationUsageProxy DisplaySetting_e2(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_STEREO <mode>","OFF | ON");
static ApplicationUsageProxy DisplaySetting_e3(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_EYE_SEPARATION <float>","physical distance between eyes");
static ApplicationUsageProxy DisplaySetting_e4(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_SCREEN_DISTANCE <float>","physical distance between eyes and screen");
static ApplicationUsageProxy DisplaySetting_e5(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_SCREEN_HEIGHT <float>","physical screen height");
static ApplicationUsageProxy DisplaySetting_e6(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_SCREEN_WIDTH <float>","physical screen width");
static ApplicationUsageProxy DisplaySetting_e7(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_SPLIT_STEREO_HORIZONTAL_EYE_MAPPING <mode>","LEFT_EYE_LEFT_VIEWPORT | LEFT_EYE_RIGHT_VIEWPORT");
static ApplicationUsageProxy DisplaySetting_e8(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_SPLIT_STEREO_HORIZONTAL_SEPARATION <float>","number of pixels between viewports");
static ApplicationUsageProxy DisplaySetting_e9(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_SPLIT_STEREO_VERTICAL_EYE_MAPPING <mode>","LEFT_EYE_TOP_VIEWPORT | LEFT_EYE_BOTTOM_VIEWPORT");
static ApplicationUsageProxy DisplaySetting_e10(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_SPLIT_STEREO_AUTO_ADJUST_ASPECT_RATIO <mode>","OFF | ON  Default to ON to compenstate for the compression of the aspect ratio when viewing in split screen stereo.  Note, if you are setting fovx and fovy explicityly OFF should be used.");
static ApplicationUsageProxy DisplaySetting_e11(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_SPLIT_STEREO_VERTICAL_SEPARATION <float>","number of pixels between viewports");
static ApplicationUsageProxy DisplaySetting_e12(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_MAX_NUMBER_OF_GRAPHICS_CONTEXTS <int>","maximum number of graphics contexts to be used with applications.");
static ApplicationUsageProxy DisplaySetting_e13(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_COMPIlE_CONTEXTS <mode>","OFF | ON Enable/disable the use a backgrouind compile contexts and threads.");
static ApplicationUsageProxy DisplaySetting_e14(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_SERIALIZE_DRAW_DISPATCH <mode>","OFF | ON Enable/disable the use a muetx to serialize the draw dispatch when there are multiple graphics threads.");
static ApplicationUsageProxy DisplaySetting_e15(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_NUM_DATABASE_THREADS <int>","Set the hint for the total number of threads to set up in the DatabasePager.");
static ApplicationUsageProxy DisplaySetting_e16(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_NUM_HTTP_DATABASE_THREADS <int>","Set the hint for the total number of threads dedicated to http requests to set up in the DatabasePager.");

void DisplaySettings::readEnvironmentalVariables()
{
    const char* ptr = 0;
    
    if ((ptr = getenv("OSG_DISPLAY_TYPE")) != 0)
    {
        if (strcmp(ptr,"MONITOR")==0)
        {
            _displayType = MONITOR;
        }
        else
        if (strcmp(ptr,"POWERWALL")==0)
        {
            _displayType = POWERWALL;
        }
        else
        if (strcmp(ptr,"REALITY_CENTER")==0)
        {
            _displayType = REALITY_CENTER;
        }
        else
        if (strcmp(ptr,"HEAD_MOUNTED_DISPLAY")==0)
        {
            _displayType = HEAD_MOUNTED_DISPLAY;
        }
    }
    
    if( (ptr = getenv("OSG_STEREO_MODE")) != 0)
    {
        if (strcmp(ptr,"QUAD_BUFFER")==0)
        {
            _stereoMode = QUAD_BUFFER;
        }
        else if (strcmp(ptr,"ANAGLYPHIC")==0)
        {
            _stereoMode = ANAGLYPHIC;
        }
        else if (strcmp(ptr,"HORIZONTAL_SPLIT")==0)
        {
            _stereoMode = HORIZONTAL_SPLIT;
        }
        else if (strcmp(ptr,"VERTICAL_SPLIT")==0)
        {
            _stereoMode = VERTICAL_SPLIT;
        }
        else if (strcmp(ptr,"LEFT_EYE")==0)
        {
            _stereoMode = LEFT_EYE;
        }
        else if (strcmp(ptr,"RIGHT_EYE")==0)
        {
            _stereoMode = RIGHT_EYE;
        }
        else if (strcmp(ptr,"HORIZONTAL_INTERLACE")==0)
        {
            _stereoMode = HORIZONTAL_INTERLACE;
        }
        else if (strcmp(ptr,"VERTICAL_INTERLACE")==0)
        {
            _stereoMode = VERTICAL_INTERLACE;
        }
        else if (strcmp(ptr,"CHECKERBOARD")==0)
        {
            _stereoMode = CHECKERBOARD;
        }
    }

    if( (ptr = getenv("OSG_STEREO")) != 0)
    {
        if (strcmp(ptr,"OFF")==0)
        {
            _stereo = false;
        }
        else
        if (strcmp(ptr,"ON")==0)
        {
            _stereo = true;
        }
    }

    if( (ptr = getenv("OSG_EYE_SEPARATION")) != 0)
    {
        _eyeSeparation = osg::asciiToFloat(ptr);
    }

    if( (ptr = getenv("OSG_SCREEN_WIDTH")) != 0)
    {
        _screenWidth = osg::asciiToFloat(ptr);
    }

    if( (ptr = getenv("OSG_SCREEN_HEIGHT")) != 0)
    {
        _screenHeight = osg::asciiToFloat(ptr);
    }

    if( (ptr = getenv("OSG_SCREEN_DISTANCE")) != 0)
    {
        _screenDistance = osg::asciiToFloat(ptr);
    }

    if( (ptr = getenv("OSG_SPLIT_STEREO_HORIZONTAL_EYE_MAPPING")) != 0)
    {
        if (strcmp(ptr,"LEFT_EYE_LEFT_VIEWPORT")==0)
        {
            _splitStereoHorizontalEyeMapping = LEFT_EYE_LEFT_VIEWPORT;
        }
        else
        if (strcmp(ptr,"LEFT_EYE_RIGHT_VIEWPORT")==0)
        {
            _splitStereoHorizontalEyeMapping = LEFT_EYE_RIGHT_VIEWPORT;
        }
    }

    if( (ptr = getenv("OSG_SPLIT_STEREO_HORIZONTAL_SEPARATION")) != 0)
    {
        _splitStereoHorizontalSeparation = atoi(ptr);
    }


    if( (ptr = getenv("OSG_SPLIT_STEREO_VERTICAL_EYE_MAPPING")) != 0)
    {
        if (strcmp(ptr,"LEFT_EYE_TOP_VIEWPORT")==0)
        {
            _splitStereoVerticalEyeMapping = LEFT_EYE_TOP_VIEWPORT;
        }
        else
        if (strcmp(ptr,"LEFT_EYE_BOTTOM_VIEWPORT")==0)
        {
            _splitStereoVerticalEyeMapping = LEFT_EYE_BOTTOM_VIEWPORT;
        }
    }
    
    if( (ptr = getenv("OSG_SPLIT_STEREO_AUTO_ADJUST_ASPECT_RATIO")) != 0)
    {
        if (strcmp(ptr,"OFF")==0)
        {
            _splitStereoAutoAdjustAspectRatio = false;
        }
        else
        if (strcmp(ptr,"ON")==0)
        {
            _splitStereoAutoAdjustAspectRatio = true;
        }
    }

    if( (ptr = getenv("OSG_SPLIT_STEREO_VERTICAL_SEPARATION")) != 0)
    {
        _splitStereoVerticalSeparation = atoi(ptr);
    }

    if( (ptr = getenv("OSG_MAX_NUMBER_OF_GRAPHICS_CONTEXTS")) != 0)
    {
        _maxNumOfGraphicsContexts = atoi(ptr);
    }

    if( (ptr = getenv("OSG_COMPIlE_CONTEXTS")) != 0)
    {
        if (strcmp(ptr,"OFF")==0)
        {
            _compileContextsHint = false;
        }
        else
        if (strcmp(ptr,"ON")==0)
        {
            _compileContextsHint = true;
        }
    }
    
    if( (ptr = getenv("OSG_SERIALIZE_DRAW_DISPATCH")) != 0)
    {
        if (strcmp(ptr,"OFF")==0)
        {
            _serializeDrawDispatch = false;
        }
        else
        if (strcmp(ptr,"ON")==0)
        {
            _serializeDrawDispatch = true;
        }
    }

    if( (ptr = getenv("OSG_NUM_DATABASE_THREADS")) != 0)
    {
        _numDatabaseThreadsHint = atoi(ptr);
    }

    if( (ptr = getenv("OSG_NUM_HTTP_DATABASE_THREADS")) != 0)
    {
        _numHttpDatabaseThreadsHint = atoi(ptr);
    }
}

void DisplaySettings::readCommandLine(ArgumentParser& arguments)
{
    if (_application.empty()) _application = arguments[0];

    // report the usage options.
    if (arguments.getApplicationUsage())
    {
        arguments.getApplicationUsage()->addCommandLineOption("--display <type>","MONITOR | POWERWALL | REALITY_CENTER | HEAD_MOUNTED_DISPLAY");
        arguments.getApplicationUsage()->addCommandLineOption("--stereo","Use default stereo mode which is ANAGLYPHIC if not overriden by environmental variable");
        arguments.getApplicationUsage()->addCommandLineOption("--stereo <mode>","ANAGLYPHIC | QUAD_BUFFER | HORIZONTAL_SPLIT | VERTICAL_SPLIT | LEFT_EYE | RIGHT_EYE | HORIZONTAL_INTERLACE | VERTICAL_INTERLACE | CHECKERBOARD | ON | OFF ");
        arguments.getApplicationUsage()->addCommandLineOption("--rgba","Request a RGBA color buffer visual");
        arguments.getApplicationUsage()->addCommandLineOption("--stencil","Request a stencil buffer visual");
        arguments.getApplicationUsage()->addCommandLineOption("--accum-rgb","Request a rgb accumulator buffer visual");
        arguments.getApplicationUsage()->addCommandLineOption("--accum-rgba","Request a rgb accumulator buffer visual");
        arguments.getApplicationUsage()->addCommandLineOption("--samples <num>","Request a multisample visual");
        arguments.getApplicationUsage()->addCommandLineOption("--cc","Request use of compile contexts and threads");
        arguments.getApplicationUsage()->addCommandLineOption("--serialize-draw <mode>","OFF | ON - set the serialization of draw dispatch");
    }

    std::string str;
    while(arguments.read("--display",str))
    {
        if (str=="MONITOR") _displayType = MONITOR;
        else if (str=="POWERWALL") _displayType = POWERWALL;
        else if (str=="REALITY_CENTER") _displayType = REALITY_CENTER;
        else if (str=="HEAD_MOUNTED_DISPLAY") _displayType = HEAD_MOUNTED_DISPLAY;
    }

    int pos;
    while ((pos=arguments.find("--stereo"))>0)
    {
        if (arguments.match(pos+1,"ANAGLYPHIC"))            { arguments.remove(pos,2); _stereo = true;_stereoMode = ANAGLYPHIC; }
        else if (arguments.match(pos+1,"QUAD_BUFFER"))      { arguments.remove(pos,2); _stereo = true;_stereoMode = QUAD_BUFFER; }
        else if (arguments.match(pos+1,"HORIZONTAL_SPLIT")) { arguments.remove(pos,2); _stereo = true;_stereoMode = HORIZONTAL_SPLIT; }
        else if (arguments.match(pos+1,"VERTICAL_SPLIT"))   { arguments.remove(pos,2); _stereo = true;_stereoMode = VERTICAL_SPLIT; }
        else if (arguments.match(pos+1,"HORIZONTAL_INTERLACE")) { arguments.remove(pos,2); _stereo = true;_stereoMode = HORIZONTAL_INTERLACE; }
        else if (arguments.match(pos+1,"VERTICAL_INTERLACE"))   { arguments.remove(pos,2); _stereo = true;_stereoMode = VERTICAL_INTERLACE; }
        else if (arguments.match(pos+1,"CHECKERBOARD"))     { arguments.remove(pos,2); _stereo = true;_stereoMode = CHECKERBOARD; }
        else if (arguments.match(pos+1,"LEFT_EYE"))         { arguments.remove(pos,2); _stereo = true;_stereoMode = LEFT_EYE; }
        else if (arguments.match(pos+1,"RIGHT_EYE"))        { arguments.remove(pos,2); _stereo = true;_stereoMode = RIGHT_EYE; }
        else if (arguments.match(pos+1,"ON"))               { arguments.remove(pos,2); _stereo = true; }
        else if (arguments.match(pos+1,"OFF"))              { arguments.remove(pos,2); _stereo = false; }
        else                                                { arguments.remove(pos); _stereo = true; }
    }

    while (arguments.read("--rgba"))
    {
        _RGB = true;
        _minimumNumberAlphaBits = 1;
    }            

    while (arguments.read("--stencil"))
    {
        _minimumNumberStencilBits = 1;
    }

    while (arguments.read("--accum-rgb"))
    {
        setMinimumNumAccumBits(8,8,8,0);
    }

    while (arguments.read("--accum-rgba"))
    {
        setMinimumNumAccumBits(8,8,8,8);
    }

    while(arguments.read("--samples",str))
    {
        _numMultiSamples = atoi(str.c_str());
    }

    while(arguments.read("--cc"))
    {
        _compileContextsHint = true;
    }

    while(arguments.read("--serialize-draw",str))
    {
        if (str=="ON") _serializeDrawDispatch = true;
        else if (str=="OFF") _serializeDrawDispatch = false;
    }

    while(arguments.read("--num-db-threads",_numDatabaseThreadsHint)) {}
    while(arguments.read("--num-http-threads",_numHttpDatabaseThreadsHint)) {}
}
