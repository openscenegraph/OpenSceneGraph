/* **************************************************************************
* OpenSceneGraph loader for Terrapage format database
* by Boris Bralo 2002
*
* based on/modifed  sgl (Scene Graph Library) loader by Brian Walsh
*
* This loader is based on/modified from Terrain Experts Performer Loader,
* and was ported to SGL by Bryan Walsh / bryanw at earthlink dot net
*
* That loader is redistributed under the terms listed on Terrain Experts
* website (www.terrex.com/www/pages/technology/technologypage.htm)
*
* "TerraPage is provided as an Open Source format for use by anyone...
* We supply the TerraPage C++ source code free of charge.  Anyone
* can use it and redistribute it as needed (including our competitors).
* We do, however, ask that you keep the TERREX copyrights intact."
*
* Copyright Terrain Experts Inc. 1999.
* All Rights Reserved.
*
*****************************************************************************/
#include "trpage_sys.h"

#include <osg/AlphaFunc>
#include <osg/Group>
#include <osg/Material>
#include <osg/TexEnv>
#include <osg/LOD>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/CullFace>
#include <osg/Light>
#include <osg/Notify>
#include <osg/PolygonOffset>
#include <osg/MatrixTransform>
#include <osg/PagedLOD>
#include <osgSim/LightPointNode>
#include <osg/Point>

#include "TrPageParser.h"
#include "TrPageArchive.h"

#include <algorithm>

using namespace txp;
using namespace osg;
using std::vector;
using std::string;

#if defined(WIN32) && defined(_MSC_VER)
#define strcasecmp stricmp
#endif

//----------------------------------------------------------------------------
// Check if the node is billboard
namespace {
    bool is_billboard (Node* node)
    {
        if (node && (node!=(Node*)1) && (strcmp(node->className(),"GeodeGroup") == 0))
        {
            GeodeGroup* group = static_cast<GeodeGroup*>(node);
            return (group->getNumChildren() && (strcmp(group->getChild(0)->className(),"Billboard") == 0));
        }
        return false;
    };
    
    void check_format(trpgTexture::ImageType type, int depth, GLenum& internalFormat, GLenum& pixelFormat, GLenum&)
    {
        switch(type)
        {
        case trpgTexture::trpg_RGB8:
            internalFormat = GL_RGB;
            pixelFormat    = GL_RGB;
            break;
        case trpgTexture::trpg_RGBA8:
            internalFormat = GL_RGBA;
            pixelFormat    = GL_RGBA;
            break;
        case trpgTexture::trpg_INT8:
            internalFormat = GL_LUMINANCE;
            pixelFormat    = GL_LUMINANCE;
            break;
        case trpgTexture::trpg_INTA8:
            internalFormat = GL_LUMINANCE_ALPHA;
            pixelFormat    = GL_LUMINANCE_ALPHA;
            break;
        case trpgTexture::trpg_FXT1:
        case trpgTexture::trpg_Filler:
        case trpgTexture::trpg_RGBX: // MCM
        case trpgTexture::trpg_Unknown:
            break;
        case trpgTexture::trpg_DDS:
        case trpgTexture::trpg_DXT1:
            if(depth == 3)
            {
                internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                pixelFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
            }
            else
            {
                internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                pixelFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            }
            break;
        case trpgTexture::trpg_DXT3:
            if(depth == 3)
            {
                // not supported.
            }
            else
            {
                internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                pixelFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            }
            break;
        case trpgTexture::trpg_DXT5:
            if(depth == 3)
            {
                // not supported.
            }
            else
            {
                internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                pixelFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            }
            break;
        }
    }
}

