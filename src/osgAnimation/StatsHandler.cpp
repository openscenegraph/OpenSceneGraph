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
#include <math.h>
#include <stdlib.h>

#include <osg/io_utils>
#include <osg/NodeVisitor>

#include <osg/MatrixTransform>
#include <osgViewer/Renderer>
#include <osgAnimation/StatsHandler>
#include <osgAnimation/EaseMotion>
#include <osgAnimation/StatsVisitor>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Renderer>
#include <osgAnimation/TimelineAnimationManager>

#include <osg/PolygonMode>
#include <osg/Geometry>
#include <iostream>
#include <cstdlib>

static unsigned int getRandomValueinRange(unsigned int v)
{
    return static_cast<unsigned int>((rand() * 1.0 * v)/(RAND_MAX-1));
}


namespace osgAnimation
{


osg::Geometry* createBackgroundRectangle(const osg::Vec3& pos, const float width, const float height, osg::Vec4& color)
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

    osg::DrawElementsUInt *base =  new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS,0);
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
        _pos = pos - osg::Vec3(0, _height, 0.1);
        setMatrix(osg::Matrix::translate(_pos));
        setDataVariance(osg::Object::DYNAMIC);
        addChild(_statsGraphGeode.get());
        _statsGraphGeode->setCullingActive(false);
    }

    void changeYposition(float y)
    {
        _pos = getMatrix().getTrans();
        _pos[1] = y - _height;
        setMatrix(osg::Matrix::translate(_pos));
    }

    void addStatGraph(osg::Stats* viewerStats, osg::Stats* stats, const osg::Vec4& color, float max, const std::string& nameBegin, const std::string& nameEnd = "")
    {
        _statsGraphGeode->addDrawable(new Graph(_width, _height, viewerStats, stats, color, max, nameBegin, nameEnd));
    }

    osg::Vec3           _pos;
    float               _width;
    float               _height;

    osg::ref_ptr<osg::Geode> _statsGraphGeode;

    struct NeverCull : public osg::DrawableCullCallback
    {
        NeverCull() {}
        bool cull(osg::NodeVisitor* /*nv*/, osg::Drawable* /*drawable*/, osg::RenderInfo* /*renderInfo*/) const { return false;}
    };


    struct Graph : public osg::Geometry
    {
        Graph(float width, float height, osg::Stats* viewerStats, osg::Stats* stats,
              const osg::Vec4& color, float max, const std::string& nameBegin, const std::string& nameEnd = "")
        {
            setDataVariance(osg::Object::DYNAMIC);
            setUseDisplayList(false);

            setVertexArray(new osg::Vec3Array);
            getVertexArray()->setDataVariance(osg::Object::DYNAMIC);
            setColor(color);

            setUpdateCallback(new GraphUpdateCallback(width, height, viewerStats, stats, max, nameBegin, nameEnd));
            setCullCallback(new NeverCull);
        }

        void setColor(const osg::Vec4& color) {
            osg::Vec4Array* colors = new osg::Vec4Array;
            colors->push_back(color);
            setColorArray(colors, osg::Array::BIND_OVERALL);
        }
    };


    struct GraphUpdateCallback : public osg::DrawableUpdateCallback
    {

        const unsigned int      _width;
        const unsigned int      _height;
        mutable unsigned int    _curX;
        osg::Stats*             _viewerStats;
        osg::Stats*             _stats;
        const float             _max;
        const std::string       _nameBegin;
        const std::string       _nameEnd;
        mutable unsigned int    _frameNumber;

        GraphUpdateCallback(float width, float height, osg::Stats* viewerStats, osg::Stats* stats,
                            float max, const std::string& nameBegin, const std::string& nameEnd = "")
            : _width((unsigned int)width), _height((unsigned int)height), _curX(0),
              _viewerStats(viewerStats), _stats(stats), _max(max), _nameBegin(nameBegin), _nameEnd(nameEnd), _frameNumber(0)
        {
        }
        virtual void update(osg::NodeVisitor* nv, osg::Drawable* drawable)
        {
            if (nv->getVisitorType() != osg::NodeVisitor::UPDATE_VISITOR)
                return;

            osg::Geometry* geometry = const_cast<osg::Geometry*>(drawable->asGeometry());
            if (!geometry) return;
            osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
            if (!vertices) return;

            unsigned int frameNumber = nv->getFrameStamp()->getFrameNumber();
            if (frameNumber == _frameNumber)
                return;


            // Get stats
            double value;
            if (_nameEnd.empty())
            {
                if (!_stats->getAttribute(_stats->getLatestFrameNumber(), _nameBegin, value ))
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

            if (!vertices->size()) {
                for (int i = 0; i < (int)_width; i++)
                    vertices->push_back(osg::Vec3(float(_curX++), 0, 0));
                // Create primitive set if none exists.
                if (geometry->getNumPrimitiveSets() == 0)
                    geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, 0));
                osg::DrawArrays* drawArrays = static_cast<osg::DrawArrays*>(geometry->getPrimitiveSet(0));
                drawArrays->setFirst(0);
                drawArrays->setCount(vertices->size());
            }
            vertices->push_back(osg::Vec3(float(_curX), float(_height) / _max * value, 0));

            unsigned int excedent = vertices->size() - _width;
            vertices->erase(vertices->begin(), vertices->begin() + excedent);

            // Make the graph scroll when there is enough data.
            // Note: We check the frame number so that even if we have
            // many graphs, the transform is translated only once per
            // frame.
            static const float increment = -1.0;
            if (_frameNumber != frameNumber)
            {
                // We know the exact layout of this part of the scene
                // graph, so this is OK...
                osg::MatrixTransform* transform =
                    geometry->getParent(0)->getParent(0)->asTransform()->asMatrixTransform();
                if (transform)
                {
                    transform->setMatrix(transform->getMatrix() * osg::Matrix::translate(osg::Vec3(increment, 0, 0)));
                }
            }

            _curX++;
            _frameNumber = frameNumber;

            geometry->dirtyBound();
        }
    };
};

