/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osg/ref_ptr>

#include <algorithm>

using namespace osg;
using namespace std;

DisplaySettings* DisplaySettings::instance()
{
    static ref_ptr<DisplaySettings> s_displaySettings = new DisplaySettings;
    return s_displaySettings.get();
}

DisplaySettings::DisplaySettings(const DisplaySettings& vs):Referenced()
{
    copy(vs);
}

DisplaySettings::~DisplaySettings()
{
}

 
 DisplaySettings& DisplaySettings::operator = (const DisplaySettings& vs)
{
    if (this==&vs) return *this;
    copy(vs);
    return *this;
}

void DisplaySettings::copy(const DisplaySettings& vs)
{
    _stereoMode = vs._stereoMode;
    _eyeSeparation = vs._eyeSeparation;
    _screenDistance = vs._screenDistance;
    _screenHeight = vs._screenHeight;

    _splitStereoHorizontalEyeMapping = vs._splitStereoHorizontalEyeMapping;
    _splitStereoHorizontalSeparation = vs._splitStereoHorizontalSeparation;

    _splitStereoVerticalEyeMapping = vs._splitStereoVerticalEyeMapping;
    _splitStereoVerticalSeparation = vs._splitStereoVerticalSeparation;

    _doubleBuffer = vs._doubleBuffer;
    _RGB = vs._RGB;
    _depthBuffer = vs._depthBuffer;
    _minimumNumberAlphaBits = vs._minimumNumberAlphaBits;
    _minimumNumberStencilBits = vs._minimumNumberStencilBits;

    _maxNumOfGraphicsContexts = vs._maxNumOfGraphicsContexts;
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
}

void DisplaySettings::setDefaults()
{
    _stereo = false;
    _stereoMode = ANAGLYPHIC;
    _eyeSeparation = 0.05f;
    _screenDistance = 0.5f;
    _screenHeight = 0.26f;

    _splitStereoHorizontalEyeMapping = LEFT_EYE_LEFT_VIEWPORT;
    _splitStereoHorizontalSeparation = 42;

    _splitStereoVerticalEyeMapping = LEFT_EYE_TOP_VIEWPORT;
    _splitStereoVerticalSeparation = 42;

    _doubleBuffer = true;
    _RGB = true;
    _depthBuffer = true;
    _minimumNumberAlphaBits = 0;
    _minimumNumberStencilBits = 0;
    
    #ifdef __sgi
    _maxNumOfGraphicsContexts = 4;
    #else
    _maxNumOfGraphicsContexts = 1;
    #endif
}

void DisplaySettings::readEnvironmentalVariables()
{
    char *ptr;
    if( (ptr = getenv("OSG_STEREO_MODE")) != 0)
    {
        if (strcmp(ptr,"QUAD_BUFFER")==0)
        {
            _stereoMode = QUAD_BUFFER;
        }
        else
        if (strcmp(ptr,"ANAGLYPHIC")==0)
        {
            _stereoMode = ANAGLYPHIC;
        }
        else
        if (strcmp(ptr,"HORIZONTAL_SPLIT")==0)
        {
            _stereoMode = HORIZONTAL_SPLIT;
        }
        else
        if (strcmp(ptr,"VERTICAL_SPLIT")==0)
        {
            _stereoMode = VERTICAL_SPLIT;
        }
        else
        if (strcmp(ptr,"LEFT_EYE")==0)
        {
            _stereoMode = LEFT_EYE;
        }
        else
        if (strcmp(ptr,"RIGHT_EYE")==0)
        {
            _stereoMode = RIGHT_EYE;
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
        _eyeSeparation = atof(ptr);
    }

    if( (ptr = getenv("OSG_SCREEN_DISTANCE")) != 0)
    {
        _screenDistance = atof(ptr);
    }

    if( (ptr = getenv("OSG_SCREEN_HEIGHT")) != 0)
    {
        _screenHeight = atof(ptr);
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

    if( (ptr = getenv("OSG_SPLIT_STEREO_VERTICAL_SEPARATION")) != 0)
    {
        _splitStereoVerticalSeparation = atoi(ptr);
    }

    if( (ptr = getenv("OSG_MAX_NUMBER_OF_GRAPHICS_CONTEXTS")) != 0)
    {
        _maxNumOfGraphicsContexts = atoi(ptr);
    }
}

void DisplaySettings::readCommandLine(std::vector<std::string>& commandLine)
{

    bool found = true;
    while (found)
    {
        found = false;

        // check for stereo based options.
        std::vector<std::string>::iterator itr = commandLine.begin();
        for(;itr!=commandLine.end();++itr)
        {
            if (*itr=="-stereo") break;
        }

        if (itr!=commandLine.end())
        {
        
            _stereo = true;

            std::vector<std::string>::iterator start = itr; 
            ++itr;
            if (itr!=commandLine.end())
            {
                if (*itr=="ANAGLYPHIC") { _stereo = true;_stereoMode = ANAGLYPHIC; ++itr; }
                else if (*itr=="QUAD_BUFFER") { _stereo = true;_stereoMode = QUAD_BUFFER; ++itr; }
                else if (*itr=="HORIZONTAL_SPLIT") { _stereo = true;_stereoMode = HORIZONTAL_SPLIT; ++itr; }
                else if (*itr=="VERTICAL_SPLIT") { _stereo = true;_stereoMode = VERTICAL_SPLIT; ++itr; }
                else if (*itr=="LEFT_EYE") { _stereo = true;_stereoMode = LEFT_EYE; ++itr; }
                else if (*itr=="RIGHT_EYE") { _stereo = true;_stereoMode = RIGHT_EYE; ++itr; }
                else if (*itr=="ON") { _stereo = true; ++itr; }
                else if (*itr=="OFF") { _stereo = false; ++itr; }
            }

            commandLine.erase(start,itr);
            found = true;
        }
        
        // check destination alpha
        itr = commandLine.begin();
        for(;itr!=commandLine.end();++itr)
        {
            if (*itr=="-rgba") break;
        }

        if (itr!=commandLine.end())
        {
            _RGB = true;
            _minimumNumberAlphaBits = 1;
            commandLine.erase(itr);
            found = true;
        }


        // check stencil buffer
        itr = commandLine.begin();
        for(;itr!=commandLine.end();++itr)
        {
            if (*itr=="-stencil") break;
        }

        if (itr!=commandLine.end())
        {
            _minimumNumberStencilBits = 1;
            commandLine.erase(itr);
            found = true;
        }

    }    
}
