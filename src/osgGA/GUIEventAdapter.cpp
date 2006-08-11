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

GUIEventAdapter::GUIEventAdapter():
    _eventType(NONE),
    _time(0.0),
    _key(0),
    _button(0),
    _Xmin(0.0),
    _Xmax(1.0),
    _Ymin(0.0),
    _Ymax(1.0),
    _mx(0.5),
    _my(0.5),
    _pressure(0.0),
    _buttonMask(0),
    _modKeyMask(0),
    _scrollingMotion(SCROLL_NONE),
    _scrollingDeltaX(0),
    _scrollingDeltaY(0),
    _mouseYOrientation(Y_INCREASING_DOWNWARDS),
    _tabletPointerType(UNKNOWN)
{}

GUIEventAdapter::GUIEventAdapter(const GUIEventAdapter& rhs):
    osg::Referenced(),
    _eventType(rhs._eventType),
    _time(rhs._time),
    _key(rhs._key),
    _button(rhs._button),
    _Xmin(rhs._Xmin),
    _Xmax(rhs._Xmax),
    _Ymin(rhs._Ymin),
    _Ymax(rhs._Ymax),
    _mx(rhs._mx),
    _my(rhs._my),
    _pressure(rhs._pressure),
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

void GUIEventAdapter::setWindowSize(float Xmin, float Ymin, float Xmax, float Ymax)
{
    _Xmin = Xmin;
    _Ymin = Ymin;
    _Xmax = Xmax;
    _Ymax = Ymax;
}
