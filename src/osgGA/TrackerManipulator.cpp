#include <osgGA/TrackerManipulator>
#include <osg/Quat>
#include <osg/Notify>
#include <osg/Transform>
#include <osgUtil/IntersectVisitor>

using namespace osg;
using namespace osgGA;


class CollectParentPaths : public osg::NodeVisitor
{
public:
    CollectParentPaths() : 
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_PARENTS) {}
        
    virtual void apply(osg::Node& node)
    {
        if (node.getNumParents()==0)
        {
            _nodePaths.push_back(getNodePath());
        }
        traverse(node);
   }
    
    osg::NodePath _nodePath;
    typedef std::vector< osg::NodePath > NodePathList;
    NodePathList _nodePaths;
};

TrackerManipulator::TrackerManipulator()
{
    _rotationMode =ELEVATION_AZIM; 
    _distance = 1.0;

    _thrown = false;

}


TrackerManipulator::~TrackerManipulator()
{
}


void TrackerManipulator::setRotationMode(RotationMode mode)
{
    _rotationMode = mode;

    // need to correct rotation.
}

void TrackerManipulator::setNode(osg::Node* node)
{
    _node = node;
    
    if (_node.get())
    {
        const osg::BoundingSphere& boundingSphere=_node->getBound();
        const float minimumDistanceScale = 0.001f;
        _minimumDistance = osg::clampBetween(
            boundingSphere._radius * minimumDistanceScale,
            0.00001f,1.0f);
            
        osg::notify(osg::INFO)<<"Setting Tracker manipulator _minimumDistance to "<<_minimumDistance<<std::endl;
    }
    if (getAutoComputeHomePosition()) computeHomePosition();    
}


const osg::Node* TrackerManipulator::getNode() const
{
    return _node.get();
}


osg::Node* TrackerManipulator::getNode()
{
    return _node.get();
}


void TrackerManipulator::home(const GUIEventAdapter& ,GUIActionAdapter& us)
{
    if (getAutoComputeHomePosition()) computeHomePosition();

    computePosition(_homeEye, _homeCenter, _homeUp);
    us.requestRedraw();
}


void TrackerManipulator::init(const GUIEventAdapter& ,GUIActionAdapter& )
{
    flushMouseEventStack();
}


void TrackerManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("Trackball: Space","Reset the viewing position to home");
    usage.addKeyboardMouseBinding("Trackball: +","When in stereo, increase the fusion distance");
    usage.addKeyboardMouseBinding("Trackball: -","When in stereo, reduse the fusion distance");
}

bool TrackerManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    switch(ea.getEventType())
    {
        case(GUIEventAdapter::PUSH):
        {
            flushMouseEventStack();
            addMouseEvent(ea);
            if (calcMovement()) us.requestRedraw();
            us.requestContinuousUpdate(false);
            _thrown = false;
            return true;
        }

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
            return true;
        }

        case(GUIEventAdapter::DRAG):
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
            }
            return false;
        default:
            return false;
    }
}


bool TrackerManipulator::isMouseMoving()
{
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

    static const float velocity = 0.1f;

    float dx = _ga_t0->getXnormalized()-_ga_t1->getXnormalized();
    float dy = _ga_t0->getYnormalized()-_ga_t1->getYnormalized();
    float len = sqrtf(dx*dx+dy*dy);
    float dt = _ga_t0->time()-_ga_t1->time();

    return (len>dt*velocity);
}


void TrackerManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}


void TrackerManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}

