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
#include <OpenThreads/ReentrantMutex>

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

//#define DEBUG_MESSAGE OSG_NOTICE
#define DEBUG_MESSAGE OSG_DEBUG

OpenGLQuerySupport::OpenGLQuerySupport():
    _extensions(0)
{
}

class OSGVIEWER_EXPORT EXTQuerySupport : public OpenGLQuerySupport
{
 public:
    EXTQuerySupport();
    void checkQuery(osg::Stats* stats, osg::State* state, osg::Timer_t startTick);
    virtual void beginQuery(unsigned int frameNumber, osg::State* state);
    virtual void endQuery(osg::State* state);
    virtual void initialize(osg::State* state, osg::Timer_t startTick);
 protected:
    GLuint createQueryObject();
    typedef std::pair<GLuint, unsigned int> QueryFrameNumberPair;
    typedef std::list<QueryFrameNumberPair> QueryFrameNumberList;
    typedef std::vector<GLuint> QueryList;

    QueryFrameNumberList                        _queryFrameNumberList;
    QueryList                                   _availableQueryObjects;
    double                                      _previousQueryTime;
};


EXTQuerySupport::EXTQuerySupport():
    _previousQueryTime(0.0)
{
}

void EXTQuerySupport::checkQuery(osg::Stats* stats, osg::State* /*state*/,
                                 osg::Timer_t startTick)
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
            GLuint64 timeElapsed = 0;
            _extensions->glGetQueryObjectui64v(query, GL_QUERY_RESULT, &timeElapsed);

            double timeElapsedSeconds = double(timeElapsed)*1e-9;
            double currentTime = osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick());
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
    _previousQueryTime = osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick());
}

GLuint EXTQuerySupport::createQueryObject()
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

void EXTQuerySupport::beginQuery(unsigned int frameNumber, osg::State* /*state*/)
{
    GLuint query = createQueryObject();
    _extensions->glBeginQuery(GL_TIME_ELAPSED, query);
    _queryFrameNumberList.push_back(QueryFrameNumberPair(query, frameNumber));
}

void EXTQuerySupport::endQuery(osg::State* /*state*/)
{
    _extensions->glEndQuery(GL_TIME_ELAPSED);
}

void OpenGLQuerySupport::initialize(osg::State* state, osg::Timer_t /*startTick*/)
{
    _extensions = state->get<osg::GLExtensions>();
}

void EXTQuerySupport::initialize(osg::State* state, osg::Timer_t startTick)
{
    OpenGLQuerySupport::initialize(state, startTick);
    _previousQueryTime = osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick());

}

class ARBQuerySupport : public OpenGLQuerySupport
{
public:
    virtual void checkQuery(osg::Stats* stats, osg::State* state,
                            osg::Timer_t startTick);

    virtual void beginQuery(unsigned int frameNumber, osg::State* state);
    virtual void endQuery(osg::State* state);
    virtual void initialize(osg::State* state, osg::Timer_t startTick);
protected:
    typedef std::pair<GLuint, GLuint> QueryPair;
    struct ActiveQuery {
        ActiveQuery() : queries(0, 0), frameNumber(0) {}
        ActiveQuery(GLuint start_, GLuint end_, int frameNumber_)
            : queries(start_, end_), frameNumber(frameNumber_)
        {
        }
        ActiveQuery(const QueryPair& queries_, unsigned int frameNumber_)
            : queries(queries_), frameNumber(frameNumber_)
        {
        }
        QueryPair queries;
        unsigned int frameNumber;
    };
    typedef std::list<ActiveQuery> QueryFrameList;
    typedef std::vector<QueryPair> QueryList;
    QueryFrameList _queryFrameList;
    QueryList _availableQueryObjects;
};

void ARBQuerySupport::initialize(osg::State* state, osg::Timer_t startTick)
{
    OpenGLQuerySupport::initialize(state, startTick);
}

