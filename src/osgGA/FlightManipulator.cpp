#include <osgGA/FlightManipulator>
#include <osg/Notify>

using namespace osg;
using namespace osgGA;

FlightManipulator::FlightManipulator()
{
    _modelScale = 0.01f;
    _velocity = 0.0f;
    _yawMode = YAW_AUTOMATICALLY_WHEN_BANKED;

    _distance = 1.0f;
}


FlightManipulator::~FlightManipulator()
{
}


void FlightManipulator::setNode(osg::Node* node)
{
    _node = node;
    if (_node.get())
    {
        const osg::BoundingSphere& boundingSphere=_node->getBound();
        _modelScale = boundingSphere._radius;
    }
}


const osg::Node* FlightManipulator::getNode() const
{
    return _node.get();
}



osg::Node* FlightManipulator::getNode()
{
    return _node.get();
}

void FlightManipulator::home(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    if(_node.get() && _camera.get())
    {

        const osg::BoundingSphere& boundingSphere=_node->getBound();

        _camera->setLookAt(
            boundingSphere._center+osg::Vec3( 0.0,-3.0f * boundingSphere._radius,0.0f),
            boundingSphere._center,
            osg::Vec3(0.0f,0.0f,1.0f));

        _velocity = 0.0f;

        us.requestRedraw();

        us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2,(ea.getYmin()+ea.getYmax())/2);

        computeLocalDataFromCamera();

        flushMouseEventStack();

    }

}


void FlightManipulator::init(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    flushMouseEventStack();

    us.requestContinuousUpdate(false);

    _velocity = 0.0f;

    if (ea.getEventType()!=GUIEventAdapter::RESIZE)
    {
        us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2,(ea.getYmin()+ea.getYmax())/2);
    }

    computeLocalDataFromCamera();
}


bool FlightManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    if(!_camera.get()) return false;

    switch(ea.getEventType())
    {
        case(GUIEventAdapter::PUSH):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::RELEASE):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::DRAG):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::MOVE):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();

            return true;
        }

        case(GUIEventAdapter::KEYDOWN):
            if (ea.getKey()==' ')
            {
                flushMouseEventStack();
                home(ea,us);
                us.requestRedraw();
                us.requestContinuousUpdate(false);
                return true;
            }
            else if (ea.getKey()=='+')
            {
                _camera->setFusionDistanceRatio(_camera->getFusionDistanceRatio()*1.25f);
                return true;
            }
            else if (ea.getKey()=='-')
            {
                _camera->setFusionDistanceRatio(_camera->getFusionDistanceRatio()/1.25f);
                return true;
            }
            else if (ea.getKey()=='q')
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

        case(GUIEventAdapter::FRAME):
            addMouseEvent(ea);
            if (calcMovement()) us.requestRedraw();
            return true;

        case(GUIEventAdapter::RESIZE):
            init(ea,us);
            us.requestRedraw();
            return true;

        default:
            return false;
    }
}

void FlightManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("Flight: Space","Reset the viewing position to home");
    usage.addKeyboardMouseBinding("Flight: +","When in stereo, increase the fusion distance");
    usage.addKeyboardMouseBinding("Flight: -","When in stereo, reduse the fusion distance");
    usage.addKeyboardMouseBinding("Flight: q","Automatically yaw when banked (default)");
    usage.addKeyboardMouseBinding("Flight: a","No yaw when banked");
}

void FlightManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}


void FlightManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}


void FlightManipulator::computeLocalDataFromCamera()
{
    // maths from gluLookAt/osg::Matrix::makeLookAt
    osg::Vec3 f(_camera->getCenterPoint()-_camera->getEyePoint());
    f.normalize();
    osg::Vec3 s(f^_camera->getUpVector());
    s.normalize();
    osg::Vec3 u(s^f);
    u.normalize();

    osg::Matrix rotation_matrix(s[0],     u[0],     -f[0],     0.0f,
                                s[1],     u[1],     -f[1],     0.0f,
                                s[2],     u[2],     -f[2],     0.0f,
                                0.0f,     0.0f,     0.0f,      1.0f);
                   
    _eye = _camera->getEyePoint();
    _distance = _camera->getLookDistance();
    _rotation.set(rotation_matrix);
    _rotation = _rotation.inverse();
     
}

void FlightManipulator::computeCameraFromLocalData()
{
    osg::Matrix new_rotation;
    new_rotation.makeRotate(_rotation);
    
    osg::Vec3 up = osg::Vec3(0.0f,1.0f,0.0) * new_rotation;
    osg::Vec3 center = (osg::Vec3(0.0f,0.0f,-_distance) * new_rotation) + _eye;

    _camera->setLookAt(_eye,center,up);
}

bool FlightManipulator::calcMovement()
{
    _camera->setFusionDistanceMode(osg::Camera::PROPORTIONAL_TO_SCREEN_DISTANCE);

    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;


    double dt = _ga_t0->time()-_ga_t1->time();

    if (dt<0.0f)
    {
        notify(WARN) << "warning dt = "<<dt<< std::endl;
        dt = 0.0f;
    }

    unsigned int buttonMask = _ga_t1->getButtonMask();
    if (buttonMask==GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        // pan model.

        _velocity += dt*_modelScale*0.05f;

    }
    else if (buttonMask==GUIEventAdapter::MIDDLE_MOUSE_BUTTON ||
        buttonMask==(GUIEventAdapter::LEFT_MOUSE_BUTTON|GUIEventAdapter::RIGHT_MOUSE_BUTTON))
    {

        _velocity = 0.0f;

    }
    else if (buttonMask==GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {

        _velocity -= dt*_modelScale*0.05f;

    }

    float mx = (_ga_t0->getXmin()+_ga_t0->getXmax())/2.0f;
    float my = (_ga_t0->getYmin()+_ga_t0->getYmax())/2.0f;

    float dx = _ga_t0->getX()-mx;
    float dy = _ga_t0->getY()-my;

    osg::Matrix rotation_matrix;
    rotation_matrix.makeRotate(_rotation);
    
    osg::Vec3 up = osg::Vec3(0.0f,1.0f,0.0) * rotation_matrix;
    osg::Vec3 lv = osg::Vec3(0.0f,0.0f,-1.0f) * rotation_matrix;

    osg::Vec3 sv = lv^up;
    sv.normalize();

    float pitch = inDegrees(dy*0.15f*dt);
    float roll = inDegrees(dx*0.1f*dt);

    osg::Quat delta_rotate;

    osg::Quat roll_rotate;
    osg::Quat pitch_rotate;

    pitch_rotate.makeRotate(pitch,sv.x(),sv.y(),sv.z());
    roll_rotate.makeRotate(roll,lv.x(),lv.y(),lv.z());

    delta_rotate = pitch_rotate*roll_rotate;

    if (_yawMode==YAW_AUTOMATICALLY_WHEN_BANKED)
    {
        float bank = asinf(sv.z());
        float yaw = inRadians(bank)*dt;
        
        osg::Quat yaw_rotate;
        yaw_rotate.makeRotate(yaw,0.0f,0.0f,1.0f);

        delta_rotate = delta_rotate*yaw_rotate;
    }

    lv *= (_velocity*dt);

    _eye += lv;
    _rotation = _rotation*delta_rotate;

    computeCameraFromLocalData();

    return true;
}
