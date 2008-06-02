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

#include <osgGA/GUIEventAdapter>

using namespace osgGA;


osg::ref_ptr<GUIEventAdapter>& GUIEventAdapter::getAccumulatedEventState()
{
    static osg::ref_ptr<GUIEventAdapter> s_eventState = new GUIEventAdapter;
    return s_eventState;
}

GUIEventAdapter::GUIEventAdapter():
    _handled(false),
    _eventType(NONE),
    _time(0.0),
    _windowX(0),
    _windowY(0),
    _windowWidth(1280),
    _windowHeight(1024),
    _key(0),
    _button(0),
    _Xmin(-1.0),
    _Xmax(1.0),
    _Ymin(-1.0),
    _Ymax(1.0),
    _mx(0.0),
    _my(0.0),
    _pressure(0.0),
    _tiltX(0.0),
    _tiltY(0.0),
    _rotation(0.0),
    _buttonMask(0),
    _modKeyMask(0),
    _scrollingMotion(SCROLL_NONE),
    _scrollingDeltaX(0),
    _scrollingDeltaY(0),
    _mouseYOrientation(Y_INCREASING_DOWNWARDS),
    _tabletPointerType(UNKNOWN)
{}

GUIEventAdapter::GUIEventAdapter(const GUIEventAdapter& rhs,const osg::CopyOp& copyop):
    osg::Object(rhs,copyop),
    _handled(rhs._handled),
    _eventType(rhs._eventType),
    _time(rhs._time),
    _context(rhs._context),
    _windowX(rhs._windowX),
    _windowY(rhs._windowY),
    _windowWidth(rhs._windowWidth),
    _windowHeight(rhs._windowHeight),
    _key(rhs._key),
    _button(rhs._button),
    _Xmin(rhs._Xmin),
    _Xmax(rhs._Xmax),
    _Ymin(rhs._Ymin),
    _Ymax(rhs._Ymax),
    _mx(rhs._mx),
    _my(rhs._my),
    _pressure(rhs._pressure),
    _tiltX(rhs._tiltX),
    _tiltY(rhs._tiltY),
    _rotation(rhs._rotation),
    _buttonMask(rhs._buttonMask),
    _modKeyMask(rhs._modKeyMask),
    _scrollingMotion(rhs._scrollingMotion),
    _scrollingDeltaX(rhs._scrollingDeltaX),
    _scrollingDeltaY(rhs._scrollingDeltaY),
    _mouseYOrientation(rhs._mouseYOrientation),
    _tabletPointerType(rhs._tabletPointerType)
{}

GUIEventAdapter::~GUIEventAdapter()
{
}

void GUIEventAdapter::setWindowRectangle(int x, int y, int width, int height, bool updateMouseRange)
{
    _windowX = x;
    _windowY = y;
    _windowWidth = width;
    _windowHeight = height;
    
    if (updateMouseRange)
    {
        setInputRange(0, 0, width, height);
    }
    
}

void GUIEventAdapter::setInputRange(float Xmin, float Ymin, float Xmax, float Ymax)
{
    _Xmin = Xmin;
    _Ymin = Ymin;
    _Xmax = Xmax;
    _Ymax = Ymax;
}

const osg::Matrix GUIEventAdapter::getPenOrientation() const
{
    float xRad = osg::DegreesToRadians ( getPenTiltY() );
    float yRad = osg::DegreesToRadians ( getPenTiltX() );
    float zRad = osg::DegreesToRadians ( getPenRotation() );
    osg::Matrix xrot = osg::Matrix::rotate ( xRad, osg::Vec3f(1.0f, 0.0f, 0.0f) );
    osg::Matrix yrot = osg::Matrix::rotate ( yRad, osg::Vec3f(0.0f, 0.0f, 1.0f) );
    osg::Matrix zrot = osg::Matrix::rotate ( zRad, osg::Vec3f(0.0f, 1.0f, 0.0f) );
    
    return ( zrot * yrot * xrot );
}
