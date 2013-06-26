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

#include <sstream>
#include <iomanip>
#include <stdio.h>

#include <osg/io_utils>

#include <osg/MatrixTransform>

#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Renderer>

#include <osg/PolygonMode>
#include <osg/Geometry>

namespace osgViewer
{


StatsHandler::StatsHandler():
    _keyEventTogglesOnScreenStats('s'),
    _keyEventPrintsOutStats('S'),
    _statsType(NO_STATS),
    _initialized(false),
    _threadingModel(ViewerBase::SingleThreaded),
    _frameRateChildNum(0),
    _viewerChildNum(0),
    _cameraSceneChildNum(0),
    _viewerSceneChildNum(0),
    _numBlocks(8),
    _blockMultiplier(10000.0),
    _statsWidth(1280.0f),
    _statsHeight(1024.0f),
    _font("fonts/arial.ttf"),
    _startBlocks(150.0f),
    _leftPos(10.0f),
    _characterSize(20.0f),
    _lineHeight(1.5f)
{
    _camera = new osg::Camera;
    _camera->setRenderer(new Renderer(_camera.get()));
    _camera->setProjectionResizePolicy(osg::Camera::FIXED);
}

bool StatsHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{

    osgViewer::View* myview = dynamic_cast<osgViewer::View*>(&aa);
    if (!myview) return false;

    osgViewer::ViewerBase* viewer = myview->getViewerBase();
    if (viewer && _threadingModelText.valid() && viewer->getThreadingModel()!=_threadingModel)
    {
        _threadingModel = viewer->getThreadingModel();
        updateThreadingModelText();
    }


    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()==_keyEventTogglesOnScreenStats)
            {
                if (viewer->getViewerStats())
                {
                    if (!_initialized)
                    {
                        setUpHUDCamera(viewer);
                        setUpScene(viewer);
                    }

                    ++_statsType;

                    if (_statsType==LAST) _statsType = NO_STATS;

                    osgViewer::ViewerBase::Cameras cameras;
                    viewer->getCameras(cameras);

                    switch(_statsType)
                    {
                        case(NO_STATS):
                        {
                            viewer->getViewerStats()->collectStats("frame_rate",false);
                            viewer->getViewerStats()->collectStats("event",false);
                            viewer->getViewerStats()->collectStats("update",false);

                            for(osgViewer::ViewerBase::Cameras::iterator itr = cameras.begin();
                                itr != cameras.end();
                                ++itr)
                            {
                                osg::Stats* stats = (*itr)->getStats();
                                if (stats)
                                {
                                    stats->collectStats("rendering",false);
                                    stats->collectStats("gpu",false);
                                    stats->collectStats("scene",false);
                                }
                            }

                            viewer->getViewerStats()->collectStats("scene",false);

                            _camera->setNodeMask(0x0);
                            _switch->setAllChildrenOff();
                            break;
                        }
                        case(FRAME_RATE):
                        {
                            viewer->getViewerStats()->collectStats("frame_rate",true);

                            _camera->setNodeMask(0xffffffff);
                            _switch->setValue(_frameRateChildNum, true);
                            break;
                        }
                        case(VIEWER_STATS):
                        {
                            ViewerBase::Scenes scenes;
                            viewer->getScenes(scenes);
                            for(ViewerBase::Scenes::iterator itr = scenes.begin();
                                itr != scenes.end();
                                ++itr)
                            {
                                Scene* scene = *itr;
                                osgDB::DatabasePager* dp = scene->getDatabasePager();
                                if (dp && dp->isRunning())
                                {
                                    dp->resetStats();
                                }
                            }

                            viewer->getViewerStats()->collectStats("event",true);
                            viewer->getViewerStats()->collectStats("update",true);

                            for(osgViewer::ViewerBase::Cameras::iterator itr = cameras.begin();
                                itr != cameras.end();
                                ++itr)
                            {
                                if ((*itr)->getStats()) (*itr)->getStats()->collectStats("rendering",true);
                                if ((*itr)->getStats()) (*itr)->getStats()->collectStats("gpu",true);
                            }

                            _camera->setNodeMask(0xffffffff);
                            _switch->setValue(_viewerChildNum, true);
                            break;
                        }
                        case(CAMERA_SCENE_STATS):
                        {
                            _camera->setNodeMask(0xffffffff);
                            _switch->setValue(_cameraSceneChildNum, true);

                            for(osgViewer::ViewerBase::Cameras::iterator itr = cameras.begin();
                                itr != cameras.end();
                                ++itr)
                            {
                                osg::Stats* stats = (*itr)->getStats();
                                if (stats)
                                {
                                    stats->collectStats("scene",true);
                                }
                            }

                            break;
                        }
                        case(VIEWER_SCENE_STATS):
                        {
                            _camera->setNodeMask(0xffffffff);
                            _switch->setValue(_viewerSceneChildNum, true);

                            viewer->getViewerStats()->collectStats("scene",true);

                            break;
                        }
                        default:
                            break;
                    }

                    aa.requestRedraw();
                }
                return true;
            }
            if (ea.getKey()==_keyEventPrintsOutStats)
            {
                if (viewer->getViewerStats())
                {
                    OSG_NOTICE<<std::endl<<"Stats report:"<<std::endl;
                    typedef std::vector<osg::Stats*> StatsList;
                    StatsList statsList;
                    statsList.push_back(viewer->getViewerStats());

                    osgViewer::ViewerBase::Contexts contexts;
                    viewer->getContexts(contexts);
                    for(osgViewer::ViewerBase::Contexts::iterator gcitr = contexts.begin();
                        gcitr != contexts.end();
                        ++gcitr)
                    {
                        osg::GraphicsContext::Cameras& cameras = (*gcitr)->getCameras();
                        for(osg::GraphicsContext::Cameras::iterator itr = cameras.begin();
                            itr != cameras.end();
                            ++itr)
                        {
                            if ((*itr)->getStats())
                            {
                                statsList.push_back((*itr)->getStats());
                            }
                        }
                    }

                    for(unsigned int i = viewer->getViewerStats()->getEarliestFrameNumber(); i< viewer->getViewerStats()->getLatestFrameNumber(); ++i)
                    {
                        for(StatsList::iterator itr = statsList.begin();
                            itr != statsList.end();
                            ++itr)
                        {
                            if (itr==statsList.begin()) (*itr)->report(osg::notify(osg::NOTICE), i);
                            else (*itr)->report(osg::notify(osg::NOTICE), i, "    ");
                        }
                        OSG_NOTICE<<std::endl;
                    }

                }
                return true;
            }
        }
        case(osgGA::GUIEventAdapter::RESIZE):
            setWindowSize(ea.getWindowWidth(), ea.getWindowHeight());
            break;
        default: break;
    }

    return false;

}

void StatsHandler::updateThreadingModelText()
{
    switch(_threadingModel)
    {
        case(osgViewer::Viewer::SingleThreaded): _threadingModelText->setText("ThreadingModel: SingleThreaded"); break;
        case(osgViewer::Viewer::CullDrawThreadPerContext): _threadingModelText->setText("ThreadingModel: CullDrawThreadPerContext"); break;
        case(osgViewer::Viewer::DrawThreadPerContext): _threadingModelText->setText("ThreadingModel: DrawThreadPerContext"); break;
        case(osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext): _threadingModelText->setText("ThreadingModel: CullThreadPerCameraDrawThreadPerContext"); break;
        case(osgViewer::Viewer::AutomaticSelection): _threadingModelText->setText("ThreadingModel: AutomaticSelection"); break;
        default:
            _threadingModelText->setText("ThreadingModel: unknown"); break;
    }
}

