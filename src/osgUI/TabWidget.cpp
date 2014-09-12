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


#include <osgUI/TabWidget>
#include <osgText/String>
#include <osgText/Font>
#include <osgText/Text>
#include <osg/Notify>
#include <osg/ValueObject>
#include <osg/io_utils>

using namespace osgUI;

TabWidget::TabWidget():
    _currentIndex(0)
{
}

TabWidget::TabWidget(const osgUI::TabWidget& tabwidget, const osg::CopyOp& copyop):
    Widget(tabwidget, copyop),
    _tabs(tabwidget._tabs)
{
}

bool TabWidget::handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event)
{
    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if (!ea) return false;

    osgGA::GUIActionAdapter* aa = ev ? ev->getActionAdapter() : 0;
    if (!aa) return false;

    if (!getHasEventFocus()) return false;

    unsigned int tabHeaderContainsPointer = _tabs.size();

    // test if active tab header contains pointer
    {
        osg::NodePath nodePath = ev->getNodePath();
        nodePath.push_back(_activeHeaderSwitch.get());

        osgUtil::LineSegmentIntersector::Intersections intersections;
        if (aa->computeIntersections(*ea, nodePath, intersections))
        {
            tabHeaderContainsPointer = _currentIndex;
        }
    }

    // test if inactive tab header contains pointer
    {
        osg::NodePath nodePath = ev->getNodePath();
        nodePath.push_back(_inactiveHeaderSwitch.get());

        osgUtil::LineSegmentIntersector::Intersections intersections;
        if (aa->computeIntersections(*ea, nodePath, intersections))
        {
            const osgUtil::LineSegmentIntersector::Intersection& Intersection = *intersections.begin();
            for(osg::NodePath::const_iterator itr = Intersection.nodePath.begin();
                itr != Intersection.nodePath.end();
                ++itr)
            {
                if ((*itr)->getUserValue("index",tabHeaderContainsPointer)) break;
            }
        }
    }

    if (tabHeaderContainsPointer>=_tabs.size()) return false;

    switch(ea->getEventType())
    {
        case(osgGA::GUIEventAdapter::SCROLL):
            if (ea->getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_DOWN)
            {
                if (getCurrentIndex()<_tabs.size()-1) setCurrentIndex(getCurrentIndex()+1);
                return true;
            }
            else if (ea->getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_UP)
            {
                if (getCurrentIndex()>0) setCurrentIndex(getCurrentIndex()-1);
                return true;
            }
            break;

        case(osgGA::GUIEventAdapter::KEYDOWN):
            if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Down || ea->getKey()==osgGA::GUIEventAdapter::KEY_Right )
            {
                if (getCurrentIndex()<_tabs.size()-1) setCurrentIndex(getCurrentIndex()+1);
                return true;
            }
            else if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Up || ea->getKey()==osgGA::GUIEventAdapter::KEY_Left )
            {
                if (getCurrentIndex()>0) setCurrentIndex(getCurrentIndex()-1);
                return true;
            }

            break;

        case(osgGA::GUIEventAdapter::RELEASE):
        {
            setCurrentIndex(tabHeaderContainsPointer);
            return true;

            break;
        }
        default:
            break;
    }

    return false;
}

void TabWidget::enterImplementation()
{
    OSG_NOTICE<<"TabWidget enter"<<std::endl;
}


void TabWidget::leaveImplementation()
{
    OSG_NOTICE<<"TabWidget leave"<<std::endl;
}

void TabWidget::setCurrentIndex(unsigned int i)
{
    // OSG_NOTICE << "TabWidget::setCurrentIndex("<<i<<")"<<std::endl;
    if (_currentIndex==i) return;

    _currentIndex = i;
    _activateWidgets();

    currrentIndexChanged(_currentIndex);
}

void TabWidget::currrentIndexChanged(unsigned int i)
{
    osg::CallbackObject* co = getCallbackObject(this, "currentIndexChanged");
    if (co)
    {
        osg::Parameters inputParameters, outputParameters;
        inputParameters.push_back(new osg::UIntValueObject("index",i));
        if (co->run(this, inputParameters, outputParameters))
        {
            return;
        }
    }
    currentIndexChangedImplementation(i);
}

void TabWidget::currentIndexChangedImplementation(unsigned int i)
{
  OSG_NOTICE<<"TabWidget::currentIndexChangedImplementation("<<i<<")"<<std::endl;
}



