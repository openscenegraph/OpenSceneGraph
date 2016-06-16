/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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

//
// OpenFlight loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include <assert.h>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/CullFace>
#include <osg/BlendFunc>
#include <osgUtil/TransformAttributeFunctor>
#include "Registry.h"
#include "Document.h"
#include "RecordInputStream.h"

#include <osg/ValueObject>

#include <algorithm>

namespace flt {

template<class ARRAY>
void reverseWindingOrder( ARRAY* data, GLenum mode, GLint first, GLint last )
{
    switch( mode )
    {
    case osg::PrimitiveSet::TRIANGLES:
    case osg::PrimitiveSet::QUADS:
    case osg::PrimitiveSet::POLYGON:
        // reverse all the vertices.
        std::reverse(data->begin()+first, data->begin()+last);
        break;
    case osg::PrimitiveSet::TRIANGLE_STRIP:
    case osg::PrimitiveSet::QUAD_STRIP:
        // reverse only the shared edges.
        for( GLint i = first; i < last-1; i+=2 )
        {
            std::swap( (*data)[i], (*data)[i+1] );
        }
        break;
    case osg::PrimitiveSet::TRIANGLE_FAN:
        // reverse all vertices except the first vertex.
        std::reverse(data->begin()+first+1, data->begin()+last);
        break;
    }
}

void addDrawableAndReverseWindingOrder( osg::Geode* geode )
{
    // Replace double sided polygons by duplicating the drawables and inverting the normals.
    std::vector<osg::Geometry*> new_drawables;

    for (size_t di=0; di<geode->getNumDrawables(); ++di)
    {
        const osg::Geometry* geometry = dynamic_cast<const osg::Geometry*>(geode->getDrawable(di));
        if(geometry)
        {
            osg::Geometry* geom = new osg::Geometry(*geometry
                , osg::CopyOp::DEEP_COPY_ARRAYS | osg::CopyOp::DEEP_COPY_PRIMITIVES);
            new_drawables.push_back(geom);

            for( size_t pi = 0; pi < geom->getNumPrimitiveSets( ); ++pi )
            {
                osg::DrawArrays* drawarray = dynamic_cast<osg::DrawArrays*>( geom->getPrimitiveSet( pi ) );
                if( drawarray )
                {
                    GLint first = drawarray->getFirst();
                    GLint last  = drawarray->getFirst()+drawarray->getCount();

                    // Invert vertex order.
                    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
                    if( vertices )
                    {
                        reverseWindingOrder( vertices, drawarray->getMode(), first, last );
                    }

                    if( osg::getBinding(geom->getNormalArray()) == osg::Array::BIND_PER_VERTEX )
                    {
                        osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geom->getNormalArray());
                        if( normals )
                        {
                            // First, invert the direction of the normals.
                            for( GLint i = first; i < last; ++i )
                            {
                                (*normals)[i] = -(*normals)[i];
                            }
                            reverseWindingOrder( normals, drawarray->getMode(), first, last );
                        }
                    }

                    if( osg::getBinding(geom->getColorArray()) == osg::Array::BIND_PER_VERTEX )
                    {
                        osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
                        if( colors )
                        {
                            reverseWindingOrder( colors, drawarray->getMode(), first, last );
                        }
                    }

                    for( size_t i = 0; i < geom->getNumTexCoordArrays(); ++i )
                    {
                        osg::Vec2Array* UVs = dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(i));
                        if( UVs )
                        {
                            reverseWindingOrder( UVs, drawarray->getMode(), first, last );
                        }
                    }
                }
            }
        }
    }

    // Now add the new geometry drawable.
    for( size_t i = 0; i < new_drawables.size( ); ++i )
    {
        geode->addDrawable( new_drawables[i] );
    }
}

/* Face record
 */
class Face : public PrimaryRecord
{
    // flags
    static const unsigned int TERRAIN_BIT      = 0x80000000u >> 0;
    static const unsigned int NO_COLOR_BIT     = 0x80000000u >> 1;
    static const unsigned int NO_ALT_COLOR_BIT = 0x80000000u >> 2;
    static const unsigned int PACKED_COLOR_BIT = 0x80000000u >> 3;
    static const unsigned int FOOTPRINT_BIT    = 0x80000000u >> 4;    // Terrain culture cutout
    static const unsigned int HIDDEN_BIT       = 0x80000000u >> 5;
    static const unsigned int ROOFLINE_BIT     = 0x80000000u >> 6;

    osg::Vec4   _primaryColor;
    uint8       _drawFlag;
    uint8       _template;
    uint16      _transparency;
    uint32      _flags;
    uint8       _lightMode;

    osg::ref_ptr<osg::Geode> _geode;
    osg::ref_ptr<osg::Geometry> _geometry;

public:

    Face() :
        _primaryColor(1,1,1,1),
        _drawFlag(SOLID_NO_BACKFACE),
        _template(FIXED_NO_ALPHA_BLENDING),
        _transparency(0),
        _flags(0),
        _lightMode(FACE_COLOR)
    {
    }

    META_Record(Face)

    META_setID(_geode)
    META_setComment(_geode)
    META_setMultitexture(_geode)

    // draw mode
    enum DrawMode
    {
        SOLID_BACKFACED = 0,
        SOLID_NO_BACKFACE = 1,
        WIREFRAME_CLOSED = 2,
        WIREFRAME_NOT_CLOSED = 3,
        SURROUND_ALTERNATE_COLOR = 4,
        OMNIDIRECTIONAL_LIGHT = 8,
        UNIDIRECTIONAL_LIGHT = 9,
        BIDIRECTIONAL_LIGHT = 10
    };

    inline DrawMode getDrawMode() const { return (DrawMode)_drawFlag; }

    // lighting
    enum LightMode
    {
        FACE_COLOR = 0,
        VERTEX_COLOR = 1,
        FACE_COLOR_LIGHTING = 2,
        VERTEX_COLOR_LIGHTING = 3
    };