// Drawcallback to draw averaged attribute
struct ValueTextDrawCallback : public virtual osg::Drawable::DrawCallback
{
    ValueTextDrawCallback(osg::Stats* stats, const std::string& name):
        _stats(stats),
        _attributeName(name),
        _frameNumber(0)
    {
    }

    /** do customized draw code.*/
    virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
    {
        osgText::Text* text = (osgText::Text*)drawable;

        unsigned int frameNumber = renderInfo.getState()->getFrameStamp()->getFrameNumber();
        if (frameNumber == _frameNumber) {
            text->drawImplementation(renderInfo);
            return;
        }

        double value;
        if (_stats->getAttribute(_stats->getLatestFrameNumber(), _attributeName, value))
        {
            sprintf(_tmpText,"%4.2f",value);
            text->setText(_tmpText);
        }
        else
        {
            text->setText("");
        }
        _frameNumber = frameNumber;
        text->drawImplementation(renderInfo);
    }

    osg::ref_ptr<osg::Stats>    _stats;
    std::string                 _attributeName;
    mutable char                _tmpText[128];
    mutable unsigned int        _frameNumber;
};





    struct StatAction
    {
        double _lastTime;
        std::string _name;
        osg::ref_ptr<osg::Group> _group;
        osg::ref_ptr<osg::Geode> _label;
        osg::ref_ptr<StatsGraph> _graph;
        osg::ref_ptr<osgText::Text> _textLabel;
        osgAnimation::OutCubicMotion _fade;

        StatAction() { _lastTime = 0; _fade = osgAnimation::OutCubicMotion(0,5); }
        void init(osg::Stats* stats, const std::string& name, const osg::Vec3& pos, float width, float heigh, const osg::Vec4& color);
        void setPosition(const osg::Vec3& pos);
#if 0
        void touch()
        {
            _lastTime = osg::Timer::instance()->time_s();
            float a = 1.0 - _fade.getValueAt(0.0);
            setAlpha(a);
        }
        bool update() {
            double t = osg::Timer::instance()->time_s();
            float alpha = 1.0 - _fade.getValueAt(t-_lastTime);
            if (t - _lastTime > _fade.getDuration())
                return true;
            setAlpha(alpha);
            return false;
        }
#endif
        void setAlpha(float v);
    };


    struct StatsTimeline : public osg::NodeCallback
    {
        static float _statsHeight;
        static float _statsWidth;

        osg::ref_ptr<osg::Geometry> _background;
        osg::ref_ptr<osgAnimation::Timeline> _timeline;
        osg::ref_ptr<osg::MatrixTransform> _group;
        std::map<std::string, StatAction > _actions;

        StatsTimeline()
        {
            _statsHeight = 1024;
            _statsWidth = 1280;
        }
        osg::MatrixTransform* createStatsForTimeline(osgAnimation::Timeline* timeline)
        {
            _timeline = timeline;

            std::string font("fonts/arial.ttf");

            float leftPos = 10.0f;
            float startBlocks = 150.0f;
            float characterSize = 20.0f;


            osg::Vec4 backgroundColor(0.0, 0.0, 0.0f, 0.3);
            float backgroundMargin = 5;
            //float backgroundSpacing = 3;

            osg::Vec4 color(1.0, 1.0, 1.0, 1.0);

            _group = new osg::MatrixTransform;
            _group->setDataVariance(osg::Object::DYNAMIC);

            {
                osg::Vec3 pos(leftPos, _statsHeight-24.0f,0.0f);
                //float topOfViewerStats = pos.y() + characterSize;
                osg::ref_ptr<osg::Stats> stats = _timeline->getStats();
                pos.y() -= characterSize + backgroundMargin;

                {
                    osg::Geode* geode = new osg::Geode();
                    _group->addChild(geode);
                    osg::ref_ptr<osgText::Text> timeLabel = new osgText::Text;
                    geode->addDrawable( timeLabel.get() );

                    timeLabel->setColor(color);
                    timeLabel->setFont(font);
                    timeLabel->setCharacterSize(characterSize);
                    timeLabel->setPosition(pos);
                    timeLabel->setText("Time: ");

                    osg::ref_ptr<osgText::Text> timeLabelValue = new osgText::Text;
                    geode->addDrawable( timeLabelValue.get() );

                    timeLabelValue->setColor(color);
                    timeLabelValue->setFont(font);
                    timeLabelValue->setCharacterSize(characterSize);
                    timeLabelValue->setPosition(pos + osg::Vec3(startBlocks, 0,0));
                    timeLabelValue->setText("0.0");

                    timeLabelValue->setDrawCallback(new ValueTextDrawCallback(stats.get(),"Timeline"));
                }
            }
            {
                osg::Vec3 pos(leftPos, _statsHeight - 24.0f ,0.0f);
                //float topOfViewerStats = pos.y();
                osg::Geode* geode = new osg::Geode;
                _background = createBackgroundRectangle(
                    pos + osg::Vec3(-backgroundMargin, backgroundMargin, 0),
                    _statsWidth - 2 * backgroundMargin,
                    (3 + 4.5 * 1) * characterSize + 2 * backgroundMargin,
                    backgroundColor);
                geode->addDrawable(_background.get());
                _group->addChild(geode);
            }

            return _group.get();
        }

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR) {
                updateGraph();
            }
            traverse(node,nv);
        }

        void updateGraph()
        {
            osgAnimation::StatsActionVisitor* visitor = _timeline->getStatsVisitor();
            if (!visitor)
                return;

            float leftPos = 10.0f;
            float characterSize = 20.0f;

            float backgroundMargin = 5;
            //float backgroundSpacing = 3;
            float graphSpacing = 5;

            float width = _statsWidth - 4 * backgroundMargin;
            float height = characterSize;
            osg::Vec3 pos(leftPos, _statsHeight-24.0f,0.0f);
            pos.y() -= characterSize *2 + backgroundMargin;

             for (std::map<std::string, StatAction >::iterator it = _actions.begin(); it != _actions.end(); ++it) {
                 (*it).second._group->setNodeMask(~osg::Node::NodeMask(1));
             }

            const std::vector<std::string>& channels = visitor->getChannels();
            // std::map<std::string,int> size;
            for (int i = 0; i < (int)channels.size(); i++) {
                std::string name = channels[i];
                if (_actions.find(name) == _actions.end()) {
                    osg::Vec4 color(getRandomValueinRange(255)/255.0, getRandomValueinRange(255)/255.0, getRandomValueinRange(255)/255.0, 1.0);
                    _actions[name].init(visitor->getStats(), name, pos, width, height, color);
                    _group->addChild(_actions[name]._group.get());
                    //_actions[name].touch();
                } else {
                    _actions[name].setPosition(pos);
                    //_actions[name].touch();
                }
                _actions[name]._group->setNodeMask(~osg::Node::NodeMask(0x0));
                //size[name] = 0;
                pos.y() -= characterSize + graphSpacing;
            }

            pos.y() -= backgroundMargin;
            osg::Vec3Array* array = static_cast<osg::Vec3Array*>(_background->getVertexArray());
            // float y = (*array)[0][1];
            // y = y - (pos.y() + backgroundMargin); //(2 * backgroundMargin + (size.size() * (characterSize + graphSpacing)));
            (*array)[1][1] = pos.y();
            (*array)[2][1] = pos.y();
            array->dirty();
            _background->dirtyBound();
        }
    };
    float StatsTimeline::_statsHeight;
    float StatsTimeline::_statsWidth;



