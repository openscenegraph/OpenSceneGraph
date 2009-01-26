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

#include <stdio.h>

#include <osg/GLExtensions>

#include <osgUtil/Optimizer>
#include <osgUtil/GLObjectsVisitor>
#include <osgUtil/Statistics>

#include <osgViewer/Renderer>
#include <osgViewer/View>

#include <osgDB/DatabasePager>
#include <osgDB/ImagePager>

#include <osg/io_utils>

#include <sstream>

using namespace osgViewer;

//#define DEBUG_MESSAGE osg::notify(osg::NOTICE)
#define DEBUG_MESSAGE osg::notify(osg::DEBUG_FP)


OpenGLQuerySupport::OpenGLQuerySupport():
    _startTick(0),
    _initialized(false),
    _timerQuerySupported(false),
    _extensions(0),
    _previousQueryTime(0.0)
{
}

void OpenGLQuerySupport::checkQuery(osg::Stats* stats)
{
    for(QueryFrameNumberList::iterator itr = _queryFrameNumberList.begin();
        itr != _queryFrameNumberList.end();
        )
    {
        GLuint query = itr->first;
        GLint available = 0;
        _extensions->glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
        if (available)
        {
            GLuint64EXT timeElapsed = 0;
            _extensions->glGetQueryObjectui64v(query, GL_QUERY_RESULT, &timeElapsed);

            double timeElapsedSeconds = double(timeElapsed)*1e-9;
            double currentTime = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());
            double estimatedEndTime = (_previousQueryTime + currentTime) * 0.5;
            double estimatedBeginTime = estimatedEndTime - timeElapsedSeconds;

            stats->setAttribute(itr->second, "GPU draw begin time", estimatedBeginTime);
            stats->setAttribute(itr->second, "GPU draw end time", estimatedEndTime);
            stats->setAttribute(itr->second, "GPU draw time taken", timeElapsedSeconds);


            itr = _queryFrameNumberList.erase(itr);
            _availableQueryObjects.push_back(query);
        }
        else
        {
            ++itr;
        }

    }
    _previousQueryTime = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());
}

GLuint OpenGLQuerySupport::createQueryObject()
{
    if (_availableQueryObjects.empty())
    {
        GLuint query;
        _extensions->glGenQueries(1, &query);
        return query;
    }
    else
    {
        GLuint query = _availableQueryObjects.back();
        _availableQueryObjects.pop_back();
        return query;
    }
}

void OpenGLQuerySupport::beginQuery(int frameNumber)
{
    GLuint query = createQueryObject();
    _extensions->glBeginQuery(GL_TIME_ELAPSED, query);
    _queryFrameNumberList.push_back(QueryFrameNumberPair(query, frameNumber));        
}

void OpenGLQuerySupport::endQuery()
{
    _extensions->glEndQuery(GL_TIME_ELAPSED);
}

void OpenGLQuerySupport::initialize(osg::State* state)
{
    if (_initialized) return;

    _initialized = true;
    _extensions = osg::Drawable::getExtensions(state->getContextID(),true);
    _timerQuerySupported = _extensions && _extensions->isTimerQuerySupported();
    _previousQueryTime = osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//  ThreadSafeQueue

Renderer::ThreadSafeQueue::ThreadSafeQueue()
{
    _block.set(false);
}

Renderer::ThreadSafeQueue::~ThreadSafeQueue()
{
}

osgUtil::SceneView* Renderer::ThreadSafeQueue::takeFront()
{
    if (_queue.empty()) _block.block();

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    if (_queue.empty()) return 0;

    osgUtil::SceneView* front = _queue.front();
    _queue.pop_front();

    if (_queue.empty()) _block.set(false);
    
    return front;
}

void Renderer::ThreadSafeQueue::add(osgUtil::SceneView* sv)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _queue.push_back(sv);
    _block.set(true);
}