void StatsHandler::reset()
{
    _initialized = false;
    _camera->setGraphicsContext(0);
    _camera->removeChildren( 0, _camera->getNumChildren() );
}

void StatsHandler::addUserStatsLine(const std::string& label, const osg::Vec4& textColor, const osg::Vec4& barColor,
                                    const std::string& timeTakenName, float multiplier, bool average, bool averageInInverseSpace,
                                    const std::string& beginTimeName, const std::string& endTimeName, float maxValue)
{
    _userStatsLines.push_back(UserStatsLine(label, textColor, barColor, timeTakenName, multiplier, average, averageInInverseSpace, beginTimeName, endTimeName, maxValue));
    reset();        // Rebuild the stats display with the new user stats line.
}

void StatsHandler::removeUserStatsLine(const std::string& label)
{
    // Hopefully the labels are unique... This could be enforced.
    for (unsigned int i = 0; i < _userStatsLines.size(); ++i)
    {
        if (_userStatsLines[i].label == label)
        {
            _userStatsLines.erase(_userStatsLines.begin() + i);
            reset();        // Rebuild the stats display without the removed user stats line.
            break;
        }
    }
}

void StatsHandler::setUpHUDCamera(osgViewer::ViewerBase* viewer)
{
    // Try GraphicsWindow first so we're likely to get the main viewer window
    osg::GraphicsContext* context = dynamic_cast<osgViewer::GraphicsWindow*>(_camera->getGraphicsContext());

    if (!context)
    {
        osgViewer::Viewer::Windows windows;
        viewer->getWindows(windows);

        if (!windows.empty()) context = windows.front();
        else
        {
            // No GraphicsWindows were found, so let's try to find a GraphicsContext
            context = _camera->getGraphicsContext();

            if (!context)
            {
                osgViewer::Viewer::Contexts contexts;
                viewer->getContexts(contexts);

                if (contexts.empty()) return;

                context = contexts.front();
            }
        }
    }

    _camera->setGraphicsContext(context);

    _camera->setRenderOrder(osg::Camera::POST_RENDER, 10);

    _camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _camera->setViewMatrix(osg::Matrix::identity());
    setWindowSize(context->getTraits()->width, context->getTraits()->height);

    // only clear the depth buffer
    _camera->setClearMask(0);

    _camera->setRenderer(new Renderer(_camera.get()));

    _initialized = true;
}

void StatsHandler::setWindowSize(int width, int height)
{
    if (width <= 0 || height <= 0)
        return;

    _camera->setViewport(0, 0, width, height);
    if (fabs(height*_statsWidth) <= fabs(width*_statsHeight))
    {
        _camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0,width*_statsHeight/height,0.0,_statsHeight));
    }
    else
    {
        _camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0,_statsWidth,_statsHeight-height*_statsWidth/width,_statsHeight));
    }
}

// Drawcallback to draw averaged attribute
struct AveragedValueTextDrawCallback : public virtual osg::Drawable::DrawCallback
{
    AveragedValueTextDrawCallback(osg::Stats* stats, const std::string& name, int frameDelta, bool averageInInverseSpace, double multiplier):
        _stats(stats),
        _attributeName(name),
        _frameDelta(frameDelta),
        _averageInInverseSpace(averageInInverseSpace),
        _multiplier(multiplier),
        _tickLastUpdated(0)
    {
    }

    /** do customized draw code.*/
    virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
    {
        osgText::Text* text = (osgText::Text*)drawable;

        osg::Timer_t tick = osg::Timer::instance()->tick();
        double delta = osg::Timer::instance()->delta_m(_tickLastUpdated, tick);

        if (delta>50) // update every 50ms
        {
            _tickLastUpdated = tick;
            double value;
            if (_stats->getAveragedAttribute( _attributeName, value, _averageInInverseSpace))
            {
                char tmpText[128];
                sprintf(tmpText,"%4.2f",value * _multiplier);
                text->setText(tmpText);
            }
            else
            {
                text->setText("");
            }
        }
        text->drawImplementation(renderInfo);
    }

    osg::ref_ptr<osg::Stats>    _stats;
    std::string                 _attributeName;
    int                         _frameDelta;
    bool                        _averageInInverseSpace;
    double                      _multiplier;
    mutable osg::Timer_t        _tickLastUpdated;
};

// Drawcallback to draw raw attribute
struct RawValueTextDrawCallback : public virtual osg::Drawable::DrawCallback
{
    RawValueTextDrawCallback(osg::Stats* stats, const std::string& name, int frameDelta, double multiplier):
        _stats(stats),
        _attributeName(name),
        _frameDelta(frameDelta),
        _multiplier(multiplier),
        _tickLastUpdated(0)
    {
    }

    /** do customized draw code.*/
    virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
    {
        osgText::Text* text = (osgText::Text*)drawable;

        osg::Timer_t tick = osg::Timer::instance()->tick();
        double delta = osg::Timer::instance()->delta_m(_tickLastUpdated, tick);

        if (delta>50) // update every 50ms
        {
            _tickLastUpdated = tick;

            unsigned int frameNumber = renderInfo.getState()->getFrameStamp()->getFrameNumber();
            double value;
            if (_stats->getAttribute(frameNumber, _attributeName, value))
            {
                char tmpText[128];
                sprintf(tmpText,"%4.2f",value * _multiplier);
                text->setText(tmpText);
            }
            else
            {
                text->setText("");
            }
        }
        text->drawImplementation(renderInfo);
    }

    osg::ref_ptr<osg::Stats>    _stats;
    std::string                 _attributeName;
    int                         _frameDelta;
    double                      _multiplier;
    mutable osg::Timer_t        _tickLastUpdated;
};

struct CameraSceneStatsTextDrawCallback : public virtual osg::Drawable::DrawCallback
{
    CameraSceneStatsTextDrawCallback(osg::Camera* camera, int cameraNumber):
        _camera(camera),
        _tickLastUpdated(0),
        _cameraNumber(cameraNumber)
    {
    }

    /** do customized draw code.*/
    virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
    {
        if (!_camera) return;

        osgText::Text* text = (osgText::Text*)drawable;

        osg::Timer_t tick = osg::Timer::instance()->tick();
        double delta = osg::Timer::instance()->delta_m(_tickLastUpdated, tick);

        if (delta > 100) // update every 100ms
        {
            _tickLastUpdated = tick;
            std::ostringstream viewStr;
            viewStr.clear();

            osg::Stats* stats = _camera->getStats();
            osgViewer::Renderer* renderer = dynamic_cast<osgViewer::Renderer*>(_camera->getRenderer());

            if (stats && renderer)
            {
                viewStr.setf(std::ios::left, std::ios::adjustfield);
                viewStr.width(14);
                // Used fixed formatting, as scientific will switch to "...e+.." notation for
                // large numbers of vertices/drawables/etc.
                viewStr.setf(std::ios::fixed);
                viewStr.precision(0);

                viewStr << std::setw(1) << "#" << _cameraNumber << std::endl;

                // Camera name
                if (!_camera->getName().empty())
                    viewStr << _camera->getName();
                viewStr << std::endl;

                unsigned int frameNumber = renderInfo.getState()->getFrameStamp()->getFrameNumber();
                if (!(renderer->getGraphicsThreadDoesCull()))
                {
                    --frameNumber;
                }

                #define STATS_ATTRIBUTE(str) \
                    if (stats->getAttribute(frameNumber, str, value)) \
                        viewStr << std::setw(8) << value << std::endl; \
                    else \
                        viewStr << std::setw(8) << "." << std::endl; \

                double value = 0.0;

                STATS_ATTRIBUTE("Visible number of lights")
                STATS_ATTRIBUTE("Visible number of render bins")
                STATS_ATTRIBUTE("Visible depth")
                STATS_ATTRIBUTE("Number of StateGraphs")
                STATS_ATTRIBUTE("Visible number of impostors")
                STATS_ATTRIBUTE("Visible number of drawables")
                STATS_ATTRIBUTE("Number of ordered leaves")
                STATS_ATTRIBUTE("Visible number of fast drawables")
                STATS_ATTRIBUTE("Visible vertex count")

                STATS_ATTRIBUTE("Visible number of PrimitiveSets")
                STATS_ATTRIBUTE("Visible number of GL_POINTS")
                STATS_ATTRIBUTE("Visible number of GL_LINES")
                STATS_ATTRIBUTE("Visible number of GL_LINE_STRIP")
                STATS_ATTRIBUTE("Visible number of GL_LINE_LOOP")
                STATS_ATTRIBUTE("Visible number of GL_TRIANGLES")
                STATS_ATTRIBUTE("Visible number of GL_TRIANGLE_STRIP")
                STATS_ATTRIBUTE("Visible number of GL_TRIANGLE_FAN")
                STATS_ATTRIBUTE("Visible number of GL_QUADS")
                STATS_ATTRIBUTE("Visible number of GL_QUAD_STRIP")
                STATS_ATTRIBUTE("Visible number of GL_POLYGON")

                text->setText(viewStr.str());
            }
        }
        text->drawImplementation(renderInfo);
    }

