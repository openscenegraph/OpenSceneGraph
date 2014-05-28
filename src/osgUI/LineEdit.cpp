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

using namespace osgUI;

LineEdit::LineEdit()
{
}

LineEdit::LineEdit(const osgUI::LineEdit& label, const osg::CopyOp& copyop):
    Widget(label, copyop),
    _text(label._text)
{
}

bool LineEdit::handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event)
{
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
                    _text.erase(_text.size()-1, 1);
                    if (_textDrawable) _textDrawable->setText(_text);
                    return true;

                }
            }
            else if (ea->getKey()>=32 && ea->getKey()<=0xff00)
            {
                _text.push_back(ea->getKey());
                if (_textDrawable) _textDrawable->setText(_text);
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
    _text = text;
    if (_textDrawable) _textDrawable->setText(_text);
}



void LineEdit::createGraphicsImplementation()
{
    Style* style = (getStyle()!=0) ? getStyle() : Style::instance().get();
    osg::ref_ptr<Node> node = style->createText(_extents, getAlignmentSettings(), getTextSettings(), _text);
    _textDrawable = dynamic_cast<osgText::Text*>(node.get());
    _textDrawable->setDataVariance(osg::Object::DYNAMIC);

    style->setupClipStateSet(_extents, getOrCreateStateSet());

    setGraphicsSubgraph(0, _textDrawable.get());
}
