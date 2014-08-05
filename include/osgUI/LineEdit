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

#ifndef OSGUI_LINEEDIT
#define OSGUI_LINEEDIT

#include <osgUI/Widget>
#include <osgUI/Validator>
#include <osgText/Text>

namespace osgUI
{

class OSGUI_EXPORT LineEdit : public osgUI::Widget
{
public:
    LineEdit();
    LineEdit(const LineEdit& label, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Node(osgUI, LineEdit);

    void setText(const std::string& text);
    const std::string& getText() const { return _text; }

    virtual bool handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event);
    virtual void createGraphicsImplementation();
    virtual void enterImplementation();
    virtual void leaveImplementation();

    void setValidator(Validator* validator) { _validator = validator; }
    Validator* getValidator() { return _validator.get(); }
    const Validator* getValidator() const { return _validator.get(); }

    virtual void textChanged(const std::string& text);
    virtual void textChangedImplementation(const std::string& text);

    virtual void returnPressed() { if (!runCallbacks("returnPressed")) returnPressedImplementation(); }
    virtual void returnPressedImplementation();

protected:
    virtual ~LineEdit() {}

    osg::ref_ptr<Validator>             _validator;

    std::string                         _text;

    // implementation detail
    osg::ref_ptr<osg::Switch>           _backgroundSwitch;
    osg::ref_ptr<osgText::Text>         _textDrawable;
};

}

#endif