    osg::observer_ptr<osg::Camera>  _camera;
    mutable osg::Timer_t            _tickLastUpdated;
    int                             _cameraNumber;
};


struct ViewSceneStatsTextDrawCallback : public virtual osg::Drawable::DrawCallback
{
    ViewSceneStatsTextDrawCallback(osgViewer::View* view, int viewNumber):
        _view(view),
        _tickLastUpdated(0),
        _viewNumber(viewNumber)
    {
    }

    /** do customized draw code.*/
    virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
    {
        if (!_view) return;

        osgText::Text* text = (osgText::Text*)drawable;

        osg::Timer_t tick = osg::Timer::instance()->tick();
        double delta = osg::Timer::instance()->delta_m(_tickLastUpdated, tick);

        if (delta > 200) // update every 100ms
        {
            _tickLastUpdated = tick;
            osg::Stats* stats = _view->getStats();
            if (stats)
            {
                std::ostringstream viewStr;
                viewStr.clear();
                viewStr.setf(std::ios::left, std::ios::adjustfield);
                viewStr.width(20);
                viewStr.setf(std::ios::fixed);
                viewStr.precision(0);

                viewStr << std::setw(1) << "#" << _viewNumber;

                // View name
                if (!_view->getName().empty())
                    viewStr << ": " << _view->getName();
                viewStr << std::endl;

                unsigned int frameNumber = renderInfo.getState()->getFrameStamp()->getFrameNumber();
                // if (!(renderer->getGraphicsThreadDoesCull()))
                {
                    --frameNumber;
                }

                #define STATS_ATTRIBUTE_PAIR(str1, str2) \
                    if (stats->getAttribute(frameNumber, str1, value)) \
                        viewStr << std::setw(9) << value; \
                    else \
                        viewStr << std::setw(9) << "."; \
                    if (stats->getAttribute(frameNumber, str2, value)) \
                        viewStr << std::setw(9) << value << std::endl; \
                    else \
                        viewStr << std::setw(9) << "." << std::endl; \

                double value = 0.0;

                // header
                viewStr << std::setw(9) << "Unique" << std::setw(9) << "Instance" << std::endl;

                STATS_ATTRIBUTE_PAIR("Number of unique StateSet","Number of instanced Stateset")
                STATS_ATTRIBUTE_PAIR("Number of unique Group","Number of instanced Group")
                STATS_ATTRIBUTE_PAIR("Number of unique Transform","Number of instanced Transform")
                STATS_ATTRIBUTE_PAIR("Number of unique LOD","Number of instanced LOD")
                STATS_ATTRIBUTE_PAIR("Number of unique Switch","Number of instanced Switch")
                STATS_ATTRIBUTE_PAIR("Number of unique Geode","Number of instanced Geode")
                STATS_ATTRIBUTE_PAIR("Number of unique Drawable","Number of instanced Drawable")
                STATS_ATTRIBUTE_PAIR("Number of unique Geometry","Number of instanced Geometry")
                STATS_ATTRIBUTE_PAIR("Number of unique Vertices","Number of instanced Vertices")
                STATS_ATTRIBUTE_PAIR("Number of unique Primitives","Number of instanced Primitives")


                text->setText(viewStr.str());
            }
            else
            {
                OSG_WARN<<std::endl<<"No valid view to collect scene stats from"<<std::endl;

                text->setText("");
            }
        }
        text->drawImplementation(renderInfo);
    }

    osg::observer_ptr<osgViewer::View>  _view;
    mutable osg::Timer_t                _tickLastUpdated;
    int                                 _viewNumber;
};

struct BlockDrawCallback : public virtual osg::Drawable::DrawCallback
{
    BlockDrawCallback(StatsHandler* statsHandler, float xPos, osg::Stats* viewerStats, osg::Stats* stats, const std::string& beginName, const std::string& endName, int frameDelta, int numFrames):
        _statsHandler(statsHandler),
        _xPos(xPos),
        _viewerStats(viewerStats),
        _stats(stats),
        _beginName(beginName),
        _endName(endName),
        _frameDelta(frameDelta),
        _numFrames(numFrames) {}

    /** do customized draw code.*/
    virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
    {
        osg::Geometry* geom = (osg::Geometry*)drawable;
        osg::Vec3Array* vertices = (osg::Vec3Array*)geom->getVertexArray();

        int frameNumber = renderInfo.getState()->getFrameStamp()->getFrameNumber();
        int startFrame = frameNumber + _frameDelta - _numFrames + 1;
        int endFrame = frameNumber + _frameDelta;
        double referenceTime;
        if (!_viewerStats->getAttribute( startFrame, "Reference time", referenceTime))
        {
            return;
        }

        unsigned int vi = 0;
        double beginValue, endValue;
        for(int i = startFrame; i <= endFrame; ++i)
        {
            if (_stats->getAttribute( i, _beginName, beginValue) &&
                _stats->getAttribute( i, _endName, endValue) )
            {
                (*vertices)[vi++].x() = _xPos + (beginValue - referenceTime) * _statsHandler->getBlockMultiplier();
                (*vertices)[vi++].x() = _xPos + (beginValue - referenceTime) * _statsHandler->getBlockMultiplier();
                (*vertices)[vi++].x() = _xPos + (endValue - referenceTime) * _statsHandler->getBlockMultiplier();
                (*vertices)[vi++].x() = _xPos + (endValue - referenceTime) * _statsHandler->getBlockMultiplier();
            }
        }

        osg::DrawArrays* drawArrays = static_cast<osg::DrawArrays*>(geom->getPrimitiveSet(0));
        drawArrays->setCount(vi);

        drawable->drawImplementation(renderInfo);
    }

    StatsHandler*               _statsHandler;
    float                       _xPos;
    osg::ref_ptr<osg::Stats>    _viewerStats;
    osg::ref_ptr<osg::Stats>    _stats;
    std::string                 _beginName;
    std::string                 _endName;
    int                         _frameDelta;
    int                         _numFrames;
};

