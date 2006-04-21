#include <osgGA/UFOManipulator>
#include <osgUtil/IntersectVisitor>

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
    _direction = osg::Vec3(0,0,-1) * R; // yep.

    _stop();
}

void UFOManipulator::setByInverseMatrix( const osg::Matrixd &invmat) 
{
    _matrix = invmat;
    _inverseMatrix.invert( _matrix );

    _position.set( _inverseMatrix(3,0), _inverseMatrix(3,1), _inverseMatrix(3,2 ));
    osg::Matrix R(_inverseMatrix);
    R(3,0) = R(3,1) = R(3,2) = 0.0;
    _direction = osg::Vec3(0,0,-1) * R; // yep.

    _stop();
}

osg::Matrixd UFOManipulator::getMatrix() const
{
    return (_offset * _matrix);
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
    osgUtil::IntersectVisitor iv;
    iv.setTraversalMask(_intersectTraversalMask);

    osg::ref_ptr<osg::LineSegment> seg = new osg::LineSegment;
    osg::Vec3 A = bs.center() + (osg::Vec3(0,0,1)*(bs.radius()*2));
    osg::Vec3 B = bs.center() + (osg::Vec3(0,0,-1)*(bs.radius()*2));

    if( (B-A).length() == 0.0)
    {
        return;
    }

    /*
    seg->set( bs.center() + (osg::Vec3(0,0,1)*(bs.radius()*2)), 
              bs.center() + (osg::Vec3(0,0,-1)*(bs.radius()*2)) );
              */
    seg->set( A, B );

    iv.addLineSegment( seg.get() );
    _node->accept(iv);

    // start with it high
    double ground = bs.radius() * 3;

    if (iv.hits())
    {
        osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(seg.get());
        osg::Vec3d ip = hitList.front().getWorldIntersectPoint();
        if( fabs(ip[2]) < ground )
            ground = ip[2];
    }
    else
    {
        //osg::notify(osg::WARN)<<"UFOManipulator : I can't find the ground!"<<std::endl;
        ground = 0.0;
    }


    osg::Vec3 p(bs.center()[0], bs.center()[1], ground + (_minHeightAboveGround*1.25) );
    setHomePosition( p, p + osg::Vec3(0,1,0), osg::Vec3(0,0,1) );
}

void UFOManipulator::init(const GUIEventAdapter&, GUIActionAdapter&)
{
    //home(ea.getTime());

    _stop();
}

void UFOManipulator::home(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&) 
{
    home(ea.getTime());
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
    _matrix.invert( _matrix );

    _offset.makeIdentity();

    _forwardSpeed = 0.0;
    _sideSpeed = 0.0;
    _upSpeed = 0.0;
}

bool UFOManipulator::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter &aa)
{

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

    return false;
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

    if( fabs( _directionRotationRate ) > _directionRotationEpsilon )
    {
        _direction = _direction * osg::Matrix::rotate( _directionRotationRate, osg::Vec3(0,0,1));
    }

    {
        osg::Vec3 _sideVec = _direction * osg::Matrix::rotate( -M_PI*0.5, osg::Vec3(0,0,1));

        _position += ((_direction       * _forwardSpeed) + 
                      (_sideVec         * _sideSpeed) +
                      (osg::Vec3(0,0,1) * _upSpeed))
                       * _dt;
    }

    _pitchOffset += _pitchOffsetRate * _dt;
    if( _pitchOffset >= M_PI || _pitchOffset < -M_PI )
        _pitchOffset *= -1;

    _yawOffset   += _yawOffsetRate   * _dt;
    if( _yawOffset >= M_PI || _yawOffset < -M_PI ) 
        _yawOffset *= -1;

    _offset       = osg::Matrix::rotate( _yawOffset, osg::Vec3(0,1,0),
                                         _pitchOffset, osg::Vec3(1,0,0),
                                         0.0, osg::Vec3(0,0,1));

    _adjustPosition();

    _inverseMatrix.makeLookAt( _position, _position + _direction, osg::Vec3(0,0,1)); 
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

    osgUtil::IntersectVisitor iv;
    iv.setTraversalMask(_intersectTraversalMask);

    // Forward line segment at 3 times our intersect distance
    osg::ref_ptr<osg::LineSegment> segForward = new osg::LineSegment;
    segForward->set(_position, _position + (_direction * (_minDistanceInFront * 3.0)) );
    iv.addLineSegment( segForward.get() );


    // Down line segment at 3 times our intersect distance
    osg::ref_ptr<osg::LineSegment> segDown = new osg::LineSegment;
    segDown->set(   _position, 
                    _position - (osg::Vec3(0,0, _minHeightAboveGround*3)));
    iv.addLineSegment( segDown.get() );

    _node->accept(iv);

    if (iv.hits())
    {
        // Check intersects infront.
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segForward.get());
            if (!hitList.empty())
            {
                osg::Vec3d ip = hitList.front().getWorldIntersectPoint();

                double d = (ip - _position).length();
            
                if( d < _minDistanceInFront )
                {
                    _position = ip + (_direction * -_minDistanceInFront);
                    _stop();
                }
            }
        }

        // Check intersects below.
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segDown.get());
            if (!hitList.empty())
            {
                osg::Vec3d ip = hitList.front().getWorldIntersectPoint();
                if( _position[2] - ip[2] < _minHeightAboveGround )
                    _position[2] = ip[2] + _minHeightAboveGround;
            }
        }

    }
}


void UFOManipulator::_stop()
{
    _forwardSpeed = 0.0;
    _sideSpeed = 0.0;
    _upSpeed = 0.0;
    _directionRotationRate = 0.0;
}

void UFOManipulator::getCurrentPositionAsLookAt( osg::Vec3 &eye, osg::Vec3 &center, osg::Vec3 &up )
{
    eye = _position;
    center = _position + _direction;
    up.set( 0, 0, 1 );
}