//----------------------------------------------------------------------------
// Get a template texture via the image helper
Texture2D* txp::GetLocalTexture(trpgrImageHelper& image_helper, const trpgTexture* tex)
{
    Texture2D* osg_texture= 0L;

    trpg2iPoint s;
    tex->GetImageSize(s);
    int32 depth;
    tex->GetImageDepth(depth);
    trpgTexture::ImageType type;
    tex->GetImageType(type);
    
    GLenum internalFormat = (GLenum)-1;
    GLenum pixelFormat = (GLenum)-1;
    GLenum dataType = GL_UNSIGNED_BYTE;

    check_format(type,depth,internalFormat , pixelFormat , dataType);
    
    if(pixelFormat!=(GLenum)-1)
    {
        osg_texture = new Texture2D();

        Image* image = new Image;
        char* data = 0L;

        bool bMipmap;
        tex->GetIsMipmap(bMipmap);
        int32 num_mipmaps = bMipmap  ?  tex->CalcNumMipmaps() : 1; // this is currently line 130

        // osg::Image do their own mipmaps
        if(num_mipmaps <= 1)
        {
            int32 size = s.x*s.y*depth; 
            // int32 size = const_cast<trpgTexture*>(tex)->MipLevelSize(1) ;
            data = new char [size];
            image_helper.GetLocalGL(tex,data,size);
            image->setImage(s.x,s.y,1,internalFormat, pixelFormat, dataType,
                    (unsigned char*)data,osg::Image::USE_NEW_DELETE);
        }
        else
        {
            int32 size = tex->CalcTotalSize();
            trpgTexture* tmp_tex = const_cast<trpgTexture*>(tex);

            data = new char [size];
            image_helper.GetLocalGL(tex,data,size);
            // Load entire texture including mipmaps
            image->setImage(s.x,s.y,1,internalFormat, pixelFormat, dataType,
                    (unsigned char*)data,
                    osg::Image::USE_NEW_DELETE);

            // now set mipmap data (offsets into image raw data)
            Image::MipmapDataType mipmaps;
            // number of offsets in osg is one less than num_mipmaps
            // because it's assumed that first offset iz 0 
            mipmaps.resize(num_mipmaps-1);
            for( int k = 1 ; k < num_mipmaps;k++ )
            {
                mipmaps[k-1] = tmp_tex->MipLevelOffset(k);
            }
            image->setMipmapData(mipmaps);

        }

        osg_texture->setImage(image);
    }
    return osg_texture;
}
//----------------------------------------------------------------------------
// Get a locale texture via the image helper
Texture2D* txp::GetTemplateTexture(trpgrImageHelper& image_helper, trpgLocalMaterial* locmat, const trpgTexture* tex, int index)
{
    Texture2D* osg_texture= 0L;

    trpg2iPoint s;
    tex->GetImageSize(s);
    int32 depth;
    tex->GetImageDepth(depth);
    trpgTexture::ImageType type;
    tex->GetImageType(type);
    
    GLenum internalFormat = (GLenum)-1;
    GLenum pixelFormat = (GLenum)-1;
    GLenum dataType = GL_UNSIGNED_BYTE;

    check_format(type,depth,internalFormat , pixelFormat , dataType);
    
    if(pixelFormat!=(GLenum)-1)
    {
        osg_texture = new Texture2D();

        Image* image = new Image;
        char* data = 0L;

        bool bMipmap;
        tex->GetIsMipmap(bMipmap);
        int32 num_mipmaps = bMipmap  ?  tex->CalcNumMipmaps() : 1; // this is currently line 130

        // osg::Image do their own mipmaps
        if(num_mipmaps <= 1)
        {
            int32 size = s.x*s.y*depth; 
            // int32 size = const_cast<trpgTexture*>(tex)->MipLevelSize(1) ;
            data = new char [size];
            image_helper.GetNthImageForLocalMat(locmat,index, data,size);

            image->setImage(s.x,s.y,1,internalFormat, pixelFormat, dataType,
                    (unsigned char*)data,osg::Image::USE_NEW_DELETE);
        }
        else
        {
            int32 size = tex->CalcTotalSize();
            trpgTexture* tmp_tex = const_cast<trpgTexture*>(tex);

            data = new char [size];

            image_helper.GetNthImageForLocalMat(locmat,index, data,size);

            // Load entire texture including mipmaps
            image->setImage(s.x,s.y,1,internalFormat, pixelFormat, dataType,
                    (unsigned char*)data,
                    osg::Image::USE_NEW_DELETE);

            // now set mipmap data (offsets into image raw data)
            Image::MipmapDataType mipmaps;
            // number of offsets in osg is one less than num_mipmaps
            // because it's assumed that first offset iz 0 
            mipmaps.resize(num_mipmaps-1);
            for( int k = 1 ; k < num_mipmaps;k++ )
            {
                mipmaps[k-1] = tmp_tex->MipLevelOffset(k);
            }
            image->setMipmapData(mipmaps);

        }

        osg_texture->setImage(image);
    }
    return osg_texture;
}

//----------------------------------------------------------------------------
//
// Group Reader Class
//
//----------------------------------------------------------------------------
// Apply transformation on geometry
class TransformFunctor : public osg::Drawable::AttributeFunctor
{
    public:
    
        osg::Matrix _m;
        osg::Matrix _im;

        TransformFunctor(const osg::Matrix& m)
        {
            _m = m;
            _im.invert(_m);
        }
            
        virtual ~TransformFunctor() {}

        virtual void apply(osg::Drawable::AttributeType type,unsigned int count,osg::Vec3* begin)
        {
            if (type == osg::Drawable::VERTICES)
            {
                osg::Vec3* end = begin+count;
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    (*itr) = (*itr)*_m;
                }
            }
            else if (type == osg::Drawable::NORMALS)
            {
                osg::Vec3* end = begin+count;
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    // note post mult by inverse for normals.
                    (*itr) = osg::Matrix::transform3x3(_im,(*itr));
                    (*itr).normalize();
                }
            }
        }

        inline void SetMatrix(const osg::Matrix& m)
        {
            _m = m;
            _im.invert(_m);
        }

};

//----------------------------------------------------------------------------
geomRead::geomRead(TrPageParser *in_parse)
{
    parse = in_parse;
}

//----------------------------------------------------------------------------
geomRead::~geomRead()
{
}

