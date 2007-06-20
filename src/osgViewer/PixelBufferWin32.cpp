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
 *
 * Some elements of GraphicsWindowWin32 have used the Producer implementation as a reference.
 * These elements are licensed under OSGPL as above, with Copyright (C) 2001-2004  Don Burns.
 */

#include <osgViewer/api/Win32/PixelBufferWin32>

#include <vector>
#include <map>
#include <sstream>
#include <windowsx.h>

using namespace osgViewer;


PixelBufferWin32::PixelBufferWin32( osg::GraphicsContext::Traits* traits ):
  _hwnd(0),
  _hdc(0),
  _hglrc(0),
  _initialized(false),
  _valid(false),
  _realized(false)
{
    _traits = traits;

    init();
    
    if (valid())
    {
        setState( new osg::State );
        getState()->setGraphicsContext(this);

        if (_traits.valid() && _traits->sharedContext)
        {
            getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
            incrementContextIDUsageCount( getState()->getContextID() );   
        }
        else
        {
            getState()->setContextID( osg::GraphicsContext::createNewContextID() );
        }
    }
}

PixelBufferWin32::~PixelBufferWin32()
{
}
    
bool PixelBufferWin32::valid() const
{
    return _valid;
}

void PixelBufferWin32::init()
{
    if (_initialized) return;

    // don't do anything right now... waiting for an implementation...    
    _valid = false;
    return;
}

bool PixelBufferWin32::realizeImplementation()
{
    osg::notify(osg::NOTICE) << "PixelBufferWin32::realizeImplementation() not implemented." << std::endl; return false;
}

bool PixelBufferWin32::isRealizedImplementation() const
{
    osg::notify(osg::NOTICE) << "PixelBufferWin32::isRealizedImplementation() not implemented." << std::endl; return false;
}

void PixelBufferWin32::closeImplementation()
{
    osg::notify(osg::NOTICE) << "PixelBufferWin32::closeImplementation() not implemented." << std::endl;
}

bool PixelBufferWin32::makeCurrentImplementation()
{
    osg::notify(osg::NOTICE) << "PixelBufferWin32::makeCurrentImplementation() not implemented." << std::endl;
    return false;
}
        
bool PixelBufferWin32::makeContextCurrentImplementation( GraphicsContext* /*readContext*/ )
{
    osg::notify(osg::NOTICE) << "PixelBufferWin32::makeContextCurrentImplementation(..) not implemented." << std::endl;
    return false;
}

bool PixelBufferWin32::releaseContextImplementation()
{
    osg::notify(osg::NOTICE) << "PixelBufferWin32::releaseContextImplementation(..) not implemented." << std::endl;
    return false;
}

void PixelBufferWin32::bindPBufferToTextureImplementation( GLenum /*buffer*/ )
{
    osg::notify(osg::NOTICE) << "PixelBufferWin32::void bindPBufferToTextureImplementation(..) not implemented." << std::endl;
}

void PixelBufferWin32::swapBuffersImplementation() 
{
    osg::notify(osg::NOTICE) << "PixelBufferWin32:: swapBuffersImplementation() not implemented." << std::endl;
}

