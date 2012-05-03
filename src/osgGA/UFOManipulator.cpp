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

/* Written by Don Burns */

#include <osgGA/UFOManipulator>
#include <osgUtil/LineSegmentIntersector>

#include <osg/io_utils>

#ifndef M_PI
# define M_PI       3.14159265358979323846  /* pi */
#endif

using namespace osgGA;

UFOManipulator::UFOManipulator():
            _t0(0.0),
            _shift(false),
            _ctrl(false)
{
    _minHeightAboveGround          = 2.0;
    _minDistanceInFront            = 5.0;

    _speedAccelerationFactor       = 0.4;
    _speedDecelerationFactor       = 0.90;

    _directionRotationRate         = 0.0;
    _directionRotationAcceleration = M_PI*0.00005;
    _directionRotationDeceleration = 0.90;

    _speedEpsilon                  = 0.02;
    _directionRotationEpsilon      = 0.0001;

    _viewOffsetDelta = M_PI * 0.0025;
    _pitchOffsetRate = 0.0;
    _pitchOffset = 0.0;

    _yawOffsetRate = 0.0;
    _yawOffset = 0.0;
    _offset.makeIdentity();

    _decelerateOffsetRate = true;
    _straightenOffset = false;

    _direction.set( 0,1,0);
    _stop();
}

UFOManipulator::~UFOManipulator()
{
}

bool UFOManipulator::intersect(const osg::Vec3d& start, const osg::Vec3d& end, osg::Vec3d& intersection) const
{
    osg::ref_ptr<osgUtil::LineSegmentIntersector> lsi = new osgUtil::LineSegmentIntersector(start,end);

    osgUtil::IntersectionVisitor iv(lsi.get());
    iv.setTraversalMask(_intersectTraversalMask);

    _node->accept(iv);

    if (lsi->containsIntersections())
    {
        intersection = lsi->getIntersections().begin()->getWorldIntersectPoint();
        return true;
    }
    return false;
}

void UFOManipulator::setNode( osg::Node *node )
{
    _node = node;

    if (getAutoComputeHomePosition())
        computeHomePosition();

    home(0.0);
}

const osg::Node* UFOManipulator::getNode() const
{
    return _node.get();
}

osg::Node* UFOManipulator::getNode()
{
    return _node.get();
}


const char* UFOManipulator::className() const
{
    return "UFO";
}

void UFOManipulator::setByMatrix( const osg::Matrixd &mat )
{
    _inverseMatrix = mat;
    _matrix.invert( _inverseMatrix );

    _position.set( _inverseMatrix(3,0), _inverseMatrix(3,1), _inverseMatrix(3,2 ));
    osg::Matrix R(_inverseMatrix);
    R(3,0) = R(3,1) = R(3,2) = 0.0;
    _direction = osg::Vec3d(0,0,-1) * R; // camera up is +Z, regardless of CoordinateFrame

    _stop();
}

void UFOManipulator::setByInverseMatrix( const osg::Matrixd &invmat)
{
    _matrix = invmat;
    _inverseMatrix.invert( _matrix );

    _position.set( _inverseMatrix(3,0), _inverseMatrix(3,1), _inverseMatrix(3,2 ));
    osg::Matrix R(_inverseMatrix);
    R(3,0) = R(3,1) = R(3,2) = 0.0;
    _direction = osg::Vec3d(0,0,-1) * R; // camera up is +Z, regardless of CoordinateFrame

    _stop();
}

osg::Matrixd UFOManipulator::getMatrix() const
{
    return (osg::Matrix::inverse(_offset) * _matrix);
}

osg::Matrixd UFOManipulator::getInverseMatrix() const
{
    return (_inverseMatrix * _offset);
}