void TrackerManipulator::setByMatrix(const osg::Matrixd& matrix)
{

    osg::Vec3 lookVector(- matrix(2,0),-matrix(2,1),-matrix(2,2));
    osg::Vec3 eye(matrix(3,0),matrix(3,1),matrix(3,2));
    
    osg::notify(INFO)<<"eye point "<<eye<<std::endl;
    osg::notify(INFO)<<"lookVector "<<lookVector<<std::endl;

    if (!_node)
    {
        _center = eye+ lookVector;
        _distance = lookVector.length();
        matrix.get(_rotation);
        return;
    }


    // need to reintersect with the Tracker
    osgUtil::IntersectVisitor iv;

    const osg::BoundingSphere& bs = _node->getBound();
    float distance = (eye-bs.center()).length() + _node->getBound().radius();
    osg::Vec3d start_segment = eye;
    osg::Vec3d end_segment = eye + lookVector*distance;

    //CoordinateFrame coordinateFrame = getCoordinateFrame(_center.x(), _center.y(), _center.z());
    //osg::notify(INFO)<<"start="<<start_segment<<"\tend="<<end_segment<<"\tupVector="<<getUpVector(coordinateFrame)<<std::endl;

    osg::ref_ptr<osg::LineSegment> segLookVector = new osg::LineSegment;
    segLookVector->set(start_segment,end_segment);
    iv.addLineSegment(segLookVector.get());

    _node->accept(iv);

    bool hitFound = false;
    if (iv.hits())
    {
        osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segLookVector.get());
        if (!hitList.empty())
        {
            notify(INFO) << "Hit Tracker ok A"<< std::endl;
            osg::Vec3d ip = hitList.front().getWorldIntersectPoint();

            _center = ip;

            _distance = (eye-ip).length();
            
            osg::Matrix rotation_matrix = osg::Matrixd::translate(0.0,0.0,-_distance)*
                                          matrix*
                                          osg::Matrixd::translate(-_center);

            rotation_matrix.get(_rotation);

            hitFound = true;
        }
    }

    if (!hitFound)
    {
        CoordinateFrame eyePointCoordFrame = getCoordinateFrame( eye );
        
        // clear the intersect visitor ready for a new test
        iv.reset(); 
               
        osg::ref_ptr<osg::LineSegment> segDowVector = new osg::LineSegment;
        segLookVector->set(eye+getUpVector(eyePointCoordFrame)*distance,
                           eye-getUpVector(eyePointCoordFrame)*distance);
        iv.addLineSegment(segLookVector.get());

        _node->accept(iv);
        
        hitFound = false;
        if (iv.hits())
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segLookVector.get());
            if (!hitList.empty())
            {
                notify(INFO) << "Hit Tracker ok B"<< std::endl;
                osg::Vec3d ip = hitList.front().getWorldIntersectPoint();

                _center = ip;

                _distance = (eye-ip).length();

                _rotation.set(0,0,0,1);

                hitFound = true;
            }
        }
    }    

    CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
    _previousUp = getUpVector(coordinateFrame);

    clampOrientation();
}

osg::Vec3d TrackerManipulator::computeCenter() const
{
    if (_trackNode.valid())
    {
        CollectParentPaths cpp;

        TrackerManipulator* non_const_this = const_cast<TrackerManipulator*>(this);
        
        non_const_this->_trackNode->accept(cpp);

        if (!cpp._nodePaths.empty())
        {
            osg::Matrix localToWorld;
            localToWorld = osg::computeLocalToWorld(cpp._nodePaths[0]);
                        
            return _trackNode->getBound().center() * localToWorld;

        }
    }
    return osg::Vec3d(0.0,0.0,0.0);
}

osg::Matrixd TrackerManipulator::getMatrix() const
{
    return osg::Matrixd::translate(0.0,0.0,_distance)*osg::Matrixd::rotate(_rotation)*osg::Matrix::translate(computeCenter());
}

osg::Matrixd TrackerManipulator::getInverseMatrix() const
{
    return osg::Matrix::translate(-computeCenter())*osg::Matrixd::rotate(_rotation.inverse())*osg::Matrixd::translate(0.0,0.0,-_distance);
}

void TrackerManipulator::computePosition(const osg::Vec3d& eye,const osg::Vec3d& center,const osg::Vec3d& up)
{
    if (!_node) return;

    // compute rotation matrix
    osg::Vec3 lv(center-eye);
    _distance = lv.length();
    _center = center;
    
    osg::notify(osg::INFO) << "In compute"<< std::endl;

    if (_node.valid())
    {
        bool hitFound = false;

        float distance = lv.length();
        float maxDistance = distance+2*(eye-_node->getBound().center()).length();
        osg::Vec3 farPosition = eye+lv*(maxDistance/distance);
        osg::Vec3 endPoint = center;
        for(int i=0;
            !hitFound && i<2;
            ++i, endPoint = farPosition)
        {
            // compute the itersection with the scene.
            osgUtil::IntersectVisitor iv;

            osg::ref_ptr<osg::LineSegment> segLookVector = new osg::LineSegment;
            segLookVector->set(eye,endPoint );
            iv.addLineSegment(segLookVector.get());

            _node->accept(iv);

            if (iv.hits())
            {
                osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segLookVector.get());
                if (!hitList.empty())
                {
                    osg::notify(osg::INFO) << "Hit Tracker ok C"<< std::endl;
                    osg::Vec3d ip = hitList.front().getWorldIntersectPoint();

                    _center = ip;
                    _distance = (ip-eye).length();

                    hitFound = true;
                }
            }
        }
    }

    // note LookAt = inv(CF)*inv(RM)*inv(T) which is equivilant to:
    // inv(R) = CF*LookAt.

    osg::Matrixd rotation_matrix = osg::Matrixd::lookAt(eye,center,up);

    rotation_matrix.get(_rotation);
    _rotation = _rotation.inverse();

    CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
    _previousUp = getUpVector(coordinateFrame);

    clampOrientation();
}