void ARBQuerySupport::beginQuery(unsigned int frameNumber, osg::State* /*state*/)
{
    QueryPair query;
    if (_availableQueryObjects.empty())
    {
        _extensions->glGenQueries(1, &query.first);
        _extensions->glGenQueries(1, &query.second);
    }
    else
    {
        query = _availableQueryObjects.back();
        _availableQueryObjects.pop_back();
    }
    _extensions->glQueryCounter(query.first, GL_TIMESTAMP);
    _queryFrameList.push_back(ActiveQuery(query, frameNumber));
}

void ARBQuerySupport::endQuery(osg::State* /*state*/)
{
    _extensions->glQueryCounter(_queryFrameList.back().queries.second,
                                GL_TIMESTAMP);
}

void ARBQuerySupport::checkQuery(osg::Stats* stats, osg::State* state,
                                 osg::Timer_t /*startTick*/)
{
    for(QueryFrameList::iterator itr = _queryFrameList.begin();
        itr != _queryFrameList.end();
        )
    {
        GLint available = 0;
        // If the end query is available, the begin query must be too.
        _extensions->glGetQueryObjectiv(itr->queries.second,
                                        GL_QUERY_RESULT_AVAILABLE, &available);
        if (available)
        {
            QueryPair queries = itr->queries;
            GLuint64 beginTimestamp = 0;
            GLuint64 endTimestamp = 0;
            _extensions->glGetQueryObjectui64v(queries.first, GL_QUERY_RESULT,
                                               &beginTimestamp);
            _extensions->glGetQueryObjectui64v(queries.second, GL_QUERY_RESULT,
                                               &endTimestamp);
            GLuint64 gpuTimestamp = state->getGpuTimestamp();
            // Have any of the timestamps wrapped around?
            int tbits = state->getTimestampBits();
            if (tbits < 64)
            {
                // If the high bits on any of the timestamp bits are
                // different then the counters may have wrapped.
                const int hiShift = (tbits - 1);
                const GLuint64 hiMask = 1 << hiShift;
                const GLuint64 sum = (beginTimestamp >> hiShift)
                    + (endTimestamp >> hiShift) + (gpuTimestamp >> hiShift);
                if (sum == 1 || sum == 2) {
                    const GLuint64 wrapAdd = 1 << tbits;
                    // Counter wrapped between begin and end?
                    if (beginTimestamp > endTimestamp)
                    {
                        endTimestamp += wrapAdd;
                    }
                    else if (gpuTimestamp < beginTimestamp
                             && beginTimestamp - gpuTimestamp > (hiMask >> 1))
                    {
                        gpuTimestamp += wrapAdd;
                    }
                    else if (endTimestamp < gpuTimestamp
                             && gpuTimestamp - endTimestamp > (hiMask >> 1))
                    {
                        beginTimestamp += wrapAdd;
                        endTimestamp += wrapAdd;
                    }
                }
            }
            GLuint64 timeElapsed = endTimestamp - beginTimestamp;
            double timeElapsedSeconds = double(timeElapsed)*1e-9;
            double gpuTick = state->getGpuTime();
                     double beginTime = 0.0;
            double endTime = 0.0;
            if (beginTimestamp > gpuTimestamp)
                beginTime = gpuTick
                    + double(beginTimestamp - gpuTimestamp) * 1e-9;
            else
                beginTime = gpuTick
                    - double(gpuTimestamp - beginTimestamp) * 1e-9;
            if (endTimestamp > gpuTimestamp)
                endTime = gpuTick
                    + double(endTimestamp - gpuTimestamp) * 1e-9;
            else
                endTime = gpuTick
                    - double(gpuTimestamp - endTimestamp) * 1e-9;
            stats->setAttribute(itr->frameNumber, "GPU draw begin time",
                                beginTime);
            stats->setAttribute(itr->frameNumber, "GPU draw end time", endTime);
            stats->setAttribute(itr->frameNumber, "GPU draw time taken",
                                timeElapsedSeconds);
            itr = _queryFrameList.erase(itr);
            _availableQueryObjects.push_back(queries);
        }
        else
        {
            ++itr;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//  ThreadSafeQueue

Renderer::ThreadSafeQueue::ThreadSafeQueue()
    : _isReleased(false)
{
}

Renderer::ThreadSafeQueue::~ThreadSafeQueue()
{
}

void Renderer::ThreadSafeQueue::release()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _isReleased = true;
    _cond.broadcast();
}

void Renderer::ThreadSafeQueue::reset()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _queue.clear();
    _isReleased = false;
}

osgUtil::SceneView* Renderer::ThreadSafeQueue::takeFront()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    // Loop in case there are spurious wakeups from the condition wait.
    while (true)
    {
        // If the queue has been released but nothing is enqueued,
        // just return. This prevents a deadlock when threading is
        // restarted.
        if (_isReleased)
        {
            if (!_queue.empty())
            {
                osgUtil::SceneView* front = _queue.front();
                _queue.pop_front();
                if (_queue.empty())
                    _isReleased = false;
                return front;
            }
            return 0;
        }
        _cond.wait(&_mutex);
    }
    return 0;                   // Can't happen
}