static OpenThreads::Mutex s_drawSerializerMutex;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//  Renderer
Renderer::Renderer(osg::Camera* camera):
    osg::GraphicsOperation("Renderer",true),
    OpenGLQuerySupport(),
    _targetFrameRate(100.0),
    _minimumTimeAvailableForGLCompileAndDeletePerFrame(0.001),
    _flushTimeRatio(0.5),
    _conservativeTimeRatio(0.5),
    _camera(camera),
    _done(false),
    _graphicsThreadDoesCull(true),
    _compileOnNextDraw(true)
{

    DEBUG_MESSAGE<<"Render::Render() "<<this<<std::endl;

    _sceneView[0] = new osgUtil::SceneView;
    _sceneView[1] = new osgUtil::SceneView;


    osg::Camera* masterCamera = _camera->getView() ? _camera->getView()->getCamera() : camera;
    osg::StateSet* stateset = masterCamera->getOrCreateStateSet();
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(_camera->getView());

    osg::DisplaySettings* ds = _camera->getDisplaySettings() ?  _camera->getDisplaySettings() :
                               ((view && view->getDisplaySettings()) ?  view->getDisplaySettings() :  osg::DisplaySettings::instance());

    unsigned int sceneViewOptions = osgUtil::SceneView::HEADLIGHT;
    if (view)
    {
        switch(view->getLightingMode())
        {
            case(osg::View::NO_LIGHT): sceneViewOptions = 0; break;
            case(osg::View::SKY_LIGHT): sceneViewOptions = osgUtil::SceneView::SKY_LIGHT; break;
            case(osg::View::HEADLIGHT): sceneViewOptions = osgUtil::SceneView::HEADLIGHT; break;
        }
    }

    _sceneView[0]->setGlobalStateSet(stateset);
    _sceneView[1]->setGlobalStateSet(stateset);
    
    _sceneView[0]->setDefaults(sceneViewOptions);
    _sceneView[1]->setDefaults(sceneViewOptions);

    _sceneView[0]->setDisplaySettings(ds);
    _sceneView[1]->setDisplaySettings(ds);

    _sceneView[0]->setCamera(_camera.get(), false);
    _sceneView[1]->setCamera(_camera.get(), false);

    // lock the mutex for the current cull SceneView to
    // prevent the draw traversal from reading from it before the cull traversal has been completed.
    _availableQueue.add(_sceneView[0].get());
    _availableQueue.add(_sceneView[1].get());
        
    DEBUG_MESSAGE<<"_availableQueue.size()="<<_availableQueue._queue.size()<<std::endl;

    _flushOperation = new osg::FlushDeletedGLObjectsOperation(0.1);
}

Renderer::~Renderer()
{
    DEBUG_MESSAGE<<"Render::~Render() "<<this<<std::endl;
}

void Renderer::setGraphicsThreadDoesCull(bool flag)
{
    if (_graphicsThreadDoesCull==flag) return;

    _graphicsThreadDoesCull = flag;
}

void Renderer::updateSceneView(osgUtil::SceneView* sceneView)
{
    osg::Camera* masterCamera = _camera->getView() ? _camera->getView()->getCamera() : _camera.get();
    osg::StateSet* stateset = masterCamera->getOrCreateStateSet();

    if (sceneView->getGlobalStateSet()!=stateset)
    {
        sceneView->setGlobalStateSet(stateset);
    }
    
    osg::GraphicsContext* context = _camera->getGraphicsContext();
    osg::State* state = context ? context->getState() : 0;
    if (sceneView->getState()!=state) 
    {
        sceneView->setState(state);
    }

    osgViewer::View* view = dynamic_cast<osgViewer::View*>(_camera->getView());

    osgDB::DatabasePager* databasePager = view ? view->getDatabasePager() : 0;
    sceneView->getCullVisitor()->setDatabaseRequestHandler(databasePager);

    osgDB::ImagePager* imagePager = view ? view->getImagePager() : 0;
    sceneView->getCullVisitor()->setImageRequestHandler(imagePager);
    
    sceneView->setFrameStamp(view ? view->getFrameStamp() : state->getFrameStamp());
    
    if (databasePager) databasePager->setCompileGLObjectsForContextID(state->getContextID(), true);
    
    osg::DisplaySettings* ds = _camera->getDisplaySettings() ?  _camera->getDisplaySettings() :
                               ((view &&view->getDisplaySettings()) ?  view->getDisplaySettings() :  osg::DisplaySettings::instance());

    sceneView->setDisplaySettings(ds);

    if (view) _startTick = view->getStartTick();
}

void Renderer::compile()
{
    DEBUG_MESSAGE<<"Renderer::compile()"<<std::endl;

    _compileOnNextDraw = false;
    
    osgUtil::SceneView* sceneView = _sceneView[0].get();
    if (!sceneView || _done) return;

    if (sceneView->getSceneData()) 
    {
        osgUtil::GLObjectsVisitor glov;
        glov.setState(sceneView->getState());
        sceneView->getSceneData()->accept(glov);
    }
}

