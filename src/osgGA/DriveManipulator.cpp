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

#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <stdlib.h>

#include <osgGA/DriveManipulator>
#include <osgUtil/LineSegmentIntersector>
#include <osg/Notify>

using namespace osg;
using namespace osgGA;

#define DRIVER_HEIGHT 15


// #define ABOSULTE_PITCH 1
// #define INCREMENTAL_PITCH 1
#define KEYBOARD_PITCH 1

static double getHeightOfDriver()
{
    double height = 1.5;
    if (getenv("OSG_DRIVE_MANIPULATOR_HEIGHT"))
    {
        height = osg::asciiToDouble(getenv("OSG_DRIVE_MANIPULATOR_HEIGHT"));
    }
    OSG_INFO<<"DriveManipulator::_height set to =="<<height<<std::endl;
    return height;
}

DriveManipulator::DriveManipulator()
{
    _modelScale = 1.0;
    _velocity = 0.0;
    _height = getHeightOfDriver();
    _buffer = _height*2.5;

    //_speedMode = USE_MOUSE_Y_FOR_SPEED;
    _speedMode = USE_MOUSE_BUTTONS_FOR_SPEED;

    _pitch = 0.0;
    _distance = 0.0;

    _pitchUpKeyPressed = false;
    _pitchDownKeyPressed = false;

}


DriveManipulator::~DriveManipulator()
{
}


void DriveManipulator::setNode(osg::Node* node)
{
    _node = node;
    if (_node.get())
    {
        const osg::BoundingSphere& boundingSphere=_node->getBound();

        _modelScale = boundingSphere._radius;
        //_height = sqrtf(_modelScale)*0.03;
        //_buffer = sqrtf(_modelScale)*0.05;

        _height = getHeightOfDriver();
        _buffer = _height*2.5;
    }
    if (getAutoComputeHomePosition()) computeHomePosition();
}


const osg::Node* DriveManipulator::getNode() const
{
    return _node.get();
}


osg::Node* DriveManipulator::getNode()
{
    return _node.get();
}

bool DriveManipulator::intersect(const osg::Vec3d& start, const osg::Vec3d& end, osg::Vec3d& intersection, osg::Vec3d& normal) const
{
    osg::ref_ptr<osgUtil::LineSegmentIntersector> lsi = new osgUtil::LineSegmentIntersector(start,end);

    osgUtil::IntersectionVisitor iv(lsi.get());
    iv.setTraversalMask(_intersectTraversalMask);

    _node->accept(iv);

    if (lsi->containsIntersections())
    {
        intersection = lsi->getIntersections().begin()->getWorldIntersectPoint();
        normal = lsi->getIntersections().begin()->getWorldIntersectNormal();
        return true;
    }
    return false;
}


void DriveManipulator::computeHomePosition()
{
    if(_node.get())
    {
        const osg::BoundingSphere& boundingSphere=_node->getBound();

        osg::Vec3d ep = boundingSphere._center;
        osg::Vec3d bp = ep;

        osg::CoordinateFrame cf=getCoordinateFrame(ep);

        ep -= getUpVector(cf)* _modelScale*0.0001;
        bp -= getUpVector(cf)* _modelScale;

        // check to see if any obstruction in front.
        bool positionSet = false;

        osg::Vec3d ip, np;
        if (intersect(ep, bp, ip, np))
        {
            osg::Vec3d uv;
            if (np * getUpVector(cf)>0.0) uv = np;
            else uv = -np;

            ep = ip;
            ep += getUpVector(cf)*_height;
            osg::Vec3d lv = uv^osg::Vec3d(1.0,0.0,0.0);

            setHomePosition(ep,ep+lv,uv);

            positionSet = true;

        }

        if (!positionSet)
        {
            bp = ep;
            bp += getUpVector(cf)*_modelScale;


            if (intersect(ep, bp, ip, np))
            {

                osg::Vec3d uv;
                if (np*getUpVector(cf)>0.0) uv = np;
                else uv = -np;

                ep = ip;
                ep += getUpVector(cf)*_height;
                osg::Vec3d lv = uv^osg::Vec3d(1.0,0.0,0.0);
                setHomePosition(ep,ep+lv,uv);

                positionSet = true;

            }
        }

        if (!positionSet)
        {
            setHomePosition(
                boundingSphere._center+osg::Vec3d( 0.0,-2.0 * boundingSphere._radius,0.0),
                boundingSphere._center+osg::Vec3d( 0.0,-2.0 * boundingSphere._radius,0.0)+osg::Vec3d(0.0,1.0,0.0),
                osg::Vec3d(0.0,0.0,1.0));
        }

    }
}