    inline LightMode getLightMode() const { return (LightMode)_lightMode; }
    inline bool isLit() const { return (_lightMode==FACE_COLOR_LIGHTING) || (_lightMode==VERTEX_COLOR_LIGHTING); }
    inline bool isGouraud() const { return (_lightMode==VERTEX_COLOR) || (_lightMode==VERTEX_COLOR_LIGHTING); }

    // flags
    inline bool noColor()         const { return (_flags & NO_COLOR_BIT)!=0; }
    inline bool isHidden()        const { return (_flags & HIDDEN_BIT)!=0; }
    inline bool isTerrain()       const { return (_flags & TERRAIN_BIT)!=0; }
    inline bool isFootprint()     const { return (_flags & FOOTPRINT_BIT)!=0; }
    inline bool isRoofline()      const { return (_flags & ROOFLINE_BIT)!=0; }
    inline bool packedColorMode() const { return (_flags & PACKED_COLOR_BIT)!=0; }

    // billboard
    enum TemplateMode
    {
        FIXED_NO_ALPHA_BLENDING = 0,
        FIXED_ALPHA_BLENDING = 1,
        AXIAL_ROTATE_WITH_ALPHA_BLENDING = 2,
        POINT_ROTATE_WITH_ALPHA_BLENDING = 4
    };

    inline TemplateMode getTemplateMode() const { return (TemplateMode)_template; }

    // transparency & alpha
    inline bool isAlphaBlend() const
    {
        return (_template==FIXED_ALPHA_BLENDING) ||
               (_template==AXIAL_ROTATE_WITH_ALPHA_BLENDING) ||
               (_template==POINT_ROTATE_WITH_ALPHA_BLENDING);
    }

    inline osg::Vec4 getPrimaryColor() const { return _primaryColor; }
    inline float getTransparency() const { return (float)_transparency / 65535.0f; }
    inline bool isTransparent() const { return _transparency > 0; }

    virtual void addChild(osg::Node& child)
    {
        // Add subface to parent.
        if (_parent.valid())
            _parent->addChild(child);
    }

    virtual void addVertex(Vertex& vertex)
    {
        osg::Vec3Array* vertices = getOrCreateVertexArray(*_geometry);
        vertices->push_back(vertex._coord);

        if (isGouraud())
        {
            osg::Vec4Array* colors = getOrCreateColorArray(*_geometry);
            if (vertex.validColor())
            {
                colors->push_back(vertex._color);
            }
            else
            {
                // Use face color if vertex color is -1 in a gouraud polygon.
                // http://www.multigen-paradigm.com/ubb/Forum1/HTML/000967.html
                // Incorporate Face transparency per osg-users thread "Open Flight
                // characteristic not reflected in the current OSG" (Sept/Oct 2011)
                colors->push_back(osg::Vec4(_primaryColor.r(), _primaryColor.g(),
                    _primaryColor.b(), ( 1.0 - getTransparency() ) ));
            }
        }

        bool strict = false; // prepare for "strict" reader option.
        if (strict)
        {
            if (vertex.validNormal())
            {
                osg::Vec3Array* normals = getOrCreateNormalArray(*_geometry);
                normals->push_back(vertex._normal);
            }
        }
        else
        {
            // Add normal only if lit.
            if (isLit())
            {
                osg::Vec3Array* normals = getOrCreateNormalArray(*_geometry);

                if (vertex.validNormal())
                    normals->push_back(vertex._normal);
                else // if lit and no normal in Vertex
                {
                    // Use previous normal if available.
                    if (normals->empty())
                        normals->push_back(osg::Vec3(0,0,1));
                    else
                        normals->push_back(normals->back());
                }
            }
        }

        for (int layer=0; layer<Vertex::MAX_LAYERS; layer++)
        {
            if (vertex.validUV(layer))
            {
                osg::Vec2Array* UVs = getOrCreateTextureArray(*_geometry,layer);
                UVs->push_back(vertex._uv[layer]);
            }
        }
    }

    virtual void addVertexUV(int unit, const osg::Vec2& uv)
    {
        osg::Vec2Array* UVs = getOrCreateTextureArray(*_geometry,unit);
        UVs->push_back(uv);
    }

    virtual void addMorphVertex(Vertex& vertex0, Vertex& /*vertex100*/)
    {
        osg::Vec3Array* vertices = getOrCreateVertexArray(*_geometry);
        vertices->push_back(vertex0._coord);

        if (isGouraud())
        {
            osg::Vec4Array* colors = getOrCreateColorArray(*_geometry);
            if (vertex0.validColor())
            {
                colors->push_back(vertex0._color);
            }
            else
            {
                // Use face color if vertex color is -1 in a gouraud polygon.
                // http://www.multigen-paradigm.com/ubb/Forum1/HTML/000967.html
                colors->push_back(_primaryColor);
            }
        }

        if (vertex0.validNormal())
        {
            osg::Vec3Array* normals = getOrCreateNormalArray(*_geometry);
            normals->push_back(vertex0._normal);
        }

        for (int layer=0; layer<Vertex::MAX_LAYERS; layer++)
        {
            if (vertex0.validUV(layer))
            {
                osg::Vec2Array* UVs = getOrCreateTextureArray(*_geometry,layer);
                UVs->push_back(vertex0._uv[layer]);
            }
        }
    }

protected:

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        std::string id = in.readString(8);
        int32 IRColor = in.readInt32();
        /*int16 relativePriority =*/ in.readInt16();
        _drawFlag = in.readUInt8(SOLID_NO_BACKFACE);
        uint8 texturedWhite = in.readUInt8();
        int16 primaryNameIndex = in.readInt16(-1);
        /*int16 secondaryNameIndex =*/ in.readInt16(-1);
        in.forward(1);
        _template = in.readUInt8(FIXED_NO_ALPHA_BLENDING);
        /*int detailTexture =*/ in.readInt16(-1);
        int textureIndex = in.readInt16(-1);
        int materialIndex = in.readInt16(-1);
        int16 surface = in.readInt16();
        int16 feature = in.readInt16();
        int32 IRMaterial = in.readInt32();
        _transparency = in.readUInt16(0);
        // version > 13
        /*uint8 influenceLOD =*/ in.readUInt8();
        /*uint8 linestyle =*/ in.readUInt8();
        _flags = in.readUInt32(0);
        _lightMode = in.readUInt8(FACE_COLOR);
        in.forward(7);
        osg::Vec4 primaryPackedColor = in.readColor32();
        /*osg::Vec4 secondaryPackedColor =*/ in.readColor32();
        // version >= VERSION_15_1
        /*int textureMappingIndex =*/ in.readInt16(-1);
        in.forward(2);
        int primaryColorIndex = in.readInt32(-1);
        /*int alternateColorIndex =*/ in.readInt32(-1);
        // version >= 16
        in.forward(2);
        int shaderIndex = in.readInt16(-1);

