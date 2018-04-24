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
#include <osg/GL>
#include <osg/os_utils>
#include <osg/ref_ptr>

#include <algorithm>
#include <string.h>

using namespace osg;
using namespace std;

#if defined(WIN32) && !defined(__CYGWIN__)
#include<windows.h>
extern "C" { OSG_EXPORT DWORD NvOptimusEnablement=0x00000001; }
#else
extern "C" { int NvOptimusEnablement=0x00000001; }
#endif

void DisplaySettings::setNvOptimusEnablement(int value)
{
    NvOptimusEnablement = value;
}

int DisplaySettings::getNvOptimusEnablement() const
{
    return NvOptimusEnablement;
}

ref_ptr<DisplaySettings>& DisplaySettings::instance()
{
    static ref_ptr<DisplaySettings> s_displaySettings = new DisplaySettings;
    return s_displaySettings;
}

OSG_INIT_SINGLETON_PROXY(ProxyInitDisplaySettings, DisplaySettings::instance())

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
    _minimumNumberAccumRedBits = vs._minimumNumberAccumRedBits;
    _minimumNumberAccumGreenBits = vs._minimumNumberAccumGreenBits;
    _minimumNumberAccumBlueBits = vs._minimumNumberAccumBlueBits;
    _minimumNumberAccumAlphaBits = vs._minimumNumberAccumAlphaBits;

    _maxNumOfGraphicsContexts = vs._maxNumOfGraphicsContexts;
    _numMultiSamples = vs._numMultiSamples;

    _compileContextsHint = vs._compileContextsHint;
    _serializeDrawDispatch = vs._serializeDrawDispatch;
    _useSceneViewForStereoHint = vs._useSceneViewForStereoHint;

    _numDatabaseThreadsHint = vs._numDatabaseThreadsHint;
    _numHttpDatabaseThreadsHint = vs._numHttpDatabaseThreadsHint;

    _application = vs._application;

    _maxTexturePoolSize = vs._maxTexturePoolSize;
    _maxBufferObjectPoolSize = vs._maxBufferObjectPoolSize;

    _implicitBufferAttachmentRenderMask = vs._implicitBufferAttachmentRenderMask;
    _implicitBufferAttachmentResolveMask = vs._implicitBufferAttachmentResolveMask;

    _glContextVersion = vs._glContextVersion;
    _glContextFlags = vs._glContextFlags;
    _glContextProfileMask = vs._glContextProfileMask;
    _swapMethod = vs._swapMethod;

    _vertexBufferHint = vs._vertexBufferHint;

    setShaderHint(_shaderHint);

    _keystoneHint = vs._keystoneHint;
    _keystoneFileNames = vs._keystoneFileNames;
    _keystones = vs._keystones;

    _OSXMenubarBehavior = vs._OSXMenubarBehavior;

    _syncSwapBuffers = vs._syncSwapBuffers;
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
    if (vs._useSceneViewForStereoHint) _useSceneViewForStereoHint = vs._useSceneViewForStereoHint;

    if (vs._numDatabaseThreadsHint>_numDatabaseThreadsHint) _numDatabaseThreadsHint = vs._numDatabaseThreadsHint;
    if (vs._numHttpDatabaseThreadsHint>_numHttpDatabaseThreadsHint) _numHttpDatabaseThreadsHint = vs._numHttpDatabaseThreadsHint;

    if (_application.empty()) _application = vs._application;

    if (vs._maxTexturePoolSize>_maxTexturePoolSize) _maxTexturePoolSize = vs._maxTexturePoolSize;
    if (vs._maxBufferObjectPoolSize>_maxBufferObjectPoolSize) _maxBufferObjectPoolSize = vs._maxBufferObjectPoolSize;

    // these are bit masks so merging them is like logical or
    _implicitBufferAttachmentRenderMask |= vs._implicitBufferAttachmentRenderMask;
    _implicitBufferAttachmentResolveMask |= vs._implicitBufferAttachmentResolveMask;

    // merge swap method to higher value
    if( vs._swapMethod > _swapMethod )
        _swapMethod = vs._swapMethod;

    _keystoneHint = _keystoneHint | vs._keystoneHint;

    // insert any unique filenames into the local list
    for(FileNames::const_iterator itr = vs._keystoneFileNames.begin();
        itr != vs._keystoneFileNames.end();
        ++itr)
    {
        const std::string& filename = *itr;
        FileNames::iterator found_itr = std::find(_keystoneFileNames.begin(), _keystoneFileNames.end(), filename);
        if (found_itr == _keystoneFileNames.end()) _keystoneFileNames.push_back(filename);
    }

    // insert unique Keystone object into local list
    for(Objects::const_iterator itr = vs._keystones.begin();
        itr != vs._keystones.end();
        ++itr)
    {
        const osg::Object* object = itr->get();
        Objects::iterator found_itr = std::find(_keystones.begin(), _keystones.end(), object);
        if (found_itr == _keystones.end()) _keystones.push_back(const_cast<osg::Object*>(object));
    }

    if (vs._OSXMenubarBehavior > _OSXMenubarBehavior)
        _OSXMenubarBehavior = vs._OSXMenubarBehavior;
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

    _splitStereoAutoAdjustAspectRatio = false;

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
    _serializeDrawDispatch = false;
    _useSceneViewForStereoHint = true;

    _numDatabaseThreadsHint = 2;
    _numHttpDatabaseThreadsHint = 1;

    _maxTexturePoolSize = 0;
    _maxBufferObjectPoolSize = 0;

    _implicitBufferAttachmentRenderMask = DEFAULT_IMPLICIT_BUFFER_ATTACHMENT;
    _implicitBufferAttachmentResolveMask = DEFAULT_IMPLICIT_BUFFER_ATTACHMENT;
    _glContextVersion = OSG_GL_CONTEXT_VERSION;
    _glContextFlags = 0;
    _glContextProfileMask = 0;

    _swapMethod = SWAP_DEFAULT;
    _syncSwapBuffers = 0;

    _vertexBufferHint = NO_PREFERENCE;
    // _vertexBufferHint = VERTEX_BUFFER_OBJECT;
    // _vertexBufferHint = VERTEX_ARRAY_OBJECT;

