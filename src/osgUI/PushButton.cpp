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


#include <osgUI/PushButton>
#include <osgText/String>
#include <osgText/Font>
#include <osgText/Text>
#include <osg/Notify>

using namespace osgUI;

PushButton::PushButton()
{
}

PushButton::PushButton(const osgUI::PushButton& pb, const osg::CopyOp& copyop):
    Widget(pb, copyop),
    _text(pb._text)
{
}

bool PushButton::handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event)
{
    if (!getHasEventFocus()) return false;

    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if (!ea) return false;

    switch(ea->getEventType())
    {
        case(osgGA::GUIEventAdapter::PUSH):
            if (_buttonSwitch.valid())
            {
                pressed();
            }
            break;
        case(osgGA::GUIEventAdapter::RELEASE):
            if (_buttonSwitch.valid())
            {
                released();
            }
            break;
        default:
            break;
    }

    return false;
}

void PushButton::enterImplementation()
{
    OSG_NOTICE<<"PushButton enter"<<std::endl;
    if (_buttonSwitch.valid()) _buttonSwitch->setSingleChildOn(1);
}


void PushButton::leaveImplementation()
{
    OSG_NOTICE<<"PushButton leave"<<std::endl;
    if (_buttonSwitch.valid()) _buttonSwitch->setSingleChildOn(0);
}

void PushButton::createGraphicsImplementation()
{
    osg::ref_ptr<osg::Group> group = new osg::Group;

    Style* style = (getStyle()!=0) ? getStyle() : Style::instance().get();


    float pressed = 0.88;
    float unFocused = 0.92;
    float withFocus = 0.97;

    osg::Vec4 frameColor(unFocused,unFocused,unFocused,1.0f);

    osg::BoundingBox extents(_extents);

    bool requiresFrame = (getFrameSettings() && getFrameSettings()->getShape()!=osgUI::FrameSettings::NO_FRAME);
    if (requiresFrame)
    {
        group->addChild(style->createFrame(_extents, getFrameSettings(), frameColor));
        extents.xMin() += getFrameSettings()->getLineWidth();
        extents.xMax() -= getFrameSettings()->getLineWidth();
        extents.yMin() += getFrameSettings()->getLineWidth();
        extents.yMax() -= getFrameSettings()->getLineWidth();
    }

    _buttonSwitch = new osg::Switch;
    _buttonSwitch->addChild(style->createPanel(extents, osg::Vec4(unFocused, unFocused,unFocused, 1.0)));
    _buttonSwitch->addChild(style->createPanel(extents, osg::Vec4(withFocus,withFocus,withFocus,1.0)));
    _buttonSwitch->addChild(style->createPanel(extents, osg::Vec4(pressed,pressed,pressed,1.0)));
    _buttonSwitch->setSingleChildOn(0);

    group->addChild(_buttonSwitch.get());

    // create label.
    osg::ref_ptr<Node> node = style->createText(extents, getAlignmentSettings(), getTextSettings(), _text);
    _textDrawable = dynamic_cast<osgText::Text*>(node.get());
    _textDrawable->setDataVariance(osg::Object::DYNAMIC);

    group->addChild(_textDrawable.get());

    style->setupClipStateSet(_extents, getOrCreateWidgetStateSet());

    setGraphicsSubgraph(0, group.get());
}

void PushButton::pressedImplementation()
{
    _buttonSwitch->setSingleChildOn(2);
}

void PushButton::releasedImplementation()
{
    _buttonSwitch->setSingleChildOn(1);
}