void Renderer::cull()
{
    DEBUG_MESSAGE<<"cull()"<<std::endl;

    if (_done || _graphicsThreadDoesCull) return;

    // note we assume lock has already been acquired.
    osgUtil::SceneView* sceneView = _availableQueue.takeFront();

    DEBUG_MESSAGE<<"cull() got SceneView "<<sceneView<<std::endl;

    if (sceneView)
    {
        updateSceneView(sceneView);

        // osg::notify(osg::NOTICE)<<"Culling buffer "<<_currentCull<<std::endl;

        // pass on the fusion distance settings from the View to the SceneView
        osgViewer::View* view = dynamic_cast<osgViewer::View*>(sceneView->getCamera()->getView());
        if (view) sceneView->setFusionDistance(view->getFusionDistanceMode(), view->getFusionDistanceValue());

        osg::Stats* stats = sceneView->getCamera()->getStats();
        osg::State* state = sceneView->getState();
        const osg::FrameStamp* fs = state->getFrameStamp();
        int frameNumber = fs ? fs->getFrameNumber() : 0;

        // do cull traversal
        osg::Timer_t beforeCullTick = osg::Timer::instance()->tick();

        sceneView->inheritCullSettings(*(sceneView->getCamera()));
        sceneView->cull();

        osg::Timer_t afterCullTick = osg::Timer::instance()->tick();

#if 0
        if (sceneView->getDynamicObjectCount()==0 && state->getDynamicObjectRenderingCompletedCallback())
        {
            // osg::notify(osg::NOTICE)<<"Completed in cull"<<std::endl;
            state->getDynamicObjectRenderingCompletedCallback()->completed(state);
        }
#endif
        if (stats && stats->collectStats("rendering"))
        {
            DEBUG_MESSAGE<<"Collecting rendering stats"<<std::endl;
        
            stats->setAttribute(frameNumber, "Cull traversal begin time", osg::Timer::instance()->delta_s(_startTick, beforeCullTick));
            stats->setAttribute(frameNumber, "Cull traversal end time", osg::Timer::instance()->delta_s(_startTick, afterCullTick));
            stats->setAttribute(frameNumber, "Cull traversal time taken", osg::Timer::instance()->delta_s(beforeCullTick, afterCullTick));
        }

        if (stats && stats->collectStats("scene"))
        {
            osgUtil::Statistics sceneStats;
            sceneView->getStats(sceneStats);
            
            stats->setAttribute(frameNumber, "Visible vertex count", static_cast<double>(sceneStats._vertexCount));
            stats->setAttribute(frameNumber, "Visible number of drawables", static_cast<double>(sceneStats.numDrawables));
            stats->setAttribute(frameNumber, "Visible number of lights", static_cast<double>(sceneStats.nlights));
            stats->setAttribute(frameNumber, "Visible number of render bins", static_cast<double>(sceneStats.nbins));
            stats->setAttribute(frameNumber, "Visible depth", static_cast<double>(sceneStats.depth));
            stats->setAttribute(frameNumber, "Visible number of materials", static_cast<double>(sceneStats.nummat));
            stats->setAttribute(frameNumber, "Visible number of impostors", static_cast<double>(sceneStats.nimpostor));

            osgUtil::Statistics::PrimitiveCountMap& pcm = sceneStats.getPrimitiveCountMap();
            stats->setAttribute(frameNumber, "Visible number of GL_POINTS", static_cast<double>(pcm[GL_POINTS]));
            stats->setAttribute(frameNumber, "Visible number of GL_LINES", static_cast<double>(pcm[GL_LINES]));
            stats->setAttribute(frameNumber, "Visible number of GL_LINE_STRIP", static_cast<double>(pcm[GL_LINE_STRIP]));
            stats->setAttribute(frameNumber, "Visible number of GL_LINE_LOOP", static_cast<double>(pcm[GL_LINE_LOOP]));
            stats->setAttribute(frameNumber, "Visible number of GL_TRIANGLES", static_cast<double>(pcm[GL_TRIANGLES]));
            stats->setAttribute(frameNumber, "Visible number of GL_TRIANGLE_STRIP", static_cast<double>(pcm[GL_TRIANGLE_STRIP]));
            stats->setAttribute(frameNumber, "Visible number of GL_TRIANGLE_FAN", static_cast<double>(pcm[GL_TRIANGLE_FAN]));
            stats->setAttribute(frameNumber, "Visible number of GL_QUADS", static_cast<double>(pcm[GL_QUADS]));
            stats->setAttribute(frameNumber, "Visible number of GL_QUAD_STRIP", static_cast<double>(pcm[GL_QUAD_STRIP]));
            stats->setAttribute(frameNumber, "Visible number of GL_POLYGON", static_cast<double>(pcm[GL_POLYGON]));
            
        }

        _drawQueue.add(sceneView);

    }

    DEBUG_MESSAGE<<"end cull() "<<this<<std::endl;
}