//----------------------------------------------------------------------------
void* geomRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgGeometry geom;
    if (!geom.Read(buf)) 
        return NULL;
    
    // Get the necessary info out of the geom
    trpgGeometry::PrimType primType;
    int numPrims;
    int numVert;
    int numNorm;
    geom.GetPrimType(primType);
    geom.GetNumPrims(numPrims);
    geom.GetNumVertex(numVert);
    geom.GetNumNormal(numNorm);
    
    // Get vertices
    Vec3Array* vertices = new Vec3Array(numVert);
    geom.GetVertices((float *)&(vertices->front()));
    
    // Turn the trpgGeometry into something Performer can understand
    Geometry *geometry      = 0L;
    
    // Get texture coordinates
    trpgTexData td;
    int num_tex;
    geom.GetNumTexCoordSets(num_tex);
    Vec2Array** tex_coords = new Vec2Array*[num_tex];
    for (int texno = 0; texno < num_tex; texno++)
    {
        tex_coords[texno] = 0L;
        if (geom.GetTexCoordSet(texno,&td))
        {
            tex_coords[texno] = new Vec2Array(numVert); 
            for (int i=0 ;i < numVert; i++)
            {
                (*(tex_coords[texno]))[i].set(td.floatData[2*i+0],td.floatData[2*i+1]);
            }
        }
    }
   
    // The normals
    Vec3Array* normals = 0L;
    if (numNorm == numVert)
    {
        normals = new Vec3Array(numVert);
        geom.GetNormals((float*)&(normals->front()));
    }
    
    // Set up the primitive type
    switch (primType)
    {
    case trpgGeometry::Triangles:
        {
            geometry = new Geometry;
            geometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::TRIANGLES,0,numPrims*3));
        }
        break;
    case trpgGeometry::Quads:
        {
            geometry = new Geometry;
            geometry->addPrimitiveSet(new DrawArrays(PrimitiveSet::QUADS,0,numPrims*4));
        }
        break;
    case trpgGeometry::TriStrips:
        {
            geometry = new osg::Geometry;
            osg::DrawArrayLengths* dal = new osg::DrawArrayLengths(osg::PrimitiveSet::TRIANGLE_STRIP,0,numPrims);
            geom.GetPrimLengths(&(dal->front()));
            geometry->addPrimitiveSet(dal);
        }
        break;
    case trpgGeometry::TriFans:
        {
            geometry = new osg::Geometry;
            osg::DrawArrayLengths* dal = new osg::DrawArrayLengths(osg::PrimitiveSet::TRIANGLE_FAN,0,numPrims);
            geom.GetPrimLengths(&(dal->front()));
            geometry->addPrimitiveSet(dal);

            // Need to flip the fans coords.
            int ind = 0;
            int i;
            for (i=0;i<numPrims;++i)
            {
                int length = (*dal)[i];
                int start=ind+1;
                int end=ind+length-1;
                // Swap from start+1 to end
                // Swap vertices, texture coords & normals
                for (; start < end; ++start, --end )
                {
                    std::swap((*vertices)[start], (*vertices)[end]);
                    for(int texno = 0; texno < num_tex; texno ++ )
                    {
                        if( tex_coords[texno] )
                        {
                            std::swap((*tex_coords[texno])[start], (*tex_coords[texno])[end]);
                        }
                    }
                    if(normals)
                    {
                        std::swap((*normals)[start], (*normals)[end]);
                    }
                }
                ind += length;
            }
        }
        break;
    default:
        
        break;
    };
    
    
    // Add it to the current parent group
    GeodeGroup *top = parse->GetCurrTop();
    Geode *geode = top->GetGeode();
    if (geometry)
    {
        // added this set use display list off since terrapage will
        // be creating and deleting these geometry leaves on the fly
        // so we don't want to be creating short lived display lists either.
        geometry->setUseDisplayList(false);

        geometry->setVertexArray(vertices);
        if (normals)
        {
            geometry->setNormalArray(normals);
            geometry->setNormalBinding(Geometry::BIND_PER_VERTEX);
        }
        // Note: Should check number of materials first
        // Note: Should be combining multiple geosets
        bool local;
        int matId;
        int  matNo;
        geom.GetNumMaterial(matNo);
        ref_ptr<StateSet>  sset = 0L;
        for(int n_mat = 0; n_mat < matNo ; ++n_mat)
        {
            ref_ptr<StateSet> tmp_ss = 0L;
            geom.GetMaterial(n_mat,matId,local);
            if( local )
                tmp_ss = (*parse->GetLocalMaterials())[matId];
            else
               tmp_ss = (*parse->GetMaterials())[matId];
            if(sset.valid()) 
            {
                if(tmp_ss.valid()){
                    osg::StateAttribute* texenv0 = tmp_ss->getTextureAttribute(0,StateAttribute::TEXENV);
                    if(texenv0) 
                        sset->setTextureAttribute(n_mat,texenv0);
                    osg::StateAttribute* tex0 = tmp_ss->getTextureAttribute(0,StateAttribute::TEXTURE);
                    if(tex0) 
                        sset->setTextureAttributeAndModes(n_mat,tex0,osg::StateAttribute::ON);
                }
//                sset->merge(*tmp_ss.get());
            }
            else 
                sset = tmp_ss;
        }

        if (tex_coords)
        {
            for (int texno = 0; texno < num_tex; texno++)
                geometry->setTexCoordArray( texno, tex_coords[texno]);
        }

        if ( is_billboard(top) )
        {
            geometry->setStateSet(sset.get());

            Billboard *billboard = static_cast<Billboard*>(top->getChild(0));

            switch (parse->getBillboardType()) {
            case trpgBillboard::Individual:
            {
                // compute center of billboard geometry
                const BoundingBox& bbox = geometry->getBound();
                Vec3 center ((bbox._min + bbox._max) * 0.5f);

                // make billboard geometry coordinates relative to computed center
                Matrix matrix;
                matrix.makeTranslate(-center[0], -center[1], -center[2]);

                TransformFunctor tf(matrix);
                geometry->accept(tf);
                geometry->dirtyBound();

                billboard->addDrawable(geometry);
                billboard->setPos(0, center);
            }
            break;
            case trpgBillboard::Group:
            {
                Vec3 center(parse->getBillboardCenter());

                // make billboard geometry coordinates relative to specified center
                Matrix matrix;
                matrix.makeTranslate(-center[0], -center[1], -center[2]);

                TransformFunctor tf(matrix);
                geometry->accept(tf);
                geometry->dirtyBound();

                billboard->addDrawable(geometry);
                billboard->setPos(0, center);
            }
            break;
            default:
                billboard->addDrawable(geometry);
                notify(WARN) << "TerraPage loader: fell through case: " <<  __FILE__ << " " << __LINE__  << ".\n";
                break;
            }
        }
        else
        {
            // if this is part of the layer we turn the PolygonOffset on
            
            if ( (parse->GetCurrLayer() == top) && geode->getNumDrawables() )
            {
                StateSet* poStateSet = new StateSet;
                PolygonOffset* polyoffset = new PolygonOffset;

                poStateSet->merge(*sset.get());
                polyoffset->setFactor(-1.0f*geode->getNumDrawables());
                polyoffset->setUnits(-20.0f*geode->getNumDrawables());
                poStateSet->setAttributeAndModes(polyoffset,osg::StateAttribute::ON);

                geometry->setStateSet(poStateSet);
                
            }
            else 
            {
                geometry->setStateSet(sset.get());
            }

            geode->addDrawable(geometry);
        }

    }
    return (void *) 1;
}