        // Create Geode or Billboard.
        switch (_template)
        {
        case AXIAL_ROTATE_WITH_ALPHA_BLENDING:
            {
                osg::Billboard* billboard = new osg::Billboard;
                billboard->setMode(osg::Billboard::AXIAL_ROT);
                _geode = billboard;
            }
            break;
        case POINT_ROTATE_WITH_ALPHA_BLENDING:
            {
                osg::Billboard* billboard = new osg::Billboard;
                billboard->setMode(osg::Billboard::POINT_ROT_WORLD);
                _geode = billboard;
            }
            break;
        default:
            _geode = new osg::Geode;
        }

        _geode->setDataVariance(osg::Object::STATIC);
        _geode->setName(id);

        _geometry = new osg::Geometry;
        _geometry->setDataVariance(osg::Object::STATIC);
        _geode->addDrawable(_geometry.get());

        // StateSet
        osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;

        // Hidden
        if (isHidden())
            _geode->setNodeMask(0);

        // Face color
        if (texturedWhite!=0 && textureIndex>=0)
        {
            _primaryColor = osg::Vec4(1,1,1,1);
        }
        else
        {
            if (packedColorMode())
            {
                _primaryColor = primaryPackedColor;
            }
            else
            {
                if (document.version() < VERSION_15_1)
                    _primaryColor = document.getColorPool()->getColor(primaryNameIndex);

                else // >= VERSION_15_1
                    _primaryColor = document.getColorPool()->getColor(primaryColorIndex);
            }
        }

        // Lighting
        stateset->setMode(GL_LIGHTING, isLit() ? osg::StateAttribute::ON : osg::StateAttribute::OFF);

        // Material
        if (isLit() || materialIndex>=0)
        {
            // MaterialPool will return a "default" material if no material is defined for materialIndex.
            // http://www.multigen-paradigm.com/ubb/Forum1/HTML/000228.html
            osg::Vec4 col = _primaryColor;
            col.a() = 1.0f - getTransparency();
            osg::Material* material = document.getOrCreateMaterialPool()->getOrCreateMaterial(materialIndex,col);
            stateset->setAttribute(material);
        }