void TabWidget::createGraphicsImplementation()
{
    Style* style = (getStyle()!=0) ? getStyle() : Style::instance().get();

    // bool requiresFrame = (getFrameSettings() && getFrameSettings()->getShape()!=osgUI::FrameSettings::NO_FRAME);

    _inactiveHeaderSwitch = new osg::Switch;
    _activeHeaderSwitch = new osg::Switch;
    _tabWidgetSwitch = new osg::Switch;

    float active = 0.84f;
    float inactive = 0.80f;
    float titleHeight = 10.0f;
    float characterWidth = titleHeight*0.7f;
    float margin = titleHeight*0.2f;

    unsigned int tabIndex = 0;

    osg::BoundingBox centerExtents = _extents;
    centerExtents.yMax() -= titleHeight;

    osg::ref_ptr<osgUI::AlignmentSettings> textAlignment = new osgUI::AlignmentSettings(osgUI::AlignmentSettings::LEFT_CENTER);

    osg::ref_ptr<FrameSettings> fs = getFrameSettings();
    if (!fs)
    {
        fs = new osgUI::FrameSettings;
        fs->setShadow(osgUI::FrameSettings::RAISED);
        fs->setLineWidth(1.0f);
    }

    osg::Vec4 dialogBackgroundColor(active,active,active,1.0);
    osg::Vec4 inactiveColor(inactive, inactive, inactive, 1.0f);

    float xPos = _extents.xMin();
    float yMin = _extents.yMax() - titleHeight - fs->getLineWidth();
    float yMax = _extents.yMax();
    float zMin = _extents.zMin();
    float zMax = _extents.zMax();

    for(Tabs::iterator itr = _tabs.begin();
        itr != _tabs.end();
        ++itr, ++tabIndex)
    {
        Tab* tab = itr->get();

        float width = tab->getText().size() * characterWidth;

        osg::BoundingBox headerExtents( xPos, yMin, zMin, xPos+width, yMax, zMax);
        osg::BoundingBox textExtents( xPos+margin, yMin, zMin, xPos+width-margin, yMax, zMax);

        osg::ref_ptr<osg::Node> textNode = style->createText(textExtents, textAlignment.get(), getTextSettings(), tab->getText());

        osg::ref_ptr<osgText::Text> text = dynamic_cast<osgText::Text*>(textNode.get());
        if (text.valid()) textExtents = text->getBoundingBox();

        // adjust position of size of text.
        float textWidth = (textExtents.xMax() - textExtents.xMin());

        headerExtents.xMax() = textExtents.xMin() + textWidth + margin;

        osg::ref_ptr<osg::Node> inactive_panel = _createTabHeader(headerExtents, fs.get(), inactiveColor);
        osg::ref_ptr<osg::Node> selected_panel = _createTabHeader(headerExtents, fs.get(), dialogBackgroundColor);

        osg::ref_ptr<osg::Group> selected_group = new osg::Group;
        selected_group->setUserValue("index",tabIndex);
        selected_group->addChild(selected_panel.get());
        selected_group->addChild(text.get());

        osg::ref_ptr<osg::Group> inactive_group = new osg::Group;
        inactive_group->setUserValue("index",tabIndex);
        inactive_group->addChild(inactive_panel.get());
        inactive_group->addChild(text.get());

        _inactiveHeaderSwitch->addChild(inactive_group.get());
        _activeHeaderSwitch->addChild(selected_group.get());
        _tabWidgetSwitch->addChild(tab->getWidget());

        xPos += textWidth+3.0*margin;

    }

    setGraphicsSubgraph(-4, _inactiveHeaderSwitch.get());

    osg::ref_ptr<osg::Node> backgroundPanel = _createTabFrame( centerExtents, fs.get(), dialogBackgroundColor);
    setGraphicsSubgraph(-3, backgroundPanel.get());

    setGraphicsSubgraph(-2, _activeHeaderSwitch.get());
    setGraphicsSubgraph(-1, _tabWidgetSwitch.get());

    _activateWidgets();
}

void TabWidget::_activateWidgets()
{
    if (_graphicsInitialized && _currentIndex<_tabs.size())
    {
        OSG_NOTICE<<"Activating widget "<<_currentIndex<<std::endl;

        _inactiveHeaderSwitch->setAllChildrenOn();
        _inactiveHeaderSwitch->setValue(_currentIndex, false);

        _activeHeaderSwitch->setAllChildrenOff();
        _activeHeaderSwitch->setValue(_currentIndex, true);

        _tabWidgetSwitch->setAllChildrenOff();
        _tabWidgetSwitch->setValue(_currentIndex, true);
    }
}

osg::Node* TabWidget::_createTabFrame(const osg::BoundingBox& extents, osgUI::FrameSettings* fs, const osg::Vec4& color)
{
    Style* style = (getStyle()!=0) ? getStyle() : Style::instance().get();
    osg::ref_ptr<osg::Group> group = new osg::Group;

    group->addChild( style->createPanel(extents, color) );
    group->addChild( style->createFrame(extents, fs, color) );

    return group.release();
}