//----------------------------------------------------------------------------
//
// Group Reader Class
//
//----------------------------------------------------------------------------
groupRead::groupRead(TrPageParser *in_parse)
{
    parse = in_parse;
}

//----------------------------------------------------------------------------
void* groupRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgGroup group;
    if (!group.Read(buf))
        return NULL;
    // Create a new Performer group
    GeodeGroup* osg_Group = new GeodeGroup();
    // Dump this group into the hierarchy
    parse->AddIntoSceneGraph(osg_Group);
    // Register the group for attachements
    int32 id;
    group.GetID(id);
    parse->AddToGroupList(id,osg_Group);
    return (void *) osg_Group;
}

//----------------------------------------------------------------------------
//
// Layer Reader Class
//
//----------------------------------------------------------------------------
layerRead::layerRead(TrPageParser *in_parse)
{
    parse = in_parse;
}

//----------------------------------------------------------------------------
void* layerRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgLayer group;
    if (!group.Read(buf))
        return NULL;
    // Create a new Performer group
    Layer* osg_Group = new Layer();
    // Dump this group into the hierarchy
    parse->AddIntoSceneGraph(osg_Group);
    // Register for attachements
    int32 id;
    group.GetID(id);
    parse->AddToGroupList(id,osg_Group);
    return (void *) osg_Group;
}

//----------------------------------------------------------------------------
//
// Attach Reader Class
//
//----------------------------------------------------------------------------
attachRead::attachRead(TrPageParser *in_parse)
{
    parse = in_parse;
}

//----------------------------------------------------------------------------
void* attachRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgAttach group;
    if (!group.Read(buf))
        return NULL;
    // Create a new Performer group
    GeodeGroup* osg_Group = new GeodeGroup();
    // Dump this group into the hierarchy
    parse->AddIntoSceneGraph(osg_Group);
    // Register for attachements
    int32 id;
    group.GetID(id);
    parse->AddToGroupList(id,osg_Group);
    // This sets the parent ID for the current tile too
    int32 parentID;
    group.GetParentID(parentID);

    //parse->SetParentID(parentID); no need of this anymore. we force PagedLOD 
    return (void *) osg_Group;
}

//----------------------------------------------------------------------------
//
// Billboard Reader Class
//
//----------------------------------------------------------------------------
billboardRead::billboardRead(TrPageParser *in_parse)
{
    parse = in_parse;
}

//----------------------------------------------------------------------------
void* billboardRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    // Read in the txp billboard
    trpgBillboard bill;
    if (!bill.Read(buf))
        return NULL;

    // Create a group with a geometry beneath for the billboard
    GeodeGroup* osg_Group = new GeodeGroup();

    if (parse->inBillboard()) {
        // we don't allow anything under a billboard except geometry
        notify(WARN) << "TerraPage loader: can only have geometry nodes beneath a billboard.\n";
    }
    else
    {
        int type, mode;
        trpg3dPoint center, axis;

        if (bill.GetType(type) && bill.GetMode(mode) && bill.GetCenter(center) && bill.GetAxis(axis)) {
            Billboard* billboard = new Billboard();

            osg_Group->SetGeode(billboard);

            // save this state for processing of the geometry node(s)
            parse->setBillboardType(type);
            parse->setBillboardCenter(center);

            // set the axis
            // NOTE: Needs update, when the billboard implementation for
            // arbitrary axis is ready
            // billboard->setAxis(Vec3(axis.x, axis.y, axis.z));
            billboard->setAxis(Vec3(0.0f,0.0,1.0f) );
            billboard->setNormal(Vec3(0.0f,-1.0,0.0f));
            
            // the mode
            switch (mode) {
            case trpgBillboard::Axial:
                billboard->setMode(Billboard::AXIAL_ROT);
                break;
            case trpgBillboard::World:
                billboard->setMode(Billboard::POINT_ROT_WORLD);
                break;
            case trpgBillboard::Eye:
                billboard->setMode(Billboard::POINT_ROT_EYE);
                break;
            default:
                notify(WARN) << "TerraPage loader: Unknown billboard type.\n";
                notify(WARN) << "TerraPage loader: fell through case: " <<  __FILE__ << " " << __LINE__  << ".\n";
                break;
            }
        }
    }

    // Dump this group into the hierarchy
    parse->AddIntoSceneGraph(osg_Group);
    // Register
    int32 id;
    bill.GetID(id);
    parse->AddToGroupList(id,osg_Group);
    
    return (void *) osg_Group;
}