osg::Geometry* StatsHandler::createBackgroundRectangle(const osg::Vec3& pos, const float width, const float height, osg::Vec4& color)
{
    osg::StateSet *ss = new osg::StateSet;

    osg::Geometry* geometry = new osg::Geometry;

    geometry->setUseDisplayList(false);
    geometry->setStateSet(ss);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices);

    vertices->push_back(osg::Vec3(pos.x(), pos.y(), 0));
    vertices->push_back(osg::Vec3(pos.x(), pos.y()-height,0));
    vertices->push_back(osg::Vec3(pos.x()+width, pos.y()-height,0));
    vertices->push_back(osg::Vec3(pos.x()+width, pos.y(),0));

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(color);
    geometry->setColorArray(colors, osg::Array::BIND_OVERALL);

    osg::DrawElementsUShort *base =  new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_FAN,0);
    base->push_back(0);
    base->push_back(1);
    base->push_back(2);
    base->push_back(3);

    geometry->addPrimitiveSet(base);

    return geometry;
}


struct StatsGraph : public osg::MatrixTransform
{
    StatsGraph(osg::Vec3 pos, float width, float height)
        : _pos(pos), _width(width), _height(height),
          _statsGraphGeode(new osg::Geode)
    {
        _pos -= osg::Vec3(0, height, 0.1);
        setMatrix(osg::Matrix::translate(_pos));
        addChild(_statsGraphGeode.get());
    }

    void addStatGraph(osg::Stats* viewerStats, osg::Stats* stats, const osg::Vec4& color, float max, const std::string& nameBegin, const std::string& nameEnd = "")
    {
        _statsGraphGeode->addDrawable(new Graph(_pos, _width, _height, viewerStats, stats, color, max, nameBegin, nameEnd));
    }

    osg::Vec3           _pos;
    float               _width;
    float               _height;

    osg::ref_ptr<osg::Geode> _statsGraphGeode;

protected:
    struct Graph : public osg::Geometry
    {
        Graph(const osg::Vec3& pos, float width, float height, osg::Stats* viewerStats, osg::Stats* stats,
              const osg::Vec4& color, float max, const std::string& nameBegin, const std::string& nameEnd = "")
        {
            setUseDisplayList(false);

            setVertexArray(new osg::Vec3Array);

            osg::Vec4Array* colors = new osg::Vec4Array;
            colors->push_back(color);
            setColorArray(colors, osg::Array::BIND_OVERALL);

            setDrawCallback(new GraphUpdateCallback(pos, width, height, viewerStats, stats, max, nameBegin, nameEnd));
        }
    };

    struct GraphUpdateCallback : public osg::Drawable::DrawCallback
    {
        GraphUpdateCallback(const osg::Vec3& pos, float width, float height, osg::Stats* viewerStats, osg::Stats* stats,
                            float max, const std::string& nameBegin, const std::string& nameEnd = "")
            : _pos(pos), _width((unsigned int)width), _height((unsigned int)height), _curX(0),
              _viewerStats(viewerStats), _stats(stats), _max(max), _nameBegin(nameBegin), _nameEnd(nameEnd)
        {
        }

        virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
        {
            osg::Geometry* geometry = const_cast<osg::Geometry*>(drawable->asGeometry());
            if (!geometry) return;
            osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
            if (!vertices) return;

            unsigned int frameNumber = renderInfo.getState()->getFrameStamp()->getFrameNumber();

            // Get stats
            double value;
            if (_nameEnd.empty())
            {
                if (!_stats->getAveragedAttribute( _nameBegin, value, true ))
                {
                    value = 0.0;
                }
            }
            else
            {
                double beginValue, endValue;
                if (_stats->getAttribute( frameNumber, _nameBegin, beginValue) &&
                    _stats->getAttribute( frameNumber, _nameEnd, endValue) )
                {
                    value = endValue - beginValue;
                }
                else
                {
                    value = 0.0;
                }
            }

            // Add new vertex for this frame.
            value = osg::clampTo(value, 0.0, double(_max));
            vertices->push_back(osg::Vec3(float(_curX), float(_height) / _max * value, 0));

            // One vertex per pixel in X.
            int excedent = vertices->size() - _width;
            if (excedent > 0)
            {
                vertices->erase(vertices->begin(), vertices->begin() + excedent);
            }

            // Create primitive set if none exists.
            if (geometry->getNumPrimitiveSets() == 0)
                geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, 0));

            // Update primitive set.
            osg::DrawArrays* drawArrays = dynamic_cast<osg::DrawArrays*>(geometry->getPrimitiveSet(0));
            if (!drawArrays) return;
            drawArrays->setFirst(0);
            drawArrays->setCount(vertices->size());

            // Make the graph scroll when there is enough data.
            // Note: We check the frame number so that even if we have
            // many graphs, the transform is translated only once per
            // frame.
            //static const float increment = -1.0;
            if (GraphUpdateCallback::_frameNumber != frameNumber)
            {
                // We know the exact layout of this part of the scene
                // graph, so this is OK...
                osg::MatrixTransform* transform =
                    geometry->getParent(0)->getParent(0)->asTransform()->asMatrixTransform();
                if (transform)
                {
                    //osg::Matrix matrix = transform->getMatrix();
                    //matrix.setTrans(-(*vertices)[0].x(), matrix.getTrans().y(), matrix.getTrans().z());
                    transform->setMatrix(osg::Matrix::translate(_pos + osg::Vec3(-(*vertices)[0].x(), 0, 0)));
                }
            }

            _curX++;
            GraphUpdateCallback::_frameNumber = frameNumber;

            geometry->dirtyBound();

            drawable->drawImplementation(renderInfo);
        }

        const osg::Vec3         _pos;
        const unsigned int      _width;
        const unsigned int      _height;
        mutable unsigned int    _curX;
        osg::Stats*             _viewerStats;
        osg::Stats*             _stats;
        const float             _max;
        const std::string       _nameBegin;
        const std::string       _nameEnd;
        static unsigned int     _frameNumber;
    };
};

unsigned int StatsGraph::GraphUpdateCallback::_frameNumber = 0;


osg::Geometry* StatsHandler::createGeometry(const osg::Vec3& pos, float height, const osg::Vec4& colour, unsigned int numBlocks)
{
    osg::Geometry* geometry = new osg::Geometry;

    geometry->setUseDisplayList(false);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices);
    vertices->reserve(numBlocks*4);

    for(unsigned int i=0; i<numBlocks; ++i)
    {
        vertices->push_back(pos+osg::Vec3(i*20, height, 0.0));
        vertices->push_back(pos+osg::Vec3(i*20, 0.0, 0.0));
        vertices->push_back(pos+osg::Vec3(i*20+10.0, 0.0, 0.0));
        vertices->push_back(pos+osg::Vec3(i*20+10.0, height, 0.0));
    }

    osg::Vec4Array* colours = new osg::Vec4Array;
    colours->push_back(colour);
    geometry->setColorArray(colours, osg::Array::BIND_OVERALL);

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, numBlocks*4));

    return geometry;
}


struct FrameMarkerDrawCallback : public virtual osg::Drawable::DrawCallback
{
    FrameMarkerDrawCallback(StatsHandler* statsHandler, float xPos, osg::Stats* viewerStats, int frameDelta, int numFrames):
        _statsHandler(statsHandler),
        _xPos(xPos),
        _viewerStats(viewerStats),
        _frameDelta(frameDelta),
        _numFrames(numFrames) {}