void UFOManipulator::computeHomePosition()
{
    if( !_node.valid() )
        return;

    osg::BoundingSphere bs = _node->getBound();

    /*
       * Find the ground - Assumption: The ground is the hit of an intersection
       * from a line segment extending from above to below the database at its
       * horizontal center, that intersects the database closest to zero. */

    osg::CoordinateFrame cf( getCoordinateFrame(bs.center()) ); // not sure what position to use here
    osg::Vec3d upVec( getUpVector(cf) );

    osg::Vec3d A = bs.center() + (upVec*(bs.radius()*2));
    osg::Vec3d B = bs.center() + (-upVec*(bs.radius()*2));

    if( (B-A).length() == 0.0)
    {
        return;
    }

    // start with it high
    double ground = bs.radius() * 3;

    osg::Vec3d ip;
    if (intersect(A, B, ip))
    {
        double d = ip.length();
        if( d < ground )
            ground = d;
    }
    else
    {
        //OSG_WARN<<"UFOManipulator : I can't find the ground!"<<std::endl;
        ground = 0.0;
    }


    osg::Vec3d p(bs.center() + upVec*( ground + _minHeightAboveGround*1.25 ) );
    setHomePosition( p, p + getFrontVector(cf), upVec );
}

void UFOManipulator::init(const GUIEventAdapter&, GUIActionAdapter&)
{
    //home(ea.getTime());

    _stop();
}

void UFOManipulator::home(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
    home(ea.getTime());
    us.requestRedraw();
    us.requestContinuousUpdate(false);

}

void UFOManipulator::home(double)
{
    if (getAutoComputeHomePosition())
        computeHomePosition();

    _position = _homeEye;
    _direction = _homeCenter - _homeEye;
    _direction.normalize();
    _directionRotationRate = 0.0;

    _inverseMatrix.makeLookAt( _homeEye, _homeCenter, _homeUp );
    _matrix.invert( _inverseMatrix );

    _offset.makeIdentity();

    _forwardSpeed = 0.0;
    _sideSpeed = 0.0;
    _upSpeed = 0.0;
}

bool UFOManipulator::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter &aa)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::FRAME):
            _frame(ea,aa);
            return false;
        default:
            break;
    }

    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYUP):
            _keyUp( ea, aa );
            return false;
            break;

        case(osgGA::GUIEventAdapter::KEYDOWN):
            _keyDown(ea, aa);
            return false;
            break;

        case(osgGA::GUIEventAdapter::FRAME):
            _frame(ea,aa);
            return false;
            break;

        default:
            return false;
    }
}

void UFOManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    /** Way too busy.  This needs to wait until we have a scrollable window
    usage.addKeyboardMouseBinding("UFO Manipulator: <SpaceBar>",        "Reset the viewing angle to 0.0");
    usage.addKeyboardMouseBinding("UFO Manipulator: <UpArrow>",         "Acceleration forward.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <DownArrow>",       "Acceleration backward (or deceleration forward");
    usage.addKeyboardMouseBinding("UFO Manipulator: <LeftArrow>",       "Rotate view and direction of travel to the left.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <RightArrow>",      "Rotate view and direction of travel to the right.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <SpaceBar>",        "Brake.  Gradually decelerates linear and rotational movement.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <Shift/UpArrow>",   "Accelerate up.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <Shift/DownArrow>", "Accelerate down.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <Shift/LeftArrow>", "Accelerate (linearly) left.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <Shift/RightArrow>","Accelerate (linearly) right.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <Shift/SpaceBar>",  "Instant brake.  Immediately stop all linear and rotational movement.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <Ctrl/UpArrow>",    "Rotate view (but not direction of travel) up.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <Ctrl/DownArrow>",  "Rotate view (but not direction of travel) down.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <Ctrl/LeftArrow>",  "Rotate view (but not direction of travel) left.");
    usage.addKeyboardMouseBinding("UFO Manipulator: <Ctrl/RightArrow>", "Rotate view (but not direction of travel) right.");
    */
    usage.addKeyboardMouseBinding("UFO: ", "Please see http://www.openscenegraph.org/html/UFOCameraManipulator.html");
    // Keep this one as it might be confusing
    usage.addKeyboardMouseBinding("UFO: H", "Reset the viewing position to home");
}