void Renderer::draw()
{
    DEBUG_MESSAGE<<"draw() "<<this<<std::endl;

    // osg::Timer_t startDrawTick = osg::Timer::instance()->tick();

    osgUtil::SceneView* sceneView = _drawQueue.takeFront();

    DEBUG_MESSAGE<<"draw() got SceneView "<<sceneView<<std::endl;

    osg::GraphicsContext* compileContext = sceneView ? osg::GraphicsContext::getCompileContext(sceneView->getState()->getContextID()) : 0;
    osg::GraphicsThread* compileThread = compileContext ? compileContext->getGraphicsThread() : 0;

    if (sceneView && !_done)
    {
        if (_compileOnNextDraw)
        {
            compile();
        }
    
        osgViewer::View* view = dynamic_cast<osgViewer::View*>(_camera->getView());
        osgDB::DatabasePager* databasePager = view ? view->getDatabasePager() : 0;

        // osg::notify(osg::NOTICE)<<"Drawing buffer "<<_currentDraw<<std::endl;

        if (_done)
        {
            osg::notify(osg::INFO)<<"Renderer::release() causing draw to exit"<<std::endl;
            return;
        }

        if (_graphicsThreadDoesCull)
        {
            osg::notify(osg::INFO)<<"Renderer::draw() completing early due to change in _graphicsThreadDoesCull flag."<<std::endl;
            return;
        }

        // osg::notify(osg::NOTICE)<<"RenderingOperation"<<std::endl;

        osg::Stats* stats = sceneView->getCamera()->getStats();
        osg::State* state = sceneView->getState();
        int frameNumber = state->getFrameStamp()->getFrameNumber();

        if (!_initialized)
        {
            initialize(state);
        }

        state->setDynamicObjectCount(sceneView->getDynamicObjectCount());

        if (sceneView->getDynamicObjectCount()==0 && state->getDynamicObjectRenderingCompletedCallback())
        {
            // osg::notify(osg::NOTICE)<<"Completed in cull"<<std::endl;
            state->getDynamicObjectRenderingCompletedCallback()->completed(state);
        }

        bool acquireGPUStats = stats && _timerQuerySupported && stats->collectStats("gpu");

        if (acquireGPUStats) 
        {
            checkQuery(stats);
        }

        // do draw traversal
        if (acquireGPUStats) 
        {
            checkQuery(stats);
            beginQuery(frameNumber);
        }

        osg::Timer_t beforeDrawTick;
        
        
        bool serializeDraw = sceneView->getDisplaySettings()->getSerializeDrawDispatch();

        if (serializeDraw) 
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_drawSerializerMutex);
            beforeDrawTick = osg::Timer::instance()->tick();
            sceneView->draw();
        }
        else
        {
            beforeDrawTick = osg::Timer::instance()->tick();
            sceneView->draw();
        }

        _availableQueue.add(sceneView);

        osg::Timer_t afterDispatchTick = osg::Timer::instance()->tick();

        double dispatchTime = osg::Timer::instance()->delta_s(beforeDrawTick, afterDispatchTick);

        // now flush delete OpenGL objects and compile any objects as required by the DatabasePager
        flushAndCompile(dispatchTime, sceneView, databasePager, compileThread);
    
        if (acquireGPUStats)
        {
            endQuery();
            checkQuery(stats);
        }

        //glFlush();
        
        osg::Timer_t afterDrawTick = osg::Timer::instance()->tick();