        // IRColor (IRC)
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != IRColor)
        {
          _geometry->setUserValue("<UA:IRC>", IRColor);
        }

        // IR Material ID (IRM)
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != IRMaterial)
        {
          _geometry->setUserValue("<UA:IRM>", IRMaterial);
        }

        // surface (SMC)
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != surface)
        {
          _geometry->setUserValue("<UA:SMC>", surface);
        }

        // feature (FID)
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != feature)
        {
          _geometry->setUserValue("<UA:FID>", feature);
        }
 
        // terrain 
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != isTerrain())
        {
          _geometry->setUserValue("<UA:Terrain>", true);
        }

        // roofline 
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != isRoofline())
        {
          _geometry->setUserValue("<UA:Roofline>", true);
        }
    
        // footprint 
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != isFootprint())
        {
          _geometry->setUserValue("<UA:Footprint>", true);
        }

        // Shaders
        if (shaderIndex >= 0)
        {
            ShaderPool* sp = document.getOrCreateShaderPool();
            osg::Program* program = sp->get(shaderIndex);
            if (program)
                stateset->setAttributeAndModes(program, osg::StateAttribute::ON);
        }

         // Texture
        TexturePool* tp = document.getOrCreateTexturePool();
        osg::StateSet* textureStateSet = tp->get(textureIndex);
        if (textureStateSet)
        {
            // Merge face stateset with texture stateset
            stateset->merge(*textureStateSet);
        }

        // Cull face
        switch(_drawFlag)
        {
        case SOLID_BACKFACED:     // Enable backface culling
        {
            static osg::ref_ptr<osg::CullFace> cullFace = new osg::CullFace(osg::CullFace::BACK);
            stateset->setAttributeAndModes(cullFace.get(), osg::StateAttribute::ON);
            break;
        }
        case SOLID_NO_BACKFACE:   // Disable backface culling
            if( document.getReplaceDoubleSidedPolys( ) )
            {
                static osg::ref_ptr<osg::CullFace> cullFace = new osg::CullFace(osg::CullFace::BACK);
                stateset->setAttributeAndModes(cullFace.get(), osg::StateAttribute::ON);
            }
            else
            {
                stateset->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
            }
            break;
        }

        // Subface
        if (document.subfaceLevel() > 0)
        {
            stateset->setAttributeAndModes(document.getSubSurfacePolygonOffset(document.subfaceLevel()), osg::StateAttribute::ON);
            stateset->setAttribute(document.getSubSurfaceDepth());

            stateset->setRenderBinDetails(document.subfaceLevel(),"RenderBin");
        }

        _geode->setStateSet(stateset.get());

        // Add to parent.
        if (_parent.valid())
            _parent->addChild(*_geode);
    }

    osg::PrimitiveSet::Mode getPrimitiveSetMode(int numVertices)
    {
        switch(getDrawMode())
        {
            case WIREFRAME_NOT_CLOSED:
                return osg::PrimitiveSet::LINE_STRIP;
            case WIREFRAME_CLOSED:
                return osg::PrimitiveSet::LINE_LOOP;
            case OMNIDIRECTIONAL_LIGHT:
            case UNIDIRECTIONAL_LIGHT:
            case BIDIRECTIONAL_LIGHT:
                return osg::PrimitiveSet::POINTS;
            default: break;
        }

        switch (numVertices)
        {
            case 1: return osg::PrimitiveSet::POINTS;
            case 2: return osg::PrimitiveSet::LINES;
            case 3: return osg::PrimitiveSet::TRIANGLES;
            case 4: return osg::PrimitiveSet::QUADS;
            default: break;
        }

        return osg::PrimitiveSet::POLYGON;
    }

    virtual void dispose(Document& document)
    {
        if (_geode.valid())
        {
            // Insert transform(s)
            if (_matrix.valid())
            {
                insertMatrixTransform(*_geode,*_matrix,_numberOfReplications);
            }

            // Add primitives, set bindings etc.
            for (unsigned int i=0; i<_geode->getNumDrawables(); ++i)
            {
                osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(_geode->getDrawable(i));
                if (geometry)
                {
                    osg::Array* vertices = geometry->getVertexArray();
                    if (vertices)
                    {
                        GLint first = 0;
                        GLsizei count = vertices->getNumElements();
                        osg::PrimitiveSet::Mode mode = getPrimitiveSetMode(count);
                        geometry->addPrimitiveSet(new osg::DrawArrays(mode,first,count));
                    }

                    // Color binding
                    if (isGouraud())
                    {
                        // Color per vertex
                        if (geometry->getColorArray()) geometry->getColorArray()->setBinding(osg::Array::BIND_PER_VERTEX);
                    }
                    else
                    {
                        // Color per face
                        osg::Vec4 col = getPrimaryColor();
                        col[3] = 1.0f - getTransparency();

                        osg::Vec4Array* colors = new osg::Vec4Array(1);
                        (*colors)[0] = col;
                        geometry->setColorArray(colors, osg::Array::BIND_OVERALL);
                    }

                    // Normal binding
                    if (isLit())
                    {
                        if (geometry->getNormalArray()) geometry->getNormalArray()->setBinding(osg::Array::BIND_PER_VERTEX);
                    }
                    else
                    {
                        geometry->setNormalArray(0);
                    }
                }
            }

            if( getDrawMode( ) == SOLID_NO_BACKFACE && document.getReplaceDoubleSidedPolys( ) )
            {
                addDrawableAndReverseWindingOrder( _geode.get() );
            }

            osg::StateSet* stateset =  _geode->getOrCreateStateSet();

            // Translucent image?
            bool isImageTranslucent = false;
            if (document.getUseTextureAlphaForTransparancyBinning())
            {
                for (unsigned int i=0; i<stateset->getTextureAttributeList().size(); ++i)
                {
                    osg::StateAttribute* sa = stateset->getTextureAttribute(i,osg::StateAttribute::TEXTURE);
                    osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(sa);
                    if (texture)
                    {
                        osg::Image* image = texture->getImage();
                        if (image && image->isImageTranslucent())
                            isImageTranslucent = true;
                    }
                }
            }

            // Transparent Material?
            bool isMaterialTransparent = false;
            osg::Material* material = dynamic_cast<osg::Material*>(stateset->getAttribute(osg::StateAttribute::MATERIAL));
            if (material)
            {
                isMaterialTransparent = material->getDiffuse(osg::Material::FRONT).a() < 0.99f;
            }

            // Enable alpha blend?
            if (isAlphaBlend() || isTransparent() || isImageTranslucent || isMaterialTransparent)
            {
                static osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
                stateset->setAttributeAndModes(blendFunc.get(), osg::StateAttribute::ON);
                stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            }

            if (document.getUseBillboardCenter())
            {
                // Set billboard rotation point to center of face.
                osg::Billboard* billboard = dynamic_cast<osg::Billboard*>(_geode.get());
                if (billboard)
                {
                    for (unsigned int i=0; i<billboard->getNumDrawables(); ++i)
                    {
                        const osg::BoundingBox& bb = billboard->getDrawable(i)->getBoundingBox();
                        billboard->setPosition(i,bb.center());

                        osgUtil::TransformAttributeFunctor tf(osg::Matrix::translate(-bb.center()));
                        billboard->getDrawable(i)->accept(tf);

                        billboard->getDrawable(i)->dirtyBound();
                    }

                    billboard->dirtyBound();
                }
            }
        }
    }
};

REGISTER_FLTRECORD(Face, FACE_OP)



/** VertexList -
  * The VertexList is a leaf record.
  * Possible parents: Face, Mesh & LightPoint
  */
class VertexListRecord : public PrimaryRecord
{
public:

    VertexListRecord() {}

    META_Record(VertexListRecord)

    virtual void addVertex(Vertex& vertex)
    {
        // forward vertex to parent.
        if (_parent.valid())
            _parent->addVertex(vertex);
    }

    virtual void addVertexUV(int layer,const osg::Vec2& uv)
    {
        // forward uv to parent.
        if (_parent.valid())
            _parent->addVertexUV(layer,uv);
    }

protected:

    virtual ~VertexListRecord() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        VertexPool* vp = document.getVertexPool();
        if (vp)
        {
            int vertices = (in.getRecordSize()-4) / 4;

            // Use the Vertex pool as a record stream.
            RecordInputStream inVP(vp->rdbuf());
            for (int n=0; n<vertices; n++)
            {
                // Get position of vertex.
                uint32 pos = in.readUInt32();

                // Get vertex from vertex pool.
                inVP.seekg((std::istream::pos_type)pos);
                inVP.readRecord(document);
            }
        }
    }
};


REGISTER_FLTRECORD(VertexListRecord, VERTEX_LIST_OP)



/** MorphVertexList -
  * The MorphVertexList is a leaf record.
  */
class MorphVertexList : public PrimaryRecord
{
    enum Mode
    {
        UNDEFINED,
        MORPH_0,
        MORPH_100
    };

    Mode _mode;
    Vertex _vertex0;
    Vertex _vertex100;

public:

    MorphVertexList():
        _mode(UNDEFINED)
    {
    }

    META_Record(MorphVertexList)

    virtual void addVertex(Vertex& vertex)
    {
        switch (_mode)
        {
        case MORPH_0:
            _vertex0 = vertex;
            break;
        case MORPH_100:
            _vertex100 = vertex;

            // forward vertex to parent.
            if (_parent.valid())
                _parent->addMorphVertex(_vertex0, _vertex100);
            break;
        case UNDEFINED:
            break;
        }
    }

    //virtual void addVertexUV(int layer,const osg::Vec2& uv)
    //{
    //    // forward uv to parent.
    //    if (_parent.valid())
    //        _parent->addVertexUV(layer,uv);
    //}

protected:

    virtual ~MorphVertexList() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        VertexPool* vp = document.getVertexPool();
        if (vp)
        {
            int vertices = (in.getRecordSize()-4) / 8;

            // Use the Vertex pool as a record stream.
            RecordInputStream inVP(vp->rdbuf());
            for (int n=0; n<vertices; n++)
            {
                // Get position of vertex.
                uint32 offset0 = in.readUInt32();
                uint32 offset100 = in.readUInt32();

                // Get vertex from vertex pool.

                // 0%
                _mode = MORPH_0;
                inVP.seekg((std::istream::pos_type)offset0);
                inVP.readRecord(document);

                // 100%
                _mode = MORPH_100;
                inVP.seekg((std::istream::pos_type)offset100);
                inVP.readRecord(document);
            }
        }
    }
};

REGISTER_FLTRECORD(MorphVertexList, MORPH_VERTEX_LIST_OP)



/* Mesh record
 */
class Mesh : public PrimaryRecord
{
    // flags
    static const unsigned int TERRAIN_BIT      = 0x80000000u >> 0;
    static const unsigned int NO_COLOR_BIT     = 0x80000000u >> 1;
    static const unsigned int NO_ALT_COLOR_BIT = 0x80000000u >> 2;
    static const unsigned int PACKED_COLOR_BIT = 0x80000000u >> 3;
    static const unsigned int FOOTPRINT_BIT    = 0x80000000u >> 4;    // Terrain culture cutout
    static const unsigned int HIDDEN_BIT       = 0x80000000u >> 5;
    static const unsigned int ROOFLINE_BIT     = 0x80000000u >> 6;

    osg::Vec4   _primaryColor;
    uint8       _drawFlag;
    uint8       _template;
    uint16      _transparency;
    uint32      _flags;
    uint8       _lightMode;

    osg::ref_ptr<osg::Geode> _geode;

public:

    Mesh() :
        _primaryColor(1,1,1,1),
        _drawFlag(SOLID_NO_BACKFACE),
        _template(FIXED_NO_ALPHA_BLENDING),
        _transparency(0),
        _flags(0),
        _lightMode(FACE_COLOR)
    {
    }

    META_Record(Mesh)

    META_setID(_geode)
    META_setComment(_geode)
    META_setMultitexture(_geode)

    // draw mode
    enum DrawMode
    {
        SOLID_BACKFACED = 0,
        SOLID_NO_BACKFACE = 1,
        WIREFRAME_CLOSED = 2,
        WIREFRAME_NOT_CLOSED = 3,
        SURROUND_ALTERNATE_COLOR = 4,
        OMNIDIRECTIONAL_LIGHT = 8,
        UNIDIRECTIONAL_LIGHT = 9,
        BIDIRECTIONAL_LIGHT = 10
    };

    inline DrawMode getDrawMode() const { return (DrawMode)_drawFlag; }

    // lighting
    enum LightMode
    {
        FACE_COLOR = 0,
        VERTEX_COLOR = 1,
        FACE_COLOR_LIGHTING = 2,
        VERTEX_COLOR_LIGHTING = 3
    };

    inline LightMode getLightMode() const { return (LightMode)_lightMode; }
    inline bool isLit() const { return (_lightMode==FACE_COLOR_LIGHTING) || (_lightMode==VERTEX_COLOR_LIGHTING); }
    inline bool isGouraud() const { return (_lightMode==VERTEX_COLOR) || (_lightMode==VERTEX_COLOR_LIGHTING); }

    // flags
    inline bool noColor()         const { return (_flags & NO_COLOR_BIT)!=0; }
    inline bool isHidden()        const { return (_flags & HIDDEN_BIT)!=0; }
    inline bool isTerrain()       const { return (_flags & TERRAIN_BIT)!=0; }
    inline bool isFootprint()     const { return (_flags & FOOTPRINT_BIT)!=0; }
    inline bool isRoofline()      const { return (_flags & ROOFLINE_BIT)!=0; }
    inline bool packedColorMode() const { return (_flags & PACKED_COLOR_BIT)!=0; }

    // billboard
    enum TemplateMode
    {
        FIXED_NO_ALPHA_BLENDING = 0,
        FIXED_ALPHA_BLENDING = 1,
        AXIAL_ROTATE_WITH_ALPHA_BLENDING = 2,
        POINT_ROTATE_WITH_ALPHA_BLENDING = 4
    };