//----------------------------------------------------------------------------
//
// LOD Reader Class
//
//----------------------------------------------------------------------------
lodRead::lodRead (TrPageParser *in_parse)
{
    parse = in_parse;
}

//----------------------------------------------------------------------------
void* lodRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgLod lod;
    if (!lod.Read(buf))
        return NULL;
    // Pull out the LOD data we'll need
    trpg3dPoint center;
    lod.GetCenter(center);
    double in,out,width;
    lod.GetLOD(in,out,width);
    double minRange = MIN(in,out);
    double maxRange = MAX(in,out+width);

    // Create a new Performer LOD
    LOD* osg_Lod = new LOD();
    Vec3 osg_Center;
    osg_Center[0] = center.x;  osg_Center[1] = center.y;  osg_Center[2] = center.z;
    osg_Lod->setCenter(osg_Center);
    osg_Lod->setRange(0,minRange, maxRange );
    
    // Our LODs are binary so we need to add a group under this LOD and attach stuff
    //  to that instead of the LOD
    GeodeGroup *osg_LodG = new GeodeGroup();
    osg_Lod->addChild(osg_LodG);
    
    // Dump this group into the hierarchy
    parse->AddIntoSceneGraph(osg_Lod);

	// Sets the current parent as potentional PagedLOD
	parse->SetPotentionalPagedLOD(parse->GetCurrTop());

    // Register for attachements
    int32 id;
    lod.GetID(id);
    // Add the sub-group to the group list, not the LOD
    parse->AddToGroupList(id,osg_LodG);
    return (void *) osg_LodG;
}

//----------------------------------------------------------------------------
//
// Model Reference Reader Class
//
//----------------------------------------------------------------------------
modelRefRead::modelRefRead(TrPageParser *in_parse)
{
    parse = in_parse;
}

//----------------------------------------------------------------------------
void *modelRefRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgModelRef model;
    if (!model.Read(buf))
        return NULL;
    // Get the matrix and pfNode for the model
    int modelID;
    model.GetModel(modelID);
    float64 mat[16];
    model.GetMatrix(mat);
    Matrix osg_Mat(
        (float)mat[0], (float)mat[1], (float)mat[2], (float)mat[3],
        (float)mat[4], (float)mat[5], (float)mat[6], (float)mat[7],
        (float)mat[8], (float)mat[9], (float)mat[10],(float)mat[11],
        (float)mat[12],(float)mat[13],(float)mat[14],(float)mat[15]
        );
    
    // Note: Array check before you do this
    Node *osg_Model = NULL;
    std::vector<osg::ref_ptr<osg::Node> >*modelList = parse->GetModels();
    if( modelList->size() > size_t(modelID) )
    {
    //(*parse->GetModels())[modelID].get();
       osg_Model = (*modelList)[modelID].get();
    // Create the SCS and position the model
    if (osg_Model) {
        MatrixTransform *scs = new MatrixTransform();
        scs->setMatrix(osg_Mat);
        scs->addChild(osg_Model);
        // Add the SCS to the hierarchy
        Group *top = parse->GetCurrTop();
        if (top)
            top->addChild(scs);
    }
    }
    return (void *) 1;
}

//----------------------------------------------------------------------------
//
// Tile Header Reader Class
//
//----------------------------------------------------------------------------
tileHeaderRead::tileHeaderRead(TrPageParser *in_parse)
{
    parse = in_parse;
}

//----------------------------------------------------------------------------
void* tileHeaderRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgTileHeader *tileHead = parse->GetTileHeaderRef();
    if (!tileHead->Read(buf))
        return NULL;
    parse->LoadLocalMaterials();
    return (void *) 1;
}


//----------------------------------------------------------------------------
//
// light Reader Class
//
//----------------------------------------------------------------------------
lightRead::lightRead (TrPageParser *in_parse)
{
    parse = in_parse;
}

