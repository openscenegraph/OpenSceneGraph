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
}