//        osg::notify(osg::NOTICE)<<"Time wait for draw = "<<osg::Timer::instance()->delta_m(startDrawTick, beforeDrawTick)<<std::endl;
//        osg::notify(osg::NOTICE)<<"     time for draw = "<<osg::Timer::instance()->delta_m(beforeDrawTick, afterDrawTick)<<std::endl;

        if (stats && stats->collectStats("rendering"))
        {
            stats->setAttribute(frameNumber, "Draw traversal begin time", osg::Timer::instance()->delta_s(_startTick, beforeDrawTick));
            stats->setAttribute(frameNumber, "Draw traversal end time", osg::Timer::instance()->delta_s(_startTick, afterDrawTick));
            stats->setAttribute(frameNumber, "Draw traversal time taken", osg::Timer::instance()->delta_s(beforeDrawTick, afterDrawTick));
        }

    }

    DEBUG_MESSAGE<<"end draw() "<<this<<std::endl;
}

void Renderer::cull_draw()
{
    DEBUG_MESSAGE<<"cull_draw() "<<this<<std::endl;

    osgUtil::SceneView* sceneView = _sceneView[0].get();
    if (!sceneView || _done) return;

    if (_done)
    {
        osg::notify(osg::INFO)<<"Render::release() causing cull_draw to exit"<<std::endl;
        return;
    }
    
    updateSceneView(sceneView);

    if (_compileOnNextDraw)
    {
        compile();
    }

    osgViewer::View* view = dynamic_cast<osgViewer::View*>(_camera->getView());
    osgDB::DatabasePager* databasePager = view ? view->getDatabasePager() : 0;

    osg::GraphicsContext* compileContext = osg::GraphicsContext::getCompileContext(sceneView->getState()->getContextID());
    osg::GraphicsThread* compileThread = compileContext ? compileContext->getGraphicsThread() : 0;


    // osg::notify(osg::NOTICE)<<"RenderingOperation"<<std::endl;

    // pass on the fusion distance settings from the View to the SceneView
    if (view) sceneView->setFusionDistance(view->getFusionDistanceMode(), view->getFusionDistanceValue());

    osg::Stats* stats = sceneView->getCamera()->getStats();
    osg::State* state = sceneView->getState();
    const osg::FrameStamp* fs = state->getFrameStamp();
    int frameNumber = fs ? fs->getFrameNumber() : 0;

    if (!_initialized)
    {
        initialize(state);
    }

    bool acquireGPUStats = stats && _timerQuerySupported && stats->collectStats("gpu");

    if (acquireGPUStats) 
    {
        checkQuery(stats);
    }

    // do cull traversal
    osg::Timer_t beforeCullTick = osg::Timer::instance()->tick();

    sceneView->inheritCullSettings(*(sceneView->getCamera()));
    sceneView->cull();

    osg::Timer_t afterCullTick = osg::Timer::instance()->tick();

    if (stats && stats->collectStats("scene"))
    {
        osgUtil::Statistics sceneStats;
        sceneView->getStats(sceneStats);

        stats->setAttribute(frameNumber, "Visible vertex count", static_cast<double>(sceneStats._vertexCount));
        stats->setAttribute(frameNumber, "Visible number of drawables", static_cast<double>(sceneStats.numDrawables));
        stats->setAttribute(frameNumber, "Visible number of lights", static_cast<double>(sceneStats.nlights));
        stats->setAttribute(frameNumber, "Visible number of render bins", static_cast<double>(sceneStats.nbins));
        stats->setAttribute(frameNumber, "Visible depth", static_cast<double>(sceneStats.depth));
        stats->setAttribute(frameNumber, "Visible number of materials", static_cast<double>(sceneStats.nummat));
        stats->setAttribute(frameNumber, "Visible number of impostors", static_cast<double>(sceneStats.nimpostor));
    }

#if 0
    if (state->getDynamicObjectCount()==0 && state->getDynamicObjectRenderingCompletedCallback())
    {
        state->getDynamicObjectRenderingCompletedCallback()->completed(state);
    }
#endif


    // do draw traversal
    if (acquireGPUStats) 
    {
        checkQuery(stats);
        beginQuery(frameNumber);
    }

    osg::Timer_t beforeDrawTick;

    bool serializeDraw = sceneView->getDisplaySettings()->getSerializeDrawDispatch();

    if (serializeDraw) 
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_drawSerializerMutex);
        
        beforeDrawTick = osg::Timer::instance()->tick();
        sceneView->draw();
    }
    else
    {
        beforeDrawTick = osg::Timer::instance()->tick();
        sceneView->draw();
    }

    osg::Timer_t afterDispatchTick = osg::Timer::instance()->tick();
    double cullAndDispatchTime = osg::Timer::instance()->delta_s(beforeCullTick, afterDispatchTick);

    // now flush delete OpenGL objects and compile any objects as required by the DatabasePager
    flushAndCompile(cullAndDispatchTime, sceneView, databasePager, compileThread);


    if (acquireGPUStats)
    {
        endQuery();
        checkQuery(stats);
    }

    osg::Timer_t afterDrawTick = osg::Timer::instance()->tick();

    if (stats && stats->collectStats("rendering"))
    {
        DEBUG_MESSAGE<<"Collecting rendering stats"<<std::endl;

        stats->setAttribute(frameNumber, "Cull traversal begin time", osg::Timer::instance()->delta_s(_startTick, beforeCullTick));
        stats->setAttribute(frameNumber, "Cull traversal end time", osg::Timer::instance()->delta_s(_startTick, afterCullTick));
        stats->setAttribute(frameNumber, "Cull traversal time taken", osg::Timer::instance()->delta_s(beforeCullTick, afterCullTick));

        stats->setAttribute(frameNumber, "Draw traversal begin time", osg::Timer::instance()->delta_s(_startTick, beforeDrawTick));
        stats->setAttribute(frameNumber, "Draw traversal end time", osg::Timer::instance()->delta_s(_startTick, afterDrawTick));
        stats->setAttribute(frameNumber, "Draw traversal time taken", osg::Timer::instance()->delta_s(beforeDrawTick, afterDrawTick));
    }

    DEBUG_MESSAGE<<"end cull_draw() "<<this<<std::endl;

}

