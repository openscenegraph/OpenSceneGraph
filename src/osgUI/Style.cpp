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
#include <osg/io_utils>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Depth>
#include <osg/TexGen>
#include <osg/AlphaFunc>
#include <osg/MatrixTransform>
#include <osg/ComputeBoundsVisitor>
#include <osgUtil/Optimizer>
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

osg::Node* Style::createFrame(const osg::BoundingBox& extents, const FrameSettings* frameSettings, const osg::Vec4& color)
{
    // OSG_NOTICE<<"createFrame"<<std::endl;

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setName("Frame");

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
    // OSG_NOTICE<<"createText"<<std::endl;

    osg::Vec4 textColor(0.0f,0.0,0.0f,1.0);

    osg::ref_ptr<osgText::Text> textDrawable = new osgText::Text;
    textDrawable->setName("Text");

    textDrawable->setText(text);
    textDrawable->setEnableDepthWrites(false);
    textDrawable->setColor(textColor);

    if (ts)
    {
        textDrawable->setFont(ts->getFont());
        textDrawable->setCharacterSize(ts->getCharacterSize());
    }

    AlignmentSettings::Alignment alignment = as ? as->getAlignment() : AlignmentSettings::CENTER_CENTER;
    textDrawable->setAlignment(static_cast<osgText::TextBase::AlignmentType>(alignment));

    switch(alignment)
    {
        case(AlignmentSettings::LEFT_TOP):
            textDrawable->setPosition( osg::Vec3(extents.xMin(), extents.yMax(), extents.zMin()) );
            break;
        case(AlignmentSettings::LEFT_CENTER):
            textDrawable->setPosition( osg::Vec3(extents.xMin(), (extents.yMin()+extents.yMax())*0.5f, extents.zMin()) );
            break;
        case(AlignmentSettings::LEFT_BOTTOM):
            textDrawable->setPosition( osg::Vec3(extents.xMin(), extents.yMin(), extents.zMin()) );
            break;

        case(AlignmentSettings::CENTER_TOP):
            textDrawable->setPosition( osg::Vec3((extents.xMin()+extents.xMax())*0.5f, extents.yMax(), extents.zMin()) );
            break;
        case(AlignmentSettings::CENTER_CENTER):
            textDrawable->setPosition( osg::Vec3((extents.xMin()+extents.xMax())*0.5f, (extents.yMin()+extents.yMax())*0.5f, extents.zMin()) );
            break;
        case(AlignmentSettings::CENTER_BOTTOM):
            textDrawable->setPosition( osg::Vec3((extents.xMin()+extents.xMax())*0.5f, extents.yMin(), extents.zMin()) );
            break;

        case(AlignmentSettings::RIGHT_TOP):
            textDrawable->setPosition( osg::Vec3(extents.xMax(), extents.yMax(), extents.zMin()) );
            break;
        case(AlignmentSettings::RIGHT_CENTER):
            textDrawable->setPosition( osg::Vec3(extents.xMax(), (extents.yMin()+extents.yMax())*0.5f, extents.zMin()) );
            break;
        case(AlignmentSettings::RIGHT_BOTTOM):
            textDrawable->setPosition( osg::Vec3(extents.xMax(), extents.yMin(), extents.zMin()) );
            break;

        case(AlignmentSettings::LEFT_BASE_LINE):
            OSG_NOTICE<<"Text : LEFT_BASE_LINE"<<std::endl;
            textDrawable->setPosition( osg::Vec3(extents.xMin(), (extents.yMin()+extents.yMax())*0.5f-textDrawable->getCharacterHeight()*0.5f, extents.zMin()) );
            break;
        case(AlignmentSettings::CENTER_BASE_LINE):
            textDrawable->setPosition( osg::Vec3((extents.xMin()+extents.xMax())*0.5f, (extents.yMin()+extents.yMax())*0.5f-textDrawable->getCharacterHeight()*0.5, extents.zMin()) );
            break;
        case(AlignmentSettings::RIGHT_BASE_LINE):
            textDrawable->setPosition( osg::Vec3(extents.xMax(), (extents.yMin()+extents.yMax())*0.5f-textDrawable->getCharacterHeight()*0.5, extents.zMin()) );
            break;

        case(AlignmentSettings::LEFT_BOTTOM_BASE_LINE):
        case(AlignmentSettings::CENTER_BOTTOM_BASE_LINE):
        case(AlignmentSettings::RIGHT_BOTTOM_BASE_LINE):

        default:
            textDrawable->setPosition( osg::Vec3(extents.xMin(), extents.yMin(), extents.zMin()) );
            break;
    }

    return textDrawable.release();
}