void Renderer::ThreadSafeQueue::add(osgUtil::SceneView* sv)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _queue.push_back(sv);
    _isReleased = true;
    _cond.broadcast();
}

static OpenThreads::ReentrantMutex s_drawSerializerMutex;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//  Renderer
Renderer::Renderer(osg::Camera* camera):
    osg::Referenced(true),
    osg::GraphicsOperation("Renderer",true),
    _camera(camera),
    _done(false),
    _graphicsThreadDoesCull(true),
    _compileOnNextDraw(true),
    _serializeDraw(false),
    _initialized(false),
    _startTick(0)
{

    DEBUG_MESSAGE<<"Render::Render() "<<this<<std::endl;

    _sceneView[0] = new osgUtil::SceneView;
    _sceneView[1] = new osgUtil::SceneView;

    osg::Camera* masterCamera = _camera->getView() ? _camera->getView()->getCamera() : camera;

    osg::StateSet* global_stateset = 0;
    osg::StateSet* secondary_stateset = 0;
    if (_camera != masterCamera)
    {
        global_stateset = masterCamera->getOrCreateStateSet();
        secondary_stateset = _camera->getStateSet();
    }
    else
    {
        global_stateset = _camera->getOrCreateStateSet();
    }

    osgViewer::View* view = dynamic_cast<osgViewer::View*>(_camera->getView());
    osgViewer::ViewerBase* viewer = view ? view->getViewerBase() : 0;
    osgUtil::IncrementalCompileOperation* ico = viewer ? viewer->getIncrementalCompileOperation() : 0;
    bool automaticFlush = (ico==NULL);

    osg::DisplaySettings* ds = _camera->getDisplaySettings() ?  _camera->getDisplaySettings() :
                               ((view && view->getDisplaySettings()) ?  view->getDisplaySettings() :  osg::DisplaySettings::instance().get());

    _serializeDraw = ds ? ds->getSerializeDrawDispatch() : false;

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

    _sceneView[0]->setAutomaticFlush(automaticFlush);
    _sceneView[0]->setGlobalStateSet(global_stateset);
    _sceneView[0]->setSecondaryStateSet(secondary_stateset);

    _sceneView[1]->setAutomaticFlush(automaticFlush);
    _sceneView[1]->setGlobalStateSet(global_stateset);
    _sceneView[1]->setSecondaryStateSet(secondary_stateset);

    _sceneView[0]->setDefaults(sceneViewOptions);
    _sceneView[1]->setDefaults(sceneViewOptions);

    if (ds->getUseSceneViewForStereoHint())
    {
        _sceneView[0]->setDisplaySettings(ds);
        _sceneView[1]->setDisplaySettings(ds);
    }
    else
    {
        _sceneView[0]->setResetColorMaskToAllOn(false);
        _sceneView[1]->setResetColorMaskToAllOn(false);
    }

    _sceneView[0]->setCamera(_camera.get(), false);
    _sceneView[1]->setCamera(_camera.get(), false);

    {
        // assign CullVisitor::Identifier so that the double buffering of SceneView doesn't interfer
        // with code that requires a consistent knowledge and which effective cull traversal to taking place
        osg::ref_ptr<osgUtil::CullVisitor::Identifier> leftEyeIdentifier = new osgUtil::CullVisitor::Identifier();
        osg::ref_ptr<osgUtil::CullVisitor::Identifier> rightEyeIdentifier = new osgUtil::CullVisitor::Identifier();

        _sceneView[0]->getCullVisitor()->setIdentifier(leftEyeIdentifier.get());
        _sceneView[0]->setCullVisitorLeft(_sceneView[0]->getCullVisitor()->clone());
        _sceneView[0]->getCullVisitorLeft()->setIdentifier(leftEyeIdentifier.get());
        _sceneView[0]->setCullVisitorRight(_sceneView[0]->getCullVisitor()->clone());
        _sceneView[0]->getCullVisitorRight()->setIdentifier(rightEyeIdentifier.get());

        _sceneView[1]->getCullVisitor()->setIdentifier(leftEyeIdentifier.get());
        _sceneView[1]->setCullVisitorLeft(_sceneView[1]->getCullVisitor()->clone());
        _sceneView[1]->getCullVisitorLeft()->setIdentifier(leftEyeIdentifier.get());
        _sceneView[1]->setCullVisitorRight(_sceneView[1]->getCullVisitor()->clone());
        _sceneView[1]->getCullVisitorRight()->setIdentifier(rightEyeIdentifier.get());
    }

    // lock the mutex for the current cull SceneView to
    // prevent the draw traversal from reading from it before the cull traversal has been completed.
    _availableQueue.add(_sceneView[0].get());
    _availableQueue.add(_sceneView[1].get());

    DEBUG_MESSAGE<<"_availableQueue.size()="<<_availableQueue._queue.size()<<std::endl;
}

