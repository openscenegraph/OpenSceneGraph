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

#ifndef OSGUI_PUSHBUTTON
#define OSGUI_PUSHBUTTON

#include <osgUI/Widget>
#include <osg/Switch>
#include <osgText/Text>

namespace osgUI
{

class OSGUI_EXPORT PushButton : public osgUI::Widget
{
public:
    PushButton();
    PushButton(const PushButton& pb, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Node(osgUI, PushButton);

    void setText(const std::string& text) { _text = text; dirty(); }
    std::string& getText() { return _text; }
    const std::string& getText() const { return _text; }

    virtual bool handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event);
    virtual void createGraphicsImplementation();
    virtual void enterImplementation();
    virtual void leaveImplementation();

    virtual void pressed() { if (!runCallbacks("pressed")) pressedImplementation(); }
    virtual void pressedImplementation();

    virtual void released() { if (!runCallbacks("released")) releasedImplementation(); }
    virtual void releasedImplementation();


protected:
    virtual ~PushButton() {}

    std::string                         _text;

    // implementation detail
    osg::ref_ptr<osg::Switch>           _buttonSwitch;
    osg::ref_ptr<osgText::Text>         _textDrawable;
};

}

#endif
