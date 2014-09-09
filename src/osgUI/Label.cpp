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


#include <osgUI/Label>
#include <osgText/String>
#include <osgText/Font>
#include <osgText/Text>

using namespace osgUI;

Label::Label()
{
}

Label::Label(const osgUI::Label& label, const osg::CopyOp& copyop):
    Widget(label, copyop),
    _text(label._text)
{
}

void Label::createGraphicsImplementation()
{
    OSG_NOTICE<<"Label::createGraphicsImplementation()"<<std::endl;

    Style* style = (getStyle()!=0) ? getStyle() : Style::instance().get();
    osg::ref_ptr<Node> node = style->createText(_extents, getAlignmentSettings(), getTextSettings(), _text);
    _textDrawable = dynamic_cast<osgText::Text*>(node.get());

    style->setupClipStateSet(_extents, getOrCreateWidgetStateSet());
    setGraphicsSubgraph(0, node.get());
}