    /** do customized draw code.*/
    virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
    {
        osg::Geometry* geom = (osg::Geometry*)drawable;
        osg::Vec3Array* vertices = (osg::Vec3Array*)geom->getVertexArray();

        int frameNumber = renderInfo.getState()->getFrameStamp()->getFrameNumber();

        int startFrame = frameNumber + _frameDelta - _numFrames + 1;
        int endFrame = frameNumber + _frameDelta;
        double referenceTime;
        if (!_viewerStats->getAttribute( startFrame, "Reference time", referenceTime))
        {
            return;
        }

        unsigned int vi = 0;
        double currentReferenceTime;
        for(int i = startFrame; i <= endFrame; ++i)
        {
            if (_viewerStats->getAttribute( i, "Reference time", currentReferenceTime))
            {
                (*vertices)[vi++].x() = _xPos + (currentReferenceTime - referenceTime) * _statsHandler->getBlockMultiplier();
                (*vertices)[vi++].x() = _xPos + (currentReferenceTime - referenceTime) * _statsHandler->getBlockMultiplier();
            }
        }

        drawable->drawImplementation(renderInfo);
    }

    StatsHandler*               _statsHandler;
    float                       _xPos;
    osg::ref_ptr<osg::Stats>    _viewerStats;
    std::string                 _endName;
    int                         _frameDelta;
    int                         _numFrames;
};

struct PagerCallback : public virtual osg::NodeCallback
{

    PagerCallback(  osgDB::DatabasePager* dp,
                    osgText::Text* minValue,
                    osgText::Text* maxValue,
                    osgText::Text* averageValue,
                    osgText::Text* filerequestlist,
                    osgText::Text* compilelist,
                    double multiplier):
        _dp(dp),
        _minValue(minValue),
        _maxValue(maxValue),
        _averageValue(averageValue),
        _filerequestlist(filerequestlist),
        _compilelist(compilelist),
        _multiplier(multiplier)
    {
    }

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        if (_dp.valid())
        {
            char tmpText[128];

            double value = _dp->getAverageTimeToMergeTiles();
            if (value>= 0.0 && value <= 1000)
            {
                sprintf(tmpText,"%4.0f",value * _multiplier);
                _averageValue->setText(tmpText);
            }
            else
            {
                _averageValue->setText("");
            }

            value = _dp->getMinimumTimeToMergeTile();
            if (value>= 0.0 && value <= 1000)
            {
                sprintf(tmpText,"%4.0f",value * _multiplier);
                _minValue->setText(tmpText);
            }
            else
            {
                _minValue->setText("");
            }

            value = _dp->getMaximumTimeToMergeTile();
            if (value>= 0.0 && value <= 1000)
            {
                sprintf(tmpText,"%4.0f",value * _multiplier);
                _maxValue->setText(tmpText);
            }
            else
            {
                _maxValue->setText("");
            }

            sprintf(tmpText,"%4d", _dp->getFileRequestListSize());
            _filerequestlist->setText(tmpText);

            sprintf(tmpText,"%4d", _dp->getDataToCompileListSize());
            _compilelist->setText(tmpText);
        }

        traverse(node,nv);
    }

    osg::observer_ptr<osgDB::DatabasePager> _dp;

    osg::ref_ptr<osgText::Text> _minValue;
    osg::ref_ptr<osgText::Text> _maxValue;
    osg::ref_ptr<osgText::Text> _averageValue;
    osg::ref_ptr<osgText::Text> _filerequestlist;
    osg::ref_ptr<osgText::Text> _compilelist;
    double                      _multiplier;
};


osg::Geometry* StatsHandler::createFrameMarkers(const osg::Vec3& pos, float height, const osg::Vec4& colour, unsigned int numBlocks)
{
    osg::Geometry* geometry = new osg::Geometry;

    geometry->setUseDisplayList(false);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices);
    vertices->reserve(numBlocks*2);

    for(unsigned int i=0; i<numBlocks; ++i)
    {
        vertices->push_back(pos+osg::Vec3(double(i)*_blockMultiplier*0.01, height, 0.0));
        vertices->push_back(pos+osg::Vec3(double(i)*_blockMultiplier*0.01, 0.0, 0.0));
    }

    osg::Vec4Array* colours = new osg::Vec4Array;
    colours->push_back(colour);
    geometry->setColorArray(colours, osg::Array::BIND_OVERALL);

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, numBlocks*2));

    return geometry;
}

osg::Geometry* StatsHandler::createTick(const osg::Vec3& pos, float height, const osg::Vec4& colour, unsigned int numTicks)
{
    osg::Geometry* geometry = new osg::Geometry;

    geometry->setUseDisplayList(false);

    osg::Vec3Array* vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices);
    vertices->reserve(numTicks*2);

    for(unsigned int i=0; i<numTicks; ++i)
    {
        float tickHeight = (i%10) ? height : height * 2.0;
        vertices->push_back(pos+osg::Vec3(double(i)*_blockMultiplier*0.001, tickHeight , 0.0));
        vertices->push_back(pos+osg::Vec3(double(i)*_blockMultiplier*0.001, 0.0, 0.0));
    }

    osg::Vec4Array* colours = new osg::Vec4Array;
    colours->push_back(colour);
    geometry->setColorArray(colours, osg::Array::BIND_OVERALL);

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, numTicks*2));

    return geometry;
}