#if defined(OSG_GLES3_AVAILABLE)
    setShaderHint(SHADER_GLES3);
#elif defined(OSG_GLES2_AVAILABLE)
    setShaderHint(SHADER_GLES2);
#elif defined(OSG_GL3_AVAILABLE)
    setShaderHint(SHADER_GL3);
#elif defined(OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE)
    setShaderHint(SHADER_NONE);
#else
    setShaderHint(SHADER_GL2);
#endif

    _keystoneHint = false;

    _OSXMenubarBehavior = MENUBAR_AUTO_HIDE;
}

void DisplaySettings::setMaxNumberOfGraphicsContexts(unsigned int num)
{
    _maxNumOfGraphicsContexts = num;
}

unsigned int DisplaySettings::getMaxNumberOfGraphicsContexts() const
{
    // OSG_NOTICE<<"getMaxNumberOfGraphicsContexts()="<<_maxNumOfGraphicsContexts<<std::endl;
    return _maxNumOfGraphicsContexts;
}

void DisplaySettings::setMinimumNumAccumBits(unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha)
{
    _minimumNumberAccumRedBits = red;
    _minimumNumberAccumGreenBits = green;
    _minimumNumberAccumBlueBits = blue;
    _minimumNumberAccumAlphaBits = alpha;
}

static ApplicationUsageProxy DisplaySetting_e0(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_DISPLAY_TYPE <type>",
        "MONITOR | POWERWALL | REALITY_CENTER | HEAD_MOUNTED_DISPLAY");
static ApplicationUsageProxy DisplaySetting_e1(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_STEREO_MODE <mode>",
        "QUAD_BUFFER | ANAGLYPHIC | HORIZONTAL_SPLIT | VERTICAL_SPLIT | LEFT_EYE | RIGHT_EYE | VERTICAL_INTERLACE | HORIZONTAL_INTERLACE");
static ApplicationUsageProxy DisplaySetting_e2(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_STEREO <mode>",
        "OFF | ON");
static ApplicationUsageProxy DisplaySetting_e3(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_EYE_SEPARATION <float>",
        "Physical distance between eyes.");
static ApplicationUsageProxy DisplaySetting_e4(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_SCREEN_DISTANCE <float>",
        "Physical distance between eyes and screen.");
static ApplicationUsageProxy DisplaySetting_e5(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_SCREEN_HEIGHT <float>",
        "Physical screen height.");
static ApplicationUsageProxy DisplaySetting_e6(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_SCREEN_WIDTH <float>",
        "Physical screen width.");
static ApplicationUsageProxy DisplaySetting_e7(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_SPLIT_STEREO_HORIZONTAL_EYE_MAPPING <mode>",
        "LEFT_EYE_LEFT_VIEWPORT | LEFT_EYE_RIGHT_VIEWPORT");
static ApplicationUsageProxy DisplaySetting_e8(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_SPLIT_STEREO_HORIZONTAL_SEPARATION <float>",
        "Number of pixels between viewports.");
static ApplicationUsageProxy DisplaySetting_e9(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_SPLIT_STEREO_VERTICAL_EYE_MAPPING <mode>",
        "LEFT_EYE_TOP_VIEWPORT | LEFT_EYE_BOTTOM_VIEWPORT");
static ApplicationUsageProxy DisplaySetting_e10(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_SPLIT_STEREO_AUTO_ADJUST_ASPECT_RATIO <mode>",
        "OFF | ON  Default to OFF to compenstate for the compression of the aspect ratio when viewing in split screen stereo.  Note, if you are setting fovx and fovy explicityly OFF should be used.");
static ApplicationUsageProxy DisplaySetting_e11(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_SPLIT_STEREO_VERTICAL_SEPARATION <float>",
        "Number of pixels between viewports.");
static ApplicationUsageProxy DisplaySetting_e12(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_MAX_NUMBER_OF_GRAPHICS_CONTEXTS <int>",
        "Maximum number of graphics contexts to be used with applications.");