struct FindTimelineStats : public osg::NodeVisitor
{
    std::vector<osg::ref_ptr<osgAnimation::Timeline> > _timelines;

    FindTimelineStats() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    void apply(osg::Node& node) {
        osg::Callback* cb = node.getUpdateCallback();
        while (cb) {
            osgAnimation::TimelineAnimationManager* tam = dynamic_cast<osgAnimation::TimelineAnimationManager*>(cb);
            if (tam)
                _timelines.push_back(tam->getTimeline());
            cb = cb->getNestedCallback();
        }
        traverse(node);
    }
};


StatsHandler::StatsHandler():
    _keyEventTogglesOnScreenStats('a'),
    _keyEventPrintsOutStats('A'),
    _statsType(NO_STATS),
    _initialized(false),
    _frameRateChildNum(0),
    _numBlocks(0),
    _blockMultiplier(double(1.0)),
    _statsWidth(1280.0f),
    _statsHeight(1024.0f)
{
    _camera = new osg::Camera;
    _camera->setRenderer(new osgViewer::Renderer(_camera.get()));
    _camera->setProjectionResizePolicy(osg::Camera::FIXED);
}

bool StatsHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::View* myview = dynamic_cast<osgViewer::View*>(&aa);
    if (!myview) return false;

    osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(myview->getViewerBase());

    if (!viewer || !viewer->getSceneData())
        return false;
    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()==_keyEventTogglesOnScreenStats)
            {
                if (viewer->getViewerStats())
                {

                    if (!_switch.get()) {
                        FindTimelineStats finder;
                        viewer->getSceneData()->accept(finder);
                        if (finder._timelines.empty())
                            return false;
                    }

                    if (!_initialized)
                    {
                        setUpHUDCamera(viewer);
                        setUpScene(dynamic_cast<osgViewer::Viewer*>(viewer));
                    }

                    ++_statsType;

                    if (_statsType==LAST) _statsType = NO_STATS;


                    switch(_statsType)
                    {
                        case(NO_STATS):
                        {
                            _camera->setNodeMask(0x0);
                            _switch->setAllChildrenOff();
                            break;
                        }
                        case(FRAME_RATE):
                        {
                            _camera->setNodeMask(0xffffffff);
                            _switch->setAllChildrenOn();
                            break;
                        }
                        default:
                            break;
                    }


                }
                return true;
            }
            if (ea.getKey()==_keyEventPrintsOutStats)
            {
                FindTimelineStats finder;
                viewer->getSceneData()->accept(finder);
                if (!finder._timelines.empty()) {
                    OSG_NOTICE<<std::endl<<"Stats report:"<<std::endl;
                    typedef std::vector<osg::Stats*> StatsList;
                    StatsList statsList;

                    for (unsigned int i = 0; i < finder._timelines.size(); i++)
                        statsList.push_back(finder._timelines[i]->getStats());

                    for(unsigned int i = statsList[0]->getEarliestFrameNumber(); i< statsList[0]->getLatestFrameNumber(); ++i)
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
        default: break;
    }

    return false;

}

