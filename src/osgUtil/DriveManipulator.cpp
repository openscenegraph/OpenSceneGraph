#include "osgUtil/DriveManipulator"
#include "osgUtil/IntersectVisitor"
#include "osg/Notify"

using namespace osg;
using namespace osgUtil;

DriveManipulator::DriveManipulator()
{
    _modelScale = 0.01f;
    _velocity = 0.0f;
    _speedMode = USE_MOUSE_Y_FOR_SPEED;
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
        _height = sqrtf(_modelScale)*0.03f;
        _buffer = sqrtf(_modelScale)*0.05f;
    }
}

osg::Node* DriveManipulator::getNode() const
{
    return _node.get();
}

void DriveManipulator::home(GUIEventAdapter& ea,GUIActionAdapter& us)
{

    if(_node.get() && _camera.get())
    {

	const osg::BoundingSphere& boundingSphere=_node->getBound();

        osg::Vec3 ep = boundingSphere._center;
        osg::Vec3 bp = ep;
        ep.z() -= _modelScale*0.0001f;
        bp.z() -= _modelScale;

        // check to see if any obstruction in front.
        IntersectVisitor iv;

        bool cameraSet = false;

        osg::ref_ptr<osg::Seg> segDown = new osg::Seg;
        segDown->set(ep,bp);
        iv.addSeg(segDown.get());

        _node->accept(iv);

        if (iv.hits())
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segDown.get());
            if (!hitList.empty())
            {
    //                notify(INFO) << "Hit terrain ok"<<endl;
                osg::Vec3 ip = hitList.front()._intersectPoint;
                osg::Vec3 np = hitList.front()._intersectNormal;
                
                osg::Vec3 uv;
                if (np.z()>0.0f) uv = np;
                else uv = -np;

                float lookDistance = _modelScale*0.1f;

                ep = ip;
                ep.z() += _height;
                osg::Vec3 lv = uv^osg::Vec3(1.0f,0.0f,0.0f);
                osg::Vec3 lp = ep+lv*lookDistance;

                _camera->setView(ep,lp,uv);
                _camera->ensureOrthogonalUpVector();
                
                cameraSet = true;

            }

        }

        if (!cameraSet)
        {
            bp = ep;
            bp.z() += _modelScale;

            osg::ref_ptr<osg::Seg> segUp = new osg::Seg;
            segUp->set(ep,bp);
            iv.addSeg(segUp.get());

            _node->accept(iv);

            if (iv.hits())
            {
                osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segUp.get());
                if (!hitList.empty())
                {
        //                notify(INFO) << "Hit terrain ok"<<endl;
                    osg::Vec3 ip = hitList.front()._intersectPoint;
                    osg::Vec3 np = hitList.front()._intersectNormal;

                osg::Vec3 uv;
                if (np.z()>0.0f) uv = np;
                else uv = -np;

                float lookDistance = _modelScale*0.1f;

                ep = ip;
                ep.z() += _height;
                osg::Vec3 lv = uv^osg::Vec3(1.0f,0.0f,0.0f);
                osg::Vec3 lp = ep+lv*lookDistance;

                _camera->setView(ep,lp,uv);
                _camera->ensureOrthogonalUpVector();
                
                cameraSet = true;

                }

            }
        }
        
        if (!cameraSet)
        {
            _camera->setView(boundingSphere._center+osg::Vec3( 0.0,-2.0f * boundingSphere._radius,0.0f),	// eye
                             boundingSphere._center,                                // look
                             osg::Vec3(0.0f,0.0f,1.0f));                           // up
        }
                             
    }

    _velocity = 0.0f;

    us.needRedraw();

    us.needWarpPointer((ea.getXmin()+ea.getXmax())/2,(ea.getYmin()+ea.getYmax())/2);

    flushMouseEventStack();

}

