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


#include <osgUI/LineEdit>
#include <osgText/String>
#include <osgText/Font>
#include <osgText/Text>
#include <osg/Notify>
#include <osg/ValueObject>

using namespace osgUI;

LineEdit::LineEdit()
{
}

LineEdit::LineEdit(const osgUI::LineEdit& label, const osg::CopyOp& copyop):
    Widget(label, copyop),
    _text(label._text)
{
}

bool LineEdit::handleImplementation(osgGA::EventVisitor* /*ev*/, osgGA::Event* event)
{
    if (!getHasEventFocus()) return false;

    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if (!ea) return false;

    switch(ea->getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
            if (ea->getKey()==osgGA::GUIEventAdapter::KEY_BackSpace ||
                ea->getKey()==osgGA::GUIEventAdapter::KEY_Delete)
            {
                if (!_text.empty())
                {
                    setText(_text.substr(0, _text.size()-1));
                    return true;

                }
            }
            else if (ea->getKey()>=32 && ea->getKey()<=0xff00)
            {
                setText(_text + std::string::value_type(ea->getKey()));
                return true;
            }
            else if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Return )
            {
                if (_validator.valid())
                {
                    std::string text_copy(_text);
                    int cursorpos;
                    if (_validator->validate(text_copy, cursorpos)==Validator::INTERMEDIATE)
                    {
                        _validator->fixup(text_copy);
                    }
                    if (text_copy!=_text) setText(text_copy);
                }

                returnPressed();
                return true;
            }

            OSG_NOTICE<<"Key pressed : "<<ea->getKey()<<std::endl;

            break;
        default:
            break;
    }

    return false;
}

void LineEdit::setText(const std::string& text)
{
    if (_text==text) return;

    std::string text_copy(text);
    if (_validator.valid())
    {
        int cursorpos = 0;
        Validator::State state = _validator->validate(text_copy, cursorpos);
        if (state==Validator::INVALID) return;
    }

    _text = text_copy;

    textChanged(_text);

    if (_textDrawable) _textDrawable->setText(_text);
}

void LineEdit::enterImplementation()
{
    OSG_NOTICE<<"LineEdit enter"<<std::endl;
    if (_backgroundSwitch.valid()) _backgroundSwitch->setSingleChildOn(1);
}


void LineEdit::leaveImplementation()
{
    OSG_NOTICE<<"LineEdit leave"<<std::endl;
    if (_backgroundSwitch.valid()) _backgroundSwitch->setSingleChildOn(0);
}


void LineEdit::textChanged(const std::string& text)
{
    osg::CallbackObject* co = getCallbackObject(this, "textChanged");
    if (co)
    {
        osg::Parameters inputParameters, outputParameters;
        inputParameters.push_back(new osg::StringValueObject("text",text));
        if (co->run(this, inputParameters, outputParameters))
        {
            return;
        }
    }
    textChangedImplementation(text);
}

void LineEdit::textChangedImplementation(const std::string& text)
{
    OSG_NOTICE<<"textChangedImplementation("<<text<<")"<<std::endl;
}

void LineEdit::returnPressedImplementation()
{
    OSG_NOTICE<<"returnPressedImplementation()"<<std::endl;
}

void LineEdit::createGraphicsImplementation()
{
    Style* style = (getStyle()!=0) ? getStyle() : Style::instance().get();

    osg::ref_ptr<osg::Group> group = new osg::Group;

    osg::BoundingBox extents(_extents);
    float unFocused = 0.92;
    float withFocus = 0.97;

    osg::Vec4 frameColor(unFocused,unFocused,unFocused,1.0f);

    bool requiresFrame = (getFrameSettings() && getFrameSettings()->getShape()!=osgUI::FrameSettings::NO_FRAME);
    if (requiresFrame)
    {
        group->addChild(style->createFrame(_extents, getFrameSettings(), frameColor));
        extents.xMin() += getFrameSettings()->getLineWidth();
        extents.xMax() -= getFrameSettings()->getLineWidth();
        extents.yMin() += getFrameSettings()->getLineWidth();
        extents.yMax() -= getFrameSettings()->getLineWidth();
    }

    // clear background of edit region
    _backgroundSwitch = new osg::Switch;
    _backgroundSwitch->addChild(style->createPanel(extents, osg::Vec4(unFocused, unFocused,unFocused, 1.0)));
    _backgroundSwitch->addChild(style->createPanel(extents, osg::Vec4(withFocus, withFocus, withFocus,1.0)));
    _backgroundSwitch->setSingleChildOn(0);
    group->addChild(_backgroundSwitch.get());

    osg::ref_ptr<Node> node = style->createText(extents, getAlignmentSettings(), getTextSettings(), _text);
    _textDrawable = dynamic_cast<osgText::Text*>(node.get());
    node->setDataVariance(osg::Object::DYNAMIC);
    group->addChild(node.get());

    style->setupClipStateSet(_extents, getOrCreateWidgetStateSet());

    setGraphicsSubgraph(0, group.get());
}
