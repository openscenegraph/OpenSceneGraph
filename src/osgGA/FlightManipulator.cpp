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
#include <osg/Notify>

using namespace osg;
using namespace osgGA;

FlightManipulator::FlightManipulator()
{
    _modelScale = 0.01f;
    _acceleration = 100.0f;
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

    if (getAutoComputeHomePosition()) computeHomePosition();    
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
    if (getAutoComputeHomePosition()) computeHomePosition();

    computePosition(_homeEye, _homeCenter, _homeUp);
    
    _velocity = 0.0;

    us.requestRedraw();
    us.requestContinuousUpdate(false);
    us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);

    flushMouseEventStack();
}


void FlightManipulator::init(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    flushMouseEventStack();

    us.requestContinuousUpdate(false);

    _velocity = 0.0f;

    if (ea.getEventType()!=GUIEventAdapter::RESIZE)
    {
        us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);
    }
}


bool FlightManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    switch(ea.getEventType())
    {
        case(GUIEventAdapter::FRAME):
            addMouseEvent(ea);
            if (calcMovement()) us.requestRedraw();
            return false;

        case(GUIEventAdapter::RESIZE):
            init(ea,us);
            us.requestRedraw();
            return true;
        default:
            break;
    }

    if (ea.getHandled()) return false;

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
        {
            if (ea.getKey()==GUIEventAdapter::KEY_Space)
            {
                flushMouseEventStack();
                home(ea,us);
                us.requestRedraw();
                us.requestContinuousUpdate(false);
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
        }

        default:
            return false;
    }
}

void FlightManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("Flight: Space","Reset the viewing position to home");
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


void FlightManipulator::setByMatrix(const osg::Matrixd& matrix)
{
    _eye = matrix.getTrans();
    _rotation = matrix.getRotate();
    _distance = 1.0f;
}

osg::Matrixd FlightManipulator::getMatrix() const
{
    return osg::Matrixd::rotate(_rotation)*osg::Matrixd::translate(_eye);
}

osg::Matrixd FlightManipulator::getInverseMatrix() const
{
    return osg::Matrixd::translate(-_eye)*osg::Matrixd::rotate(_rotation.inverse());
}

void FlightManipulator::computePosition(const osg::Vec3& eye,const osg::Vec3& center,const osg::Vec3& up)
{
    osg::Vec3d lv = center-eye;

    osg::Vec3 f(lv);
    f.normalize();
    osg::Vec3 s(f^up);
    s.normalize();
    osg::Vec3 u(s^f);
    u.normalize();
    
    osg::Matrixd rotation_matrix(s[0],     u[0],     -f[0],     0.0f,
                                s[1],     u[1],     -f[1],     0.0f,
                                s[2],     u[2],     -f[2],     0.0f,
                                0.0f,     0.0f,     0.0f,      1.0f);
                   
    _eye = eye;
    _distance = lv.length();
    _rotation = rotation_matrix.getRotate().inverse();
}


bool FlightManipulator::calcMovement()
{
    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;


    double dt = _ga_t0->getTime()-_ga_t1->getTime();

    if (dt<0.0f)
    {
        notify(INFO) << "warning dt = "<<dt<< std::endl;
        dt = 0.0f;
    }

    unsigned int buttonMask = _ga_t1->getButtonMask();
    if (buttonMask==GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        // pan model.

        _velocity += dt*(_acceleration+_velocity);

    }
    else if (buttonMask==GUIEventAdapter::MIDDLE_MOUSE_BUTTON ||
        buttonMask==(GUIEventAdapter::LEFT_MOUSE_BUTTON|GUIEventAdapter::RIGHT_MOUSE_BUTTON))
    {

        _velocity = 0.0f;

    }
    else if (buttonMask==GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {

        _velocity -= dt*(_acceleration+_velocity);

    }

    float dx = _ga_t0->getXnormalized();
    float dy = _ga_t0->getYnormalized();

    osg::CoordinateFrame cf=getCoordinateFrame(_eye);

    osg::Matrixd rotation_matrix;
    rotation_matrix.makeRotate(_rotation);
    
    osg::Vec3d up = osg::Vec3(0.0,1.0,0.0) * rotation_matrix;
    osg::Vec3d lv = osg::Vec3(0.0,0.0,-1.0) * rotation_matrix;

    osg::Vec3d sv = lv^up;
    sv.normalize();

    double pitch = -inDegrees(dy*50.0f*dt);
    double roll = inDegrees(dx*50.0f*dt);

    osg::Quat delta_rotate;

    osg::Quat roll_rotate;
    osg::Quat pitch_rotate;

    pitch_rotate.makeRotate(pitch,sv.x(),sv.y(),sv.z());
    roll_rotate.makeRotate(roll,lv.x(),lv.y(),lv.z());

    delta_rotate = pitch_rotate*roll_rotate;

    if (_yawMode==YAW_AUTOMATICALLY_WHEN_BANKED)
    {
        //float bank = asinf(sv.z());
        double bank = asinf(sv *getUpVector(cf));
        double yaw = inRadians(bank)*dt;
        
        osg::Quat yaw_rotate;
        //yaw_rotate.makeRotate(yaw,0.0f,0.0f,1.0f);

        yaw_rotate.makeRotate(yaw,getUpVector(cf));


        delta_rotate = delta_rotate*yaw_rotate;
    }

    lv *= (_velocity*dt);

    _eye += lv;
    _rotation = _rotation*delta_rotate;

    return true;
}