void StatsHandler::setUpScene(osgViewer::ViewerBase* viewer)
{
    _switch = new osg::Switch;

    _camera->addChild(_switch.get());

    osg::StateSet* stateset = _switch->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
#ifdef OSG_GL1_AVAILABLE
    stateset->setAttribute(new osg::PolygonMode(), osg::StateAttribute::PROTECTED);
#endif

    // collect all the relevant cameras
    ViewerBase::Cameras validCameras;
    viewer->getCameras(validCameras);

    ViewerBase::Cameras cameras;
    for(ViewerBase::Cameras::iterator itr = validCameras.begin();
        itr != validCameras.end();
        ++itr)
    {
        if ((*itr)->getStats())
        {
            cameras.push_back(*itr);
        }
    }

    // check for query time support
    unsigned int numCamrasWithTimerQuerySupport = 0;
    for(ViewerBase::Cameras::iterator citr = cameras.begin();
        citr != cameras.end();
        ++citr)
    {
        if ((*citr)->getGraphicsContext())
        {
            const osg::State* state = (*citr)->getGraphicsContext()->getState();
            unsigned int contextID = state->getContextID();
            const osg::Drawable::Extensions* extensions = osg::Drawable::getExtensions(contextID, false);
            if (extensions &&
                (((extensions->isARBTimerQuerySupported()
                  && state->getTimestampBits() > 0))
                 || extensions->isTimerQuerySupported()))
            {
                ++numCamrasWithTimerQuerySupport;
            }
        }
    }

    bool acquireGPUStats = numCamrasWithTimerQuerySupport==cameras.size();

    osg::Vec3 pos(_leftPos, _statsHeight-24.0f,0.0f);

    osg::Vec4 colorFR(1.0f,1.0f,1.0f,1.0f);
    osg::Vec4 colorFRAlpha(1.0f,1.0f,1.0f,0.5f);
    osg::Vec4 colorUpdate( 0.0f,1.0f,0.0f,1.0f);
    osg::Vec4 colorUpdateAlpha( 0.0f,1.0f,0.0f,0.5f);
    osg::Vec4 colorEvent(0.0f, 1.0f, 0.5f, 1.0f);
    osg::Vec4 colorEventAlpha(0.0f, 1.0f, 0.5f, 0.5f);
    osg::Vec4 colorCull( 0.0f,1.0f,1.0f,1.0f);
    osg::Vec4 colorCullAlpha( 0.0f,1.0f,1.0f,0.5f);
    osg::Vec4 colorDraw( 1.0f,1.0f,0.0f,1.0f);
    osg::Vec4 colorDrawAlpha( 1.0f,1.0f,0.0f,0.5f);
    osg::Vec4 colorGPU( 1.0f,0.5f,0.0f,1.0f);
    osg::Vec4 colorGPUAlpha( 1.0f,0.5f,0.0f,0.5f);

    osg::Vec4 colorDP( 1.0f,1.0f,0.5f,1.0f);


    // frame rate stats
    {
        osg::Geode* geode = new osg::Geode();
        _frameRateChildNum = _switch->getNumChildren();
        _switch->addChild(geode, false);

        osg::ref_ptr<osgText::Text> frameRateLabel = new osgText::Text;
        geode->addDrawable( frameRateLabel.get() );

        frameRateLabel->setColor(colorFR);
        frameRateLabel->setFont(_font);
        frameRateLabel->setCharacterSize(_characterSize);
        frameRateLabel->setPosition(pos);
        frameRateLabel->setText("Frame Rate: ");

        pos.x() = frameRateLabel->getBound().xMax();

        osg::ref_ptr<osgText::Text> frameRateValue = new osgText::Text;
        geode->addDrawable( frameRateValue.get() );

        frameRateValue->setColor(colorFR);
        frameRateValue->setFont(_font);
        frameRateValue->setCharacterSize(_characterSize);
        frameRateValue->setPosition(pos);
        frameRateValue->setText("0.0");

        frameRateValue->setDrawCallback(new AveragedValueTextDrawCallback(viewer->getViewerStats(),"Frame rate",-1, true, 1.0));

        pos.y() -= _characterSize*_lineHeight;

    }

    osg::Vec4 backgroundColor(0.0, 0.0, 0.0f, 0.3);
    osg::Vec4 staticTextColor(1.0, 1.0, 0.0f, 1.0);
    osg::Vec4 dynamicTextColor(1.0, 1.0, 1.0f, 1.0);
    float backgroundMargin = 5;
    float backgroundSpacing = 3;


    // viewer stats
    {
        osg::Group* group = new osg::Group;
        _viewerChildNum = _switch->getNumChildren();
        _switch->addChild(group, false);

        _statsGeode = new osg::Geode();
        group->addChild(_statsGeode.get());


        {
            pos.x() = _leftPos;

            _threadingModelText = new osgText::Text;
            _statsGeode->addDrawable( _threadingModelText.get() );

            _threadingModelText->setColor(colorFR);
            _threadingModelText->setFont(_font);
            _threadingModelText->setCharacterSize(_characterSize);
            _threadingModelText->setPosition(pos);

            updateThreadingModelText();

            pos.y() -= _characterSize*_lineHeight;
        }

        float topOfViewerStats = pos.y() + _characterSize;

        double cameraSize = _lineHeight * 3.0 * cameras.size();
        if(!acquireGPUStats) //reduce size if GPU stats not needed
        {
            cameraSize -= _lineHeight * cameras.size();
        }

        double userStatsLinesSize = _lineHeight * _userStatsLines.size();

        _statsGeode->addDrawable(createBackgroundRectangle(
            pos + osg::Vec3(-backgroundMargin, _characterSize + backgroundMargin, 0),
            _statsWidth - 2 * backgroundMargin,
            (3 + cameraSize + userStatsLinesSize) * _characterSize + 2 * backgroundMargin,
            backgroundColor) );

        // Add user stats lines before the normal viewer and per-camera stats.
        for (unsigned int i = 0; i < _userStatsLines.size(); ++i)
        {
            pos.x() = _leftPos;

            UserStatsLine& line = _userStatsLines[i];
            createTimeStatsLine(line.label, pos, line.textColor, line.barColor, viewer->getViewerStats(), viewer->getViewerStats(),
                line.timeTakenName, line.multiplier, line.average, line.averageInInverseSpace, line.beginTimeName, line.endTimeName);

            pos.y() -= _characterSize*_lineHeight;
        }

        {
            pos.x() = _leftPos;

            createTimeStatsLine("Event", pos, colorUpdate, colorUpdateAlpha, viewer->getViewerStats(), viewer->getViewerStats(),
                "Event traversal time taken", 1000.0, true, false, "Event traversal begin time", "Event traversal end time");

            pos.y() -= _characterSize*_lineHeight;
        }

        {
            pos.x() = _leftPos;

            createTimeStatsLine("Update", pos, colorUpdate, colorUpdateAlpha, viewer->getViewerStats(), viewer->getViewerStats(),
                "Update traversal time taken", 1000.0, true, false, "Update traversal begin time", "Update traversal end time");

            pos.y() -= _characterSize*_lineHeight;
        }

        pos.x() = _leftPos;

        // add camera stats
        for(ViewerBase::Cameras::iterator citr = cameras.begin();
            citr != cameras.end();
            ++citr)
        {
            createCameraTimeStats(pos, acquireGPUStats, viewer->getViewerStats(), *citr);
        }

        // add frame ticks
        {
            osg::Geode* geode = new osg::Geode;
            group->addChild(geode);

            osg::Vec4 colourTicks(1.0f,1.0f,1.0f, 0.5f);

            pos.x() = _startBlocks;
            pos.y() += _characterSize;
            float height = topOfViewerStats - pos.y();

            osg::Geometry* ticks = createTick(pos, 5.0f, colourTicks, 100);
            geode->addDrawable(ticks);

            osg::Geometry* frameMarkers = createFrameMarkers(pos, height, colourTicks, _numBlocks + 1);
            frameMarkers->setDrawCallback(new FrameMarkerDrawCallback(this, _startBlocks, viewer->getViewerStats(), 0, _numBlocks + 1));
            geode->addDrawable(frameMarkers);

            pos.x() = _leftPos;
        }

        // Stats line graph
        {
            pos.y() -= (backgroundSpacing + 2 * backgroundMargin);
            float width = _statsWidth - 4 * backgroundMargin;
            float height = 5 * _characterSize;

            // Create a stats graph and add any stats we want to track with it.
            StatsGraph* statsGraph = new StatsGraph(pos, width, height);
            group->addChild(statsGraph);

            statsGraph->addStatGraph(viewer->getViewerStats(), viewer->getViewerStats(), colorFR, 100, "Frame rate");
            statsGraph->addStatGraph(viewer->getViewerStats(), viewer->getViewerStats(), colorEvent, 0.016, "Event traversal time taken");
            statsGraph->addStatGraph(viewer->getViewerStats(), viewer->getViewerStats(), colorUpdate, 0.016, "Update traversal time taken");

            for (unsigned int i = 0; i < _userStatsLines.size(); ++i)
            {
                UserStatsLine& line = _userStatsLines[i];
                if (!line.timeTakenName.empty() && line.average)
                {
                    statsGraph->addStatGraph(viewer->getViewerStats(), viewer->getViewerStats(), line.textColor, line.maxValue, line.timeTakenName);
                }
            }

            for(ViewerBase::Cameras::iterator citr = cameras.begin();
                citr != cameras.end();
                ++citr)
            {
                statsGraph->addStatGraph(viewer->getViewerStats(), (*citr)->getStats(), colorCull, 0.016, "Cull traversal time taken");
                statsGraph->addStatGraph(viewer->getViewerStats(), (*citr)->getStats(), colorDraw, 0.016, "Draw traversal time taken");
                if(acquireGPUStats)
                {
                    statsGraph->addStatGraph(viewer->getViewerStats(), (*citr)->getStats(), colorGPU, 0.016, "GPU draw time taken");
                }
            }

            _statsGeode->addDrawable(createBackgroundRectangle( pos + osg::Vec3(-backgroundMargin, backgroundMargin, 0),
                                                                width + 2 * backgroundMargin,
                                                                height + 2 * backgroundMargin,
                                                                backgroundColor) );

            pos.x() = _leftPos;
            pos.y() -= height + 2 * backgroundMargin;
        }

        // Databasepager stats
        ViewerBase::Scenes scenes;
        viewer->getScenes(scenes);
        for(ViewerBase::Scenes::iterator itr = scenes.begin();
            itr != scenes.end();
            ++itr)
        {
            Scene* scene = *itr;
            osgDB::DatabasePager* dp = scene->getDatabasePager();
            if (dp && dp->isRunning())
            {
                pos.y() -= (_characterSize + backgroundSpacing);

                _statsGeode->addDrawable(createBackgroundRectangle(    pos + osg::Vec3(-backgroundMargin, _characterSize + backgroundMargin, 0),
                                                                       _statsWidth - 2 * backgroundMargin,
                                                                       _characterSize + 2 * backgroundMargin,
                                                                       backgroundColor));

                osg::ref_ptr<osgText::Text> averageLabel = new osgText::Text;
                _statsGeode->addDrawable( averageLabel.get() );

                averageLabel->setColor(colorDP);
                averageLabel->setFont(_font);
                averageLabel->setCharacterSize(_characterSize);
                averageLabel->setPosition(pos);
                averageLabel->setText("DatabasePager time to merge new tiles - average: ");

                pos.x() = averageLabel->getBound().xMax();

                osg::ref_ptr<osgText::Text> averageValue = new osgText::Text;
                _statsGeode->addDrawable( averageValue.get() );

                averageValue->setColor(colorDP);
                averageValue->setFont(_font);
                averageValue->setCharacterSize(_characterSize);
                averageValue->setPosition(pos);
                averageValue->setText("1000");

                pos.x() = averageValue->getBound().xMax() + 2.0f*_characterSize;


                osg::ref_ptr<osgText::Text> minLabel = new osgText::Text;
                _statsGeode->addDrawable( minLabel.get() );

                minLabel->setColor(colorDP);
                minLabel->setFont(_font);
                minLabel->setCharacterSize(_characterSize);
                minLabel->setPosition(pos);
                minLabel->setText("min: ");

                pos.x() = minLabel->getBound().xMax();

                osg::ref_ptr<osgText::Text> minValue = new osgText::Text;
                _statsGeode->addDrawable( minValue.get() );

                minValue->setColor(colorDP);
                minValue->setFont(_font);
                minValue->setCharacterSize(_characterSize);
                minValue->setPosition(pos);
                minValue->setText("1000");

                pos.x() = minValue->getBound().xMax() + 2.0f*_characterSize;

                osg::ref_ptr<osgText::Text> maxLabel = new osgText::Text;
                _statsGeode->addDrawable( maxLabel.get() );

                maxLabel->setColor(colorDP);
                maxLabel->setFont(_font);
                maxLabel->setCharacterSize(_characterSize);
                maxLabel->setPosition(pos);
                maxLabel->setText("max: ");

                pos.x() = maxLabel->getBound().xMax();

                osg::ref_ptr<osgText::Text> maxValue = new osgText::Text;
                _statsGeode->addDrawable( maxValue.get() );

                maxValue->setColor(colorDP);
                maxValue->setFont(_font);
                maxValue->setCharacterSize(_characterSize);
                maxValue->setPosition(pos);
                maxValue->setText("1000");

                pos.x() = maxValue->getBound().xMax();

                osg::ref_ptr<osgText::Text> requestsLabel = new osgText::Text;
                _statsGeode->addDrawable( requestsLabel.get() );

                requestsLabel->setColor(colorDP);
                requestsLabel->setFont(_font);
                requestsLabel->setCharacterSize(_characterSize);
                requestsLabel->setPosition(pos);
                requestsLabel->setText("requests: ");

                pos.x() = requestsLabel->getBound().xMax();

                osg::ref_ptr<osgText::Text> requestList = new osgText::Text;
                _statsGeode->addDrawable( requestList.get() );

                requestList->setColor(colorDP);
                requestList->setFont(_font);
                requestList->setCharacterSize(_characterSize);
                requestList->setPosition(pos);
                requestList->setText("0");

                pos.x() = requestList->getBound().xMax() + 2.0f*_characterSize;;

                osg::ref_ptr<osgText::Text> compileLabel = new osgText::Text;
                _statsGeode->addDrawable( compileLabel.get() );

                compileLabel->setColor(colorDP);
                compileLabel->setFont(_font);
                compileLabel->setCharacterSize(_characterSize);
                compileLabel->setPosition(pos);
                compileLabel->setText("tocompile: ");

                pos.x() = compileLabel->getBound().xMax();

                osg::ref_ptr<osgText::Text> compileList = new osgText::Text;
                _statsGeode->addDrawable( compileList.get() );

                compileList->setColor(colorDP);
                compileList->setFont(_font);
                compileList->setCharacterSize(_characterSize);
                compileList->setPosition(pos);
                compileList->setText("0");

                pos.x() = maxLabel->getBound().xMax();

                _statsGeode->setCullCallback(new PagerCallback(dp, minValue.get(), maxValue.get(), averageValue.get(), requestList.get(), compileList.get(), 1000.0));
            }

            pos.x() = _leftPos;
        }
    }

    // Camera scene stats
    {
        pos.y() -= (_characterSize + backgroundSpacing + 2 * backgroundMargin);

        osg::Group* group = new osg::Group;
        _cameraSceneChildNum = _switch->getNumChildren();
        _switch->addChild(group, false);

        osg::Geode* geode = new osg::Geode();
        geode->setCullingActive(false);
        group->addChild(geode);
        geode->addDrawable(createBackgroundRectangle(pos + osg::Vec3(-backgroundMargin, _characterSize + backgroundMargin, 0),
                                                        10 * _characterSize + 2 * backgroundMargin,
                                                        22 * _characterSize + 2 * backgroundMargin,
                                                        backgroundColor));

        // Camera scene & primitive stats static text
        osg::ref_ptr<osgText::Text> camStaticText = new osgText::Text;
        geode->addDrawable( camStaticText.get() );
        camStaticText->setColor(staticTextColor);
        camStaticText->setFont(_font);
        camStaticText->setCharacterSize(_characterSize);
        camStaticText->setPosition(pos);

        std::ostringstream viewStr;
        viewStr.clear();
        viewStr.setf(std::ios::left, std::ios::adjustfield);
        viewStr.width(14);
        viewStr << "Camera" << std::endl;
        viewStr << "" << std::endl; // placeholder for Camera name
        viewStr << "Lights" << std::endl;
        viewStr << "Bins" << std::endl;
        viewStr << "Depth" << std::endl;
        viewStr << "State graphs" << std::endl;
        viewStr << "Imposters" << std::endl;
        viewStr << "Drawables" << std::endl;
        viewStr << "Sorted Drawables" << std::endl;
        viewStr << "Fast Drawables" << std::endl;
        viewStr << "Vertices" << std::endl;
        viewStr << "PrimitiveSets" << std::endl;
        viewStr << "Points" << std::endl;
        viewStr << "Lines" << std::endl;
        viewStr << "Line strips" << std::endl;
        viewStr << "Line loops" << std::endl;
        viewStr << "Triangles" << std::endl;
        viewStr << "Tri. strips" << std::endl;
        viewStr << "Tri. fans" << std::endl;
        viewStr << "Quads" << std::endl;
        viewStr << "Quad strips" << std::endl;
        viewStr << "Polygons" << std::endl;
        viewStr.setf(std::ios::right,std::ios::adjustfield);
        camStaticText->setText(viewStr.str());

        // Move camera block to the right
        pos.x() += 10 * _characterSize + 2 * backgroundMargin + backgroundSpacing;

        // Add camera scene stats, one block per camera
        int cameraCounter = 0;
        for(ViewerBase::Cameras::iterator citr = cameras.begin(); citr != cameras.end(); ++citr)
        {
            geode->addDrawable(createBackgroundRectangle(pos + osg::Vec3(-backgroundMargin, _characterSize + backgroundMargin, 0),
                                                            5 * _characterSize + 2 * backgroundMargin,
                                                            22 * _characterSize + 2 * backgroundMargin,
                                                            backgroundColor));

            // Camera scene stats
            osg::ref_ptr<osgText::Text> camStatsText = new osgText::Text;
            geode->addDrawable( camStatsText.get() );

            camStatsText->setColor(dynamicTextColor);
            camStatsText->setFont(_font);
            camStatsText->setCharacterSize(_characterSize);
            camStatsText->setPosition(pos);
            camStatsText->setText("");
            camStatsText->setDrawCallback(new CameraSceneStatsTextDrawCallback(*citr, cameraCounter));

            // Move camera block to the right
            pos.x() +=  5 * _characterSize + 2 * backgroundMargin + backgroundSpacing;
            cameraCounter++;
        }
    }

    // Viewer scene stats
    {
        osg::Group* group = new osg::Group;
        _viewerSceneChildNum = _switch->getNumChildren();
        _switch->addChild(group, false);

        osg::Geode* geode = new osg::Geode();
        geode->setCullingActive(false);
        group->addChild(geode);

        geode->addDrawable(createBackgroundRectangle(pos + osg::Vec3(-backgroundMargin, _characterSize + backgroundMargin, 0),
                                                        6 * _characterSize + 2 * backgroundMargin,
                                                        12 * _characterSize + 2 * backgroundMargin,
                                                        backgroundColor));

        // View scene stats static text
        osg::ref_ptr<osgText::Text> camStaticText = new osgText::Text;
        geode->addDrawable( camStaticText.get() );
        camStaticText->setColor(staticTextColor);
        camStaticText->setFont(_font);
        camStaticText->setCharacterSize(_characterSize);
        camStaticText->setPosition(pos);

        std::ostringstream viewStr;
        viewStr.clear();
        viewStr.setf(std::ios::left, std::ios::adjustfield);
        viewStr.width(14);
        viewStr << "View" << std::endl;
        viewStr << " " << std::endl;
        viewStr << "Stateset" << std::endl;
        viewStr << "Group" << std::endl;
        viewStr << "Transform" << std::endl;
        viewStr << "LOD" << std::endl;
        viewStr << "Switch" << std::endl;
        viewStr << "Geode" << std::endl;
        viewStr << "Drawable" << std::endl;
        viewStr << "Geometry" << std::endl;
        viewStr << "Vertices" << std::endl;
        viewStr << "Primitives" << std::endl;
        viewStr.setf(std::ios::right, std::ios::adjustfield);
        camStaticText->setText(viewStr.str());

        // Move viewer block to the right
        pos.x() += 6 * _characterSize + 2 * backgroundMargin + backgroundSpacing;

        std::vector<osgViewer::View*> views;
        viewer->getViews(views);

        std::vector<osgViewer::View*>::iterator it;
        int viewCounter = 0;
        for (it = views.begin(); it != views.end(); ++it)
        {
            geode->addDrawable(createBackgroundRectangle(pos + osg::Vec3(-backgroundMargin, _characterSize + backgroundMargin, 0),
                                                            10 * _characterSize + 2 * backgroundMargin,
                                                            12 * _characterSize + 2 * backgroundMargin,
                                                            backgroundColor));

            // Text for scene statistics
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setColor(dynamicTextColor);
            text->setFont(_font);
            text->setCharacterSize(_characterSize);
            text->setPosition(pos);
            text->setDrawCallback(new ViewSceneStatsTextDrawCallback(*it, viewCounter));

            pos.x() += 10 * _characterSize + 2 * backgroundMargin + backgroundSpacing;
            viewCounter++;
        }
    }
}