bool TrackerManipulator::calcMovement()
{
    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

    double dx = _ga_t0->getXnormalized()-_ga_t1->getXnormalized();
    double dy = _ga_t0->getYnormalized()-_ga_t1->getYnormalized();


    // return if there is no movement.
    if (dx==0 && dy==0) return false;


    if (_trackNode.valid())
    {
        osg::notify(osg::NOTICE)<<"_trackNode="<<_trackNode->getName()<<std::endl;

        CollectParentPaths cpp;
        _trackNode->accept(cpp);

        osg::notify(osg::NOTICE)<<"number of nodepaths = "<<cpp._nodePaths.size()<<std::endl;
        
        if (!cpp._nodePaths.empty())
        {
            osg::Matrix localToWorld;
            localToWorld = osg::computeLocalToWorld(cpp._nodePaths[0]);
            
            _center = _trackNode->getBound().center() * localToWorld;

            
        }
    }        

    unsigned int buttonMask = _ga_t1->getButtonMask();
    if (buttonMask==GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {

        if (_rotationMode==ELEVATION_AZIM_ROLL)
        {
            // rotate camera.
            osg::Vec3 axis;
            double angle;

            double px0 = _ga_t0->getXnormalized();
            double py0 = _ga_t0->getYnormalized();

            double px1 = _ga_t1->getXnormalized();
            double py1 = _ga_t1->getYnormalized();


            trackball(axis,angle,px1,py1,px0,py0);

            osg::Quat new_rotate;
            new_rotate.makeRotate(angle,axis);

            _rotation = _rotation*new_rotate;
        }
        else
        {
            osg::Matrix rotation_matrix;
            rotation_matrix.set(_rotation);

            osg::Vec3d lookVector = -getUpVector(rotation_matrix);
            osg::Vec3d sideVector = getSideVector(rotation_matrix);
            osg::Vec3d upVector = getFrontVector(rotation_matrix);
            
            CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
            osg::Vec3d localUp = getUpVector(coordinateFrame);
            //osg::Vec3d localUp = _previousUp;
            

            osg::Vec3d forwardVector = localUp^sideVector;
            sideVector = forwardVector^localUp;

            forwardVector.normalize();
            sideVector.normalize();
            
            osg::Quat rotate_elevation;
            rotate_elevation.makeRotate(dy,sideVector);

            osg::Quat rotate_azim;
            rotate_azim.makeRotate(-dx,localUp);
            
            _rotation = _rotation * rotate_elevation * rotate_azim;
            
        }
        
        return true;

    }
    else if (buttonMask==GUIEventAdapter::MIDDLE_MOUSE_BUTTON ||
        buttonMask==(GUIEventAdapter::LEFT_MOUSE_BUTTON|GUIEventAdapter::RIGHT_MOUSE_BUTTON))
    {

        // pan model.
        double scale = -0.3f*_distance;

        osg::Matrix rotation_matrix;
        rotation_matrix.set(_rotation);


        // compute look vector.
        osg::Vec3d lookVector = -getUpVector(rotation_matrix);
        osg::Vec3d sideVector = getSideVector(rotation_matrix);
        osg::Vec3d upVector = getFrontVector(rotation_matrix);

        // CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
        // osg::Vec3d localUp = getUpVector(coordinateFrame);
        osg::Vec3d localUp = _previousUp;

        osg::Vec3d forwardVector =localUp^sideVector;
        sideVector = forwardVector^localUp;

        forwardVector.normalize();
        sideVector.normalize();

        osg::Vec3d dv = forwardVector * (dy*scale) + sideVector * (dx*scale);

        _center += dv;

        // need to recompute the itersection point along the look vector.
        
        if (_node.valid())
        {

            // now reorientate the coordinate frame to the frame coords.
            CoordinateFrame coordinateFrame =  getCoordinateFrame(_center);

            // need to reintersect with the Tracker
            osgUtil::IntersectVisitor iv;

            double distance = _node->getBound().radius()*0.1f;
            osg::Vec3d start_segment = _center + getUpVector(coordinateFrame) * distance;
            osg::Vec3d end_segment = start_segment - getUpVector(coordinateFrame) * (2.0f*distance);

            osg::notify(INFO)<<"start="<<start_segment<<"\tend="<<end_segment<<"\tupVector="<<getUpVector(coordinateFrame)<<std::endl;

            osg::ref_ptr<osg::LineSegment> segLookVector = new osg::LineSegment;
            segLookVector->set(start_segment,end_segment);
            iv.addLineSegment(segLookVector.get());

            _node->accept(iv);

            bool hitFound = false;
            if (iv.hits())
            {
                osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segLookVector.get());
                if (!hitList.empty())
                {
                    notify(INFO) << "Hit Tracker ok"<< std::endl;
                    osg::Vec3d ip = hitList.front().getWorldIntersectPoint();
                    _center = ip;

                    hitFound = true;
                }
            }

            if (!hitFound)
            {
                // ??
                osg::notify(INFO)<<"TrackerManipulator unable to intersect with Tracker."<<std::endl;
            }

            coordinateFrame = getCoordinateFrame(_center);
            osg::Vec3d new_localUp = getUpVector(coordinateFrame);


            osg::Quat pan_rotation;
            pan_rotation.makeRotate(localUp,new_localUp);

            if (!pan_rotation.zeroRotation())
            {
                _rotation = _rotation * pan_rotation;
                _previousUp = new_localUp;
                //osg::notify(osg::NOTICE)<<"Rotating from "<<localUp<<" to "<<new_localUp<<"  angle = "<<acos(localUp*new_localUp/(localUp.length()*new_localUp.length()))<<std::endl;

                //clampOrientation();
            }
            else
            {
                osg::notify(osg::INFO)<<"New up orientation nearly inline - no need to rotate"<<std::endl;
            }
        }        

        return true;
    }
    else if (buttonMask==GUIEventAdapter::RIGHT_MOUSE_BUTTON)
    {

        // zoom model.

        double fd = _distance;
        double scale = 1.0f+dy;
        if (fd*scale>_minimumDistance)
        {

            _distance *= scale;

        } else
        {
            _distance = _minimumDistance;
        }
        
        return true;

    }

    return false;
}

