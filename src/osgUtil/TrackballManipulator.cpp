#include <osgUtil/TrackballManipulator>
#include <osg/Types>
#include <osg/Notify>

using namespace osg;
using namespace osgUtil;

TrackballManipulator::TrackballManipulator()
{
    _modelScale = 0.01f;
    _minimumZoomScale = 0.05f;
    _thrown = false;
}


TrackballManipulator::~TrackballManipulator()
{
}


void TrackballManipulator::setNode(osg::Node* node)
{
    _node = node;
    if (_node.get())
    {
        const osg::BoundingSphere& boundingSphere=_node->getBound();
        _modelScale = boundingSphere._radius;
    }
}


const osg::Node* TrackballManipulator::getNode() const
{
    return _node.get();
}


                                 /*ea*/
void TrackballManipulator::home(const GUIEventAdapter& ,GUIActionAdapter& us)
{
    if(_node.get() && _camera.get())
    {

        const osg::BoundingSphere& boundingSphere=_node->getBound();

        _camera->setView(boundingSphere._center+osg::Vec3( 0.0,-2.0f * boundingSphere._radius,0.0f),
                        boundingSphere._center,
                        osg::Vec3(0.0f,0.0f,1.0f));

        us.requestRedraw();
    }

}


void TrackballManipulator::init(const GUIEventAdapter& ,GUIActionAdapter& )
{
    flushMouseEventStack();
}


bool TrackballManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    if(!_camera.get()) return false;

    switch(ea.getEventType())
    {
        case(GUIEventAdapter::PUSH):
        {
            flushMouseEventStack();
            addMouseEvent(ea);
            if (calcMovement()) us.requestRedraw();
            us.requestContinuousUpdate(false);
            _thrown = false;
        }
        return true;
        case(GUIEventAdapter::RELEASE):
        {
            if (ea.getButtonMask()==0)
            {

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
        }
        return true;
        case(GUIEventAdapter::DRAG):
        {
            addMouseEvent(ea);
            if (calcMovement()) us.requestRedraw();
            us.requestContinuousUpdate(false);
            _thrown = false;
        }
        return true;
        case(GUIEventAdapter::MOVE):
        {
        }
        return false;
        case(GUIEventAdapter::KEYBOARD):
            if (ea.getKey()==' ')
            {
                flushMouseEventStack();
                _thrown = false;
                home(ea,us);
                us.requestRedraw();
                us.requestContinuousUpdate(false);
                return true;
            }
            return false;
        case(GUIEventAdapter::FRAME):
            if (_thrown)
            {
                if (calcMovement()) us.requestRedraw();
                return true;
            }
            return false;
        default:
            return false;
    }
}


bool TrackballManipulator::isMouseMoving()
{
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

    static const float velocity = 100.0f;

    float dx = _ga_t0->getX()-_ga_t1->getX();
    float dy = _ga_t0->getY()-_ga_t1->getY();
    float len = sqrtf(dx*dx+dy*dy);
    float dt = _ga_t0->time()-_ga_t1->time();

    return (len>dt*velocity);
}


void TrackballManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}


void TrackballManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}