void StatsHandler::createTimeStatsLine(const std::string& lineLabel,
                                       osg::Vec3 pos, const osg::Vec4& textColor, const osg::Vec4& barColor,
                                       osg::Stats* viewerStats, osg::Stats* stats,
                                       const std::string& timeTakenName, float multiplier, bool average, bool averageInInverseSpace,
                                       const std::string& beginTimeName, const std::string& endTimeName)
{
    osg::ref_ptr<osgText::Text> label = new osgText::Text;
    _statsGeode->addDrawable( label.get() );

    label->setColor(textColor);
    label->setFont(_font);
    label->setCharacterSize(_characterSize);
    label->setPosition(pos);
    label->setText(lineLabel + ": ");

    pos.x() = label->getBound().xMax();

    osg::ref_ptr<osgText::Text> value = new osgText::Text;
    _statsGeode->addDrawable( value.get() );

    value->setColor(textColor);
    value->setFont(_font);
    value->setCharacterSize(_characterSize);
    value->setPosition(pos);
    value->setText("0.0");

    if (!timeTakenName.empty())
    {
        if (average)
        {
            value->setDrawCallback(new AveragedValueTextDrawCallback(stats, timeTakenName, -1, averageInInverseSpace, multiplier));
        }
        else
        {
            value->setDrawCallback(new RawValueTextDrawCallback(stats, timeTakenName, -1, multiplier));
        }
    }

    if (!beginTimeName.empty() && !endTimeName.empty())
    {
        pos.x() = _startBlocks;
        osg::Geometry* geometry = createGeometry(pos, _characterSize *0.8, barColor, _numBlocks);
        geometry->setDrawCallback(new BlockDrawCallback(this, _startBlocks, viewerStats, stats, beginTimeName, endTimeName, -1, _numBlocks));
        _statsGeode->addDrawable(geometry);
    }
}

