#include "GliderManipulator.h"

#include <osg/Notify>

using namespace osg;
using namespace osgGA;

GliderManipulator::GliderManipulator()
{
    _modelScale = 0.01f;
    _velocity = 0.0f;
    _yawMode = YAW_AUTOMATICALLY_WHEN_BANKED;
}


GliderManipulator::~GliderManipulator()
{
}


void GliderManipulator::setNode(osg::Node* node)
{
    _node = node;
    if (_node.get())
    {
        const osg::BoundingSphere& boundingSphere=_node->getBound();
        _modelScale = boundingSphere._radius;
    }
}


const osg::Node* GliderManipulator::getNode() const
{
    return _node.get();
}


void GliderManipulator::home(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    if(_node.get() && _camera.get())
    {

        const osg::BoundingSphere& boundingSphere=_node->getBound();

        osg::Vec3 eye = boundingSphere._center+osg::Vec3(-boundingSphere._radius*0.25f,-boundingSphere._radius*0.25f,-boundingSphere._radius*0.03f);

        _camera->setView(eye,
            eye+osg::Vec3(1.0f,1.0f,-0.1f),
            osg::Vec3(0.0f,0.0f,1.0f));

        _velocity = boundingSphere._radius*0.01f;

        us.requestRedraw();

        us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);

        flushMouseEventStack();

    }

}


void GliderManipulator::init(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    flushMouseEventStack();

    us.requestContinuousUpdate(false);

    const osg::BoundingSphere& boundingSphere=_node->getBound();
    _velocity = boundingSphere._radius*0.01f;

    if (ea.getEventType()!=GUIEventAdapter::RESIZE)
    {
        us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);
    }

    _camera->setFusionDistanceRatio(1/1000.0f);

}


bool GliderManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
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


void GliderManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}


void GliderManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}


bool GliderManipulator::calcMovement()
{
    _camera->setFusionDistanceMode(osg::Camera::PROPORTIONAL_TO_SCREEN_DISTANCE);
    _camera->setFusionDistanceRatio(1/300.0f);
    
    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;


    float dt = _ga_t0->time()-_ga_t1->time();

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

    float dx = _ga_t0->getXnormalized();
    float dy = _ga_t0->getYnormalized();

    osg::Vec3 center = _camera->getEyePoint();
    osg::Vec3 sv = _camera->getSideVector();
    osg::Vec3 lv = _camera->getLookVector();

    float pitch = inDegrees(dy*70.0f*dt);
    float roll = inDegrees(dx*60.0f*dt);

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