//----------------------------------------------------------------------------
void* lightRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgLight light;
    if (!light.Read(buf))
        return NULL;
	int attr_index;
	light.GetAttrIndex(attr_index);

	DefferedLightAttribute& dla = parse->GetLightAttribute(attr_index); 
	osgSim::LightPointNode* node = dla.lightPoint.get();

	uint32 nvert;
	light.GetNumVertices(nvert);

	if( node->getLightPoint(0)._sector.valid() ) // osgSim::LigthPoint is a must
	{
		for(unsigned int i = 0; i < nvert; i++){
			trpg3dPoint pt;
			light.GetVertex(i, pt);
			osg::Matrix matrix;
	//        matrix.makeTranslate(pt.x,pt.y,pt.z);
			matrix.makeRotate(osg::Quat(0.0,dla.attitude));
			matrix.setTrans(pt.x,pt.y,pt.z);
			osg::MatrixTransform* trans = new osg::MatrixTransform();
			trans->setMatrix(matrix);
			trans->addChild(node);
			parse->AddIntoSceneGraph(trans);
		}
	}
	else { //Fall back to osg::Points
	    Vec3Array* vertices = new Vec3Array(nvert);
	    Vec4Array* colors = new Vec4Array(nvert);

		for(unsigned int i = 0; i < nvert; i++){
			trpg3dPoint pt;
			light.GetVertex(i, pt);
			(*vertices)[i] = osg::Vec3(pt.x,pt.y,pt.z);
			(*colors)[i] = node->getLightPoint(0)._color;
		}

		osg::Geometry* geom = new osg::Geometry();
        geom->addPrimitiveSet(new DrawArrays(PrimitiveSet::POINTS,0,nvert));
		geom->setVertexArray(vertices);
		geom->setColorArray(colors);
		geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		geom->setUseDisplayList(false);
		geom->setStateSet(dla.fallback.get());

	    GeodeGroup *top = parse->GetCurrTop();
		Geode *geode = top->GetGeode();
		geode->addDrawable(geom);
		
	}

    return (void *) 1;
}


/* ********************************** */

//----------------------------------------------------------------------------
// Constructor for scene graph parser
TrPageParser::TrPageParser(TrPageArchive* parent)
{
    parent_ = parent;
    currTop = NULL;
    top = NULL;
    layerDepth = 0;
    currLayer = NULL;
    in_billboard = false;
    
    // Register the readers
    AddCallback(TRPG_GEOMETRY,new geomRead(this));
    AddCallback(TRPG_GROUP,new groupRead(this));
    AddCallback(TRPG_ATTACH,new attachRead(this));
    AddCallback(TRPG_BILLBOARD,new billboardRead(this));
    AddCallback(TRPG_LOD,new lodRead(this));
    AddCallback(TRPG_MODELREF,new modelRefRead(this));
    AddCallback(TRPG_LAYER,new layerRead(this));
    AddCallback(TRPGTILEHEADER,new tileHeaderRead(this));
    AddCallback(TRPG_LIGHT,new lightRead(this));
//    AddCallback(TRPGLIGHTATTR,new lightAttrRead(this));
//    AddCallback(TRPGLIGHTTABLE,new lightTableRead(this));
}

//----------------------------------------------------------------------------
// Destructor for scene graph parser
TrPageParser::~TrPageParser()
{
}

//----------------------------------------------------------------------------
// Return a reference to the Tile Header
// Doesn't do much if you haven't just read a tile
trpgTileHeader *TrPageParser::GetTileHeaderRef()
{
    return &tileHead;
}

//----------------------------------------------------------------------------
// Converts to PagedLOD
void TrPageParser::ConvertToPagedLOD(osg::Group* group)
{
	if (group->getNumChildren() == 2)
	{
		osg::LOD* loLOD = dynamic_cast<osg::LOD*>(group->getChild(0));
		osg::LOD* hiLOD = dynamic_cast<osg::LOD*>(group->getChild(1));

		if (loLOD && hiLOD)
		{
			osg::Group *g = dynamic_cast<osg::Group*>(hiLOD->getChild(0));
			if (!g) return;
			if (g->getNumChildren()) return;

			char pagedLODfile[1024];
			sprintf(pagedLODfile,
				"%s\\subtiles%d_%dx%d.txp",
				parent_->getDir(),
				_tileLOD,
				_tileX,
				_tileY
			);

			osg::PagedLOD* pagedlod = new osg::PagedLOD;

			pagedlod->addChild(loLOD->getChild(0),loLOD->getMinRange(0),loLOD->getMaxRange(0));
			pagedlod->setRange(1,0.0f,hiLOD->getMaxRange(0));
			pagedlod->setFileName(1,pagedLODfile);
			pagedlod->setCenter(hiLOD->getCenter());

			group->addChild(pagedlod);

			group->removeChild(loLOD);
			group->removeChild(hiLOD);
		}
	}
}
//----------------------------------------------------------------------------
// Parse a buffer and return a (chunk of) Performer
//  scene graph.
Group *TrPageParser::ParseScene(trpgReadBuffer &buf,vector<ref_ptr<StateSet> > &in_mat,vector<ref_ptr<Node> > &in_model)
{
    top = currTop = new GeodeGroup();
    materials = &in_mat;
    local_materials.clear();
    models = &in_model;
    parentID = -1;
    
    // All the setup is handled in the constructor.
    // Just parse and return the top
    if (!Parse(buf))
    {
        notify(WARN) << "trpgFPParser::ParseScene failed to parse tile.\n";
        return NULL;
    }

	// Puts PagedLODs on the right places
	for (std::map<osg::Group*,int>::iterator i = _pagedLods.begin(); i != _pagedLods.end(); i++)
	{
		ConvertToPagedLOD((*i).first);
	}
	_pagedLods.clear();
    
    Group *ret = top;
    top = currTop = NULL;
    return ret;
}

