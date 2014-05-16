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

#include <osgUI/Style>
#include <osg/Geode>
#include <osgText/Text>

using namespace osgUI;

osg::ref_ptr<Style>& Style::instance()
{
    static osg::ref_ptr<Style> s_style = new Style;
    return s_style;
}

OSG_INIT_SINGLETON_PROXY(StyleSingletonProxy, Style::instance())

Style::Style()
{
}

Style::Style(const Style& style, const osg::CopyOp& copyop):
    osg::Object(style, copyop)
{
}

osg::Node* Style::createPanel(const osg::BoundingBox& extents, const osg::Vec4& colour)
{
    OSG_NOTICE<<"Creating Panel"<<std::endl;
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices.get());

    vertices->push_back( osg::Vec3(extents.xMin(), extents.yMin(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMin(), extents.yMax(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMax(), extents.yMin(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMax(), extents.yMax(), extents.zMin()) );

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    geometry->setColorArray(colours, osg::Array::BIND_OVERALL);

    colours->push_back( colour );

    geometry->addPrimitiveSet( new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 4) );

    return geometry.release();
}

osg::Node* Style::createFrame(const osg::BoundingBox& extents, const FrameSettings* frameSettings)
{
    return 0;
}

osg::Node* Style::createText(const osg::BoundingBox& extents, const AlignmentSettings* as, const TextSettings* ts, const std::string& text)
{
    osg::ref_ptr<osgText::Text> textDrawable = new osgText::Text;

    textDrawable->setText(text);
    textDrawable->setPosition( osg::Vec3(extents.xMin(), extents.yMin(), extents.zMin()) );

    if (ts)
    {
        textDrawable->setFont(ts->getFont());
        textDrawable->setCharacterSize(ts->getCharacterSize());
    }

    if (as)
    {
        osgText::TextBase::AlignmentType alignmentType = static_cast<osgText::TextBase::AlignmentType>(as->getAlignment());
        textDrawable->setAlignment(alignmentType);
    }

    return textDrawable.release();
}

osg::Node* Style::createIcon(const osg::BoundingBox& extents, const std::string& filename)
{
    return 0;
}

