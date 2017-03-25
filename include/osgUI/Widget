/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
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

#ifndef OSGUI_WIDGET
#define OSGUI_WIDGET

#include <osg/Group>
#include <osg/BoundingBox>
#include <osg/ScriptEngine>
#include <osgGA/Event>
#include <osgGA/EventVisitor>

#include <osgUI/Style>

namespace osgUI
{

class OSGUI_EXPORT Widget : public osg::Group
{
public:
    Widget();
    Widget(const Widget& widget, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Node(osgUI, Widget);

    virtual void traverse(osg::NodeVisitor& nv);
    virtual void traverseImplementation(osg::NodeVisitor& nv);

    virtual bool handle(osgGA::EventVisitor* ev, osgGA::Event* event);
    virtual bool handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event);

    typedef std::vector<osgUtil::LineSegmentIntersector::Intersection> Intersections;
    virtual bool computeIntersections(osgGA::EventVisitor* ev, osgGA::GUIEventAdapter* event, Intersections& intersections, osg::Node::NodeMask traversalMask = 0xffffffff) const;
    virtual bool computePositionInLocalCoordinates(osgGA::EventVisitor* ev, osgGA::GUIEventAdapter* event, osg::Vec3d& localPosition) const;
    virtual bool computeExtentsPositionInLocalCoordinates(osgGA::EventVisitor* ev, osgGA::GUIEventAdapter* event, osg::Vec3d& localPosition, bool withinExtents=true) const;

    virtual void dirty();

    typedef std::map<int, osg::ref_ptr<osg::Node> > GraphicsSubgraphMap;

    /** Set the subgraph to be used to render the widget.*/
    void setGraphicsSubgraph(int orderNum, osg::Node* node) { _graphicsSubgraphMap[orderNum] = node; _graphicsInitialized = true; }
    /** Get the subgraph to be used to render the widget.*/
    osg::Node* getGraphicsSubgraph(int orderNum) { GraphicsSubgraphMap::iterator itr = _graphicsSubgraphMap.find(orderNum); return (itr!=_graphicsSubgraphMap.end()) ? itr->second.get() :  0; }
    /** Get the const subgraph to be used to render the widget.*/
    const osg::Node* getGraphicsSubgraph(int orderNum) const { GraphicsSubgraphMap::const_iterator itr = _graphicsSubgraphMap.find(orderNum); return (itr!=_graphicsSubgraphMap.end()) ? itr->second.get() :  0; }

    void setGraphicsSubgraphMap(const GraphicsSubgraphMap& gsm) { _graphicsSubgraphMap = gsm; _graphicsInitialized = true; }
    GraphicsSubgraphMap& getGraphicsSubgraphMap() { return _graphicsSubgraphMap; }
    const GraphicsSubgraphMap& getGraphicsSubgraphMap() const { return _graphicsSubgraphMap; }

    /** Set the WidgetStateSet is used internally by Widgets to manage state that decorates the subgraph.
      * WidgetStateSet is not serialized and is typically populated by the Widget::createGraphics() implementation,
      * end users will not normally touoch the WidgetStateSet, use the normal Node::setStateSet() if you want to apply
      * your own state to the Widget and it's subgraphs.*/
    void setWidgetStateSet(osg::StateSet* stateset) { _widgetStateSet = stateset; }
    osg::StateSet* getWidgetStateSet() { return _widgetStateSet.get(); }
    const osg::StateSet* getWidgetStateSet() const { return _widgetStateSet.get(); }
    osg::StateSet* getOrCreateWidgetStateSet() { if (!_widgetStateSet) _widgetStateSet = new osg::StateSet; return _widgetStateSet.get(); }


    /** createGraphics entry method, calls either callback object named "createGraphics" or the createGraphicsImplementation() method.*/
    virtual void createGraphics();

    /** createGraphicsImplementation method that creates the subgraph that will render the widget and assigns it to the Widget via the Widet::setGraphicsSubgraph() method.*/
    virtual void createGraphicsImplementation();


    virtual void setExtents(const osg::BoundingBoxf& bb);
    const osg::BoundingBoxf& getExtents() const { return _extents; }

