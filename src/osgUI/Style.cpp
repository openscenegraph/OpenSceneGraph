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
    _clipTexture->setBorderColor(osg::Vec4f(1.0f,1.0f,1.0f,0.0f));
    _clipTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
    _clipTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
    _clipTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    _clipTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);

    //image = osgDB::readImageFile("Images/lz.rgb");
    //_clipTexture->setImage(image.get());

    _disabledDepthWrite = new osg::Depth(osg::Depth::LESS,0.0, 1.0,false);
    _enabledDepthWrite = new osg::Depth(osg::Depth::LESS,0.0, 1.0,true);
    _disableColorWriteMask = new osg::ColorMask(false, false, false, false);
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

osg::Node* Style::createDepthSetPanel(const osg::BoundingBox& extents)
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices.get());

    vertices->push_back( osg::Vec3(extents.xMin(), extents.yMin(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMin(), extents.yMax(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMax(), extents.yMin(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMax(), extents.yMax(), extents.zMin()) );

    geometry->addPrimitiveSet( new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 4) );

    osg::ref_ptr<osg::StateSet> stateset = geometry->getOrCreateStateSet();
    stateset->setAttributeAndModes( _enabledDepthWrite.get(), osg::StateAttribute::ON);
    stateset->setAttributeAndModes( _disableColorWriteMask.get() );

    return geometry.release();
}

osg::Node* Style::createFrame(const osg::BoundingBox& extents, const FrameSettings* frameSettings, const osg::Vec4& color)
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

    float topScale = 1.0f;
    float bottomScale = 1.0f;
    float leftScale = 1.0f;
    float rightScale = 1.0f;

    if (frameSettings)
    {
        switch(frameSettings->getShadow())
        {
            case(FrameSettings::PLAIN):
                // default settings are appropriate for PLAIN
                break;
            case(FrameSettings::SUNKEN):
                topScale = 0.6f;
                bottomScale = 1.2f;
                leftScale = 0.8f;
                rightScale = 0.8f;
                break;
            case(FrameSettings::RAISED):
                topScale = 1.2f;
                bottomScale = 0.6f;
                leftScale = 0.8f;
                rightScale = 0.8f;
                break;
        }
    }

    osg::Vec4 topColor(osg::minimum(color.r()*topScale,1.0f), osg::minimum(color.g()*topScale,1.0f), osg::minimum(color.b()*topScale,1.0f), color.a());
    osg::Vec4 bottomColor(osg::minimum(color.r()*bottomScale,1.0f), osg::minimum(color.g()*bottomScale,1.0f), osg::minimum(color.b()*bottomScale,1.0f), color.a());
    osg::Vec4 leftColor(osg::minimum(color.r()*leftScale,1.0f), osg::minimum(color.g()*leftScale,1.0f), osg::minimum(color.b()*leftScale,1.0f), color.a());
    osg::Vec4 rightColor(osg::minimum(color.r()*rightScale,1.0f), osg::minimum(color.g()*rightScale,1.0f), osg::minimum(color.b()*rightScale,1.0f), color.a());

    float lineWidth = frameSettings ? frameSettings->getLineWidth() : 1.0f;

    osg::Vec3 outerBottomLeft(extents.xMin(), extents.yMin(), extents.zMin());
    osg::Vec3 outerBottomRight(extents.xMax(), extents.yMin(), extents.zMin());
    osg::Vec3 outerTopLeft(extents.xMin(), extents.yMax(), extents.zMin());
    osg::Vec3 outerTopRight(extents.xMax(), extents.yMax(), extents.zMin());

    osg::Vec3 innerBottomLeft(extents.xMin()+lineWidth, extents.yMin()+lineWidth, extents.zMin());
    osg::Vec3 innerBottomRight(extents.xMax()-lineWidth, extents.yMin()+lineWidth, extents.zMin());
    osg::Vec3 innerTopLeft(extents.xMin()+lineWidth, extents.yMax()-lineWidth, extents.zMin());
    osg::Vec3 innerTopRight(extents.xMax()-lineWidth, extents.yMax()-lineWidth, extents.zMin());

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices.get());

    vertices->push_back( outerBottomLeft );   // 0
    vertices->push_back( outerBottomRight );  // 1
    vertices->push_back( outerTopLeft );      // 2
    vertices->push_back( outerTopRight );     // 3

    vertices->push_back( innerBottomLeft );  // 4
    vertices->push_back( innerBottomRight ); // 5
    vertices->push_back( innerTopLeft );     // 6
    vertices->push_back( innerTopRight );    // 7

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    geometry->setColorArray(colours.get(), osg::Array::BIND_PER_PRIMITIVE_SET);

    // bottom
    {
        colours->push_back(bottomColor);

        osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
        geometry->addPrimitiveSet(primitives.get());
        primitives->push_back(4);
        primitives->push_back(0);
        primitives->push_back(5);
        primitives->push_back(1);
    }

    // top
    {
        colours->push_back(topColor);

        osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
        geometry->addPrimitiveSet(primitives.get());
        primitives->push_back(2);
        primitives->push_back(6);
        primitives->push_back(3);
        primitives->push_back(7);
    }

    // left
    {
        colours->push_back(leftColor);

        osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
        geometry->addPrimitiveSet(primitives.get());
        primitives->push_back(2);
        primitives->push_back(0);
        primitives->push_back(6);
        primitives->push_back(4);
    }

    // right
    {
        colours->push_back(rightColor);

        osg::ref_ptr<osg::DrawElementsUShort> primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
        geometry->addPrimitiveSet(primitives.get());
        primitives->push_back(7);
        primitives->push_back(5);
        primitives->push_back(3);
        primitives->push_back(1);
    }

    return geometry.release();
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
    stateset->setAttributeAndModes( _disabledDepthWrite.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
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
