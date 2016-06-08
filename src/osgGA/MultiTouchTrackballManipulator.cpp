/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
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


#include <osgGA/MultiTouchTrackballManipulator>
#include <osg/io_utils>

using namespace osg;
using namespace osgGA;



/// Constructor.
MultiTouchTrackballManipulator::MultiTouchTrackballManipulator( int flags )
   : inherited( flags )
{
    setVerticalAxisFixed( false );
}


/// Constructor.
MultiTouchTrackballManipulator::MultiTouchTrackballManipulator( const MultiTouchTrackballManipulator& tm, const CopyOp& copyOp )
    : osg::Object(tm, copyOp), osg::Callback(tm, copyOp), inherited( tm, copyOp )
{
}


void MultiTouchTrackballManipulator::handleMultiTouchDrag(const GUIEventAdapter* now, const GUIEventAdapter* last, const double eventTimeDelta)
{
    const osg::Vec2 pt_1_now( now->getTouchData()->get(0).x, now->getTouchData()->get(0).y);
    const osg::Vec2 pt_2_now( now->getTouchData()->get(1).x, now->getTouchData()->get(1).y);
    const osg::Vec2 pt_1_last( last->getTouchData()->get(0).x, last->getTouchData()->get(0).y);
    const osg::Vec2 pt_2_last( last->getTouchData()->get(1).x, last->getTouchData()->get(1).y);

    const float gap_now((pt_1_now - pt_2_now).length());
    const float gap_last((pt_1_last - pt_2_last).length());

    // osg::notify(osg::ALWAYS) << gap_now << " " << gap_last << std::endl;

    const float relativeChange = (gap_last - gap_now)/gap_last;


    // zoom gesture
    if (fabs(relativeChange) > 0.02)
        zoomModel( relativeChange , true );

    // drag gesture
    const osg::Vec2 delta = ((pt_1_last - pt_1_now) + (pt_2_last - pt_2_now)) / 2.0f;

    const float scale = -0.3f * _distance * getThrowScale( eventTimeDelta );
    //osg::notify(osg::ALWAYS) << "drag: " << delta << " scale: " << scale << std::endl;

    panModel( delta.x() * scale, delta.y() * scale);
}


bool MultiTouchTrackballManipulator::handle( const GUIEventAdapter& ea, GUIActionAdapter& us )
{

    bool handled(false);

    switch(ea.getEventType())
    {

        case osgGA::GUIEventAdapter::PUSH:
        case osgGA::GUIEventAdapter::DRAG:
        case osgGA::GUIEventAdapter::RELEASE:
            if (ea.isMultiTouchEvent())
            {
                double eventTimeDelta = 1/60.0; //_ga_t0->getTime() - _ga_t1->getTime();
                if( eventTimeDelta < 0. )
                {
                    OSG_WARN << "Manipulator warning: eventTimeDelta = " << eventTimeDelta << std::endl;
                    eventTimeDelta = 0.;
                }
                osgGA::GUIEventAdapter::TouchData* data = ea.getTouchData();

                // three touches or two taps for home position
                if ((data->getNumTouchPoints() == 3) || ((data->getNumTouchPoints() == 1) && (data->get(0).tapCount >= 2)))
                {
                    flushMouseEventStack();
                    _thrown = false;
                    home(ea,us);
                    handled = true;
                }

                else if (data->getNumTouchPoints() >= 2)
                {
                    if ((_lastEvent.valid()) && (_lastEvent->getTouchData()->getNumTouchPoints() >= 2))
                    {
                        handleMultiTouchDrag(&ea, _lastEvent.get(), eventTimeDelta);
                    }

                    handled = true;
                }

                _lastEvent = new GUIEventAdapter(ea);

                // check if all touches ended
                unsigned int num_touches_ended(0);
                for(osgGA::GUIEventAdapter::TouchData::iterator i = data->begin(); i != data->end(); ++i)
                {
                    if ((*i).phase == osgGA::GUIEventAdapter::TOUCH_ENDED)
                        num_touches_ended++;
                }

                if(num_touches_ended == data->getNumTouchPoints())
                {
                    _lastEvent = NULL;
                }

            }
            break;
        default:
            break;
    }

    return handled ? handled : TrackballManipulator::handle(ea, us);
}
