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

#include <osgGA/FlightManipulator>

using namespace osg;
using namespace osgGA;



/// Constructor.
FlightManipulator::FlightManipulator( int flags )
    : inherited( flags ),
      _yawMode( YAW_AUTOMATICALLY_WHEN_BANKED )
{
}


/// Constructor.
FlightManipulator::FlightManipulator( const FlightManipulator& fm, const CopyOp& copyOp )
    : osg::Object(fm, copyOp),
      osg::Callback(fm, copyOp),
      inherited( fm, copyOp ),
      _yawMode( fm._yawMode )
{
}


void FlightManipulator::init( const GUIEventAdapter& ea, GUIActionAdapter& us )
{
    inherited::init( ea, us );

    // center mouse pointer
    centerMousePointer( ea, us );
}


void FlightManipulator::home( const GUIEventAdapter& ea, GUIActionAdapter& us )
{
    inherited::home( ea, us );

    // center mouse pointer
    centerMousePointer( ea, us );
}


// doc in parent
bool FlightManipulator::handleFrame( const GUIEventAdapter& ea, GUIActionAdapter& us )
{
    addMouseEvent( ea );
    if( performMovement() )
        us.requestRedraw();

    return false;
}


// doc in parent
bool FlightManipulator::handleMouseMove( const GUIEventAdapter& ea, GUIActionAdapter& us )
{
    return flightHandleEvent( ea, us );
}


// doc in parent
bool FlightManipulator::handleMouseDrag( const GUIEventAdapter& ea, GUIActionAdapter& us )
{
    return flightHandleEvent( ea, us );
}


// doc in parent
bool FlightManipulator::handleMousePush( const GUIEventAdapter& ea, GUIActionAdapter& us )
{
    return flightHandleEvent( ea, us );
}


// doc in parent
bool FlightManipulator::handleMouseRelease( const GUIEventAdapter& ea, GUIActionAdapter& us )
{
    return flightHandleEvent( ea, us );
}


bool FlightManipulator::handleKeyDown( const GUIEventAdapter& ea, GUIActionAdapter& us )
{
    if( inherited::handleKeyDown( ea, us ) )
        return true;

    if( ea.getKey() == 'q' )
    {

        _yawMode = YAW_AUTOMATICALLY_WHEN_BANKED;
        return true;

    }
    else if (ea.getKey()=='a')
    {

        _yawMode = NO_AUTOMATIC_YAW;
        return true;
   }

   return false;
}


/// General flight-style event handler
bool FlightManipulator::flightHandleEvent( const GUIEventAdapter& ea, GUIActionAdapter& us )
{
    addMouseEvent( ea );
    us.requestContinuousUpdate( true );
    if( performMovement() )
        us.requestRedraw();

    return true;
}


void FlightManipulator::getUsage( ApplicationUsage& usage ) const
{
    inherited::getUsage( usage );
    usage.addKeyboardMouseBinding( getManipulatorName() + ": q", "Automatically yaw when banked (default)" );
    usage.addKeyboardMouseBinding( getManipulatorName() + ": a", "No yaw when banked" );
}


/** Configure the Yaw control for the flight model. */
void FlightManipulator::setYawControlMode( YawControlMode ycm )
{
    _yawMode = ycm;
}


bool FlightManipulator::performMovement()
{
    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;


    double eventTimeDelta = _ga_t0->getTime()-_ga_t1->getTime();

    if (eventTimeDelta<0.0f)
    {
        OSG_WARN << "Manipulator warning: eventTimeDelta = " << eventTimeDelta << std::endl;
        eventTimeDelta = 0.0f;
    }

    unsigned int buttonMask = _ga_t1->getButtonMask();
    if (buttonMask==GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        performMovementLeftMouseButton(eventTimeDelta, 0., 0.);
    }
    else if (buttonMask==GUIEventAdapter::MIDDLE_MOUSE_BUTTON ||
        buttonMask==(GUIEventAdapter::LEFT_MOUSE_BUTTON|GUIEventAdapter::RIGHT_MOUSE_BUTTON))
    {
        performMovementMiddleMouseButton(eventTimeDelta, 0., 0.);
    }
    else if (buttonMask==GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {
        performMovementRightMouseButton(eventTimeDelta, 0., 0.);
    }

    float dx = _ga_t0->getXnormalized();
    float dy = _ga_t0->getYnormalized();

    CoordinateFrame cf=getCoordinateFrame(_eye);

    Matrixd rotation_matrix;
    rotation_matrix.makeRotate(_rotation);

    Vec3d up = Vec3d(0.0,1.0,0.0) * rotation_matrix;
    Vec3d lv = Vec3d(0.0,0.0,-1.0) * rotation_matrix;

    Vec3d sv = lv^up;
    sv.normalize();

    double pitch = -inDegrees(dy*50.0f*eventTimeDelta);
    double roll = inDegrees(dx*50.0f*eventTimeDelta);

    Quat delta_rotate;

    Quat roll_rotate;
    Quat pitch_rotate;

    pitch_rotate.makeRotate(pitch,sv.x(),sv.y(),sv.z());
    roll_rotate.makeRotate(roll,lv.x(),lv.y(),lv.z());

    delta_rotate = pitch_rotate*roll_rotate;

    if (_yawMode==YAW_AUTOMATICALLY_WHEN_BANKED)
    {
        //float bank = asinf(sv.z());
        double bank = asinf(sv *getUpVector(cf));
        double yaw = inRadians(bank)*eventTimeDelta;

        Quat yaw_rotate;
        //yaw_rotate.makeRotate(yaw,0.0f,0.0f,1.0f);

        yaw_rotate.makeRotate(yaw,getUpVector(cf));


        delta_rotate = delta_rotate*yaw_rotate;
    }

    lv *= (_velocity*eventTimeDelta);

    _eye += lv;
    _rotation = _rotation*delta_rotate;

    return true;
}


bool FlightManipulator::performMovementLeftMouseButton( const double eventTimeDelta, const double /*dx*/, const double /*dy*/ )
{
    // pan model
    _velocity += eventTimeDelta * (_acceleration + _velocity);
    return true;
}


bool FlightManipulator::performMovementMiddleMouseButton( const double /*eventTimeDelta*/, const double /*dx*/, const double /*dy*/ )
{
    _velocity = 0.0f;
    return true;
}


bool FlightManipulator::performMovementRightMouseButton( const double eventTimeDelta, const double /*dx*/, const double /*dy*/ )
{
    _velocity -= eventTimeDelta * (_acceleration + _velocity);
    return true;
}