void StatsHandler::createCameraTimeStats(osg::Vec3& pos, bool acquireGPUStats, osg::Stats* viewerStats, osg::Camera* camera)
{
    osg::Stats* stats = camera->getStats();
    if (!stats) return;

    osg::Vec4 colorCull( 0.0f,1.0f,1.0f,1.0f);
    osg::Vec4 colorCullAlpha( 0.0f,1.0f,1.0f,0.5f);
    osg::Vec4 colorDraw( 1.0f,1.0f,0.0f,1.0f);
    osg::Vec4 colorDrawAlpha( 1.0f,1.0f,0.0f,0.5f);
    osg::Vec4 colorGPU( 1.0f,0.5f,0.0f,1.0f);
    osg::Vec4 colorGPUAlpha( 1.0f,0.5f,0.0f,0.5f);

    {
        pos.x() = _leftPos;

        createTimeStatsLine("Cull", pos, colorCull, colorCullAlpha, viewerStats, stats,
            "Cull traversal time taken", 1000.0, true, false, "Cull traversal begin time", "Cull traversal end time");

        pos.y() -= _characterSize*_lineHeight;
    }

    {
        pos.x() = _leftPos;

        createTimeStatsLine("Draw", pos, colorDraw, colorDrawAlpha, viewerStats, stats,
            "Draw traversal time taken", 1000.0, true, false, "Draw traversal begin time", "Draw traversal end time");

        pos.y() -= _characterSize*_lineHeight;
    }

    if (acquireGPUStats)
    {
        pos.x() = _leftPos;

        createTimeStatsLine("GPU", pos, colorGPU, colorGPUAlpha, viewerStats, stats,
            "GPU draw time taken", 1000.0, true, false, "GPU draw begin time", "GPU draw end time");

        pos.y() -= _characterSize*_lineHeight;
    }
}


void StatsHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding(_keyEventTogglesOnScreenStats,"On screen stats.");
    usage.addKeyboardMouseBinding(_keyEventPrintsOutStats,"Output stats to console.");
}

}
