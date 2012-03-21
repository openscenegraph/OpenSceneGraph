
#include <osgGA/SphericalManipulator>
#include <osg/Quat>
#include <osg/Notify>
#include <osg/BoundsChecking>

using namespace osg;
using namespace osgGA;

//--------------------------------------------------------------------------------------------------
SphericalManipulator::SphericalManipulator()
{
    _modelScale = 0.01;
    _minimumZoomScale = 0.1;
    _thrown = false;
    _allowThrow = true;

    _distance=1.0;
    _homeDistance=1.0;

    _zoomDelta = 0.1;
    _heading=0.0;
    _elevation=osg::PI_2;

    _rotationMode = ELEVATION_HEADING;
}
//--------------------------------------------------------------------------------------------------
SphericalManipulator::~SphericalManipulator()
{
}
//--------------------------------------------------------------------------------------------------
void SphericalManipulator::setNode(osg::Node* node)
{
    _node = node;
    if (_node.get())
    {
        const osg::BoundingSphere& boundingSphere=_node->getBound();
        _modelScale = boundingSphere._radius;
    }
    if (getAutoComputeHomePosition()) computeHomePosition();
}
//--------------------------------------------------------------------------------------------------
const osg::Node* SphericalManipulator::getNode() const
{
    return _node.get();
}
//--------------------------------------------------------------------------------------------------
osg::Node* SphericalManipulator::getNode()
{
    return _node.get();
}
//--------------------------------------------------------------------------------------------------
void SphericalManipulator::setRotationMode(RotationMode mode)
{
     if(_rotationMode == mode)
         return;

     _rotationMode=mode;

    if(_rotationMode == MAP)
        _elevation=PI_2;
}
//--------------------------------------------------------------------------------------------------
bool SphericalManipulator::setDistance(double distance)
{
    if(distance <= 0)
        return false;

    _distance=distance;

    return true;
}