osg::Node* Style::createIcon(const osg::BoundingBox& extents, const std::string& filename, const osg::Vec4& color)
{
    osg::ref_ptr<osg::Object> object = osgDB::readRefObjectFile(filename);
    if (!object)
    {
        //OSG_NOTICE<<"Warning: Style::createIcon(.., "<<filename<<") could not find icon file."<<std::endl;
        //return 0;
    }

    osg::ref_ptr<osg::Image> image = dynamic_cast<osg::Image*>(object.get());
    if (image.valid())
    {
        osg::Vec3 center(extents.center());
        float width = extents.xMax()-extents.xMin();
        float height = extents.yMax()-extents.yMin();
        float extentsAspectRatio = height/width;

        float imageAspectRatio = static_cast<float>(image->t())/static_cast<float>(image->s());
        if (imageAspectRatio>extentsAspectRatio) width *= (extentsAspectRatio/imageAspectRatio);
        else height *= (imageAspectRatio/extentsAspectRatio);

        osg::ref_ptr<osg::Geometry> geometry = osg::createTexturedQuadGeometry(osg::Vec3(center.x()-width*0.5f,center.y()-height*0.5f,center.z()),
                                                                               osg::Vec3(width, 0.0f, 0.0f),
                                                                               osg::Vec3(0.0f, height, 0.0f));

        osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
        colors->push_back(color);
        geometry->setColorArray(colors.get(), osg::Array::BIND_OVERALL);

        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image.get());

        osg::ref_ptr<osg::StateSet> stateset = geometry->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::ON);

        if (image->isImageTranslucent())
        {
            stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
        }

        return geometry.release();
    }

    osg::ref_ptr<osg::Node> node = dynamic_cast<osg::Node*>(object.get());
    if (!node)
    {
        OSG_NOTICE<<"Warning: Style::createIcon(.., "<<filename<<") could not find icon file."<<std::endl;

        osg::ref_ptr<osg::ShapeDrawable> ds = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0,0.0,0.0),1.0));

        node = ds.get();

        //return 0;
    }

    osg::ComputeBoundsVisitor cbv;
    node->accept(cbv);
    osg::BoundingBox bb = cbv.getBoundingBox();
    osg::Vec3 bb_size(bb.xMax()-bb.xMin(), bb.zMax()-bb.zMin(), bb.zMax()-bb.zMin());

    osg::Vec3 scale( (bb_size.x()>0) ? (extents.xMax()-extents.xMin())/bb_size.x() : 1.0f,
                     (bb_size.y()>0) ? (extents.yMax()-extents.yMin())/bb_size.y() : 1.0f,
                     (bb_size.z()>0) ? (extents.zMax()-extents.zMin())/bb_size.z() : 1.0f);

    float minNonZeroScale = scale.x();
    if (scale.y()!=0.0 && scale.y()<minNonZeroScale) minNonZeroScale = scale.y();
    if (scale.z()!=0.0 && scale.z()<minNonZeroScale) minNonZeroScale = scale.z();

    scale.set(minNonZeroScale, minNonZeroScale, minNonZeroScale);

    // create Transform to rescale subgraph
    osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;
    transform->setMatrix(osg::Matrix::translate(-bb.center()) *
                         osg::Matrix::scale(scale) *
                         osg::Matrix::translate(extents.center()));


    transform->setDataVariance(osg::Transform::STATIC);
    transform->addChild(node.get());

    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->addChild(transform.get());

    {
        osgUtil::Optimizer::FlattenStaticTransformsVisitor fstv;
        group->accept(fstv);
        fstv.removeTransforms(group.get());
    }

    if (group->getNumChildren()==1)
    {
        node = group->getChild(0);

        // remove references to avoid node from node being unreferenced after the node ref_ptr<> is released().
        group = 0;
        transform = 0;

        return node.release();
    }
    else
    {
        OSG_NOTICE<<"Warning: Style::createIcon(.., "<<filename<<"), error in creation of icon."<<std::endl;
        return 0;
    }
}