    inline TemplateMode getTemplateMode() const { return (TemplateMode)_template; }

    // transparency & alpha
    inline bool isAlphaBlend() const
    {
        return (_template==FIXED_ALPHA_BLENDING) ||
               (_template==AXIAL_ROTATE_WITH_ALPHA_BLENDING) ||
               (_template==POINT_ROTATE_WITH_ALPHA_BLENDING);
    }

    inline osg::Vec4 getPrimaryColor() const { return _primaryColor; }
    inline float getTransparency() const { return (float)_transparency / 65535.0f; }
    inline bool isTransparent() const { return _transparency > 0; }

    virtual void addChild(osg::Node& child)
    {
        // Add subface to parent.
        if (_parent.valid())
            _parent->addChild(child);
    }

    virtual void addGeometry(osg::Geometry& geometry)
    {
        _geode->addDrawable(&geometry);
    }

protected:

    virtual void readRecord(RecordInputStream& in, Document& document)
    {
        std::string id = in.readString(8);
        in.forward(4);
        int32 IRColor = in.readInt32();
        /*int16 relativePriority =*/ in.readInt16();
        _drawFlag = in.readUInt8(SOLID_NO_BACKFACE);
        uint8 texturedWhite = in.readUInt8();
        int16 primaryNameIndex = in.readInt16(-1);
        /*int16 secondaryNameIndex =*/ in.readInt16(-1);
        in.forward(1);
        _template = in.readUInt8(FIXED_NO_ALPHA_BLENDING);
        /*int detailTexture =*/ in.readInt16(-1);
        int textureIndex = in.readInt16(-1);
        int materialIndex = in.readInt16(-1);
        int16 surface = in.readInt16();
        int16 feature = in.readInt16();
        int32 IRMaterial = in.readInt32(-1);
        _transparency = in.readUInt16(0);
        // version > 13
        /*uint8 influenceLOD =*/ in.readUInt8();
        /*uint8 linestyle =*/ in.readUInt8();
        _flags = in.readUInt32(0);
        _lightMode = in.readUInt8(FACE_COLOR);
        in.forward(7);
        osg::Vec4 primaryPackedColor = in.readColor32();
        /*osg::Vec4 secondaryPackedColor =*/ in.readColor32();
        // version >= VERSION_15_1
        /*int textureMappingIndex =*/ in.readInt16(-1);
        in.forward(2);
        int primaryColorIndex = in.readInt32(-1);
        /*int alternateColorIndex =*/ in.readInt32(-1);
        // version >= 16
        in.forward(2);
        int shaderIndex = in.readInt16(-1);

        // Create Geode or Billboard.
        switch (_template)
        {
        case AXIAL_ROTATE_WITH_ALPHA_BLENDING:
            {
                osg::Billboard* billboard = new osg::Billboard;
                billboard->setMode(osg::Billboard::AXIAL_ROT);
                _geode = billboard;
            }
            break;
        case POINT_ROTATE_WITH_ALPHA_BLENDING:
            {
                osg::Billboard* billboard = new osg::Billboard;
                billboard->setMode(osg::Billboard::POINT_ROT_WORLD);
                _geode = billboard;
            }
            break;
        default:
            _geode = new osg::Geode;
        }

        _geode->setDataVariance(osg::Object::STATIC);
        _geode->setName(id);

        // StateSet
        osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;

        // Hidden
        if (isHidden())
            _geode->setNodeMask(0);

        // Face color
        if (texturedWhite!=0 && textureIndex>=0)
        {
            _primaryColor = osg::Vec4(1,1,1,1);
        }
        else
        {
            if (packedColorMode())
            {
                _primaryColor = primaryPackedColor;
            }
            else
            {
                if (document.version() < VERSION_15_1)
                    _primaryColor = document.getColorPool()->getColor(primaryNameIndex);

                else // >= VERSION_15_1
                    _primaryColor = document.getColorPool()->getColor(primaryColorIndex);
            }
        }

        // Lighting
        stateset->setMode(GL_LIGHTING, isLit() ? osg::StateAttribute::ON : osg::StateAttribute::OFF);

        // Material
        if (isLit() || materialIndex>=0)
        {
            // MaterialPool will return a "default" material if no material is defined for materialIndex.
            // http://www.multigen-paradigm.com/ubb/Forum1/HTML/000228.html
            osg::Vec4 col = _primaryColor;
            col.a() = 1.0f - getTransparency();
            osg::Material* material = document.getOrCreateMaterialPool()->getOrCreateMaterial(materialIndex,col);
            stateset->setAttribute(material);
        }

        // IRColor (IRC)
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != IRColor)
        {
          _geode->setUserValue("<UA:IRC>", IRColor);
        }