void DriveManipulator::init(GUIEventAdapter& ea,GUIActionAdapter& us)
{
    flushMouseEventStack();

    us.needContinuousUpdate(false);

    _velocity = 0.0f;

    osg::Vec3 ep = _camera->getEyePoint();
    osg::Vec3 sv = _camera->getSideVector();
    osg::Vec3 bp = ep;
    bp.z() -= _modelScale;

    // check to see if any obstruction in front.
    IntersectVisitor iv;

    bool cameraSet = false;

    osg::ref_ptr<osg::Seg> segDown = new osg::Seg;
    segDown->set(ep,bp);
    iv.addSeg(segDown.get());

    _node->accept(iv);

    if (iv.hits())
    {
        osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segDown.get());
        if (!hitList.empty())
        {
//                notify(INFO) << "Hit terrain ok"<<endl;
            osg::Vec3 ip = hitList.front()._intersectPoint;
            osg::Vec3 np = hitList.front()._intersectNormal;

            osg::Vec3 uv;
            if (np.z()>0.0f) uv = np;
            else uv = -np;

            float lookDistance = _modelScale*0.1f;

            ep = ip+uv*_height;
            osg::Vec3 lv = uv^sv;
            osg::Vec3 lp = ep+lv*lookDistance;

            _camera->setView(ep,lp,uv);
            _camera->ensureOrthogonalUpVector();

            cameraSet = true;

        }

    }

    if (!cameraSet)
    {
        bp = ep;
        bp.z() += _modelScale;

        osg::ref_ptr<osg::Seg> segUp = new osg::Seg;
        segUp->set(ep,bp);
        iv.addSeg(segUp.get());

        _node->accept(iv);

        if (iv.hits())
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segUp.get());
            if (!hitList.empty())
            {
    //                notify(INFO) << "Hit terrain ok"<<endl;
                osg::Vec3 ip = hitList.front()._intersectPoint;
                osg::Vec3 np = hitList.front()._intersectNormal;

            osg::Vec3 uv;
            if (np.z()>0.0f) uv = np;
            else uv = -np;

            float lookDistance = _modelScale*0.1f;

            ep = ip+uv*_height;
            osg::Vec3 lv = uv^sv;
            osg::Vec3 lp = ep+lv*lookDistance;

            _camera->setView(ep,lp,uv);
            _camera->ensureOrthogonalUpVector();

            cameraSet = true;

            }

        }
    }

    us.needWarpPointer((ea.getXmin()+ea.getXmax())/2,(ea.getYmin()+ea.getYmax())/2);

}

bool DriveManipulator::update(GUIEventAdapter& ea,GUIActionAdapter& us)
{
    if(!_camera.get()) return false;
    
    switch(ea.getEventType())
    {
    case(GUIEventAdapter::PUSH):
        {
            
            addMouseEvent(ea);
            us.needContinuousUpdate(true);
            if (calcMovement()) us.needRedraw();

        }
        return true;
    case(GUIEventAdapter::RELEASE):
        {
            
            addMouseEvent(ea);
            us.needContinuousUpdate(true);
            if (calcMovement()) us.needRedraw();

        }
        return true;
    case(GUIEventAdapter::DRAG):
        {
            
            addMouseEvent(ea);
            us.needContinuousUpdate(true);
            if (calcMovement()) us.needRedraw();

        }
        return true;
    case(GUIEventAdapter::MOVE):
        {
            
            addMouseEvent(ea);
            us.needContinuousUpdate(true);
            if (calcMovement()) us.needRedraw();

        }
        return true;

    case(GUIEventAdapter::KEYBOARD):
        if (ea.getKey()==' ') 
        {
            flushMouseEventStack();
            home(ea,us);
            us.needRedraw();
            us.needContinuousUpdate(false);
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

        return false;
    case(GUIEventAdapter::FRAME):
        addMouseEvent(ea);
        if (calcMovement()) us.needRedraw();
        return true;
    case(GUIEventAdapter::RESIZE):
        {
            init(ea,us);
            us.needRedraw();
        }
        return true;
    default:
        return false;
    }
}

void DriveManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}

void DriveManipulator::addMouseEvent(GUIEventAdapter& ea)
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}


