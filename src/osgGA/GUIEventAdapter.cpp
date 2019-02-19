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
    _eventType(NONE),
    _windowX(0),
    _windowY(0),
    _windowWidth(1280),
    _windowHeight(1024),
    _key(0),
    _unmodifiedKey(0),
    _button(0),
    _Xmin(-1.0),
    _Xmax(1.0),
    _Ymin(-1.0),
    _Ymax(1.0),
    _mx(0.0),
    _my(0.0),
    _buttonMask(0),
    _modKeyMask(0),
    _mouseYOrientation(Y_INCREASING_DOWNWARDS),
    _scrolling(),
    _tabletPen(),
    _touchData(NULL)
{}

GUIEventAdapter::GUIEventAdapter(const GUIEventAdapter& rhs,const osg::CopyOp& copyop):
    osgGA::Event(rhs,copyop),
    _eventType(rhs._eventType),
    _context(rhs._context),
    _windowX(rhs._windowX),
    _windowY(rhs._windowY),
    _windowWidth(rhs._windowWidth),
    _windowHeight(rhs._windowHeight),
    _key(rhs._key),
    _unmodifiedKey(rhs._unmodifiedKey),
    _button(rhs._button),
    _Xmin(rhs._Xmin),
    _Xmax(rhs._Xmax),
    _Ymin(rhs._Ymin),
    _Ymax(rhs._Ymax),
    _mx(rhs._mx),
    _my(rhs._my),
    _buttonMask(rhs._buttonMask),
    _modKeyMask(rhs._modKeyMask),
    _mouseYOrientation(rhs._mouseYOrientation),
    _scrolling(rhs._scrolling),
    _tabletPen(rhs._tabletPen),
    _touchData(NULL)
{
    if(TouchData* td = rhs.getTouchData())
        setTouchData(osg::clone(td, copyop));
}

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
        setInputRange(0, 0, width - 1, height - 1);
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

void GUIEventAdapter::addTouchPoint(unsigned int id, TouchPhase phase, float x, float y, unsigned int tapCount)
{
    if (!_touchData.valid()) {
        _touchData = new TouchData();
        setX(x);
        setY(y);
    }

    _touchData->addTouchPoint(id, phase, x, y, tapCount);
}

void GUIEventAdapter::copyPointerDataFrom(const osgGA::GUIEventAdapter& sourceEvent)
{
    setGraphicsContext(const_cast<osg::GraphicsContext*>(sourceEvent.getGraphicsContext()));
    setX(sourceEvent.getX());
    setY(sourceEvent.getY());
    setInputRange(sourceEvent.getXmin(), sourceEvent.getYmin(), sourceEvent.getXmax(), sourceEvent.getYmax());
    setButtonMask(sourceEvent.getButtonMask());
    setMouseYOrientation(sourceEvent.getMouseYOrientation());
    setPointerDataList(sourceEvent.getPointerDataList());
}



void GUIEventAdapter::setMouseYOrientationAndUpdateCoords(osgGA::GUIEventAdapter::MouseYOrientation myo)
{
    if ( myo==_mouseYOrientation )
    return;

    setMouseYOrientation( myo );

    _my = _Ymax - _my + _Ymin;
    if( isMultiTouchEvent() )
    {
        for( TouchData::iterator itr =  getTouchData()->begin(); itr != getTouchData()->end(); itr++ ) 
            itr->y = _Ymax - itr->y + _Ymin;
    }
}
