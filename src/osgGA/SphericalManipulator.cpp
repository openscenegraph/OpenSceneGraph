
#include <osgGA/SphericalManipulator>
#include <osg/Quat>
#include <osg/Notify>
#include <osg/BoundsChecking>

using namespace osg;
using namespace osgGA;

//--------------------------------------------------------------------------------------------------
SphericalManipulator::SphericalManipulator()
{
    _modelScale = 0.01f;
    _minimumZoomScale = 0.1f;
    _thrown = false;

    _distance=1.0f;
    _homeDistance=1.0f;

    _zoomDelta = 0.1f;
    _azimuth=0;
    _zenith=osg::PI_2;

    setRotationMode(MODE_3D);
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

    if(_rotationMode == MODE_2D)
        _zenith=0;
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

    _azimuth=3*PI_2;
    _zenith=PI_2;
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
        if (_thrown)
        {
            if (calcMovement()) us.requestRedraw();
        }
        return false;
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
                        _thrown = true;
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
    _center=osg::Vec3(0,0,-_distance)*matrix;
    _azimuth=atan2(-matrix(0,0),matrix(0,1));
    if(_rotationMode != MODE_2D)
        _zenith=acos(matrix(2,2));
}
//--------------------------------------------------------------------------------------------------
osg::Matrixd SphericalManipulator::getMatrix() const
{
    return
        osg::Matrixd::translate(osg::Vec3d(0,0,_distance))*
        osg::Matrixd::rotate(_zenith,1,0,0)*
        osg::Matrixd::rotate(PI_2+_azimuth,0,0,1)*
        osg::Matrixd::translate(_center);
}
//--------------------------------------------------------------------------------------------------
osg::Matrixd SphericalManipulator::getInverseMatrix() const
{
    return
        osg::Matrixd::translate(-_center)*
        osg::Matrixd::rotate(PI_2+_azimuth,0,0,-1)*
        osg::Matrixd::rotate(_zenith,-1,0,0)*
        osg::Matrixd::translate(osg::Vec3d(0,0,-_distance));
}
//--------------------------------------------------------------------------------------------------
double SphericalManipulator::computeAngles(const osg::Vec3d &vec,double& azimuth,double& zenith)
{
    osg::Vec3d lv(vec);
    double distance=lv.length();
    if(distance > 0.0)
    {
        lv /= distance;
    }

    azimuth=atan2(lv.y(),lv.x());
    zenith=acos(lv.z());

    return distance;
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
    
    if (buttonMask==GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        // rotate camera.

        if(_rotationMode == MODE_2D)
        {
            float pxc = (_ga_t0->getXmax()+_ga_t0->getXmin())/2;
            float pyc = (_ga_t0->getYmax()+_ga_t0->getYmin())/2;

            float px0 = _ga_t0->getX();
            float py0 = _ga_t0->getY();

            float px1 = _ga_t1->getX();
            float py1 = _ga_t1->getY();

            float angle=atan2(py1-pyc,px1-pxc)-atan2(py0-pyc,px0-pxc);

            _azimuth+=angle;
            if(_azimuth < -PI)
                _azimuth+=2*PI;
            else if(_azimuth > PI)
                _azimuth-=2*PI;
        }
        else
        {
            if((_rotationMode != MODE_3D_VERTICAL) && ((_ga_t1->getModKeyMask() & GUIEventAdapter::MODKEY_SHIFT) == 0))
            {
                _azimuth-=dx*PI_2;

                if(_azimuth < 0)
                    _azimuth+=2*PI;
                else if(_azimuth > 2*PI)
                    _azimuth-=2*PI;
            }

            if((_rotationMode != MODE_3D_HORIZONTAL) && ((_ga_t1->getModKeyMask() & GUIEventAdapter::MODKEY_ALT) == 0))
            {
                _zenith+=dy*PI_4;

                // Only allows vertical rotation of 180deg 
                if(_zenith < 0)
                    _zenith=0;
                else if(_zenith > PI)
                    _zenith=PI;
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
        rotation_matrix=osg::Matrixd::rotate(_zenith,1,0,0)*osg::Matrixd::rotate(PI_2+_azimuth,0,0,1);

        osg::Vec3 dv(dx*scale,dy*scale,0);
        _center += dv*rotation_matrix;

        return true;

    }
    else if (buttonMask==GUIEventAdapter::RIGHT_MOUSE_BUTTON || _ga_t0->getEventType()==GUIEventAdapter::SCROLL)
    {

        // zoom model.

        double fd = _distance;
        double scale = 1.0+dy;
        if(fd*scale > _modelScale*_minimumZoomScale)
        {
            _distance *= scale;
        }
        else
        {
            notify(osg::DEBUG_INFO) << "Pushing forward"<<std::endl;
            // push the camera forward.
            scale = -fd;

            osg::Matrix rotation_matrix=osg::Matrixd::rotate(_zenith,1,0,0)*
                osg::Matrixd::rotate(PI_2+_azimuth,0,0,1);

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