void UFOManipulator::_keyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter & )
{
    switch( ea.getKey() )
    {
        case osgGA::GUIEventAdapter::KEY_Control_L:
        case osgGA::GUIEventAdapter::KEY_Control_R:
            _ctrl = false;
            _decelerateOffsetRate = true;
            _straightenOffset = false;
            break;

        case osgGA::GUIEventAdapter::KEY_Shift_L:
        case osgGA::GUIEventAdapter::KEY_Shift_R:
            _shift = false;
            _decelerateUpSideRate = true;
            break;
    }
}

void UFOManipulator::_keyDown( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter & )
{
    switch( ea.getKey() )
    {
        case osgGA::GUIEventAdapter::KEY_Control_L:
        case osgGA::GUIEventAdapter::KEY_Control_R:
            _ctrl = true;
            break;

        case osgGA::GUIEventAdapter::KEY_Shift_L :
        case osgGA::GUIEventAdapter::KEY_Shift_R :
            _shift = true;
            break;

        case osgGA::GUIEventAdapter::KEY_Up:
            if( _ctrl )
            {
                _pitchOffsetRate -= _viewOffsetDelta;
                _decelerateOffsetRate = false;
            }
            else
            {
                if( _shift )
                {
                    _upSpeed += _speedAccelerationFactor;
                    _decelerateUpSideRate = false;
                }
                else
                    _forwardSpeed += _speedAccelerationFactor;
            }
            break;

        case osgGA::GUIEventAdapter::KEY_Down:
            if( _ctrl )
            {
                _pitchOffsetRate += _viewOffsetDelta;
                _decelerateOffsetRate = false;
            }
            else
            {
                if( _shift )
                {
                    _upSpeed -= _speedAccelerationFactor;
                    _decelerateUpSideRate = false;
                }
                else
                    _forwardSpeed -= _speedAccelerationFactor;
            }
            break;

        case osgGA::GUIEventAdapter::KEY_Right:
            if( _ctrl )
            {
                _yawOffsetRate += _viewOffsetDelta;
                _decelerateOffsetRate = false;
            }
            else
            {
                if(_shift)
                {
                    _sideSpeed += _speedAccelerationFactor;
                    _decelerateUpSideRate = false;
                }
                else
                    _directionRotationRate -= _directionRotationAcceleration;
            }
            break;

        case osgGA::GUIEventAdapter::KEY_Left:
            if( _ctrl )
            {
                _yawOffsetRate -= _viewOffsetDelta;
                _decelerateOffsetRate = false;
            }
            else
            {
                if(_shift)
                {
                    _sideSpeed -= _speedAccelerationFactor;
                    _decelerateUpSideRate = false;
                }
                else
                    _directionRotationRate += _directionRotationAcceleration;
            }
            break;

        case osgGA::GUIEventAdapter::KEY_Return:
            if( _ctrl )
            {
                _straightenOffset = true;
            }
            break;

        case ' ':
            if( _shift )
            {
                _stop();
            }
            else
            {
                if( fabs(_forwardSpeed) > 0.0 )
                {
                    _forwardSpeed *= _speedDecelerationFactor;

                    if( fabs(_forwardSpeed ) < _speedEpsilon )
                        _forwardSpeed = 0.0;
                }
                if( fabs(_sideSpeed) > 0.0 )
                {
                    _sideSpeed *= _speedDecelerationFactor;

                    if( fabs( _sideSpeed ) < _speedEpsilon )
                        _sideSpeed = 0.0;
                }

                if( fabs(_upSpeed) > 0.0 )
                {
                    _upSpeed *= _speedDecelerationFactor;

                    if( fabs( _upSpeed ) < _speedEpsilon )
                        _sideSpeed = 0.0;
                }


                if( fabs(_directionRotationRate ) > 0.0 )
                {
                    _directionRotationRate *= _directionRotationDeceleration;
                    if( fabs( _directionRotationRate ) < _directionRotationEpsilon )
                        _directionRotationRate = 0.0;
                }

            }
            break;

        case 'H':
            home(ea.getTime());
            break;
    }

}