void Renderer::flushAndCompile(double currentElapsedFrameTime, osgUtil::SceneView* sceneView, osgDB::DatabasePager* databasePager, osg::GraphicsThread* compileThread)
{
    
    double targetFrameRate = _targetFrameRate;
    double minimumTimeAvailableForGLCompileAndDeletePerFrame = _minimumTimeAvailableForGLCompileAndDeletePerFrame;

    if (databasePager)
    {
        targetFrameRate = std::min(targetFrameRate, databasePager->getTargetFrameRate());
        minimumTimeAvailableForGLCompileAndDeletePerFrame = std::min(minimumTimeAvailableForGLCompileAndDeletePerFrame, databasePager->getMinimumTimeAvailableForGLCompileAndDeletePerFrame());
    }
    
    double targetFrameTime = 1.0/targetFrameRate;

    double availableTime = std::max((targetFrameTime - currentElapsedFrameTime)*_conservativeTimeRatio,
                                    minimumTimeAvailableForGLCompileAndDeletePerFrame);

    double flushTime = availableTime * _flushTimeRatio;
    double compileTime = availableTime - flushTime;

#if 0
    osg::notify(osg::NOTICE)<<"total availableTime = "<<availableTime*1000.0<<std::endl;
    osg::notify(osg::NOTICE)<<"      flushTime     = "<<flushTime*1000.0<<std::endl;
    osg::notify(osg::NOTICE)<<"      compileTime   = "<<compileTime*1000.0<<std::endl;
#endif

    if (compileThread)
    {
        compileThread->add(_flushOperation.get());
    }
    else
    {
        sceneView->flushDeletedGLObjects(flushTime);
    }

    // if any time left over from flush add this to compile time.        
    if (flushTime>0.0) compileTime += flushTime;

#if 0
    osg::notify(osg::NOTICE)<<"      revised compileTime   = "<<compileTime*1000.0<<std::endl;
#endif

    if (databasePager && databasePager->requiresExternalCompileGLObjects(sceneView->getState()->getContextID()))
    {
        databasePager->compileGLObjects(*(sceneView->getState()), compileTime);
    }
}

void Renderer::operator () (osg::Object* object)
{
    osg::GraphicsContext* context = dynamic_cast<osg::GraphicsContext*>(object);
    if (context) operator()(context);

    osg::Camera* camera = dynamic_cast<osg::Camera*>(object);
    if (camera) cull();
}

void Renderer::operator () (osg::GraphicsContext* context)
{
    if (_graphicsThreadDoesCull)
    {
        cull_draw();
    }
    else
    {
        draw();
    }
}

void Renderer::release()
{
    osg::notify(osg::INFO)<<"Renderer::release()"<<std::endl;
    _done = true;

    _availableQueue.release();
    _drawQueue.release();
}
