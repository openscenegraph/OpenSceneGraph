#include <osgUtil/FlightManipulator>
#include <osg/Types>
#include <osg/Notify>

using namespace osg;
using namespace osgUtil;

FlightManipulator::FlightManipulator()
{
    _modelScale = 0.01f;
    _velocity = 0.0f;
    _yawMode = YAW_AUTOMATICALLY_WHEN_BANKED;
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


void FlightManipulator::home(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    if(_node.get() && _camera.get())
    {

        const osg::BoundingSphere& boundingSphere=_node->getBound();

        _camera->setLookAt(
            boundingSphere._center+osg::Vec3( 0.0,-2.0f * boundingSphere._radius,0.0f),
            boundingSphere._center,
            osg::Vec3(0.0f,0.0f,1.0f));

        _velocity = 0.0f;

        us.requestRedraw();

        us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2,(ea.getYmin()+ea.getYmax())/2);

        flushMouseEventStack();

    }

}


void FlightManipulator::init(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    flushMouseEventStack();

    us.requestContinuousUpdate(false);

    _velocity = 0.0f;

    us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2,(ea.getYmin()+ea.getYmax())/2);

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

        }
        return true;
        case(GUIEventAdapter::RELEASE):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();

        }
        return true;
        case(GUIEventAdapter::DRAG):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();

        }
        return true;
        case(GUIEventAdapter::MOVE):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();

        }
        return true;

        case(GUIEventAdapter::KEYBOARD):
            if (ea.getKey()==' ')
            {
                flushMouseEventStack();
                home(ea,us);
                us.requestRedraw();
                us.requestContinuousUpdate(false);
                return true;
            }
            return false;
        case(GUIEventAdapter::FRAME):
            addMouseEvent(ea);
            if (calcMovement()) us.requestRedraw();
            return true;
        case(GUIEventAdapter::RESIZE):
        {
            init(ea,us);
            us.requestRedraw();
        }
        return true;
        default:
            return false;
    }
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


bool FlightManipulator::calcMovement()
{
    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

    float dt = _ga_t0->time()-_ga_t1->time();

    if (dt<0.0f)
    {
        notify(WARN) << "warning dt = "<<dt<<endl;
        dt = 0.0f;
    }

    unsigned int buttonMask = _ga_t1->getButtonMask();
    if (buttonMask==GUIEventAdapter::LEFT_BUTTON)
    {
        // pan model.

        _velocity += dt*_modelScale*0.05f;

    }
    else if (buttonMask==GUIEventAdapter::MIDDLE_BUTTON ||
        buttonMask==(GUIEventAdapter::LEFT_BUTTON|GUIEventAdapter::RIGHT_BUTTON))
    {

        _velocity = 0.0f;

    }
    else if (buttonMask==GUIEventAdapter::RIGHT_BUTTON)
    {

        _velocity -= dt*_modelScale*0.05f;

    }

    float mx = (_ga_t0->getXmin()+_ga_t0->getXmax())/2.0f;
    float my = (_ga_t0->getYmin()+_ga_t0->getYmax())/2.0f;

    float dx = _ga_t0->getX()-mx;
    float dy = _ga_t0->getY()-my;

    osg::Vec3 center = _camera->getEyePoint();
    osg::Vec3 sv = _camera->getSideVector();
    osg::Vec3 lv = _camera->getLookVector();

    float pitch = inDegrees(dy*0.15f*dt);
    float roll = inDegrees(dx*0.1f*dt);

    osg::Matrix mat;
    mat.makeTranslate(-center);
    mat *= Matrix::rotate(pitch,sv.x(),sv.y(),sv.z());
    mat *= Matrix::rotate(roll,lv.x(),lv.y(),lv.z());
    if (_yawMode==YAW_AUTOMATICALLY_WHEN_BANKED)
    {
        float bank = asinf(sv.z());
        float yaw = inRadians(bank)*dt;
        mat *= Matrix::rotate(yaw,0.0f,0.0f,1.0f);
    }

    lv *= (_velocity*dt);

    mat *= Matrix::translate(center+lv);

    _camera->transformLookAt(mat);

    return true;
}
