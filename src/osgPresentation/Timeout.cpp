/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2018 Robert Osfield
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

#include <osgPresentation/Timeout>
#include <osgUtil/CullVisitor>
#include <osgGA/EventVisitor>

using namespace osgPresentation;


class OperationVisitor : public osg::NodeVisitor
{
public:

    enum Operation
    {
        ENTER,
        LEAVE,
        RESET
    };

    OperationVisitor(Operation op) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _operation(op), _sleepTime(0.0) {}

    void apply(osg::Node& node)
    {
        if (node.getStateSet()) process(node.getStateSet());
        traverse(node);
    }

    void apply(osg::Geode& geode)
    {
        apply(static_cast<osg::Node&>(geode));

        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            osg::Drawable* drawable = geode.getDrawable(i);
            if (drawable->getStateSet()) process(drawable->getStateSet());
        }
    }

    virtual void process(osg::StateSet* ss)
    {
        for(unsigned int i=0;i<ss->getTextureAttributeList().size();++i)
        {
            osg::Texture* texture = dynamic_cast<osg::Texture*>(ss->getTextureAttribute(i,osg::StateAttribute::TEXTURE));
            osg::Image* image = texture ? texture->getImage(0) : 0;
            osg::ImageStream* imageStream = dynamic_cast<osg::ImageStream*>(image);
            if (imageStream) process(imageStream);
        }
    }

    void process(osg::ImageStream* video)
    {
        if (_operation==ENTER)
        {
            video->rewind();
            video->play();

            _sleepTime = 0.2;
        }
        else if (_operation==LEAVE)
        {
            video->pause();
        }
        else if (_operation==RESET)
        {
            video->rewind();

            _sleepTime = 0.2;
        }
    }

    double sleepTime() const { return _sleepTime; }

    Operation   _operation;
    double      _sleepTime;
};


HUDSettings::HUDSettings(double slideDistance, float eyeOffset, unsigned int leftMask, unsigned int rightMask):
    _slideDistance(slideDistance),
    _eyeOffset(eyeOffset),
    _leftMask(leftMask),
    _rightMask(rightMask)
{
}

HUDSettings::~HUDSettings()
{
}

bool HUDSettings::getModelViewMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
{
    matrix.makeLookAt(osg::Vec3d(0.0,0.0,0.0),osg::Vec3d(0.0,_slideDistance,0.0),osg::Vec3d(0.0,0.0,1.0));

    if (nv)
    {
        if (nv->getTraversalMask()==_leftMask)
        {
            matrix.postMultTranslate(osg::Vec3(_eyeOffset,0.0,0.0));
        }
        else if (nv->getTraversalMask()==_rightMask)
        {
            matrix.postMultTranslate(osg::Vec3(-_eyeOffset,0.0,0.0));
        }
    }

    return true;
}

bool HUDSettings::getInverseModelViewMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
{
    osg::Matrix modelView;
    getModelViewMatrix(modelView,nv);
    matrix.invert(modelView);
    return true;
}