void DriveManipulator::home(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    if (getAutoComputeHomePosition()) computeHomePosition();

    computePosition(_homeEye, _homeCenter, _homeUp);

    _velocity = 0.0;

    _pitch = 0.0;

    us.requestRedraw();
    us.requestContinuousUpdate(false);

    us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);

    flushMouseEventStack();
}

void DriveManipulator::init(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    flushMouseEventStack();

    us.requestContinuousUpdate(false);

    _velocity = 0.0;

    osg::Vec3d ep = _eye;

    osg::CoordinateFrame cf=getCoordinateFrame(ep);

    Matrixd rotation_matrix;
    rotation_matrix.makeRotate(_rotation);
    osg::Vec3d sv = osg::Vec3d(1.0,0.0,0.0) * rotation_matrix;
    osg::Vec3d bp = ep;
    bp -= getUpVector(cf)*_modelScale;

    bool positionSet = false;
    osg::Vec3d ip, np;
    if (intersect(ep, bp, ip, np))
    {

        osg::Vec3d uv;
        if (np*getUpVector(cf)>0.0) uv = np;
        else uv = -np;

        ep = ip+uv*_height;
        osg::Vec3d lv = uv^sv;

        computePosition(ep,ep+lv,uv);

        positionSet = true;
    }

    if (!positionSet)
    {
        bp = ep;
        bp += getUpVector(cf)*_modelScale;

        if (intersect(ep, bp, ip, np))
        {

            osg::Vec3d uv;
            if (np*getUpVector(cf)>0.0f) uv = np;
            else uv = -np;

            ep = ip+uv*_height;
            osg::Vec3d lv = uv^sv;

            computePosition(ep,ep+lv,uv);

            positionSet = true;
        }
    }

    if (ea.getEventType()!=GUIEventAdapter::RESIZE)
    {
        us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);
    }
}


bool DriveManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
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
                return true;
            }
            else if (ea.getKey()=='q')
            {
                _speedMode = USE_MOUSE_Y_FOR_SPEED;
                return true;
            }
            else if (ea.getKey()=='a')
            {
                _speedMode = USE_MOUSE_BUTTONS_FOR_SPEED;
                return true;
            }
#ifdef KEYBOARD_PITCH
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Up ||
                     ea.getKey()=='9')
            {
                _pitchUpKeyPressed = true;
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Down ||
                     ea.getKey()=='6')
            {
                _pitchDownKeyPressed = true;
                return true;
            }
#endif
            return false;
        }

        case(GUIEventAdapter::KEYUP):
        {
#ifdef KEYBOARD_PITCH
            if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Up ||
                     ea.getKey()=='9')
            {
                _pitchUpKeyPressed = false;
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Down ||
                     ea.getKey()=='6')
            {
                _pitchDownKeyPressed = false;
                return true;
            }
#endif
            return false;
        }

        default:
            return false;
    }
}

void DriveManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("Drive: Space","Reset the viewing position to home");
    usage.addKeyboardMouseBinding("Drive: q","Use mouse y for controlling speed");
    usage.addKeyboardMouseBinding("Drive: a","Use mouse middle,right mouse buttons for speed");
    usage.addKeyboardMouseBinding("Drive: Down","Cursor down key to look downwards");
    usage.addKeyboardMouseBinding("Drive: Up","Cursor up key to look upwards");
}


void DriveManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}


void DriveManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}

void DriveManipulator::setByMatrix(const osg::Matrixd& matrix)
{
    _eye = matrix.getTrans();
    _rotation = matrix.getRotate();
}

osg::Matrixd DriveManipulator::getMatrix() const
{
    return osg::Matrixd::rotate(_pitch,1.0,0.0,0.0)*osg::Matrixd::rotate(_rotation)*osg::Matrixd::translate(_eye);
}

osg::Matrixd DriveManipulator::getInverseMatrix() const
{
    return osg::Matrixd::translate(-_eye)*osg::Matrixd::rotate(_rotation.inverse())*osg::Matrixd::rotate(-_pitch,1.0,0.0,0.0);
}

void DriveManipulator::computePosition(const osg::Vec3d& eye,const osg::Vec3d& center,const osg::Vec3d& up)
{
    osg::Vec3d lv = center-eye;

    osg::Vec3d f(lv);
    f.normalize();
    osg::Vec3d s(f^up);
    s.normalize();
    osg::Vec3d u(s^f);
    u.normalize();

    osg::Matrixd rotation_matrix(s[0],     u[0],     -f[0],     0.0,
                                 s[1],     u[1],     -f[1],     0.0,
                                 s[2],     u[2],     -f[2],     0.0,
                                 0.0,       0.0,       0.0,     1.0);

    _eye = eye;
    _rotation = rotation_matrix.getRotate().inverse();
}