    void setStyle(Style* style) { _style = style; }
    Style* getStyle() { return _style.get(); }
    const Style* getStyle() const { return _style.get(); }

    void setAlignmentSettings(AlignmentSettings* alignmentSettings) { _alignmentSettings = alignmentSettings; }
    AlignmentSettings* getAlignmentSettings() { return _alignmentSettings.get(); }
    const AlignmentSettings* getAlignmentSettings() const { return _alignmentSettings.get(); }

    void setFrameSettings(FrameSettings* textSettings) { _frameSettings = textSettings; }
    FrameSettings* getFrameSettings() { return _frameSettings.get(); }
    const FrameSettings* getFrameSettings() const { return _frameSettings.get(); }

    void setTextSettings(TextSettings* textSettings) { _textSettings = textSettings; }
    TextSettings* getTextSettings() { return _textSettings.get(); }
    const TextSettings* getTextSettings() const { return _textSettings.get(); }

    /** set whether the widget should fill the extents of its background.*/
    virtual void setAutoFillBackground(bool enabled) { _autoFillBackground = enabled; }
    /** get whether the widget should fill the extents of its background.*/
    virtual bool getAutoFillBackground() const { return _autoFillBackground; }

    /** set the visibility of the widget.*/
    virtual void setVisible(bool visible) { _visible = visible; }
    /** get the visibility of the widget.*/
    virtual bool getVisible() const { return _visible; }

    /** set whether the widget is enabled for user interaction.*/
    virtual void setEnabled(bool enabled) { _enabled = enabled; }
    /** get whether the widget is enabled for user interaction.*/
    virtual bool getEnabled() const { return _enabled; }

    enum FocusBehaviour
    {
        CLICK_TO_FOCUS,
        FOCUS_FOLLOWS_POINTER,
        EVENT_DRIVEN_FOCUS_DISABLED
    };

    void setFocusBehaviour(FocusBehaviour behaviour) { _focusBehaviour = behaviour; }
    FocusBehaviour getFocusBehaviour() const { return _focusBehaviour; }

    /** update the focus according to events.*/
    virtual void updateFocus(osg::NodeVisitor& nv);

    /** set whether the widget has focus or not.*/
    virtual void setHasEventFocus(bool focus);

    /** get whether the widget has focus or not.*/
    virtual bool getHasEventFocus() const;


    /** invoke all callbacks with specified names providing input and output parameters.*/
    bool runCallbacks(const std::string& name, osg::Parameters& inputParameters, osg::Parameters& outputParameters) { return osg::runNamedCallbackObjects(this, name, inputParameters, outputParameters); }

    /** invoke all callbacks with specified names without any specified input or output parameters.*/
    bool runCallbacks(const std::string& name) { osg::Parameters inputParameters, outputParameters; return osg::runNamedCallbackObjects(this, name, inputParameters, outputParameters); }


    /** Compute the bounding sphere of the widget.*/
    virtual osg::BoundingSphere computeBound() const;

    /** update any focus related graphics+state to the focused state.*/
    virtual void enter();
    virtual void enterImplementation();

    /** update any focus related graphics+state to the unfocused state.*/
    virtual void leave();
    virtual void leaveImplementation();

    /** resize all GLObjectBuffers.*/
    virtual void resizeGLObjectBuffers(unsigned int maxSize);
    /** resize all GLObjectBuffers.*/
    virtual void releaseGLObjects(osg::State* = 0) const;

protected:
    virtual ~Widget() {}

    FocusBehaviour                      _focusBehaviour;
    bool                                _hasEventFocus;
    bool                                _graphicsInitialized;

    GraphicsSubgraphMap                 _graphicsSubgraphMap;
    osg::ref_ptr<osg::StateSet>         _widgetStateSet;

    osg::BoundingBoxf                   _extents;

    osg::ref_ptr<Style>                 _style;

    osg::ref_ptr<AlignmentSettings>     _alignmentSettings;
    osg::ref_ptr<FrameSettings>         _frameSettings;
    osg::ref_ptr<TextSettings>          _textSettings;
    bool                                _autoFillBackground;

    bool                                _visible;
    bool                                _enabled;
};

}

#endif