bool DriveManipulator::calcMovement()
{
    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

    float dt = _ga_t0->time()-_ga_t1->time();


    if (dt<0.0f)
    {
	notify(WARN) << "warning dt = "<<dt<<endl;
	dt = 0.0f;
    }

    switch(_speedMode)
    {
        case(USE_MOUSE_Y_FOR_SPEED):
        {    
            float my = (_ga_t0->getYmin()+_ga_t0->getYmax())/2.0f;
            float dy = _ga_t0->getY()-my;
            _velocity = -_modelScale*0.0002f*dy;
            break;
        }
        case(USE_MOUSE_BUTTONS_FOR_SPEED):
        {    
            unsigned int buttonMask = _ga_t1->getButtonMask();
            if (buttonMask==GUIEventAdapter::LEFT_BUTTON)
            {
                // pan model.

                _velocity += dt*_modelScale*0.02f;

            }
            else if (buttonMask==GUIEventAdapter::MIDDLE_BUTTON || 
                     buttonMask==(GUIEventAdapter::LEFT_BUTTON|GUIEventAdapter::RIGHT_BUTTON))
            {

                _velocity = 0.0f;

            }
            else if (buttonMask==GUIEventAdapter::RIGHT_BUTTON)
            {

                _velocity -= dt*_modelScale*0.02f;

            }
            break; 
        }
    }


    // rotate the camera.
    osg::Vec3 center = _camera->getEyePoint();
    osg::Vec3 uv = _camera->getUpVector();
    
    float mx = (_ga_t0->getXmin()+_ga_t0->getXmax())/2.0f;

    float dx = _ga_t0->getX()-mx;

    float yaw = dx*0.1f*dt;

    osg::Matrix mat;
    mat.makeTrans(-center.x(),-center.y(),-center.z());
    mat.postRot(yaw,uv.x(),uv.y(),uv.z());
    mat.postTrans(center.x(),center.y(),center.z());

    center = _camera->getEyePoint();
    uv = _camera->getUpVector();
    
    _camera->mult(*_camera,mat);


    // get the new forward (look) vector.
    osg::Vec3 sv = _camera->getSideVector();
    osg::Vec3 lv = _camera->getLookPoint()-_camera->getEyePoint();
    float lookDistance = lv.length();
    lv.normalize();

    // movement is big enough the move the eye point along the look vector.
    if (fabsf(_velocity*dt)>1e-8)
    {
        osg::Vec3 ep = _camera->getEyePoint();
        float distanceToMove = _velocity*dt;

        float signedBuffer;
        if (distanceToMove>=0.0f) signedBuffer=_buffer;
        else signedBuffer=-_buffer;

        // check to see if any obstruction in front.
        IntersectVisitor iv;
        osg::ref_ptr<osg::Seg> segForward = new osg::Seg;
        segForward->set(ep,ep+lv*(signedBuffer+distanceToMove));
        iv.addSeg(segForward.get());

        _node->accept(iv);

        if (iv.hits())
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segForward.get());
            if (!hitList.empty())
            {
//                notify(INFO) << "Hit obstruction"<<endl;
                osg::Vec3 ip = hitList.front()._intersectPoint;
                distanceToMove = (ip-ep).length()-_buffer;
                _velocity = 0.0f;
            }

        }


        // check to see if forward point is correct height above terrain.
        osg::Vec3 fp = ep+lv*distanceToMove;
        osg::Vec3 lfp = fp-uv*_height*5;

        iv.reset();
        
        osg::ref_ptr<osg::Seg> segNormal = new osg::Seg;
        segNormal->set(fp,lfp);
        iv.addSeg(segNormal.get());

        _node->accept(iv);

        if (iv.hits())
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segNormal.get());
            if (!hitList.empty())
            {
//                notify(INFO) << "Hit terrain ok"<<endl;
                osg::Vec3 ip = hitList.front()._intersectPoint;
                osg::Vec3 np = hitList.front()._intersectNormal;

                if (uv*np>0.0f) uv = np;
                else uv = -np;

                ep = ip+uv*_height;
                lv = uv^sv;
                osg::Vec3 lp = ep+lv*lookDistance;
                
                _camera->setView(ep,lp,uv);
                _camera->ensureOrthogonalUpVector();

                return true;
                
            }

        }

        // no hit on the terrain found therefore resort to a fall under
        // under the influence of gravity.
        osg::Vec3 dp = lfp;
        dp.z() -= 2*_modelScale;
    
        iv.reset();
        
        osg::ref_ptr<osg::Seg> segFall = new osg::Seg;
        segFall->set(lfp,dp);
        iv.addSeg(segFall.get());

        _node->accept(iv);

        if (iv.hits())
        {
            osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segFall.get());
            if (!hitList.empty())
            {

                notify(INFO) << "Hit terrain on decent ok"<<endl;
                osg::Vec3 ip = hitList.front()._intersectPoint;
                osg::Vec3 np = hitList.front()._intersectNormal;

                if (uv*np>0.0f) uv = np;
                else uv = -np;

                ep = ip+uv*_height;
                lv = uv^sv;
                osg::Vec3 lp = ep+lv*lookDistance;
                
                _camera->setView(ep,lp,uv);
                _camera->ensureOrthogonalUpVector();
                
                return true;
            }
        }

        // no collision with terrain has been found therefore track horizontally.

        lv *= (_velocity*dt);
        ep += lv;
        osg::Vec3 lp = _camera->getLookPoint()+lv;

        _camera->setView(ep,lp,uv);
        _camera->ensureOrthogonalUpVector();        

    }



    return true;
}