bool DriveManipulator::calcMovement()
{
    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

    double dt = _ga_t0->getTime()-_ga_t1->getTime();

    if (dt<0.0f)
    {
        OSG_INFO << "warning dt = "<<dt<< std::endl;
        dt = 0.0;
    }

    double accelerationFactor = _height*10.0;

    switch(_speedMode)
    {
        case(USE_MOUSE_Y_FOR_SPEED):
        {
            double dy = _ga_t0->getYnormalized();
            _velocity = _height*dy;
            break;
        }
        case(USE_MOUSE_BUTTONS_FOR_SPEED):
        {
            unsigned int buttonMask = _ga_t1->getButtonMask();
            if (buttonMask==GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                // pan model.

                _velocity += dt*accelerationFactor;

            }
            else if (buttonMask==GUIEventAdapter::MIDDLE_MOUSE_BUTTON ||
                buttonMask==(GUIEventAdapter::LEFT_MOUSE_BUTTON|GUIEventAdapter::RIGHT_MOUSE_BUTTON))
            {

                _velocity = 0.0;

            }
            else if (buttonMask==GUIEventAdapter::RIGHT_MOUSE_BUTTON)
            {

                _velocity -= dt*accelerationFactor;

            }
            break;
        }
    }

    osg::CoordinateFrame cf=getCoordinateFrame(_eye);

    osg::Matrixd rotation_matrix;
    rotation_matrix.makeRotate(_rotation);

    osg::Vec3d up = osg::Vec3d(0.0,1.0,0.0) * rotation_matrix;
    osg::Vec3d lv = osg::Vec3d(0.0,0.0,-1.0) * rotation_matrix;
    osg::Vec3d sv = osg::Vec3d(1.0,0.0,0.0) * rotation_matrix;

    // rotate the camera.
    double dx = _ga_t0->getXnormalized();

    double yaw = -inDegrees(dx*50.0*dt);


#ifdef KEYBOARD_PITCH
    double pitch_delta = 0.5;
    if (_pitchUpKeyPressed) _pitch += pitch_delta*dt;
    if (_pitchDownKeyPressed) _pitch -= pitch_delta*dt;
#endif

#if defined(ABOSULTE_PITCH)
    // absolute pitch
    double dy = _ga_t0->getYnormalized();
    _pitch = -dy*0.5;
#elif defined(INCREMENTAL_PITCH)
    // incremental pitch
    double dy = _ga_t0->getYnormalized();
    _pitch += dy*dt;
#endif

    osg::Quat yaw_rotation;
    yaw_rotation.makeRotate(yaw,up);

    _rotation *= yaw_rotation;

    rotation_matrix.makeRotate(_rotation);

    sv = osg::Vec3d(1.0,0.0,0.0) * rotation_matrix;

    // movement is big enough the move the eye point along the look vector.
    if (fabs(_velocity*dt)>1e-8)
    {
        double distanceToMove = _velocity*dt;

        double signedBuffer;
        if (distanceToMove>=0.0) signedBuffer=_buffer;
        else signedBuffer=-_buffer;

        // check to see if any obstruction in front.
        osg::Vec3d ip, np;
        if (intersect(_eye,_eye+lv*(signedBuffer+distanceToMove), ip, np))
        {
            if (distanceToMove>=0.0)
            {
                distanceToMove = (ip-_eye).length()-_buffer;
            }
            else
            {
                distanceToMove = _buffer-(ip-_eye).length();
            }

            _velocity = 0.0;
        }

        // check to see if forward point is correct height above terrain.
        osg::Vec3d fp = _eye + lv*distanceToMove;
        osg::Vec3d lfp = fp - up*(_height*5.0);


        if (intersect(fp, lfp, ip, np))
        {
            if (up*np>0.0) up = np;
            else up = -np;

            _eye = ip+up*_height;

            lv = up^sv;

            computePosition(_eye,_eye+lv,up);

            return true;

        }

        // no hit on the terrain found therefore resort to a fall under
        // under the influence of gravity.
        osg::Vec3d dp = lfp;
        dp -= getUpVector(cf)* (2.0*_modelScale);

        if (intersect(lfp, dp, ip, np))
        {

            if (up*np>0.0) up = np;
            else up = -np;

            _eye = ip+up*_height;

            lv = up^sv;

            computePosition(_eye,_eye+lv,up);

            return true;
        }

        // no collision with terrain has been found therefore track horizontally.

        lv *= (_velocity*dt);

        _eye += lv;
    }

    return true;
}