//--------------------------------------------------------------------------------------------------
void SphericalManipulator::home(double /*currentTime*/)
{
    if(getAutoComputeHomePosition())
        computeHomePosition();

    _heading=3*PI_2;
    _elevation=0.0;
    _center=_homeCenter;
    _distance=_homeDistance;

    _thrown = false;
}
//--------------------------------------------------------------------------------------------------
void SphericalManipulator::home(const GUIEventAdapter& ea ,GUIActionAdapter& us)
{
    home(ea.getTime());
    us.requestRedraw();
    us.requestContinuousUpdate(false);
}
//--------------------------------------------------------------------------------------------------
void SphericalManipulator::init(const GUIEventAdapter& ,GUIActionAdapter& )
{
    flushMouseEventStack();
}
//--------------------------------------------------------------------------------------------------
void SphericalManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("Spherical: Space","Reset the viewing position to home");
    usage.addKeyboardMouseBinding("Spherical: SHIFT","Rotates vertically only");
    usage.addKeyboardMouseBinding("Spherical: ALT","Rotates horizontally only");
}
//--------------------------------------------------------------------------------------------------
void SphericalManipulator::zoomOn(const osg::BoundingSphere& bound)
{
    computeViewPosition(bound,_modelScale,_distance,_center);
    _thrown = false;
}
//--------------------------------------------------------------------------------------------------
bool SphericalManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    switch(ea.getEventType())
    {
    case(GUIEventAdapter::FRAME):
        {
            double current_frame_time = ea.getTime();

            _delta_frame_time = current_frame_time - _last_frame_time;
            _last_frame_time = current_frame_time;

            if (_thrown)
            {
                if (calcMovement()) us.requestRedraw();
            }
            return false;
        }
    default:
        break;
    }

    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
    case(GUIEventAdapter::PUSH):
        {
            flushMouseEventStack();
            addMouseEvent(ea);
            us.requestContinuousUpdate(false);
            _thrown = false;
            return true;
        }

    case(GUIEventAdapter::RELEASE):
        {
            if (ea.getButtonMask()==0)
            {
                double timeSinceLastRecordEvent = _ga_t0.valid() ? (ea.getTime() - _ga_t0->getTime()) : DBL_MAX;
                if (timeSinceLastRecordEvent>0.02) flushMouseEventStack();

                if (isMouseMoving())
                {
                    if (calcMovement())
                    {
                        us.requestRedraw();
                        us.requestContinuousUpdate(true);
                        _thrown = _allowThrow;
                    }
                }
                else
                {
                    flushMouseEventStack();
                    addMouseEvent(ea);
                    if (calcMovement()) us.requestRedraw();
                    us.requestContinuousUpdate(false);
                    _thrown = false;
                }

            }
            else
            {
                flushMouseEventStack();
                addMouseEvent(ea);
                if (calcMovement()) us.requestRedraw();
                us.requestContinuousUpdate(false);
                _thrown = false;
            }
            return true;
        }

    case(GUIEventAdapter::DRAG):
    case(GUIEventAdapter::SCROLL):
        {
            addMouseEvent(ea);
            if (calcMovement()) us.requestRedraw();
            us.requestContinuousUpdate(false);
            _thrown = false;
            return true;
        }

    case(GUIEventAdapter::MOVE):
        {
            return false;
        }

    case(GUIEventAdapter::KEYDOWN):
        if (ea.getKey()== GUIEventAdapter::KEY_Space)
        {
            flushMouseEventStack();
            _thrown = false;
            home(ea,us);
            return true;
        }
        return false;

    case(GUIEventAdapter::FRAME):
        if (_thrown)
        {
            if (calcMovement()) us.requestRedraw();
        }
        return false;

    default:
        return false;
    }
    return false;
}
//--------------------------------------------------------------------------------------------------
bool SphericalManipulator::isMouseMoving()
{
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

    const float velocity = 0.1f;

    float dx = _ga_t0->getXnormalized()-_ga_t1->getXnormalized();
    float dy = _ga_t0->getYnormalized()-_ga_t1->getYnormalized();
    float len = sqrtf(dx*dx+dy*dy);
    float dt = _ga_t0->getTime()-_ga_t1->getTime();

    return (len>dt*velocity);
}
//--------------------------------------------------------------------------------------------------
void SphericalManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}
//--------------------------------------------------------------------------------------------------
void SphericalManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}
//--------------------------------------------------------------------------------------------------
void SphericalManipulator::setByMatrix(const osg::Matrixd& matrix)
{
    _center=osg::Vec3d(0,0,-_distance)*matrix;

    _heading=atan2(-matrix(0,0),matrix(0,1));

    if(_rotationMode != MAP)
    {
        _elevation=asin(matrix(2,2));
    }
}
//--------------------------------------------------------------------------------------------------
osg::Matrixd SphericalManipulator::getMatrix() const
{
    return osg::Matrixd::translate(osg::Vec3d(0.0,0.0,_distance))*
           osg::Matrixd::rotate(PI_2-_elevation,1.0,0.0,0.0)*
           osg::Matrixd::rotate(PI_2+_heading,0.0,0.0,1.0)*
           osg::Matrixd::translate(_center);
}
//--------------------------------------------------------------------------------------------------
osg::Matrixd SphericalManipulator::getInverseMatrix() const
{
    return osg::Matrixd::translate(-_center)*
           osg::Matrixd::rotate(PI_2+_heading,0.0,0.0,-1.0)*
           osg::Matrixd::rotate(PI_2-_elevation,-1.0,0.0,0.0)*
           osg::Matrixd::translate(osg::Vec3d(0.0,0.0,-_distance));
}