static ApplicationUsageProxy DisplaySetting_e13(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_COMPILE_CONTEXTS <mode>",
        "OFF | ON Disable/enable the use of background compiled contexts and threads.");
static ApplicationUsageProxy DisplaySetting_e14(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_SERIALIZE_DRAW_DISPATCH <mode>",
        "OFF | ON Disable/enable the use of a mutex to serialize the draw dispatch when there are multiple graphics threads.");
static ApplicationUsageProxy DisplaySetting_e15(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_USE_SCENEVIEW_FOR_STEREO <mode>",
        "OFF | ON Disable/enable the hint to use osgUtil::SceneView to implement stereo when required..");
static ApplicationUsageProxy DisplaySetting_e16(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_NUM_DATABASE_THREADS <int>",
        "Set the hint for the total number of threads to set up in the DatabasePager.");
static ApplicationUsageProxy DisplaySetting_e17(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_NUM_HTTP_DATABASE_THREADS <int>",
        "Set the hint for the total number of threads dedicated to http requests to set up in the DatabasePager.");
static ApplicationUsageProxy DisplaySetting_e18(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_MULTI_SAMPLES <int>",
        "Set the hint for the number of samples to use when multi-sampling.");
static ApplicationUsageProxy DisplaySetting_e19(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_TEXTURE_POOL_SIZE <int>",
        "Set the hint for the size of the texture pool to manage.");
static ApplicationUsageProxy DisplaySetting_e20(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_BUFFER_OBJECT_POOL_SIZE <int>",
        "Set the hint for the size of the vertex buffer object pool to manage.");
static ApplicationUsageProxy DisplaySetting_e21(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_FBO_POOL_SIZE <int>",
        "Set the hint for the size of the frame buffer object pool to manage.");
static ApplicationUsageProxy DisplaySetting_e22(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_IMPLICIT_BUFFER_ATTACHMENT_RENDER_MASK",
        "OFF | DEFAULT | [~]COLOR | [~]DEPTH | [~]STENCIL. Substitute missing buffer attachments for render FBO.");
static ApplicationUsageProxy DisplaySetting_e23(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_IMPLICIT_BUFFER_ATTACHMENT_RESOLVE_MASK",
        "OFF | DEFAULT | [~]COLOR | [~]DEPTH | [~]STENCIL. Substitute missing buffer attachments for resolve FBO.");
static ApplicationUsageProxy DisplaySetting_e24(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_GL_CONTEXT_VERSION <major.minor>",
        "Set the hint for the GL version to create contexts for.");
static ApplicationUsageProxy DisplaySetting_e25(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_GL_CONTEXT_FLAGS <uint>",
        "Set the hint for the GL context flags to use when creating contexts.");
static ApplicationUsageProxy DisplaySetting_e26(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_GL_CONTEXT_PROFILE_MASK <uint>",
        "Set the hint for the GL context profile mask to use when creating contexts.");
static ApplicationUsageProxy DisplaySetting_e27(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_SWAP_METHOD <method>",
        "DEFAULT | EXCHANGE | COPY | UNDEFINED. Select preferred swap method.");
static ApplicationUsageProxy DisplaySetting_e28(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_KEYSTONE ON | OFF",
        "Specify the hint to whether the viewer should set up keystone correction.");
static ApplicationUsageProxy DisplaySetting_e29(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_KEYSTONE_FILES <filename>[:filename]..",
        "Specify filenames of keystone parameter files. Under Windows use ; to deliminate files, otherwise use :");
static ApplicationUsageProxy DisplaySetting_e30(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_MENUBAR_BEHAVIOR <behavior>",
        "OSX Only : Specify the behavior of the menubar (AUTO_HIDE, FORCE_HIDE, FORCE_SHOW)");
static ApplicationUsageProxy DisplaySetting_e31(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_NvOptimusEnablement <value>",
        "Set the hint to NvOptimus of whether to enable it or not, set 1 to enable, 0 to disable");
static ApplicationUsageProxy DisplaySetting_e32(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_VERTEX_BUFFER_HINT <value>",
        "Set the hint to what backend osg::Geometry implementation to use. NO_PREFERENCE | VERTEX_BUFFER_OBJECT | VERTEX_ARRAY_OBJECT");
static ApplicationUsageProxy DisplaySetting_e33(ApplicationUsage::ENVIRONMENTAL_VARIABLE,
        "OSG_TEXT_SHADER_TECHNIQUE <value>",
        "Set the defafult osgText::ShaderTechnique. ALL_FEATURES | ALL | GREYSCALE | SIGNED_DISTANCE_FIELD | SDF | NO_TEXT_SHADER | NONE");