Timeout::Timeout(HUDSettings* hudSettings):
    _previousFrameNumber(-1),
    _timeOfLastEvent(0.0),
    _displayTimeout(false),
    _idleDurationBeforeTimeoutDisplay(DBL_MAX),
    _idleDurationBeforeTimeoutAction(DBL_MAX),
    _keyStartsTimoutDisplay(0),
    _keyDismissTimoutDisplay(0),
    _keyRunTimeoutAction(0)
{
    _hudSettings = hudSettings;
    setCullingActive(false);
    setNumChildrenRequiringEventTraversal(1);
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
Timeout::Timeout(const Timeout& timeout,const osg::CopyOp& copyop):
    osg::Transform(timeout, copyop),
    _hudSettings(timeout._hudSettings),
    _previousFrameNumber(timeout._previousFrameNumber),
    _timeOfLastEvent(timeout._timeOfLastEvent),
    _displayTimeout(timeout._displayTimeout),
    _idleDurationBeforeTimeoutDisplay(timeout._idleDurationBeforeTimeoutDisplay),
    _idleDurationBeforeTimeoutAction(timeout._idleDurationBeforeTimeoutAction),
    _keyStartsTimoutDisplay(timeout._keyStartsTimoutDisplay),
    _keyDismissTimoutDisplay(timeout._keyDismissTimoutDisplay),
    _keyRunTimeoutAction(timeout._keyRunTimeoutAction),
    _displayBroadcastKeyPos(timeout._displayBroadcastKeyPos),
    _dismissBroadcastKeyPos(timeout._dismissBroadcastKeyPos),
    _actionKeyPos(timeout._actionKeyPos),
    _actionBroadcastKeyPos(timeout._actionBroadcastKeyPos),
    _actionJumpData(timeout._actionJumpData)
{
    setDataVariance(osg::Object::DYNAMIC);
    setReferenceFrame(osg::Transform::ABSOLUTE_RF);
}

Timeout::~Timeout()
{
}

bool Timeout::computeLocalToWorldMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const
{
    if (_hudSettings.valid()) return _hudSettings->getModelViewMatrix(matrix,nv);
    else return false;
}

bool Timeout::computeWorldToLocalMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const
{
    if (_hudSettings.valid()) return _hudSettings->getInverseModelViewMatrix(matrix,nv);
    else return false;
}

void Timeout::broadcastEvent(osgViewer::Viewer* viewer, const osgPresentation::KeyPosition& keyPos)
{
    osg::ref_ptr<osgGA::GUIEventAdapter> event = new osgGA::GUIEventAdapter;

    if (keyPos._key!=0) event->setEventType(osgGA::GUIEventAdapter::KEYDOWN);
    else event->setEventType(osgGA::GUIEventAdapter::MOVE);

    if (keyPos._key!=0) event->setKey(keyPos._key);
    if (keyPos._x!=FLT_MAX) event->setX(keyPos._x);
    if (keyPos._y!=FLT_MAX) event->setY(keyPos._y);

    event->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

    // dispatch cloned event to devices
    osgViewer::View::Devices& devices = viewer->getDevices();
    for(osgViewer::View::Devices::iterator i = devices.begin(); i != devices.end(); ++i)
    {
        if((*i)->getCapabilities() & osgGA::Device::SEND_EVENTS)
        {
            (*i)->sendEvent(*event);
        }
    }
}
void Timeout::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor* cv = nv.asCullVisitor();
        if (_displayTimeout && cv)
        {
            osgUtil::RenderStage* previous_stage = cv->getCurrentRenderBin()->getStage();

            osg::ref_ptr<osgUtil::RenderStage> rs = new osgUtil::RenderStage;

            osg::ColorMask* colorMask = previous_stage->getColorMask();
            rs->setColorMask(colorMask);

            // set up the viewport.
            osg::Viewport* viewport = previous_stage->getViewport();
            rs->setViewport( viewport );

            rs->setClearMask(GL_DEPTH_BUFFER_BIT);

            // record the render bin, to be restored after creation
            // of the render to text
            osgUtil::RenderBin* previousRenderBin = cv->getCurrentRenderBin();

            // set the current renderbin to be the newly created stage.
            cv->setCurrentRenderBin(rs.get());

            // traverse the subgraph
            {
                Transform::traverse(nv);
            }

            // restore the previous renderbin.
            cv->setCurrentRenderBin(previousRenderBin);

            // and the render to texture stage to the current stages
            // dependency list.
            cv->getCurrentRenderBin()->getStage()->addPostRenderStage(rs.get(),0);
        }
    }
    else if (nv.getVisitorType()==osg::NodeVisitor::EVENT_VISITOR)
    {
        int deltaFrameNumber = (nv.getFrameStamp()->getFrameNumber()-_previousFrameNumber);
        _previousFrameNumber = nv.getFrameStamp()->getFrameNumber();

        bool needToRecordEventTime = false;
        bool needToAction = false;

        if (deltaFrameNumber>1)
        {
            needToRecordEventTime = true;
        }

        bool previous_displayTimeout = _displayTimeout;
        bool needToDismiss = false;

        osgGA::EventVisitor* ev = nv.asEventVisitor();
        osgViewer::Viewer* viewer = ev ? dynamic_cast<osgViewer::Viewer*>(ev->getActionAdapter()) : 0;
        if (ev)
        {
            osgGA::EventQueue::Events& events = ev->getEvents();
            for(osgGA::EventQueue::Events::iterator itr = events.begin();
                itr != events.end();
                ++itr)
            {
                osgGA::GUIEventAdapter* event = (*itr)->asGUIEventAdapter();
                if (!event) continue;

                bool keyEvent = event->getEventType()==osgGA::GUIEventAdapter::KEYDOWN ||  event->getEventType()==osgGA::GUIEventAdapter::KEYUP;

                if (keyEvent && event->getKey()==_keyStartsTimoutDisplay)
                {
                    OSG_NOTICE<<"_keyStartsTimoutDisplay pressed"<<std::endl;
                    _displayTimeout = true;
                }
                else if (keyEvent && event->getKey()==_keyDismissTimoutDisplay)
                {
                    OSG_NOTICE<<"_keyDismissTimoutDisplay pressed"<<std::endl;
                    needToRecordEventTime = true;
                    needToDismiss = _displayTimeout;
                    _displayTimeout = false;
                }
                else if (keyEvent && event->getKey()==_keyRunTimeoutAction)
                {
                    OSG_NOTICE<<"_keyRunTimeoutAction pressed"<<std::endl;
                    _displayTimeout = false;
                    needToRecordEventTime = true;
                    needToAction = true;
                }
                else if (event->getEventType()!=osgGA::GUIEventAdapter::FRAME)
                {
                    needToRecordEventTime = true;
                    needToDismiss = _displayTimeout;
                    _displayTimeout = false;
                }
            }
        }


        if (needToRecordEventTime)
        {
            _timeOfLastEvent = nv.getFrameStamp()->getReferenceTime();
        }

        double timeSinceLastEvent = nv.getFrameStamp() ? nv.getFrameStamp()->getReferenceTime()-_timeOfLastEvent : 0.0;

        if (timeSinceLastEvent>_idleDurationBeforeTimeoutDisplay)
        {
            _displayTimeout = true;
        }

        if (timeSinceLastEvent>_idleDurationBeforeTimeoutAction)
        {
            _displayTimeout = false;
            needToAction = true;
            needToDismiss = false;
        }

        if (!previous_displayTimeout && _displayTimeout)
        {
            if (viewer && (_displayBroadcastKeyPos._key!=0 || _displayBroadcastKeyPos._x!=FLT_MAX || _displayBroadcastKeyPos._y!=FLT_MAX))
            {
                OSG_NOTICE<<"Doing display broadcast key event"<<_displayBroadcastKeyPos._key<<std::endl;
                broadcastEvent(viewer, _displayBroadcastKeyPos);
            }

            OperationVisitor leave(OperationVisitor::ENTER);
            accept(leave);

            if (leave.sleepTime()!=0.0)
            {
                OSG_NOTICE<<"Pausing for "<<leave.sleepTime()<<std::endl;
                OpenThreads::Thread::microSleep(static_cast<unsigned int>(1000000.0*leave.sleepTime()));
                OSG_NOTICE<<"Finished Pause "<<std::endl;
            }

        }


        if (needToDismiss)
        {
            if (viewer && (_dismissBroadcastKeyPos._key!=0 || _dismissBroadcastKeyPos._x!=FLT_MAX || _dismissBroadcastKeyPos._y!=FLT_MAX))
            {
                OSG_NOTICE<<"Doing dismiss broadcast key event"<<_dismissBroadcastKeyPos._key<<std::endl;
                broadcastEvent(viewer, _dismissBroadcastKeyPos);
            }

            OperationVisitor leave(OperationVisitor::LEAVE);
            accept(leave);
        }

        Transform::traverse(nv);


        if (needToAction)
        {
            OSG_NOTICE<<"Do timeout action"<<std::endl;
            _previousFrameNumber = -1;
            _timeOfLastEvent = nv.getFrameStamp()->getReferenceTime();


            if (_actionJumpData.requiresJump())
            {
                OSG_NOTICE<<"Doing timeout jump"<<std::endl;
                _actionJumpData.jump(SlideEventHandler::instance());
            }

            if (_actionKeyPos._key!=0 || _actionKeyPos._x!=FLT_MAX || _actionKeyPos._y!=FLT_MAX)
            {
                OSG_NOTICE<<"Doing timeout key event"<<_actionKeyPos._key<<std::endl;
                if (SlideEventHandler::instance()) SlideEventHandler::instance()->dispatchEvent(_actionKeyPos);
            }

            if (viewer && (_actionBroadcastKeyPos._key!=0 || _actionBroadcastKeyPos._x!=FLT_MAX || _actionBroadcastKeyPos._y!=FLT_MAX))
            {
                OSG_NOTICE<<"Doing timeout broadcast key event"<<_actionBroadcastKeyPos._key<<std::endl;
                broadcastEvent(viewer, _actionBroadcastKeyPos);
            }

        }

    }
    else if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (_displayTimeout) Transform::traverse(nv);
    }
    else
    {
        if (strcmp(nv.className(),"FindOperatorsVisitor")==0)
        {
            OSG_NOTICE<<"Timeout::traverse() "<<nv.className()<<", ignoring traversal"<<std::endl;
        }
        else
        {
            Transform::traverse(nv);
        }
    }



}