//--------------------------------------------------------------------------------------------------
bool SphericalManipulator::calcMovement()
{
    // mouse scroll is only a single event
    if (_ga_t0.get()==NULL) return false;

    float dx=0.0f;
    float dy=0.0f;
    unsigned int buttonMask=osgGA::GUIEventAdapter::NONE;

    if (_ga_t0->getEventType()==GUIEventAdapter::SCROLL)
    {
        dy = _ga_t0->getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP ? _zoomDelta : -_zoomDelta;
        buttonMask=GUIEventAdapter::SCROLL;
    }
    else
    {

        if (_ga_t1.get()==NULL) return false;
        dx = _ga_t0->getXnormalized()-_ga_t1->getXnormalized();
        dy = _ga_t0->getYnormalized()-_ga_t1->getYnormalized();
        float distance = sqrtf(dx*dx + dy*dy);

        // return if movement is too fast, indicating an error in event values or change in screen.
        if (distance>0.5)
        {
            return false;
        }

        // return if there is no movement.
        if (distance==0.0f)
        {
            return false;
        }

        buttonMask = _ga_t1->getButtonMask();
    }

    double throwScale =  (_thrown && _ga_t0.valid() && _ga_t1.valid()) ?
        _delta_frame_time / (_ga_t0->getTime() - _ga_t1->getTime()) : 1.0;

    if (buttonMask==GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        // rotate camera.

        if(_rotationMode == MAP)
        {
            float pxc = (_ga_t0->getXmax()+_ga_t0->getXmin())/2;
            float pyc = (_ga_t0->getYmax()+_ga_t0->getYmin())/2;

            float px0 = _ga_t0->getX();
            float py0 = _ga_t0->getY();

            float px1 = _ga_t1->getX();
            float py1 = _ga_t1->getY();

            float angle=atan2(py1-pyc,px1-pxc)-atan2(py0-pyc,px0-pxc);

            _heading+=throwScale*angle;
            if(_heading < -PI)
                _heading+=2*PI;
            else if(_heading > PI)
                _heading-=2*PI;
        }
        else
        {
            if((_rotationMode != ELEVATION) && ((_ga_t1->getModKeyMask() & GUIEventAdapter::MODKEY_SHIFT) == 0))
            {
                _heading-=throwScale*dx*PI_2;

                if(_heading < 0)
                    _heading+=2*PI;
                else if(_heading > 2*PI)
                    _heading-=2*PI;
            }

            if((_rotationMode != HEADING) && ((_ga_t1->getModKeyMask() & GUIEventAdapter::MODKEY_ALT) == 0))
            {
                _elevation-=throwScale*dy*osg::PI_4;

                // Only allows vertical rotation of 180deg
                if(_elevation < -osg::PI_2)
                    _elevation=-osg::PI_2;
                else if(_elevation > osg::PI_2)
                    _elevation=osg::PI_2;
            }
        }

        return true;
    }
    else if (buttonMask==GUIEventAdapter::MIDDLE_MOUSE_BUTTON ||
        buttonMask==(GUIEventAdapter::LEFT_MOUSE_BUTTON|GUIEventAdapter::RIGHT_MOUSE_BUTTON))
    {
        // pan model.

        float scale = -0.3f*_distance;

        osg::Matrix rotation_matrix;
        rotation_matrix=osg::Matrixd::rotate(_elevation,-1,0,0)*osg::Matrixd::rotate(PI_2+_heading,0,0,1);

        osg::Vec3d dv(throwScale*dx*scale,0,throwScale*dy*scale);
        _center += dv*rotation_matrix;

        return true;

    }
    else if (buttonMask==GUIEventAdapter::RIGHT_MOUSE_BUTTON || _ga_t0->getEventType()==GUIEventAdapter::SCROLL)
    {

        // zoom model.

        double fd = _distance;
        double scale = 1.0+throwScale*dy;
        if(fd*scale > _modelScale*_minimumZoomScale)
        {
            _distance *= scale;
        }
        else
        {
            OSG_DEBUG << "Pushing forward"<<std::endl;
            // push the camera forward.
            scale = -fd;

            osg::Matrix rotation_matrix=osg::Matrixd::rotate(_elevation,-1,0,0)*
                osg::Matrixd::rotate(PI_2+_heading,0,0,1);

            osg::Vec3d dv = (osg::Vec3d(0.0f,0.0f,-1.0f)*rotation_matrix)*(dy*scale);

            _center += dv;
        }

        return true;
    }

    return false;
}
//--------------------------------------------------------------------------------------------------
void SphericalManipulator::computeHomePosition()
{
    if(getNode())
        computeViewPosition(getNode()->getBound(),_modelScale,_homeDistance,_homeCenter);
}
//--------------------------------------------------------------------------------------------------
void SphericalManipulator::computeViewPosition(const osg::BoundingSphere& bound,
    double& scale,double& distance,osg::Vec3d& center)
{
    scale=bound._radius;
    distance=3.5*bound._radius;
    if(distance <= 0)
        distance=1;
    center=bound._center;
}