void TrackerManipulator::clampOrientation()
{
    if (_rotationMode==ELEVATION_AZIM)
    {
        osg::Matrix rotation_matrix;
        rotation_matrix.set(_rotation);

        osg::Vec3d lookVector = -getUpVector(rotation_matrix);
        osg::Vec3d upVector = getFrontVector(rotation_matrix);

        CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
        osg::Vec3d localUp = getUpVector(coordinateFrame);
        //osg::Vec3d localUp = _previousUp;

        osg::Vec3d sideVector = lookVector ^ localUp;

        if (sideVector.length()<0.1)
        {
            osg::notify(osg::INFO)<<"Side vector short "<<sideVector.length()<<std::endl;

            sideVector = upVector^localUp;
            sideVector.normalize();
        
        }

        Vec3d newUpVector = sideVector^lookVector;
        newUpVector.normalize();

        osg::Quat rotate_roll;
        rotate_roll.makeRotate(upVector,newUpVector);

        if (!rotate_roll.zeroRotation())
        {
            _rotation = _rotation * rotate_roll;
        }
    }
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
void TrackerManipulator::trackball(osg::Vec3& axis,double & angle, double  p1x, double  p1y, double  p2x, double  p2y)
{
    /*
     * First, figure out z-coordinates for projection of P1 and P2 to
     * deformed sphere
     */

    osg::Matrix rotation_matrix(_rotation);


    osg::Vec3d uv = osg::Vec3d(0.0,1.0,0.0)*rotation_matrix;
    osg::Vec3d sv = osg::Vec3d(1.0,0.0,0.0)*rotation_matrix;
    osg::Vec3d lv = osg::Vec3d(0.0,0.0,-1.0)*rotation_matrix;

    osg::Vec3d p1 = sv*p1x+uv*p1y-lv*tb_project_to_sphere(TRACKBALLSIZE,p1x,p1y);
    osg::Vec3d p2 = sv*p2x+uv*p2y-lv*tb_project_to_sphere(TRACKBALLSIZE,p2x,p2y);

    /*
     *  Now, we want the cross product of P1 and P2
     */

// Robert,
//
// This was the quick 'n' dirty  fix to get the trackball doing the right 
// thing after fixing the Quat rotations to be right-handed.  You may want
// to do something more elegant.
//   axis = p1^p2;
axis = p2^p1;
    axis.normalize();

    /*
     *  Figure out how much to rotate around that axis.
     */
    double t = (p2-p1).length() / (2.0*TRACKBALLSIZE);

    /*
     * Avoid problems with out-of-control values...
     */
    if (t > 1.0) t = 1.0;
    if (t < -1.0) t = -1.0;
    angle = inRadians(asin(t));

}


/*
 * Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
 * if we are away from the center of the sphere.
 */
double TrackerManipulator::tb_project_to_sphere(double  r, double  x, double  y)
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
