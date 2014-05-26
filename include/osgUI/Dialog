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

#ifndef OSGUI_DIALOG
#define OSGUI_DIALOG

#include <osgUI/Widget>
#include <osgText/Text>

namespace osgUI
{

class OSGUI_EXPORT Dialog : public osgUI::Widget
{
public:
    Dialog();
    Dialog(const Dialog& dialog, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Node(osgUI, Dialog);

    void setTitle(const std::string& text) { _title = text; dirty(); }
    std::string& getTitle() { return _title; }
    const std::string& getTitle() const { return _title; }

    bool handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event);

    virtual void createGraphicsImplementation();

protected:
    virtual ~Dialog() {}

    std::string                         _title;

    osg::ref_ptr<osg::Group>            _group;

    // implementation detail
    osg::ref_ptr<osgText::Text>         _titleDrawable;
};

}

#endif