Renderer::~Renderer()
{
    DEBUG_MESSAGE<<"Render::~Render() "<<this<<std::endl;
}

void Renderer::initialize(osg::State* state)
{
    if (!_initialized)
    {
        _initialized = true;
        osg::GLExtensions* ext = state->get<osg::GLExtensions>();
        if (ext->isARBTimerQuerySupported && state->getTimestampBits() > 0)
            _querySupport = new ARBQuerySupport();
        else if (ext->isTimerQuerySupported)
            _querySupport = new EXTQuerySupport();
        if (_querySupport.valid())
            _querySupport->initialize(state, _startTick);
    }
}

void Renderer::setGraphicsThreadDoesCull(bool flag)
{
    if (_graphicsThreadDoesCull==flag) return;

    _graphicsThreadDoesCull = flag;
}

void Renderer::updateSceneView(osgUtil::SceneView* sceneView)
{
    osg::Camera* masterCamera = _camera->getView() ? _camera->getView()->getCamera() : _camera.get();

    osg::StateSet* global_stateset = 0;
    osg::StateSet* secondary_stateset = 0;
    if (_camera != masterCamera)
    {
        global_stateset = masterCamera->getOrCreateStateSet();
        secondary_stateset = _camera->getStateSet();
    }
    else
    {
        global_stateset = _camera->getOrCreateStateSet();
    }

    if (sceneView->getGlobalStateSet()!=global_stateset)
    {
        sceneView->setGlobalStateSet(global_stateset);
    }

    if (sceneView->getSecondaryStateSet()!=secondary_stateset)
    {
        sceneView->setSecondaryStateSet(secondary_stateset);
    }

    osg::GraphicsContext* context = _camera->getGraphicsContext();
    osg::State* state = context ? context->getState() : 0;
    if (sceneView->getState()!=state)
    {
        sceneView->setState(state);
    }

    osgViewer::View* view = dynamic_cast<osgViewer::View*>(_camera->getView());
    osgViewer::ViewerBase* viewer = view ? view->getViewerBase() : 0;
    osgUtil::IncrementalCompileOperation* ico = viewer ? viewer->getIncrementalCompileOperation() : 0;
    bool automaticFlush = (ico==NULL);

    sceneView->setAutomaticFlush(automaticFlush);

    osgDB::DatabasePager* databasePager = view ? view->getDatabasePager() : 0;
    sceneView->getCullVisitor()->setDatabaseRequestHandler(databasePager);

    osgDB::ImagePager* imagePager = view ? view->getImagePager() : 0;
    sceneView->getCullVisitor()->setImageRequestHandler(imagePager);

    sceneView->setFrameStamp(view ? view->getFrameStamp() : state->getFrameStamp());

    osg::DisplaySettings* ds = _camera->getDisplaySettings() ?  _camera->getDisplaySettings() :
                               ((view &&view->getDisplaySettings()) ?  view->getDisplaySettings() :  osg::DisplaySettings::instance().get());

    if (ds->getUseSceneViewForStereoHint())
    {
        sceneView->setDisplaySettings(ds);
    }

    if (view)
    {
        _startTick = view->getStartTick();
        if (state) state->setStartTick(_startTick);
    }
}