void StatsHandler::reset()
{
    _initialized = false;
    _camera->setGraphicsContext(0);
    _camera->removeChildren( 0, _camera->getNumChildren() );
}

void StatsHandler::setUpHUDCamera(osgViewer::ViewerBase* viewer)
{
    osgViewer::GraphicsWindow* window = dynamic_cast<osgViewer::GraphicsWindow*>(_camera->getGraphicsContext());

    if (!window)
    {
        osgViewer::Viewer::Windows windows;
        viewer->getWindows(windows);

        if (windows.empty()) return;

        window = windows.front();
    }

    _camera->setGraphicsContext(window);

    _camera->setViewport(0, 0, window->getTraits()->width, window->getTraits()->height);

    _camera->setRenderOrder(osg::Camera::POST_RENDER, 10);

    _camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0,_statsWidth,0.0,_statsHeight));
    _camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    _camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    _camera->setClearMask(0); //GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //-1);
    _camera->setAllowEventFocus(false);
    _camera->setCullMask(0x1);
    osgViewer::Viewer* v = dynamic_cast<osgViewer::Viewer*>(viewer);
    if(v)
        v->getSceneData()->asGroup()->addChild(_camera.get());
    _initialized = true;
}

void StatsHandler::setUpScene(osgViewer::Viewer* viewer)
{
    if (!viewer->getSceneData())
        return;

    FindTimelineStats finder;
    viewer->getSceneData()->accept(finder);
    if (finder._timelines.empty())
        return;

    _switch = new osg::Switch;
    osg::StateSet* stateset = _switch->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    stateset->setAttribute(new osg::PolygonMode(), osg::StateAttribute::PROTECTED);

    _group = new osg::Group;
    _camera->addChild(_switch.get());
    _switch->addChild(_group.get());

    for (int i = 0; i < (int)finder._timelines.size(); i++) {
        StatsTimeline* s = new StatsTimeline;
        osg::MatrixTransform* m = s->createStatsForTimeline(finder._timelines[i].get());
        m->setUpdateCallback(s);
        m->setMatrix(osg::Matrix::translate(0, -i * 100, 0));
        _group->addChild(m);
    }
}




