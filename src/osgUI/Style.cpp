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
#include <osg/Depth>
#include <osg/TexGen>
#include <osg/AlphaFunc>
#include <osgText/Text>
#include <osgDB/ReadFile>

using namespace osgUI;

osg::ref_ptr<Style>& Style::instance()
{
    static osg::ref_ptr<Style> s_style = new Style;
    return s_style;
}

OSG_INIT_SINGLETON_PROXY(StyleSingletonProxy, Style::instance())

Style::Style()
{
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage(1,1,1,GL_RGBA, GL_FLOAT);
    *(reinterpret_cast<osg::Vec4f*>(image->data(0,0,0))) = osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f);

    _clipTexture = new osg::Texture2D;
    _clipTexture->setImage(image.get());
    _clipTexture->setBorderColor(osg::Vec4f(1.0f,1.0f,0.5f,0.0f));
    _clipTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
    _clipTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
    _clipTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    _clipTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);

    //image = osgDB::readImageFile("Images/lz.rgb");
    //_clipTexture->setImage(image.get());
}

Style::Style(const Style& style, const osg::CopyOp& copyop):
    osg::Object(style, copyop),
    _clipTexture(style._clipTexture)
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


    textDrawable->setEnableDepthWrites(false);

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

void Style::setupDialogStateSet(osg::StateSet* stateset)
{
    stateset->setRenderBinDetails(5, "TraversalOrderBin", osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    stateset->setAttributeAndModes( new osg::Depth(osg::Depth::LESS,0.0, 1.0,false), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
}

void Style::setupClipStateSet(const osg::BoundingBox& extents, osg::StateSet* stateset)
{
    unsigned int clipTextureUnit = 1;

    stateset->setAttributeAndModes( new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.0f), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    stateset->setTextureAttributeAndModes( clipTextureUnit, _clipTexture.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    osg::Matrixd matrix = osg::Matrixd::translate(osg::Vec3(-extents.xMin(), -extents.yMin(), -extents.zMin()))*
                          osg::Matrixd::scale(osg::Vec3(1.0f/(extents.xMax()-extents.xMin()), 1.0f/(extents.yMax()-extents.yMin()), 1.0f));

    osg::ref_ptr<osg::TexGen> texgen = new osg::TexGen;
    texgen->setPlanesFromMatrix(matrix);
    texgen->setMode(osg::TexGen::OBJECT_LINEAR);
    stateset->setTextureAttributeAndModes( clipTextureUnit, texgen.get(), osg::StateAttribute::ON);
}