void Renderer::compile()
{
    DEBUG_MESSAGE<<"Renderer::compile()"<<std::endl;


    _compileOnNextDraw = false;

    osgUtil::SceneView* sceneView = _sceneView[0].get();
    if (!sceneView || _done) return;

    sceneView->getState()->checkGLErrors("Before Renderer::compile");

    if (sceneView->getSceneData())
    {
        osgUtil::GLObjectsVisitor glov;
        glov.setState(sceneView->getState());
        sceneView->getSceneData()->accept(glov);
    }

    sceneView->getState()->checkGLErrors("After Renderer::compile");
}

static void collectSceneViewStats(unsigned int frameNumber, osgUtil::SceneView* sceneView, osg::Stats* stats)
{
    osgUtil::Statistics sceneStats;
    sceneView->getStats(sceneStats);

    stats->setAttribute(frameNumber, "Visible vertex count", static_cast<double>(sceneStats._vertexCount));
    stats->setAttribute(frameNumber, "Visible number of drawables", static_cast<double>(sceneStats.numDrawables));
    stats->setAttribute(frameNumber, "Visible number of fast drawables", static_cast<double>(sceneStats.numFastDrawables));
    stats->setAttribute(frameNumber, "Visible number of lights", static_cast<double>(sceneStats.nlights));
    stats->setAttribute(frameNumber, "Visible number of render bins", static_cast<double>(sceneStats.nbins));
    stats->setAttribute(frameNumber, "Visible depth", static_cast<double>(sceneStats.depth));
    stats->setAttribute(frameNumber, "Number of StateGraphs", static_cast<double>(sceneStats.numStateGraphs));
    stats->setAttribute(frameNumber, "Visible number of impostors", static_cast<double>(sceneStats.nimpostor));
    stats->setAttribute(frameNumber, "Number of ordered leaves", static_cast<double>(sceneStats.numOrderedLeaves));

    unsigned int totalNumPrimitiveSets = 0;
    const osgUtil::Statistics::PrimitiveValueMap& pvm = sceneStats.getPrimitiveValueMap();
    for(osgUtil::Statistics::PrimitiveValueMap::const_iterator pvm_itr = pvm.begin();
        pvm_itr != pvm.end();
        ++pvm_itr)
    {
        totalNumPrimitiveSets += pvm_itr->second.first;
    }
    stats->setAttribute(frameNumber, "Visible number of PrimitiveSets", static_cast<double>(totalNumPrimitiveSets));

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

        // OSG_NOTICE<<"Culling buffer "<<_currentCull<<std::endl;

        // pass on the fusion distance settings from the View to the SceneView
        osgViewer::View* view = dynamic_cast<osgViewer::View*>(sceneView->getCamera()->getView());
        if (view) sceneView->setFusionDistance(view->getFusionDistanceMode(), view->getFusionDistanceValue());

        osg::Stats* stats = sceneView->getCamera()->getStats();
        osg::State* state = sceneView->getState();
        const osg::FrameStamp* fs = state->getFrameStamp();
        unsigned int frameNumber = fs ? fs->getFrameNumber() : 0;

        // do cull traversal
        osg::Timer_t beforeCullTick = osg::Timer::instance()->tick();

        sceneView->inheritCullSettings(*(sceneView->getCamera()));
        sceneView->cull();

        osg::Timer_t afterCullTick = osg::Timer::instance()->tick();

#if 0
        if (sceneView->getDynamicObjectCount()==0 && state->getDynamicObjectRenderingCompletedCallback())
        {
            // OSG_NOTICE<<"Completed in cull"<<std::endl;
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
            collectSceneViewStats(frameNumber, sceneView, stats);
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

    if (sceneView && !_done)
    {
        // since we are running the draw thread in parallel with the main thread it's possible to unreference Camera's
        // that are still being used by this rendering thread, so to prevent this we'll take references to all these
        // Camera's and the clear these references once we've completed the whole draw dispatch.
        sceneView->collateReferencesToDependentCameras();

        if (_compileOnNextDraw)
        {
            compile();
        }

        // OSG_NOTICE<<"Drawing buffer "<<_currentDraw<<std::endl;

        if (_done)
        {
            OSG_INFO<<"Renderer::release() causing draw to exit"<<std::endl;
            return;
        }

        if (_graphicsThreadDoesCull)
        {
            OSG_INFO<<"Renderer::draw() completing early due to change in _graphicsThreadDoesCull flag."<<std::endl;
            return;
        }

        // OSG_NOTICE<<"RenderingOperation"<<std::endl;

        osg::Stats* stats = sceneView->getCamera()->getStats();
        osg::State* state = sceneView->getState();
        unsigned int frameNumber = sceneView->getFrameStamp()->getFrameNumber();

        if (!_initialized)
        {
            initialize(state);
        }

        state->setDynamicObjectCount(sceneView->getDynamicObjectCount());

        if (sceneView->getDynamicObjectCount()==0 && state->getDynamicObjectRenderingCompletedCallback())
        {
            // OSG_NOTICE<<"Completed in cull"<<std::endl;
            state->getDynamicObjectRenderingCompletedCallback()->completed(state);
        }

        bool acquireGPUStats = stats && _querySupport && stats->collectStats("gpu");

        if (acquireGPUStats)
        {
            _querySupport->checkQuery(stats, state, _startTick);
        }

        // do draw traversal
        if (acquireGPUStats)
        {
            _querySupport->checkQuery(stats, state, _startTick);
            _querySupport->beginQuery(frameNumber, state);
        }

        osg::Timer_t beforeDrawTick;


        if (_serializeDraw)
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

        if (acquireGPUStats)
        {
            _querySupport->endQuery(state);
            _querySupport->checkQuery(stats, state, _startTick);
        }

        //glFlush();

        osg::Timer_t afterDrawTick = osg::Timer::instance()->tick();

//        OSG_NOTICE<<"Time wait for draw = "<<osg::Timer::instance()->delta_m(startDrawTick, beforeDrawTick)<<std::endl;
//        OSG_NOTICE<<"     time for draw = "<<osg::Timer::instance()->delta_m(beforeDrawTick, afterDrawTick)<<std::endl;

        if (stats && stats->collectStats("rendering"))
        {
            stats->setAttribute(frameNumber, "Draw traversal begin time", osg::Timer::instance()->delta_s(_startTick, beforeDrawTick));
            stats->setAttribute(frameNumber, "Draw traversal end time", osg::Timer::instance()->delta_s(_startTick, afterDrawTick));
            stats->setAttribute(frameNumber, "Draw traversal time taken", osg::Timer::instance()->delta_s(beforeDrawTick, afterDrawTick));
        }

        sceneView->clearReferencesToDependentCameras();
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
        OSG_INFO<<"Render::release() causing cull_draw to exit"<<std::endl;
        return;
    }

    updateSceneView(sceneView);

    if (_compileOnNextDraw)
    {
        compile();
    }

    osgViewer::View* view = dynamic_cast<osgViewer::View*>(_camera->getView());

    // OSG_NOTICE<<"RenderingOperation"<<std::endl;

    // pass on the fusion distance settings from the View to the SceneView
    if (view) sceneView->setFusionDistance(view->getFusionDistanceMode(), view->getFusionDistanceValue());

    osg::Stats* stats = sceneView->getCamera()->getStats();
    osg::State* state = sceneView->getState();
    const osg::FrameStamp* fs = sceneView->getFrameStamp();
    unsigned int frameNumber = fs ? fs->getFrameNumber() : 0;

    if (!_initialized)
    {
        initialize(state);
    }

    bool acquireGPUStats = stats && _querySupport && stats->collectStats("gpu");

    if (acquireGPUStats)
    {
        _querySupport->checkQuery(stats, state, _startTick);
    }

    // do cull traversal
    osg::Timer_t beforeCullTick = osg::Timer::instance()->tick();

    sceneView->inheritCullSettings(*(sceneView->getCamera()));
    sceneView->cull();

    osg::Timer_t afterCullTick = osg::Timer::instance()->tick();

    if (stats && stats->collectStats("scene"))
    {
        collectSceneViewStats(frameNumber, sceneView, stats);
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
        _querySupport->checkQuery(stats, state, _startTick);
        _querySupport->beginQuery(frameNumber, state);
    }

    osg::Timer_t beforeDrawTick;

    if (_serializeDraw)
    {
        OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(s_drawSerializerMutex);

        beforeDrawTick = osg::Timer::instance()->tick();
        sceneView->draw();
    }
    else
    {
        beforeDrawTick = osg::Timer::instance()->tick();
        sceneView->draw();
    }

    if (acquireGPUStats)
    {
        _querySupport->endQuery(state);
        _querySupport->checkQuery(stats, state, _startTick);
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

void Renderer::operator () (osg::Object* object)
{
    osg::GraphicsContext* context = dynamic_cast<osg::GraphicsContext*>(object);
    if (context) operator()(context);

    osg::Camera* camera = dynamic_cast<osg::Camera*>(object);
    if (camera) cull();
}

void Renderer::operator () (osg::GraphicsContext* /*context*/)
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
    OSG_INFO<<"Renderer::release()"<<std::endl;
    _done = true;

    _availableQueue.release();
    _drawQueue.release();
}

void Renderer::reset(){
    _availableQueue.reset();
    _availableQueue.add(_sceneView[0].get());
    _availableQueue.add(_sceneView[1].get());
    _drawQueue.reset();
}

void Renderer::setCameraRequiresSetUp(bool flag)
{
    for (int i = 0; i < 2; ++i)
    {
        osgUtil::SceneView* sv = getSceneView(i);
        osgUtil::RenderStage* rs = sv ? sv->getRenderStage() : 0;
        if (rs) rs->setCameraRequiresSetUp(flag);
        rs = sv ? sv->getRenderStageLeft() : 0;
        if (rs) rs->setCameraRequiresSetUp(flag);
        rs = sv ? sv->getRenderStageRight() : 0;
        if (rs) rs->setCameraRequiresSetUp(flag);
    }
}

bool Renderer::getCameraRequiresSetUp() const
{
    bool result = false;
    for (int i = 0; i < 2; ++i)
    {
        const osgUtil::SceneView* sv = getSceneView(i);
        const osgUtil::RenderStage* rs = sv ? sv->getRenderStage() : 0;
        if (rs) result = result || rs->getCameraRequiresSetUp();
        rs = sv ? sv->getRenderStageLeft() : 0;
        if (rs) result = result || rs->getCameraRequiresSetUp();
        rs = sv ? sv->getRenderStageRight() : 0;
        if (rs) result = result || rs->getCameraRequiresSetUp();
    }
    return result;
}
