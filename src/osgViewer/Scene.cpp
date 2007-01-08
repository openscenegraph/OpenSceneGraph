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

#include <osgViewer/Scene>

using namespace osgViewer;

Scene::Scene():
    _firstFrame(true)
{
    _frameStamp = new osg::FrameStamp;
    _frameStamp->setFrameNumber(0);
    _frameStamp->setReferenceTime(0);

    _updateVisitor = new osgUtil::UpdateVisitor;
    _updateVisitor->setFrameStamp(_frameStamp.get());
    
    setDatabasePager(new osgDB::DatabasePager);
}

Scene::~Scene()
{
}

void Scene::init()
{
    osg::notify(osg::NOTICE)<<"Scene::init() not implementated yet."<<std::endl;
}


void Scene::setSceneData(osg::Node* node)
{
    _sceneData = node;
    
    if (_databasePager.valid())
    {    
        // register any PagedLOD that need to be tracked in the scene graph
        _databasePager->registerPagedLODs(node);
    }
}

osg::Node* Scene::getSceneData()
{
    return _sceneData.get();
}

const osg::Node* Scene::getSceneData() const
{
    return _sceneData.get();
}

void Scene::setDatabasePager(osgDB::DatabasePager* dp)
{
    _databasePager = dp;
}

void Scene::addEventHandler(osgGA::GUIEventHandler*)
{
    osg::notify(osg::NOTICE)<<"Scene::addEventHandler() not implementated yet."<<std::endl;
}

void Scene::frameAdvance()
{
    // double previousTime = _frameStamp->getReferenceTime();
    
    _frameStamp->setReferenceTime(osg::Timer::instance()->time_s());
    _frameStamp->setFrameNumber(_frameStamp->getFrameNumber()+1);
    
    // osg::notify(osg::NOTICE)<<"Frame rate = "<<1.0/(_frameStamp->getReferenceTime()-previousTime)<<std::endl;
}

void Scene::frameEventTraversal()
{
    _eventQueue->frame( _frameStamp->getReferenceTime() );

    osgGA::EventQueue::Events events;
    _eventQueue->takeEvents(events);
    
    if (_eventVisitor.valid())
    {
        _eventVisitor->setTraversalNumber(_frameStamp->getFrameNumber());
    }
    
    for(osgGA::EventQueue::Events::iterator itr = events.begin();
        itr != events.end();
        ++itr)
    {
        osgGA::GUIEventAdapter* event = itr->get();
    
        bool handled = false;
        
        if (_eventVisitor.valid() && getSceneData())
        {
            _eventVisitor->reset();
            _eventVisitor->addEvent( event );
            
            getSceneData()->accept(*_eventVisitor);
            
            if (_eventVisitor->getEventHandled())  handled = true;
        }
        
        for(EventHandlers::iterator hitr = _eventHandlers.begin();
            hitr != _eventHandlers.end() && !handled;
            ++hitr)
        {
//            handled = (*hitr)->handle( *event, *this, 0, 0);
        }
    }
}

void Scene::frameUpdateTraversal()
{
    if (!getSceneData()) return;
    
    getSceneData()->accept(*_updateVisitor);
    
    if (_databasePager.valid())
    {    
        // tell the DatabasePager the frame number of that the scene graph is being actively used to render a frame
        _databasePager->signalBeginFrame(_frameStamp.get());

        // syncronize changes required by the DatabasePager thread to the scene graph
        _databasePager->updateSceneGraph(_frameStamp->getReferenceTime());
    }
}


 