        // IR Material ID (IRM)
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != IRMaterial)
        {
          _geode->setUserValue("<UA:IRM>", IRMaterial);
        }

        // surface (SMC)
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != surface)
        {
          _geode->setUserValue("<UA:SMC>", surface);
        }

        // feature (FID)
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != feature)
        {
          _geode->setUserValue("<UA:FID>", feature);
        }

        // terrain
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != isTerrain())
        {
          _geode->setUserValue("<UA:Terrain>", true);
        }

        // roofline 
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != isRoofline())
        {
          _geode->setUserValue("<UA:Roofline>", true);
        }

        // footprint 
        if (document.getPreserveNonOsgAttrsAsUserData() && 0 != isFootprint())
        {
          _geode->setUserValue("<UA:Footprint>", true);
        }

        // Shaders
        if (shaderIndex >= 0)
        {
            ShaderPool* sp = document.getOrCreateShaderPool();
            osg::Program* program = sp->get(shaderIndex);
            if (program)
                stateset->setAttributeAndModes(program, osg::StateAttribute::ON);
        }

         // Texture
        TexturePool* tp = document.getOrCreateTexturePool();
        osg::StateSet* textureStateSet = tp->get(textureIndex);
        if (textureStateSet)
        {
            // Merge face stateset with texture stateset
            stateset->merge(*textureStateSet);
        }

        // Cull face
        switch(_drawFlag)
        {
        case SOLID_BACKFACED:     // Enable backface culling
        {
            static osg::ref_ptr<osg::CullFace> cullFace = new osg::CullFace(osg::CullFace::BACK);
            stateset->setAttributeAndModes(cullFace.get(), osg::StateAttribute::ON);
            break;
        }
        case SOLID_NO_BACKFACE:   // Disable backface culling
            if( document.getReplaceDoubleSidedPolys( ) )
            {
                static osg::ref_ptr<osg::CullFace> cullFace = new osg::CullFace(osg::CullFace::BACK);
                stateset->setAttributeAndModes(cullFace.get(), osg::StateAttribute::ON);
            }
            else
            {
                stateset->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
            }
            break;
        }

        // Subface
        if (document.subfaceLevel() > 0)
        {
            stateset->setAttributeAndModes(document.getSubSurfacePolygonOffset(document.subfaceLevel()), osg::StateAttribute::ON);
            stateset->setAttribute(document.getSubSurfaceDepth());

            stateset->setRenderBinDetails(document.subfaceLevel(),"RenderBin");
        }

        _geode->setStateSet(stateset.get());

        // Add to parent.
        if (_parent.valid())
            _parent->addChild(*_geode);
    }

    virtual void dispose(Document& document)
    {
        if (_geode.valid())
        {
            // Insert transform(s)
            if (_matrix.valid())
            {
                insertMatrixTransform(*_geode,*_matrix,_numberOfReplications);
            }

            if( getDrawMode( ) == SOLID_NO_BACKFACE && document.getReplaceDoubleSidedPolys( ) )
            {
                addDrawableAndReverseWindingOrder( _geode.get() );
            }

            osg::StateSet* stateset =  _geode->getOrCreateStateSet();

            // Translucent image?
            bool isImageTranslucent = false;
            if (document.getUseTextureAlphaForTransparancyBinning())
            {
                for (unsigned int i=0; i<stateset->getTextureAttributeList().size(); ++i)
                {
                    osg::StateAttribute* sa = stateset->getTextureAttribute(i,osg::StateAttribute::TEXTURE);
                    osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(sa);
                    if (texture)
                    {
                        osg::Image* image = texture->getImage();
                        if (image && image->isImageTranslucent())
                            isImageTranslucent = true;
                    }
                }
            }

            // Transparent Material?
            bool isMaterialTransparent = false;
            osg::Material* material = dynamic_cast<osg::Material*>(stateset->getAttribute(osg::StateAttribute::MATERIAL));
            if (material)
            {
                isMaterialTransparent = material->getDiffuse(osg::Material::FRONT).a() < 0.99f;
            }

            // Enable alpha blend?
            if (isAlphaBlend() || isTransparent() || isImageTranslucent || isMaterialTransparent)
            {
                static osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
                stateset->setAttributeAndModes(blendFunc.get(), osg::StateAttribute::ON);
                stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            }

            if (document.getUseBillboardCenter())
            {
                // Set billboard rotation point to center of face.
                osg::Billboard* billboard = dynamic_cast<osg::Billboard*>(_geode.get());
                if (billboard)
                {
                    for (unsigned int i=0; i<billboard->getNumDrawables(); ++i)
                    {
                        const osg::BoundingBox& bb = billboard->getDrawable(i)->getBoundingBox();
                        billboard->setPosition(i,bb.center());

                        osgUtil::TransformAttributeFunctor tf(osg::Matrix::translate(-bb.center()));
                        billboard->getDrawable(i)->accept(tf);

                        billboard->getDrawable(i)->dirtyBound();
                    }

                    billboard->dirtyBound();
                }
            }
        }
    }
};

REGISTER_FLTRECORD(Mesh, MESH_OP)




/** LocalVertexPool -
  */
class LocalVertexPool : public Record
{
    // Attribute Mask
    static const unsigned int HAS_POSITION      = 0x80000000u >> 0;
    static const unsigned int HAS_COLOR_INDEX   = 0x80000000u >> 1;
    static const unsigned int HAS_RGBA_COLOR    = 0x80000000u >> 2;
    static const unsigned int HAS_NORMAL        = 0x80000000u >> 3;
    static const unsigned int HAS_BASE_UV       = 0x80000000u >> 4;
    static const unsigned int HAS_UV_LAYER1     = 0x80000000u >> 5;
    static const unsigned int HAS_UV_LAYER2     = 0x80000000u >> 6;
    static const unsigned int HAS_UV_LAYER3     = 0x80000000u >> 7;
    static const unsigned int HAS_UV_LAYER4     = 0x80000000u >> 8;
    static const unsigned int HAS_UV_LAYER5     = 0x80000000u >> 9;
    static const unsigned int HAS_UV_LAYER6     = 0x80000000u >> 10;
    static const unsigned int HAS_UV_LAYER7     = 0x80000000u >> 11;

public:

    LocalVertexPool() {}

    META_Record(LocalVertexPool)

protected:

    virtual ~LocalVertexPool() {}