//----------------------------------------------------------------------------
// Read local materials
void TrPageParser::LoadLocalMaterials()
{
    // new to 2.0 LOCAL materials
    trpgrImageHelper image_helper(parent_->GetEndian(),parent_->getDir(),*parent_->GetMaterialTable(),*parent_->GetTexTable());
    trpgTileHeader* tile_head = GetTileHeaderRef();

    int n_materials;
    tile_head->GetNumLocalMaterial(n_materials);

    int n_mat;
    tile_head->GetNumMaterial(n_mat);

    local_materials.clear();
    local_materials.resize(n_materials);
    {
        for (int i = 0; i < n_materials; i++)
        {
            StateSet* osg_state_set = new StateSet;
            
            trpgLocalMaterial locmat;
            tile_head->GetLocalMaterial(i,locmat);
            
            const trpgMaterial* mat;
            const trpgTexture *tex;

            int32 size;
            image_helper.GetImageInfoForLocalMat(&locmat, &mat,&tex,size);

            int num_tex;
            mat->GetNumTexture(num_tex);
            for (int texNo = 0 ; texNo < num_tex; ++texNo)
            {
                int texId;
                trpgTextureEnv texEnv;
                mat->GetTexture(texNo,texId,texEnv);

                // Set up texture environment
                TexEnv       *osg_texenv       = new TexEnv();
                int32 te_mode;
                texEnv.GetEnvMode(te_mode);
                switch( te_mode )
                {
                case trpgTextureEnv::Alpha :
                    osg_texenv->setMode(TexEnv::REPLACE);
                break;
                case trpgTextureEnv::Decal:
                    osg_texenv->setMode(TexEnv::DECAL);
                    break;
                case trpgTextureEnv::Blend :
                    osg_texenv->setMode(TexEnv::BLEND);
                    break;
                case trpgTextureEnv::Modulate :
                    osg_texenv->setMode(TexEnv::MODULATE);
                    break;
                }
            
                osg_state_set->setTextureAttribute(texNo,osg_texenv);

                image_helper.GetNthImageInfoForLocalMat(&locmat, texNo, &mat,&tex,size);
            
                trpgTexture::ImageMode mode;
                tex->GetImageMode(mode);
                Texture2D* osg_texture = 0L;
                if(mode == trpgTexture::Template) 
                    osg_texture = GetTemplateTexture(image_helper,&locmat, tex, texNo);
                else if(mode == trpgTexture::Local) 
                    osg_texture = GetLocalTexture(image_helper,tex);
                else if(mode == trpgTexture::Global)
                    osg_texture = parent_->getGlobalTexture(texId);

                if(osg_texture)
                {
                    if(osg_texture->getImage())
                    {
                        GLenum gltype = osg_texture->getImage()->getPixelFormat();
                        if( gltype == GL_RGBA || gltype == GL_LUMINANCE_ALPHA )
                        {
                            osg_state_set->setMode(GL_BLEND,StateAttribute::ON);
                            osg_state_set->setRenderingHint(StateSet::TRANSPARENT_BIN);
                        }
                    }
                    else
                    {
                        notify(WARN) << "No image\n";
                    }
                    osg_state_set->setTextureAttributeAndModes(texNo,osg_texture, StateAttribute::ON);

                    int wrap_s, wrap_t;   
                    texEnv.GetWrap(wrap_s, wrap_t);
                    osg_texture->setWrap(Texture2D::WRAP_S, wrap_s == trpgTextureEnv::Repeat ? Texture2D::REPEAT: Texture2D::CLAMP );
                    osg_texture->setWrap(Texture2D::WRAP_T, wrap_t == trpgTextureEnv::Repeat ? Texture2D::REPEAT: Texture2D::CLAMP );
                }
                else
                {
                    notify(WARN) << "No texture\n";
                }
            }

            Material     *osg_material     = new Material;
            
            float64 alpha;
            mat->GetAlpha(alpha);
            
            trpgColor color;
            mat->GetAmbient(color);
            osg_material->setAmbient( Material::FRONT_AND_BACK , 
                Vec4(color.red, color.green, color.blue, alpha));
            mat->GetDiffuse(color);
            osg_material->setDiffuse(Material::FRONT_AND_BACK , 
                Vec4(color.red, color.green, color.blue, alpha));
            
            mat->GetSpecular(color);
            osg_material->setSpecular(Material::FRONT_AND_BACK , 
                Vec4(color.red, color.green, color.blue, alpha));
            mat->GetEmission(color);
            osg_material->setEmission(Material::FRONT_AND_BACK , 
                Vec4(color.red, color.green, color.blue, alpha));
            
            float64 shinines;
            mat->GetShininess(shinines);
            osg_material->setShininess(Material::FRONT_AND_BACK , (float)shinines);
            
            osg_material->setAlpha(Material::FRONT_AND_BACK ,(float)alpha);
            osg_state_set->setAttributeAndModes(osg_material, StateAttribute::ON);
            
            if( alpha < 1.0f )
            {
                osg_state_set->setMode(GL_BLEND,StateAttribute::ON);
                osg_state_set->setRenderingHint(StateSet::TRANSPARENT_BIN);
            }
            
            
        /* This controls what alpha values in a texture mean.  It can take the values:
            None,Always,Equal,GreaterThanOrEqual,GreaterThan,
            LessThanOrEqual,LessThan,Never,NotEqual
        */
            int alphaFunc;
            mat->GetAlphaFunc(alphaFunc);
            if( alphaFunc>=GL_NEVER && alphaFunc<=GL_ALWAYS)
            {
                float64 ref;
                mat->GetAlphaRef(ref);
                AlphaFunc *osg_alpha_func = new AlphaFunc;
                osg_alpha_func->setFunction((AlphaFunc::ComparisonFunction)alphaFunc,(float)ref);
                osg_state_set->setAttributeAndModes(osg_alpha_func, StateAttribute::ON);
            }

            int cullMode;
            mat->GetCullMode(cullMode);
            
            // Culling mode in txp means opposite from osg i.e. Front-> show front face
            if( cullMode != trpgMaterial::FrontAndBack)
            {
                CullFace* cull_face = new CullFace;
                switch (cullMode)
                {
                case trpgMaterial::Front:
                    cull_face->setMode(CullFace::BACK);
                    break;
                case trpgMaterial::Back:
                    cull_face->setMode(CullFace::FRONT);
                    break;
                }
                osg_state_set->setAttributeAndModes(cull_face, StateAttribute::ON);
            }
            local_materials[i] = osg_state_set;
        }
     }
}
//----------------------------------------------------------------------------
// Start Children
// This is called when the parser hits a push.
// We'll want to make the node it's handing us the "top" node
bool TrPageParser::StartChildren(void *in_node)
{
    // Make a node
    GeodeGroup *node = (GeodeGroup *)in_node;

    // If we are under layer we need to drop all groups. Then
    // the current parent is still the layer node
    if (layerDepth==0)
    {
        // Set it as current parent
        currTop = node;
    }

    // 
    // track whether we are under a billboard in the scene graph
    //
    if (is_billboard(currTop))
        in_billboard = true;

    // Chek if it's layer 
    if (node && (in_node != (void*)1) && (strcasecmp(node->className(),"Layer") == 0))
    {
        if (layerDepth==0)
            currLayer = (Layer*)node;
        layerDepth++;
    }
    
    return true;
}