void StatAction::init(osg::Stats* stats, const std::string& name, const osg::Vec3& pos, float width, float height, const osg::Vec4& color)
{
    std::string font("fonts/arial.ttf");
    float characterSize = 20.0f;
    float startBlocks = 150.0f;

    _name = name;
    _group = new osg::Group;

    _label = new osg::Geode;
    _textLabel = new osgText::Text;
    _label->addDrawable(_textLabel.get());
    _textLabel->setDataVariance(osg::Object::DYNAMIC);
    _textLabel->setColor(color);
    _textLabel->setFont(font);
    _textLabel->setCharacterSize(characterSize);
    _textLabel->setPosition(pos - osg::Vec3(0, height, 0));
    _textLabel->setText(name);

    StatsGraph* graph = new StatsGraph(pos + osg::Vec3(startBlocks, 0,0) , width-startBlocks, height);
    graph->setCullingActive(false);
    graph->addStatGraph(stats, stats, color, 1.0, name);
    _graph = graph;

    _group->addChild(_label.get());
    _group->addChild(_graph.get());
}
void StatAction::setAlpha(float v)
{
    std::cout << this << " color alpha " << v << std::endl;
    osg::Vec4 color = _textLabel->getColor();
    color[3] = v;
    _textLabel->setColor(color);
    for (int i = 0; i < (int) _graph->_statsGraphGeode->getNumDrawables(); i++) {
        StatsGraph::Graph* g = static_cast<StatsGraph::Graph*>(_graph->_statsGraphGeode->getDrawable(0));
        g->setColor(color);
    }
}

void StatAction::setPosition(const osg::Vec3& pos)
{
    float characterSize = 20.0f;
    _graph->changeYposition(pos[1]);
    _textLabel->setPosition(pos - osg::Vec3(0, characterSize,0));

}


void StatsHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("s","On screen stats.");
    usage.addKeyboardMouseBinding("S","Output stats to console.");
}

}