    virtual void readRecord(RecordInputStream& in, Document& document)
    {

        uint32 vertices = in.readUInt32();
        uint32 mask = in.readUInt32();

        osg::ref_ptr<VertexList> _vertexList = new VertexList(vertices);


        for (unsigned int n=0; n<vertices; n++)
        {
            Vertex vertex;

            if (mask & HAS_POSITION)
            {
                osg::Vec3d coord = in.readVec3d();
                vertex.setCoord(coord*document.unitScale());

                if (!coord.valid())
                {
                    OSG_NOTICE<<"Warning: data error detected in LocalVertexPool::readRecord coord="<<coord.x()<<" "<<coord.y()<<" "<<coord.z()<<std::endl;
                }
            }

            if (mask & HAS_COLOR_INDEX)
            {
                uint32 alphaIndex = in.readUInt32();
                int index = alphaIndex & 0x00ffffff;
                uint8 alpha = alphaIndex >> 24;
                osg::Vec4 color = document.getColorPool()->getColor(index);
                color.a() = (float)alpha/255;
                vertex.setColor(color);

                if (!color.valid())
                {
                    OSG_NOTICE<<"Warning: data error detected in LocalVertexPool::readRecord color="<<color.r()<<" "<<color.g()<<" "<<color.b()<<" "<<color.a()<<std::endl;
                }
            }

            if (mask & HAS_RGBA_COLOR)
            {
                osg::Vec4f color = in.readColor32();
                vertex.setColor(color);

                if (!color.valid())
                {
                    OSG_NOTICE<<"Warning: data error detected in LocalVertexPool::readRecord color="<<color.r()<<" "<<color.g()<<" "<<color.b()<<" "<<color.a()<<std::endl;
                }
            }

            if (mask & HAS_NORMAL)
            {
                osg::Vec3f normal = in.readVec3f();
                vertex.setNormal(normal);

                if (!normal.valid())
                {
                    OSG_NOTICE<<"Warning: data error detected in LocalVertexPool::readRecord normal="<<normal.x()<<" "<<normal.y()<<" "<<normal.z()<<std::endl;
                }
            }

            for (unsigned int layer=0; layer<8; layer++)
            {
                if (mask & (HAS_BASE_UV >> layer))
                {
                    osg::Vec2f uv = in.readVec2f();
                    vertex.setUV(layer,uv);

                    if (!uv.valid())
                    {
                        OSG_NOTICE<<"Warning: data error detected in LocalVertexPool::readRecord uv="<<uv.x()<<" "<<uv.y()<<std::endl;
                    }

                }
            }

            (*_vertexList)[n] = vertex;
        }

        if (_parent.valid())
            _parent->setLocalVertexPool(_vertexList.get());

    }
};

REGISTER_FLTRECORD(LocalVertexPool, LOCAL_VERTEX_POOL_OP)




/** MeshPrimitive -
  */
class MeshPrimitive : public PrimaryRecord
{
    enum PrimitiveType
    {
        TRIANGLE_STRIP = 1,
        TRIANGLE_FAN = 2,
        QUADRILATERAL_STRIP = 3,
        INDEXED_POLYGON = 4
    };

public:

    MeshPrimitive() {}

    META_Record(MeshPrimitive)

protected:

    virtual ~MeshPrimitive() {}

    virtual void readRecord(RecordInputStream& in, Document& /*document*/)
    {
        Mesh* mesh = dynamic_cast<Mesh*>(_parent.get());
        if (!mesh) return;

        VertexList* vertexList = mesh->getLocalVertexPool();
        if (!vertexList) return;

        int16 type = in.readInt16();
        uint16 indexSize = in.readUInt16();
        uint32 vertexCount = in.readUInt32();

        GLenum mode = 0;
        switch(type)
        {
        case TRIANGLE_STRIP:
            mode = osg::PrimitiveSet::TRIANGLE_STRIP;
            break;
        case TRIANGLE_FAN:
            mode = osg::PrimitiveSet::TRIANGLE_FAN;
            break;
        case QUADRILATERAL_STRIP:
            mode = osg::PrimitiveSet::QUAD_STRIP;
            break;
        case INDEXED_POLYGON:
            mode = osg::PrimitiveSet::POLYGON;
            break;
        }

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->addPrimitiveSet(new osg::DrawArrays(mode,0,vertexCount));

        for (unsigned int n=0; n<vertexCount; n++)
        {
            unsigned int index = 0;
            switch (indexSize)
            {
            case 1:
                index = in.readUInt8();
                break;
            case 2:
                index = in.readUInt16();
                break;
            case 4:
                index = in.readUInt32();
                break;
            }

            if (index < vertexList->size())
            {
                Vertex& vertex = (*vertexList)[index];

                osg::Vec3Array* vertices = getOrCreateVertexArray(*geometry);
                vertices->push_back(vertex._coord);

                if (vertex.validColor())
                {
                    osg::Vec4Array* colors = getOrCreateColorArray(*geometry);
                    colors->push_back(vertex._color);
                }

                if (vertex.validNormal())
                {
                    osg::Vec3Array* normals = getOrCreateNormalArray(*geometry);
                    normals->push_back(vertex._normal);
                }

                for (int layer=0; layer<Vertex::MAX_LAYERS; layer++)
                {
                    if (vertex.validUV(layer))
                    {
                        osg::Vec2Array* UVs = getOrCreateTextureArray(*geometry,layer);
                        UVs->push_back(vertex._uv[layer]);
                    }
                }
            }
        }

        // Color binding
        if (mesh->isGouraud())
        {
            // Color per vertex
            if (geometry->getColorArray()) geometry->getColorArray()->setBinding(osg::Array::BIND_PER_VERTEX);
        }
        else
        {
            // Color per face
            osg::Vec4 col = mesh->getPrimaryColor();
            col[3] = 1.0f - mesh->getTransparency();


            osg::Vec4Array* colors = new osg::Vec4Array(1);
            (*colors)[0] = col;
            geometry->setColorArray(colors, osg::Array::BIND_OVERALL);
        }

        // Normal binding
        if (mesh->isLit())
        {
            if (geometry->getNormalArray()) geometry->getNormalArray()->setBinding(osg::Array::BIND_PER_VERTEX);
        }
        else
        {
            geometry->setNormalArray(0);
        }

        mesh->addGeometry(*geometry);

    }
};

REGISTER_FLTRECORD(MeshPrimitive, MESH_PRIMITIVE_OP)



} // end namespace