//----------------------------------------------------------------------------
// This is called when the parser hits a pop.
// We'll want to look on the parent list (in trpgSceneParser)
// for the parent above the current one.
// If there isn't one, we'll just stick things in our top group.
bool TrPageParser::EndChildren(void *in_node)
{
    // Chek if it's layer
    osg::Node* node = (osg::Node*)in_node;
    if (node && (in_node != (void*)1) && (strcasecmp(node->className(),"Layer") == 0))
    {
        if (layerDepth > 0)
            layerDepth--;
        else
        {
            notify(WARN) << "Layer depth < 0 ???\n";
        }
        if (layerDepth==0)
        {
            currLayer = NULL;
            for (unsigned int i = 0; i < deadNodes.size(); i++)
            {
                osg::ref_ptr<Node> deadNode = deadNodes[i];
            }
            deadNodes.clear();
        }
    }

    // 
    // track whether we are under a billboard in the scene graph
    //
    if (is_billboard(node))
        in_billboard = false;

    // if we are under layer all the groups are dropped out, so no need to do this
    if (layerDepth==0)
    {
        // Get the parent above the current one
        int pos = parents.size()-2;
        if (pos < 0)
        {
            // Nothing above the current one.  Fall back on our top group
            currTop = top;
        }
        else
        {
            currTop = (GeodeGroup *)parents[pos];
        }
    }

    return true;
}

//----------------------------------------------------------------------------
// Return the current top node
GeodeGroup *TrPageParser::GetCurrTop()
{
    if (currTop)
    {
        return currTop;
    }
    else
    {
        return top;
    }
}

//----------------------------------------------------------------------------
// Add the given pfGroup to the group list at position ID
bool TrPageParser::AddToGroupList(int ID,GeodeGroup *group)
{
    // we dont do this if we are under layer
    if (layerDepth==0)
    {
        // Note: check bounds
        groupList[ID] = group;
    }
    
    return true;
}

//----------------------------------------------------------------------------
// Initialize the group list
void TrPageParser::SetMaxGroupID(int maxGroupID)
{
    //notify(WARN) << "trpgFPParser: max group ID = " << maxGroupID << std::endl;
    // Initialize the group list with -1's
    groupList.resize(0);
    // Note: Fix this
    for (int i=0;i<maxGroupID;i++)
        groupList.push_back(NULL);
}

//----------------------------------------------------------------------------
// Use this to add nodes into the scenegraph
void TrPageParser::AddIntoSceneGraph(osg::Node* node)
{
    // chek first if we are under some layer
    if (layerDepth)
    {
        deadNodes.push_back(node);
    }
    else
    {
        Group *top = GetCurrTop();
        if (top)
            top->addChild(node);
    }
}

//----------------------------------------------------------------------------
// Return the current layer node (also used during parsing)
Layer *TrPageParser::GetCurrLayer()
{
    return currLayer;
}

DefferedLightAttribute& TrPageParser::GetLightAttribute(int idx)
{
	return parent_->GetLightAttribute(idx);
}