bool TrackballManipulator::calcMovement()
{
    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

    float dx = dx = _ga_t0->getX()-_ga_t1->getX();
    float dy = _ga_t0->getY()-_ga_t1->getY();

    // return if there is no movement.
    if (dx==0 && dy==0) return false;

    unsigned int buttonMask = _ga_t1->getButtonMask();
    if (buttonMask==GUIEventAdapter::LEFT_BUTTON)
    {

        // rotate camera.

        osg::Vec3 center = _camera->getCenterPoint();
        osg::Vec3 axis;
        float angle;

        float mx0 = (_ga_t0->getXmin()+_ga_t0->getXmax())/2.0f;
        float rx0 = (_ga_t0->getXmax()-_ga_t0->getXmin())/2.0f;

        float my0 = (_ga_t0->getYmin()+_ga_t0->getYmax())/2.0f;
        float ry0 = (_ga_t0->getYmax()-_ga_t0->getYmin())/2.0f;

        float mx1 = (_ga_t0->getXmin()+_ga_t1->getXmax())/2.0f;
        float rx1 = (_ga_t0->getXmax()-_ga_t1->getXmin())/2.0f;

        float my1 = (_ga_t1->getYmin()+_ga_t1->getYmax())/2.0f;
        float ry1 = (_ga_t1->getYmax()-_ga_t1->getYmin())/2.0f;

        float px0 = (_ga_t0->getX()-mx0)/rx0;
        float py0 = (my0-_ga_t0->getY())/ry0;

        float px1 = (_ga_t1->getX()-mx1)/rx1;
        float py1 = (my1-_ga_t1->getY())/ry1;

        trackball(axis,angle,px1,py1,px0,py0);

        osg::Matrix mat;
        mat.makeTrans(-center.x(),-center.y(),-center.z());
        mat *= Matrix::rotate(angle,axis.x(),axis.y(),axis.z());
        mat *= Matrix::trans(center.x(),center.y(),center.z());

        _camera->transformLookAt(mat);

        return true;

    }
    else if (buttonMask==GUIEventAdapter::MIDDLE_BUTTON ||
        buttonMask==(GUIEventAdapter::LEFT_BUTTON|GUIEventAdapter::RIGHT_BUTTON))
    {

        // pan model.

        float scale = 0.0015f*_camera->getFocalLength();

        osg::Vec3 uv = _camera->getUpVector();
        osg::Vec3 sv = _camera->getSideVector();
        osg::Vec3 dv = uv*(dy*scale)-sv*(dx*scale);

        osg::Matrix mat;
        mat.makeTrans(dv.x(),dv.y(),dv.z());

        _camera->transformLookAt(mat);

        return true;

    }
    else if (buttonMask==GUIEventAdapter::RIGHT_BUTTON)
    {

        // zoom model.

        float fd = _camera->getFocalLength();
        float scale = 1.0f-dy*0.001f;
        if (fd*scale>_modelScale*_minimumZoomScale)
        {
            // zoom camera in.
            osg::Vec3 center = _camera->getCenterPoint();

            osg::Matrix mat;
            mat.makeTrans(-center.x(),-center.y(),-center.z());
            mat *= Matrix::scale(scale,scale,scale);
            mat *= Matrix::trans(center.x(),center.y(),center.z());

            _camera->transformLookAt(mat);

        }
        else
        {

            //            notify(DEBUG_INFO) << "Pushing forward"<<endl;
            // push the camera forward.
            float scale = 0.0015f*fd;
            osg::Vec3 dv = _camera->getLookVector()*(dy*scale);

            osg::Matrix mat;
            mat.makeTrans(dv.x(),dv.y(),dv.z());

            _camera->transformLookAt(mat);

        }

        return true;

    }

    return false;
}


/*
 * This size should really be based on the distance from the center of
 * rotation to the point on the object underneath the mouse.  That
 * point would then track the mouse as closely as possible.  This is a
 * simple example, though, so that is left as an Exercise for the
 * Programmer.
 */
const float TRACKBALLSIZE = 0.8f;

/*
 * Ok, simulate a track-ball.  Project the points onto the virtual
 * trackball, then figure out the axis of rotation, which is the cross
 * product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
 * Note:  This is a deformed trackball-- is a trackball in the center,
 * but is deformed into a hyperbolic sheet of rotation away from the
 * center.  This particular function was chosen after trying out
 * several variations.
 *
 * It is assumed that the arguments to this routine are in the range
 * (-1.0 ... 1.0)
 */
void TrackballManipulator::trackball(osg::Vec3& axis,float& angle, float p1x, float p1y, float p2x, float p2y)
{
    /*
     * First, figure out z-coordinates for projection of P1 and P2 to
     * deformed sphere
     */

    osg::Vec3 uv = _camera->getUpVector();
    osg::Vec3 sv = _camera->getSideVector();
    osg::Vec3 lv = _camera->getLookVector();

    osg::Vec3 p1 = sv*p1x+uv*p1y-lv*tb_project_to_sphere(TRACKBALLSIZE,p1x,p1y);
    osg::Vec3 p2 = sv*p2x+uv*p2y-lv*tb_project_to_sphere(TRACKBALLSIZE,p2x,p2y);

    /*
     *  Now, we want the cross product of P1 and P2
     */
    axis = p1^p2;
    axis.normalize();

    /*
     *  Figure out how much to rotate around that axis.
     */
    float t = (p2-p1).length() / (2.0*TRACKBALLSIZE);

    /*
     * Avoid problems with out-of-control values...
     */
    if (t > 1.0) t = 1.0;
    if (t < -1.0) t = -1.0;
    angle = asin(t) * 180.0f/M_PI;

}


/*
 * Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
 * if we are away from the center of the sphere.
 */
float TrackballManipulator::tb_project_to_sphere(float r, float x, float y)
{
    float d, t, z;

    d = sqrt(x*x + y*y);
                                 /* Inside sphere */
    if (d < r * 0.70710678118654752440)
    {
        z = sqrt(r*r - d*d);
    }                            /* On hyperbola */
    else
    {
        t = r / 1.41421356237309504880;
        z = t*t / d;
    }
    return z;
}