void DisplaySettings::readEnvironmentalVariables()
{
    std::string value;
    if (getEnvVar("OSG_DISPLAY_TYPE", value))
    {
        if (value=="MONITOR")
        {
            _displayType = MONITOR;
        }
        else
        if (value=="POWERWALL")
        {
            _displayType = POWERWALL;
        }
        else
        if (value=="REALITY_CENTER")
        {
            _displayType = REALITY_CENTER;
        }
        else
        if (value=="HEAD_MOUNTED_DISPLAY")
        {
            _displayType = HEAD_MOUNTED_DISPLAY;
        }
    }

    if (getEnvVar("OSG_STEREO_MODE", value))
    {
        if (value=="QUAD_BUFFER")
        {
            _stereoMode = QUAD_BUFFER;
        }
        else if (value=="ANAGLYPHIC")
        {
            _stereoMode = ANAGLYPHIC;
        }
        else if (value=="HORIZONTAL_SPLIT")
        {
            _stereoMode = HORIZONTAL_SPLIT;
        }
        else if (value=="VERTICAL_SPLIT")
        {
            _stereoMode = VERTICAL_SPLIT;
        }
        else if (value=="LEFT_EYE")
        {
            _stereoMode = LEFT_EYE;
        }
        else if (value=="RIGHT_EYE")
        {
            _stereoMode = RIGHT_EYE;
        }
        else if (value=="HORIZONTAL_INTERLACE")
        {
            _stereoMode = HORIZONTAL_INTERLACE;
        }
        else if (value=="VERTICAL_INTERLACE")
        {
            _stereoMode = VERTICAL_INTERLACE;
        }
        else if (value=="CHECKERBOARD")
        {
            _stereoMode = CHECKERBOARD;
        }
    }

    if (getEnvVar("OSG_STEREO", value))
    {
        if (value=="OFF")
        {
            _stereo = false;
        }
        else
        if (value=="ON")
        {
            _stereo = true;
        }
    }

    getEnvVar("OSG_EYE_SEPARATION", _eyeSeparation);
    getEnvVar("OSG_SCREEN_WIDTH", _screenWidth);
    getEnvVar("OSG_SCREEN_HEIGHT", _screenHeight);
    getEnvVar("OSG_SCREEN_DISTANCE", _screenDistance);

    if (getEnvVar("OSG_SPLIT_STEREO_HORIZONTAL_EYE_MAPPING", value))
    {
        if (value=="LEFT_EYE_LEFT_VIEWPORT")
        {
            _splitStereoHorizontalEyeMapping = LEFT_EYE_LEFT_VIEWPORT;
        }
        else
        if (value=="LEFT_EYE_RIGHT_VIEWPORT")
        {
            _splitStereoHorizontalEyeMapping = LEFT_EYE_RIGHT_VIEWPORT;
        }
    }

    getEnvVar("OSG_SPLIT_STEREO_HORIZONTAL_SEPARATION", _splitStereoHorizontalSeparation);


    if (getEnvVar("OSG_SPLIT_STEREO_VERTICAL_EYE_MAPPING", value))
    {
        if (value=="LEFT_EYE_TOP_VIEWPORT")
        {
            _splitStereoVerticalEyeMapping = LEFT_EYE_TOP_VIEWPORT;
        }
        else
        if (value=="LEFT_EYE_BOTTOM_VIEWPORT")
        {
            _splitStereoVerticalEyeMapping = LEFT_EYE_BOTTOM_VIEWPORT;
        }
    }

    if (getEnvVar("OSG_SPLIT_STEREO_AUTO_ADJUST_ASPECT_RATIO", value))
    {
        if (value=="OFF")
        {
            _splitStereoAutoAdjustAspectRatio = false;
        }
        else
        if (value=="ON")
        {
            _splitStereoAutoAdjustAspectRatio = true;
        }
    }

    getEnvVar("OSG_SPLIT_STEREO_VERTICAL_SEPARATION", _splitStereoVerticalSeparation);

    getEnvVar("OSG_MAX_NUMBER_OF_GRAPHICS_CONTEXTS", _maxNumOfGraphicsContexts);

    if (getEnvVar("OSG_COMPILE_CONTEXTS", value))
    {
        if (value=="OFF")
        {
            _compileContextsHint = false;
        }
        else
        if (value=="ON")
        {
            _compileContextsHint = true;
        }
    }

    if (getEnvVar("OSG_SERIALIZE_DRAW_DISPATCH", value))
    {
        if (value=="OFF")
        {
            _serializeDrawDispatch = false;
        }
        else
        if (value=="ON")
        {
            _serializeDrawDispatch = true;
        }
    }

    if (getEnvVar("OSG_USE_SCENEVIEW_FOR_STEREO", value))
    {
        if (value=="OFF")
        {
            _useSceneViewForStereoHint = false;
        }
        else
        if (value=="ON")
        {
            _useSceneViewForStereoHint = true;
        }
    }

    getEnvVar("OSG_NUM_DATABASE_THREADS", _numDatabaseThreadsHint);

    getEnvVar("OSG_NUM_HTTP_DATABASE_THREADS", _numHttpDatabaseThreadsHint);

    getEnvVar("OSG_MULTI_SAMPLES", _numMultiSamples);

    getEnvVar("OSG_TEXTURE_POOL_SIZE", _maxTexturePoolSize);

    getEnvVar("OSG_BUFFER_OBJECT_POOL_SIZE", _maxBufferObjectPoolSize);


    {  // Read implicit buffer attachments combinations for both render and resolve mask
        const char * variable[] = {
            "OSG_IMPLICIT_BUFFER_ATTACHMENT_RENDER_MASK",
            "OSG_IMPLICIT_BUFFER_ATTACHMENT_RESOLVE_MASK",
        };

        int * mask[] = {
            &_implicitBufferAttachmentRenderMask,
            &_implicitBufferAttachmentResolveMask,
        };

        for( unsigned int n = 0; n < sizeof( variable ) / sizeof( variable[0] ); n++ )
        {
            std::string str;
            if (getEnvVar(variable[n], str))
            {
                if(str.find("OFF")!=std::string::npos) *mask[n] = 0;

                if(str.find("~DEFAULT")!=std::string::npos) *mask[n] ^= DEFAULT_IMPLICIT_BUFFER_ATTACHMENT;
                else if(str.find("DEFAULT")!=std::string::npos) *mask[n] |= DEFAULT_IMPLICIT_BUFFER_ATTACHMENT;

                if(str.find("~COLOR")!=std::string::npos) *mask[n] ^= IMPLICIT_COLOR_BUFFER_ATTACHMENT;
                else if(str.find("COLOR")!=std::string::npos) *mask[n] |= IMPLICIT_COLOR_BUFFER_ATTACHMENT;

                if(str.find("~DEPTH")!=std::string::npos) *mask[n] ^= IMPLICIT_DEPTH_BUFFER_ATTACHMENT;
                else if(str.find("DEPTH")!=std::string::npos) *mask[n] |= (int)IMPLICIT_DEPTH_BUFFER_ATTACHMENT;

                if(str.find("~STENCIL")!=std::string::npos) *mask[n] ^= (int)IMPLICIT_STENCIL_BUFFER_ATTACHMENT;
                else if(str.find("STENCIL")!=std::string::npos) *mask[n] |= (int)IMPLICIT_STENCIL_BUFFER_ATTACHMENT;
            }
        }
    }

    if (getEnvVar("OSG_GL_VERSION", value) || getEnvVar("OSG_GL_CONTEXT_VERSION", value))
    {
        _glContextVersion = value;
    }

    getEnvVar("OSG_GL_CONTEXT_FLAGS", _glContextFlags);

    getEnvVar("OSG_GL_CONTEXT_PROFILE_MASK", _glContextProfileMask);

    if (getEnvVar("OSG_SWAP_METHOD", value))
    {
        if (value=="DEFAULT")
        {
            _swapMethod = SWAP_DEFAULT;
        }
        else
        if (value=="EXCHANGE")
        {
            _swapMethod = SWAP_EXCHANGE;
        }
        else
        if (value=="COPY")
        {
            _swapMethod = SWAP_COPY;
        }
        else
        if (value=="UNDEFINED")
        {
            _swapMethod = SWAP_UNDEFINED;
        }

    }

    if (getEnvVar("OSG_SYNC_SWAP_BUFFERS", value))
    {
        if (value=="OFF")
        {
            _syncSwapBuffers = 0;
        }
        else
        if (value=="ON")
        {
            _syncSwapBuffers = 1;
        }
        else
        {
            _syncSwapBuffers = atoi(value.c_str());
        }
    }


    if (getEnvVar("OSG_VERTEX_BUFFER_HINT", value))
    {
        if (value=="VERTEX_BUFFER_OBJECT" || value=="VBO")
        {
            OSG_INFO<<"OSG_VERTEX_BUFFER_HINT set to VERTEX_BUFFER_OBJECT"<<std::endl;
            _vertexBufferHint = VERTEX_BUFFER_OBJECT;
        }
        else if (value=="VERTEX_ARRAY_OBJECT" || value=="VAO")
        {
            OSG_INFO<<"OSG_VERTEX_BUFFER_HINT set to VERTEX_ARRAY_OBJECT"<<std::endl;
            _vertexBufferHint = VERTEX_ARRAY_OBJECT;
        }
        else
        {
            OSG_INFO<<"OSG_VERTEX_BUFFER_HINT set to NO_PREFERENCE"<<std::endl;
            _vertexBufferHint = NO_PREFERENCE;
        }
    }


    if (getEnvVar("OSG_SHADER_HINT", value))
    {
        if (value=="GL2")
        {
            setShaderHint(SHADER_GL2);
        }
        else if (value=="GL3")
        {
            setShaderHint(SHADER_GL3);
        }
        else if (value=="GLES2")
        {
            setShaderHint(SHADER_GLES2);
        }
        else if (value=="GLES3")
        {
            setShaderHint(SHADER_GLES3);
        }
        else if (value=="NONE")
        {
            setShaderHint(SHADER_NONE);
        }
    }

    if (getEnvVar("OSG_TEXT_SHADER_TECHNIQUE", value))
    {
        setTextShaderTechnique(value);
    }

    if (getEnvVar("OSG_KEYSTONE", value))
    {
        if (value=="OFF")
        {
            _keystoneHint = false;
        }
        else
        if (value=="ON")
        {
            _keystoneHint = true;
        }
    }


    if (getEnvVar("OSG_KEYSTONE_FILES", value))
    {
    #if defined(WIN32) && !defined(__CYGWIN__)
        char delimitor = ';';
    #else
        char delimitor = ':';
    #endif

        std::string paths(value);
        if (!paths.empty())
        {
            std::string::size_type start = 0;
            std::string::size_type end;
            while ((end = paths.find_first_of(delimitor,start))!=std::string::npos)
            {
                _keystoneFileNames.push_back(std::string(paths,start,end-start));
                start = end+1;
            }

            std::string lastPath(paths,start,std::string::npos);
            if (!lastPath.empty())
                _keystoneFileNames.push_back(lastPath);
        }
    }

    if (getEnvVar("OSG_MENUBAR_BEHAVIOR", value))
    {
        if (value=="AUTO_HIDE")
        {
            _OSXMenubarBehavior = MENUBAR_AUTO_HIDE;
        }
        else
        if (value=="FORCE_HIDE")
        {
            _OSXMenubarBehavior = MENUBAR_FORCE_HIDE;
        }
        else
        if (value=="FORCE_SHOW")
        {
            _OSXMenubarBehavior = MENUBAR_FORCE_SHOW;
        }
    }

    int enable = 0;
    if (getEnvVar("OSG_NvOptimusEnablement", enable))
    {
        setNvOptimusEnablement(enable);
    }
}

