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
    // OSG_NOTICE<<"TabWidget::handleImplementation"<<std::endl;

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

    _unselectedHeaderSwitch = new osg::Switch;
    _selectedHeaderSwitch = new osg::Switch;
    _tabWidgetSwitch = new osg::Switch;

    float unselected = 0.92f;
    float selected = 0.97f;
    float titleHeight = 10.0f;
    float characterWidth = titleHeight*0.7f;
    float margin = titleHeight*0.1f;
    float xPos = _extents.xMin();
    float yMin = _extents.yMax()-titleHeight;
    float yMax = _extents.yMax();
    float zMin = _extents.zMin();
    float zMax = _extents.zMax();

    for(Tabs::iterator itr = _tabs.begin();
        itr != _tabs.end();
        ++itr)
    {
        Tab* tab = itr->get();

        float width = tab->getText().size() * characterWidth;

        osg::BoundingBox headerExtents( xPos, yMin, zMin, xPos+width, yMax, zMax);

        OSG_NOTICE<<"headerExtents = "
                  <<headerExtents.xMin()<<", "<<headerExtents.yMin()<<", "<<headerExtents.zMin()<<", "
                  <<headerExtents.xMax()<<", "<<headerExtents.yMax()<<", "<<headerExtents.zMax()<<std::endl;

        osg::ref_ptr<osg::Node> text = style->createText(headerExtents, getAlignmentSettings(), getTextSettings(), tab->getText());
        osg::ref_ptr<osg::Node> unselected_panel = style->createPanel(headerExtents, osg::Vec4(unselected, unselected, unselected, 1.0f));
        osg::ref_ptr<osg::Node> selected_panel = style->createPanel(headerExtents, osg::Vec4(selected, selected, selected, 1.0f));

        osg::ref_ptr<osg::Group> selected_group = new osg::Group;
        selected_group->addChild(selected_panel.get());
        selected_group->addChild(text.get());

        osg::ref_ptr<osg::Group> unselected_group = new osg::Group;
        unselected_group->addChild(unselected_panel.get());
        unselected_group->addChild(text.get());

        _unselectedHeaderSwitch->addChild(unselected_group.get());
        _selectedHeaderSwitch->addChild(selected_group.get());
        _tabWidgetSwitch->addChild(tab->getWidget());

        xPos += width+margin;

    }

    setGraphicsSubgraph(-3, _unselectedHeaderSwitch.get());
    setGraphicsSubgraph(-2, _selectedHeaderSwitch.get());
    setGraphicsSubgraph(-1, _tabWidgetSwitch.get());

    _activateWidgets();
}

void TabWidget::_activateWidgets()
{
    if (_graphicsInitialized && _currentIndex<_tabs.size())
    {
        OSG_NOTICE<<"Activating widget "<<_currentIndex<<std::endl;

        _unselectedHeaderSwitch->setAllChildrenOn();
        _unselectedHeaderSwitch->setValue(_currentIndex, false);

        _selectedHeaderSwitch->setAllChildrenOff();
        _selectedHeaderSwitch->setValue(_currentIndex, true);

        _tabWidgetSwitch->setAllChildrenOff();
        _tabWidgetSwitch->setValue(_currentIndex, true);
    }
}
