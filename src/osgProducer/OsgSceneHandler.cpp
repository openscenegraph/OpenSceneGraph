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

#include <OpenThreads/Mutex>

#include <osgDB/Registry>

#include <osgProducer/OsgSceneHandler>

using namespace osgUtil;
using namespace osgProducer;

OsgSceneHandler::OsgSceneHandler( osg::DisplaySettings *ds) :
    _sceneView(new osgUtil::SceneView(ds)),
    _collectStats(false)
{
    _frameStartTick = 0;
    _previousFrameStartTick = 0;
    _flushOfAllDeletedGLObjectsOnNextFrame = false;
    _cleanUpOnNextFrame = false;
}

void OsgSceneHandler::init()
{
    static OpenThreads::Mutex mutex;
    osg::notify(osg::INFO)<<"entering "<<this<<" init."<<std::endl;
    mutex.lock();
    osg::notify(osg::INFO)<<"   running "<<this<<" init."<<std::endl;

    _sceneView->SceneView::init();

    osg::notify(osg::INFO)<<"   done "<<this<<" init."<<std::endl;
    mutex.unlock();
    osg::notify(osg::INFO)<<"   unlocked "<<this<<" init."<<std::endl;
}

void OsgSceneHandler::clearImplementation(Producer::Camera& /*camera*/)
{
    _previousFrameStartTick = _frameStartTick;
    _frameStartTick = osg::Timer::instance()->tick();

    osgDB::DatabasePager* dp = osgDB::Registry::instance()->getDatabasePager();
    if (dp)
    {
        dp->signalBeginFrame(getSceneView()->getState()->getFrameStamp());
    }

    // no-op right now as scene view manages its own clear.
}

void OsgSceneHandler::cullImplementation(Producer::Camera &cam) 
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_cullMutex);

    _sceneView->getProjectionMatrix().set(cam.getProjectionMatrix());
    _sceneView->getViewMatrix().set(cam.getPositionAndAttitudeMatrix());

    int x, y;
    unsigned int w, h;
    cam.getProjectionRectangle( x, y, w, h );

    _sceneView->setViewport( x, y, w, h );

    osg::Vec4 clear_color;
    cam.getClearColor(clear_color[0],clear_color[1],clear_color[2],clear_color[3]);
    _sceneView->setClearColor(clear_color);

    _sceneView->cull();

    if (_collectStats)
    {
        _stats.reset();
        _sceneView->getStats(_stats);
    }
}

bool OsgSceneHandler::getStats(Statistics& primStats)
{
    if (!_collectStats) return false;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_cullMutex);

    primStats.add(_stats);

    return true;
}

void OsgSceneHandler::drawImplementation(Producer::Camera &) 
{
    // dipatch the draw traversal of the scene graph
    _sceneView->SceneView::draw();
    

    // for the database pager now manage any GL object operations that are required.
    osgDB::DatabasePager* dp = osgDB::Registry::instance()->getDatabasePager();
    if (dp)
    {
    
#if 1
        double timeForCullAndDraw = osg::Timer::instance()->delta_s(_frameStartTick, osg::Timer::instance()->tick());

        double targeFrameTime = 1.0/dp->getTargetFrameRate();
        
        double drawCostFactor = 2.0; // must be greater than 1 to account for the extra cost of emptying the OpenGL fifo.
        double frameFactor = 0.9; // must be less than 1, to compensate for extra time spent in update and swap buffers etc.
        double timeLeftTillEndOfFrame = targeFrameTime*frameFactor - timeForCullAndDraw*drawCostFactor;
        double availableTime = timeLeftTillEndOfFrame / drawCostFactor; // account for the fifo when download texture objects.
        
        // clamp the available time by the prescribed minimum
        if (availableTime<dp->getMinimumTimeAvailableForGLCompileAndDeletePerFrame())
        { 
            availableTime = dp->getMinimumTimeAvailableForGLCompileAndDeletePerFrame();
        }
        
        static unsigned int _numFramesThatNoTimeAvailable = 0;
        static unsigned int _maxNumFramesThatNoTimeAvailable = 10;

        if (_numFramesThatNoTimeAvailable>_maxNumFramesThatNoTimeAvailable)
        {
            availableTime = 0.0025; // 2.5ms.
        }

        if (availableTime>0.0)
        {
            _numFramesThatNoTimeAvailable = 0;
        
            // osg::notify(osg::NOTICE)<<"Time available = "<<availableTime<<std::endl;
        
            dp->compileGLObjects(*(getSceneView()->getState()),availableTime);

            // flush deleted GL objects.
            getSceneView()->flushDeletedGLObjects(availableTime);
        }
        else
        {
            ++_numFramesThatNoTimeAvailable;
        }
#else    
        double timeForPreviousFrame = osg::Timer::instance()->delta_s(_previousFrameStartTick, _frameStartTick);
        double timeForCullAndDraw = osg::Timer::instance()->delta_s(_frameStartTick, osg::Timer::instance()->tick());

        double minimumTargetMaxFrameTime = 0.010; // 10ms.
        double targetMaxFrameTime = osg::minimum(timeForPreviousFrame, minimumTargetMaxFrameTime);
        
        double drawCostFactor = 2.0; // must be greater than 1 to account for the extra cost of emptying the OpenGL fifo.
        double frameFactor = 0.9; // must be less than 1, to compensate for extra time spent in update and swap buffers etc.
        double timeLeftTillEndOfFrame = targetMaxFrameTime*frameFactor - timeForCullAndDraw*drawCostFactor;
        double availableTime = timeLeftTillEndOfFrame / drawCostFactor; // account for the fifo when download texture objects.

        static unsigned int _numFramesThatNoTimeAvailable = 0;
        static unsigned int _maxNumFramesThatNoTimeAvailable = 10;

        if (_numFramesThatNoTimeAvailable>_maxNumFramesThatNoTimeAvailable)
        {
            availableTime = 0.0025; // 2.5ms.
        }

        if (availableTime>0.0)
        {
            _numFramesThatNoTimeAvailable = 0;
        
            // osg::notify(osg::NOTICE)<<"Time available = "<<availableTime<<std::endl;
        
            dp->compileGLObjects(*(getSceneView()->getState()),availableTime);

            // flush deleted GL objects.
            getSceneView()->flushDeletedGLObjects(availableTime);
        }
        else
        {
            ++_numFramesThatNoTimeAvailable;
        }
#endif
        dp->signalEndFrame();
    }    
}

void OsgSceneHandler::setContextID( int id )
{
    _sceneView->getState()->setContextID( id );
}