void UFOManipulator::_frame( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter & )
{
    double t1 = ea.getTime();
    if( _t0 == 0.0 )
    {
        _t0 = ea.getTime();
        _dt = 0.0;
    }
    else
    {
        _dt = t1 - _t0;
        _t0 = t1;
    }

    osg::CoordinateFrame cf( getCoordinateFrame(_position) );
    osg::Vec3d upVec( getUpVector(cf) );
    if( fabs( _directionRotationRate ) > _directionRotationEpsilon )
    {
      _direction = _direction * osg::Matrix::rotate( _directionRotationRate, upVec);
    }

    {
      osg::Vec3d _sideVec = _direction * osg::Matrix::rotate( -M_PI*0.5, upVec);

        _position += ((_direction       * _forwardSpeed) +
                      (_sideVec         * _sideSpeed) +
                      (upVec * _upSpeed))
                       * _dt;

    }

    _pitchOffset += _pitchOffsetRate * _dt;
    if( _pitchOffset >= M_PI || _pitchOffset < -M_PI )
        _pitchOffset *= -1;

    _yawOffset   += _yawOffsetRate   * _dt;
    if( _yawOffset >= M_PI || _yawOffset < -M_PI )
        _yawOffset *= -1;

    _offset       = osg::Matrix::rotate( _yawOffset, getSideVector(cf),
                                         _pitchOffset, getFrontVector(cf),
                                         0.0, upVec);

    _adjustPosition();

    _inverseMatrix.makeLookAt( _position, _position + _direction, upVec);
    _matrix.invert(_inverseMatrix);

    if( _decelerateUpSideRate )
    {
        _upSpeed   *= 0.98;
        _sideSpeed *= 0.98;
    }

    if( _decelerateOffsetRate )
    {
        _yawOffsetRate   *= 0.98;
        _pitchOffsetRate *= 0.98;
    }

    if( _straightenOffset )
    {
        if( _shift )
        {
            _pitchOffset = 0.0;
            _yawOffset = 0.0;
            _pitchOffsetRate = 0.0;
            _yawOffsetRate   = 0.0;
        }
        else
        {
            _pitchOffsetRate = 0.0;
            _yawOffsetRate   = 0.0;
            _pitchOffset *= 0.99;
            _yawOffset *= 0.99;

            if( fabs(_pitchOffset ) < 0.01 )
                _pitchOffset = 0.0;
            if( fabs(_yawOffset ) < 0.01 )
                _pitchOffset = 0.0;

        }
        if( _pitchOffset == 0.0 && _yawOffset == 0.0 )
            _straightenOffset = false;
    }
}

void UFOManipulator::_adjustPosition()
{
    if( !_node.valid() )
        return;

    // Forward line segment at 3 times our intersect distance


    typedef std::vector<osg::Vec3d> Intersections;
    Intersections intersections;

    // Check intersects infront.
    osg::Vec3d ip;
    if (intersect(_position,
                  _position + (_direction * (_minDistanceInFront * 3.0)),
                  ip ))
    {
        double d = (ip - _position).length();

        if( d < _minDistanceInFront )
        {
            _position = ip + (_direction * -_minDistanceInFront);
            _stop();
        }
    }

    // Check intersects below.
    osg::CoordinateFrame cf( getCoordinateFrame(_position) );
    osg::Vec3d upVec( getUpVector(cf) );

    if (intersect(_position,
                  _position - upVec*_minHeightAboveGround*3,
                  ip ))
    {
        double d = (ip - _position).length();

        if( d < _minHeightAboveGround )
          _position = ip + (upVec * _minHeightAboveGround);
    }
}


void UFOManipulator::_stop()
{
    _forwardSpeed = 0.0;
    _sideSpeed = 0.0;
    _upSpeed = 0.0;
    _directionRotationRate = 0.0;
}

void UFOManipulator::getCurrentPositionAsLookAt( osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up )
{
    eye = _position;
    center = _position + _direction;
    up.set(getUpVector(getCoordinateFrame(_position)));
}

