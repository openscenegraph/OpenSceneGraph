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

#include <osgPresentation/Timeout>
#include <osgUtil/CullVisitor>
#include <osgGA/EventVisitor>

using namespace osgPresentation;


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
    _idleDurationBeforeTimeoutDisplay(5.0),
    _idleDurationBeforeTimeoutAction(6.0)
{
    _hudSettings = hudSettings;
    setCullingActive(false);
    setNumChildrenRequiringEventTraversal(1);
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
Timeout::Timeout(const Timeout& timeout,const osg::CopyOp& copyop):
    osg::Transform(timeout, copyop),
    _hudSettings(timeout._hudSettings)
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

void Timeout::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        double timeSinceLastEvent = nv.getFrameStamp() ? nv.getFrameStamp()->getReferenceTime()-_timeOfLastEvent : 0.0;
        bool needToDisplay = (timeSinceLastEvent>_idleDurationBeforeTimeoutDisplay);

        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (needToDisplay && cv)
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
            // dependancy list.
            cv->getCurrentRenderBin()->getStage()->addPostRenderStage(rs.get(),0);
        }
    }
    else if (nv.getVisitorType()==osg::NodeVisitor::EVENT_VISITOR)
    {
        int deltaFrameNumber = (nv.getFrameStamp()->getFrameNumber()-_previousFrameNumber);
        _previousFrameNumber = nv.getFrameStamp()->getFrameNumber();

        bool needToRecordEventTime = false;
        
        if (deltaFrameNumber>1)
        {
            needToRecordEventTime = true;
        }
        else
        {        
            osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(&nv);
            if (ev)
            {
                osgGA::EventQueue::Events& events = ev->getEvents();
                for(osgGA::EventQueue::Events::iterator itr = events.begin();
                    itr != events.end();
                    ++itr)
                {
                    osgGA::GUIEventAdapter* event = itr->get();
                    if (event->getEventType()!=osgGA::GUIEventAdapter::FRAME)
                    {
                        needToRecordEventTime = true;
                    }
                }
            }
        }
        
        if (needToRecordEventTime)
        {
            _timeOfLastEvent = nv.getFrameStamp()->getReferenceTime();
        }
        
        Transform::traverse(nv);

        double timeSinceLastEvent = nv.getFrameStamp() ? nv.getFrameStamp()->getReferenceTime()-_timeOfLastEvent : 0.0;
        bool needToAction = (timeSinceLastEvent>_idleDurationBeforeTimeoutAction);

        if (needToAction)
        {
            OSG_NOTICE<<"Do action"<<std::endl;
            _previousFrameNumber = -1;
            _timeOfLastEvent = nv.getFrameStamp()->getReferenceTime();

            if (_jumpData.requiresJump())
            {
                OSG_NOTICE<<"Doing jump"<<std::endl;
                _jumpData.jump(SlideEventHandler::instance());
            }
        }
        
    }
    else
    {
        Transform::traverse(nv);
    }

}