void DisplaySettings::readCommandLine(ArgumentParser& arguments)
{
    if (_application.empty()) _application = arguments[0];

    // report the usage options.
    if (arguments.getApplicationUsage())
    {
        arguments.getApplicationUsage()->addCommandLineOption("--display <type>","MONITOR | POWERWALL | REALITY_CENTER | HEAD_MOUNTED_DISPLAY");
        arguments.getApplicationUsage()->addCommandLineOption("--stereo","Use default stereo mode which is ANAGLYPHIC if not overridden by environmental variable");
        arguments.getApplicationUsage()->addCommandLineOption("--stereo <mode>","ANAGLYPHIC | QUAD_BUFFER | HORIZONTAL_SPLIT | VERTICAL_SPLIT | LEFT_EYE | RIGHT_EYE | HORIZONTAL_INTERLACE | VERTICAL_INTERLACE | CHECKERBOARD | ON | OFF ");
        arguments.getApplicationUsage()->addCommandLineOption("--rgba","Request a RGBA color buffer visual");
        arguments.getApplicationUsage()->addCommandLineOption("--stencil","Request a stencil buffer visual");
        arguments.getApplicationUsage()->addCommandLineOption("--accum-rgb","Request a rgb accumulator buffer visual");
        arguments.getApplicationUsage()->addCommandLineOption("--accum-rgba","Request a rgb accumulator buffer visual");
        arguments.getApplicationUsage()->addCommandLineOption("--samples <num>","Request a multisample visual");
        arguments.getApplicationUsage()->addCommandLineOption("--cc","Request use of compile contexts and threads");
        arguments.getApplicationUsage()->addCommandLineOption("--serialize-draw <mode>","OFF | ON - set the serialization of draw dispatch");
        arguments.getApplicationUsage()->addCommandLineOption("--implicit-buffer-attachment-render-mask","OFF | DEFAULT | [~]COLOR | [~]DEPTH | [~]STENCIL. Substitute missing buffer attachments for render FBO");
        arguments.getApplicationUsage()->addCommandLineOption("--implicit-buffer-attachment-resolve-mask","OFF | DEFAULT | [~]COLOR | [~]DEPTH | [~]STENCIL. Substitute missing buffer attachments for resolve FBO");
        arguments.getApplicationUsage()->addCommandLineOption("--gl-version <major.minor>","Set the hint of which GL version to use when creating graphics contexts.");
        arguments.getApplicationUsage()->addCommandLineOption("--gl-flags <mask>","Set the hint of which GL flags projfile mask to use when creating graphics contexts.");
        arguments.getApplicationUsage()->addCommandLineOption("--gl-profile-mask <mask>","Set the hint of which GL context profile mask to use when creating graphics contexts.");
        arguments.getApplicationUsage()->addCommandLineOption("--swap-method <method>","DEFAULT | EXCHANGE | COPY | UNDEFINED. Select preferred swap method.");
        arguments.getApplicationUsage()->addCommandLineOption("--keystone <filename>","Specify a keystone file to be used by the viewer for keystone correction.");
        arguments.getApplicationUsage()->addCommandLineOption("--keystone-on","Set the keystone hint to true to tell the viewer to do keystone correction.");
        arguments.getApplicationUsage()->addCommandLineOption("--keystone-off","Set the keystone hint to false.");
        arguments.getApplicationUsage()->addCommandLineOption("--menubar-behavior <behavior>","Set the menubar behavior (AUTO_HIDE | FORCE_HIDE | FORCE_SHOW)");
        arguments.getApplicationUsage()->addCommandLineOption("--sync","Enable sync of swap buffers");
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

    while(arguments.read("--sync"))
    {
        _syncSwapBuffers = 1;
    }

    if (arguments.read("--keystone",str))
    {
        _keystoneHint = true;

        if (!_keystoneFileNames.empty()) _keystoneFileNames.clear();
        _keystoneFileNames.push_back(str);

        while(arguments.read("--keystone",str))
        {
            _keystoneFileNames.push_back(str);
        }
    }

    if (arguments.read("--keystone-on"))
    {
        _keystoneHint = true;
    }

    if (arguments.read("--keystone-off"))
    {
        _keystoneHint = false;
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

    while(arguments.read("--texture-pool-size",_maxTexturePoolSize)) {}
    while(arguments.read("--buffer-object-pool-size",_maxBufferObjectPoolSize)) {}

    {  // Read implicit buffer attachments combinations for both render and resolve mask
        const char* option[] = {
            "--implicit-buffer-attachment-render-mask",
            "--implicit-buffer-attachment-resolve-mask",
        };

        int * mask[] = {
            &_implicitBufferAttachmentRenderMask,
            &_implicitBufferAttachmentResolveMask,
        };

        for( unsigned int n = 0; n < sizeof( option ) / sizeof( option[0]); n++ )
        {
            while(arguments.read( option[n],str))
            {
                if(str.find("OFF")!=std::string::npos) *mask[n] = 0;

                if(str.find("~DEFAULT")!=std::string::npos) *mask[n] ^= DEFAULT_IMPLICIT_BUFFER_ATTACHMENT;
                else if(str.find("DEFAULT")!=std::string::npos) *mask[n] |= DEFAULT_IMPLICIT_BUFFER_ATTACHMENT;

                if(str.find("~COLOR")!=std::string::npos) *mask[n] ^= IMPLICIT_COLOR_BUFFER_ATTACHMENT;
                else if(str.find("COLOR")!=std::string::npos) *mask[n] |= IMPLICIT_COLOR_BUFFER_ATTACHMENT;

                if(str.find("~DEPTH")!=std::string::npos) *mask[n] ^= IMPLICIT_DEPTH_BUFFER_ATTACHMENT;
                else if(str.find("DEPTH")!=std::string::npos) *mask[n] |= IMPLICIT_DEPTH_BUFFER_ATTACHMENT;

                if(str.find("~STENCIL")!=std::string::npos) *mask[n] ^= IMPLICIT_STENCIL_BUFFER_ATTACHMENT;
                else if(str.find("STENCIL")!=std::string::npos) *mask[n] |= IMPLICIT_STENCIL_BUFFER_ATTACHMENT;
            }
        }
    }

    while (arguments.read("--gl-version", _glContextVersion)) {}
    while (arguments.read("--gl-flags", _glContextFlags)) {}
    while (arguments.read("--gl-profile-mask", _glContextProfileMask)) {}

    while(arguments.read("--swap-method",str))
    {
        if (str=="DEFAULT") _swapMethod = SWAP_DEFAULT;
        else if (str=="EXCHANGE") _swapMethod = SWAP_EXCHANGE;
        else if (str=="COPY") _swapMethod = SWAP_COPY;
        else if (str=="UNDEFINED") _swapMethod = SWAP_UNDEFINED;
    }

    while(arguments.read("--menubar-behavior",str))
    {
        if (str=="AUTO_HIDE") _OSXMenubarBehavior = MENUBAR_AUTO_HIDE;
        else if (str=="FORCE_HIDE") _OSXMenubarBehavior = MENUBAR_FORCE_HIDE;
        else if (str=="FORCE_SHOW") _OSXMenubarBehavior = MENUBAR_FORCE_SHOW;
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Helper funciotns for computing projection and view matrices of left and right eyes
//
osg::Matrixd DisplaySettings::computeLeftEyeProjectionImplementation(const osg::Matrixd& projection) const
{
    double iod = getEyeSeparation();
    double sd = getScreenDistance();
    double scale_x = 1.0;
    double scale_y = 1.0;

    if (getSplitStereoAutoAdjustAspectRatio())
    {
        switch(getStereoMode())
        {
            case(HORIZONTAL_SPLIT):
                scale_x = 2.0;
                break;
            case(VERTICAL_SPLIT):
                scale_y = 2.0;
                break;
            default:
                break;
        }
    }

    if (getDisplayType()==HEAD_MOUNTED_DISPLAY)
    {
        // head mounted display has the same projection matrix for left and right eyes.
        return osg::Matrixd::scale(scale_x,scale_y,1.0) *
               projection;
    }
    else
    {
        // all other display types assume working like a projected power wall
        // need to shjear projection matrix to account for asymetric frustum due to eye offset.
        return osg::Matrixd(1.0,0.0,0.0,0.0,
                           0.0,1.0,0.0,0.0,
                           iod/(2.0*sd),0.0,1.0,0.0,
                           0.0,0.0,0.0,1.0) *
               osg::Matrixd::scale(scale_x,scale_y,1.0) *
               projection;
    }
}

osg::Matrixd DisplaySettings::computeLeftEyeViewImplementation(const osg::Matrixd& view, double eyeSeperationScale) const
{
    double iod = getEyeSeparation();
    double es = 0.5f*iod*eyeSeperationScale;

    return view *
           osg::Matrixd(1.0,0.0,0.0,0.0,
                       0.0,1.0,0.0,0.0,
                       0.0,0.0,1.0,0.0,
                       es,0.0,0.0,1.0);
}

osg::Matrixd DisplaySettings::computeRightEyeProjectionImplementation(const osg::Matrixd& projection) const
{
    double iod = getEyeSeparation();
    double sd = getScreenDistance();
    double scale_x = 1.0;
    double scale_y = 1.0;

    if (getSplitStereoAutoAdjustAspectRatio())
    {
        switch(getStereoMode())
        {
            case(HORIZONTAL_SPLIT):
                scale_x = 2.0;
                break;
            case(VERTICAL_SPLIT):
                scale_y = 2.0;
                break;
            default:
                break;
        }
    }

    if (getDisplayType()==HEAD_MOUNTED_DISPLAY)
    {
        // head mounted display has the same projection matrix for left and right eyes.
        return osg::Matrixd::scale(scale_x,scale_y,1.0) *
               projection;
    }
    else
    {
        // all other display types assume working like a projected power wall
        // need to shjear projection matrix to account for asymetric frustum due to eye offset.
        return osg::Matrixd(1.0,0.0,0.0,0.0,
                           0.0,1.0,0.0,0.0,
                           -iod/(2.0*sd),0.0,1.0,0.0,
                           0.0,0.0,0.0,1.0) *
               osg::Matrixd::scale(scale_x,scale_y,1.0) *
               projection;
    }
}

osg::Matrixd DisplaySettings::computeRightEyeViewImplementation(const osg::Matrixd& view, double eyeSeperationScale) const
{
    double iod = getEyeSeparation();
    double es = 0.5*iod*eyeSeperationScale;

    return view *
           osg::Matrixd(1.0,0.0,0.0,0.0,
                       0.0,1.0,0.0,0.0,
                       0.0,0.0,1.0,0.0,
                       -es,0.0,0.0,1.0);
}

void DisplaySettings::setShaderHint(ShaderHint hint, bool setShaderValues)
{
    _shaderHint = hint;
    if (setShaderValues)
    {
        switch(_shaderHint)
        {
        case(SHADER_GLES3) :
            setValue("OSG_GLSL_VERSION", "#version 300 es");
            setValue("OSG_PRECISION_FLOAT", "precision highp float;");
            setValue("OSG_VARYING_IN", "in");
            setValue("OSG_VARYING_OUT", "out");
            OSG_INFO<<"DisplaySettings::SHADER_GLES3"<<std::endl;
            break;
        case(SHADER_GLES2) :
            setValue("OSG_GLSL_VERSION", "");
            setValue("OSG_PRECISION_FLOAT", "precision highp float;");
            setValue("OSG_VARYING_IN", "varying");
            setValue("OSG_VARYING_OUT", "varying");
            OSG_INFO<<"DisplaySettings::SHADER_GLES2"<<std::endl;
            break;
        case(SHADER_GL3) :
            setValue("OSG_GLSL_VERSION", "#version 330");
            setValue("OSG_PRECISION_FLOAT", "");
            setValue("OSG_VARYING_IN", "in");
            setValue("OSG_VARYING_OUT", "out");
            OSG_INFO<<"DisplaySettings::SHADER_GL3"<<std::endl;
            break;
        case(SHADER_GL2) :
            setValue("OSG_GLSL_VERSION", "");
            setValue("OSG_PRECISION_FLOAT", "");
            setValue("OSG_VARYING_IN", "varying");
            setValue("OSG_VARYING_OUT", "varying");
            OSG_INFO<<"DisplaySettings::SHADER_GL2"<<std::endl;
            break;
        case(SHADER_NONE) :
            setValue("OSG_GLSL_VERSION", "");
            setValue("OSG_PRECISION_FLOAT", "");
            setValue("OSG_VARYING_IN", "varying");
            setValue("OSG_VARYING_OUT", "varying");
            OSG_INFO<<"DisplaySettings::NONE"<<std::endl;
            break;
        }
    }
}

void DisplaySettings::setValue(const std::string& name, const std::string& value)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_valueMapMutex);

    _valueMap[name] = value;
}

bool DisplaySettings::getValue(const std::string& name, std::string& value, bool use_env_fallback) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_valueMapMutex);

    ValueMap::iterator itr = _valueMap.find(name);
    if (itr!=_valueMap.end())
    {
        value = itr->second;
        OSG_INFO<<"DisplaySettings::getValue("<<name<<") found existing value = ["<<value<<"]"<<std::endl;
        return true;
    }

    if (!use_env_fallback) return false;

    std::string str;
    if (getEnvVar(name.c_str(), str))
    {
        OSG_INFO<<"DisplaySettings::getValue("<<name<<") found getEnvVar value = ["<<value<<"]"<<std::endl;
        _valueMap[name] = value = str;
        return true;

    }
    else
    {
        return false;
    }
}