osg::Node* TabWidget::_createTabHeader(const osg::BoundingBox& extents, osgUI::FrameSettings* frameSettings, const osg::Vec4& color)
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setName("Frame");

    float topScale = 1.0f;
    float bottomScale = 1.0f;
    float leftScale = 1.0f;
    float rightScale = 1.0f;

    if (frameSettings)
    {
        switch(frameSettings->getShadow())
        {
            case(FrameSettings::PLAIN):
                // default settings are appropriate for PLAIN
                break;
            case(FrameSettings::SUNKEN):
                topScale = 0.6f;
                bottomScale = 1.2f;
                leftScale = 0.8f;
                rightScale = 0.8f;
                break;
            case(FrameSettings::RAISED):
                topScale = 1.2f;
                bottomScale = 0.6f;
                leftScale = 0.8f;
                rightScale = 0.8f;
                break;
        }
    }

    osg::Vec4 topColor(osg::minimum(color.r()*topScale,1.0f), osg::minimum(color.g()*topScale,1.0f), osg::minimum(color.b()*topScale,1.0f), color.a());
    osg::Vec4 bottomColor(osg::minimum(color.r()*bottomScale,1.0f), osg::minimum(color.g()*bottomScale,1.0f), osg::minimum(color.b()*bottomScale,1.0f), color.a());
    osg::Vec4 leftColor(osg::minimum(color.r()*leftScale,1.0f), osg::minimum(color.g()*leftScale,1.0f), osg::minimum(color.b()*leftScale,1.0f), color.a());
    osg::Vec4 rightColor(osg::minimum(color.r()*rightScale,1.0f), osg::minimum(color.g()*rightScale,1.0f), osg::minimum(color.b()*rightScale,1.0f), color.a());

    float lineWidth = frameSettings ? frameSettings->getLineWidth() : 1.0f;

    osg::Vec3 outerBottomLeft(extents.xMin(), extents.yMin()+lineWidth, extents.zMin());
    osg::Vec3 outerBottomRight(extents.xMax(), extents.yMin()+lineWidth, extents.zMin());
    osg::Vec3 outerTopLeft(extents.xMin(), extents.yMax(), extents.zMin());
    osg::Vec3 outerTopRight(extents.xMax(), extents.yMax(), extents.zMin());

    osg::Vec3 innerBottomLeft(extents.xMin()+lineWidth, extents.yMin(), extents.zMin());
    osg::Vec3 innerBottomRight(extents.xMax()-lineWidth, extents.yMin(), extents.zMin());
    osg::Vec3 innerTopLeft(extents.xMin()+lineWidth, extents.yMax()-lineWidth, extents.zMin());
    osg::Vec3 innerTopRight(extents.xMax()-lineWidth, extents.yMax()-lineWidth, extents.zMin());

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices.get());

    vertices->push_back( outerBottomLeft );   // 0
    vertices->push_back( outerBottomRight );  // 1
    vertices->push_back( outerTopLeft );      // 2
    vertices->push_back( outerTopRight );     // 3

    vertices->push_back( innerBottomLeft );  // 4
    vertices->push_back( innerBottomRight ); // 5
    vertices->push_back( innerTopLeft );     // 6
    vertices->push_back( innerTopRight );    // 7

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    geometry->setColorArray(colours.get(), osg::Array::BIND_PER_PRIMITIVE_SET);

    // bottom
    {
        colours->push_back(bottomColor);

        osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
        geometry->addPrimitiveSet(primitives.get());
        primitives->push_back(4);
        primitives->push_back(0);
        primitives->push_back(5);
        primitives->push_back(1);
    }

    // top
    {
        colours->push_back(topColor);

        osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
        geometry->addPrimitiveSet(primitives.get());
        primitives->push_back(2);
        primitives->push_back(6);
        primitives->push_back(3);
        primitives->push_back(7);
    }

    // left
    {
        colours->push_back(leftColor);

        osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
        geometry->addPrimitiveSet(primitives.get());
        primitives->push_back(2);
        primitives->push_back(0);
        primitives->push_back(6);
        primitives->push_back(4);
    }

    // right
    {
        colours->push_back(rightColor);

        osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
        geometry->addPrimitiveSet(primitives.get());
        primitives->push_back(7);
        primitives->push_back(5);
        primitives->push_back(3);
        primitives->push_back(1);
    }


    // center
    {
        colours->push_back(color);

        osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
        geometry->addPrimitiveSet(primitives.get());
        primitives->push_back(6);
        primitives->push_back(4);
        primitives->push_back(7);
        primitives->push_back(5);
    }

    return geometry.release();
}