osg::Node* Style::createPanel(const osg::BoundingBox& extents, const osg::Vec4& colour)
{
    // OSG_NOTICE<<"createPanel"<<std::endl;

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setName("Panel");

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices.get());

    vertices->push_back( osg::Vec3(extents.xMin(), extents.yMin(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMin(), extents.yMax(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMax(), extents.yMin(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMax(), extents.yMax(), extents.zMin()) );

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    geometry->setColorArray(colours.get(), osg::Array::BIND_OVERALL);

    colours->push_back( colour );

    geometry->addPrimitiveSet( new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 4) );

    return geometry.release();
}


osg::Node* Style::createDepthSetPanel(const osg::BoundingBox& extents)
{
    // OSG_NOTICE<<"createDepthSetPanel"<<std::endl;

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setName("DepthSetPanel");

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices.get());

    vertices->push_back( osg::Vec3(extents.xMin(), extents.yMin(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMin(), extents.yMax(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMax(), extents.yMin(), extents.zMin()) );
    vertices->push_back( osg::Vec3(extents.xMax(), extents.yMax(), extents.zMin()) );

    geometry->addPrimitiveSet( new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 4) );

    osg::ref_ptr<osg::StateSet> stateset = geometry->getOrCreateStateSet();
    stateset->setAttributeAndModes( _enabledDepthWrite.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
    stateset->setAttributeAndModes( _disableColorWriteMask.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
    stateset->setRenderBinDetails(20, "TraversalOrderBin", osg::StateSet::OVERRIDE_PROTECTED_RENDERBIN_DETAILS);
    stateset->setNestRenderBins(false);

    return geometry.release();
}


void Style::setupDialogStateSet(osg::StateSet* stateset, int binNum)
{
    stateset->setRenderBinDetails(binNum, "TraversalOrderBin", osg::StateSet::OVERRIDE_PROTECTED_RENDERBIN_DETAILS);
    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    stateset->setAttributeAndModes( _disabledDepthWrite.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    stateset->setNestRenderBins(false);
}

void Style::setupPopupStateSet(osg::StateSet* /*stateset*/, int /*binNum*/)
{
}

void Style::setupClipStateSet(const osg::BoundingBox& extents, osg::StateSet* stateset)
{
    unsigned int clipTextureUnit = 1;

    stateset->setAttributeAndModes( new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.0f), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    stateset->setTextureAttributeAndModes( clipTextureUnit, _clipTexture.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    osg::Matrixd matrix = osg::Matrixd::translate(osg::Vec3(-extents.xMin(), -extents.yMin(), -extents.zMin()))*
                          osg::Matrixd::scale(osg::Vec3(1.0f/(extents.xMax()-extents.xMin()), 1.0f/(extents.yMax()-extents.yMin()), 1.0f));

    OSG_NOTICE<<"setupClipState("
            <<extents.xMin()<<", "<<extents.yMin()<<", "<<extents.zMin()<<", "
            <<extents.xMax()<<", "<<extents.yMax()<<", "<<extents.zMax()<<")"<<std::endl;


    osg::ref_ptr<osg::TexGen> texgen = new osg::TexGen;
    texgen->setPlanesFromMatrix(matrix);
    texgen->setMode(osg::TexGen::OBJECT_LINEAR);
    stateset->setTextureAttributeAndModes( clipTextureUnit, texgen.get(), osg::StateAttribute::ON);
}
