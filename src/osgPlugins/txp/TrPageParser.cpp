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
#include <osg/Texture>
#include <osg/TexEnv>
#include <osg/LOD>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/Matrix>
#include <osg/Transform>
#include <osg/GeoSet>
#include <osg/CullFace>
#include <osg/Light>
#include <osg/Transparency>
#include <osg/Notify>


#include "TrPageParser.h"
#include "TrPageArchive.h"
/*
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
*/
#include <algorithm>

using namespace txp;
using namespace osg;
using std::vector;
using std::string;

Texture* txp::GetLocalTexture(trpgrImageHelper& image_helper, trpgLocalMaterial* locmat, const trpgTexture* tex)
{
    Texture* osg_texture= 0L;

    trpg2iPoint s;
    tex->GetImageSize(s);
    int32 depth;
    tex->GetImageDepth(depth);
    trpgTexture::ImageType type;
    tex->GetImageType(type);
    
    Texture::InternalFormatMode internalFormat = Texture::USE_IMAGE_DATA_FORMAT;
    
    GLenum gltype = (GLenum)-1;
    switch(type)
    {
    case trpgTexture::trpg_RGB8:
        gltype = GL_RGB;
        break;
    case trpgTexture::trpg_RGBA8:
        gltype = GL_RGBA;
        break;
    case trpgTexture::trpg_INT8:
        gltype = GL_LUMINANCE;
        break;
    case trpgTexture::trpg_INTA8:
        gltype = GL_LUMINANCE_ALPHA;
        break;
    case trpgTexture::trpg_FXT1:
    case trpgTexture::trpg_Filler:
    case trpgTexture::trpg_RGBX: // MCM
    case trpgTexture::trpg_Unknown:
    case trpgTexture::trpg_DDS:
        break;
    case trpgTexture::trpg_DXT1:
        if(depth == 3)
        {
            gltype = GL_RGB;
        }
        else
        {
            gltype = GL_RGBA;
        }
        internalFormat = Texture::USE_S3TC_DXT1_COMPRESSION;
        break;
    case trpgTexture::trpg_DXT3:
        if(depth == 3)
        {
            gltype = GL_RGB;
        }
        else
        {
            gltype = GL_RGBA;
        }
        internalFormat = Texture::USE_S3TC_DXT3_COMPRESSION;
        break;
    case trpgTexture::trpg_DXT5:
        if(depth == 3)
        {
            gltype = GL_RGB;
        }
        else
        {
            gltype = GL_RGBA;
        }
        internalFormat = Texture::USE_S3TC_DXT5_COMPRESSION;
        break;
    }
    
    if(gltype!=(GLenum)-1)
    {
        osg_texture = new Texture();
        osg_texture->setInternalFormatMode(internalFormat);

        Image* image = new Image;
        char* data = 0L;

        int32 num_mipmaps = tex->CalcNumMipmaps();
        // osg::Image do their own mipmaps
        if(num_mipmaps <= 1)
        {
            int32 size = s.x*s.y*depth; 
            // int32 size = const_cast<trpgTexture*>(tex)->MipLevelSize(1) ;
            data = (char*)::malloc(size);

            if(locmat)            
               image_helper.GetImageForLocalMat(locmat,data,size);
            else
                image_helper.GetLocalGL(tex,data,size);

            image->setImage(s.x,s.y,1,depth,
                    gltype,GL_UNSIGNED_BYTE,
                    (unsigned char*)data);
        }
        else
        {
            int32 size = tex->CalcTotalSize();
            trpgTexture* tmp_tex = const_cast<trpgTexture*>(tex);

            data = (char*)::malloc(size);

            if(locmat)            
               image_helper.GetImageForLocalMat(locmat,data,size);
            else
               image_helper.GetLocalGL(tex,data,size);

            // Load entire texture including mipmaps
            image->setImage(s.x,s.y,1,depth,
                    gltype,GL_UNSIGNED_BYTE,
                    (unsigned char*)data);

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
        //  AAAARRRGH! TOOK ME 2 DAYS TO FIGURE IT OUT
        //  EVERY IMAGE HAS TO HAVE UNIQUE NAME FOR OPTIMIZER NOT TO OPTIMIZE IT OFF
        //
        static int unique = 0;
        char unique_name[256];
        sprintf(unique_name,"TXP_TEX_UNIQUE%d",unique++);

        image->setFileName(unique_name);

        osg_texture->setImage(image);
    }
    return osg_texture;
}

geomRead::geomRead(TrPageParser *in_parse)
{
    parse = in_parse;
}

geomRead::~geomRead()
{
}

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
    int matId;
    geom.GetPrimType(primType);
    geom.GetNumPrims(numPrims);
    geom.GetNumVertex(numVert);
    bool local;
    geom.GetMaterial(0,matId, local);
    geom.GetNumNormal(numNorm);
    
    Vec3* vertices = new Vec3[numVert];
    // Get vertices
    geom.GetVertices(vertices);
    // Turn the trpgGeometry into something Performer can understand
    GeoSet *gset      = 0L;
    
    // Get texture coordinates
    Vec2*  tex_coords = 0L; 
    trpgTexData td;
    if (geom.GetTexCoordSet(0,&td))
    {
        tex_coords = new Vec2[numVert]; 
        for (int i=0 ;i < numVert; i++)
        {
            tex_coords[i][0] = td.floatData[2*i+0];
            tex_coords[i][1] = td.floatData[2*i+1];
        }
    }
    
    Vec3* normals = 0L;
    if (numNorm == numVert)
    {
        normals = new Vec3[numVert];
        geom.GetNormals(normals);
    }
    
    Geode *geode = new Geode();
    // Set up the primitive type
    switch (primType)
    {
    case trpgGeometry::Triangles:
        {
            gset = new GeoSet;
            gset->setPrimType(GeoSet::TRIANGLES);
        }
        break;
    case trpgGeometry::TriStrips:
        {
            // Need primitive lengths too
            int* primitives = new int[numPrims];
            geom.GetPrimLengths(primitives);
            
            // Define GeoSet
            gset = new GeoSet;
            gset->setPrimType(GeoSet::TRIANGLE_STRIP);
            gset->setPrimLengths(primitives);
        }
        break;
    case trpgGeometry::TriFans:
        {
            // Need primitive lengths too
            int* primitives = new int[numPrims];
            geom.GetPrimLengths(primitives);
            
            // Need to flip the fans
            int ind = 0;
            for (int i=0;i<numPrims;i++)
            {
                int start=ind+1;
                int end=primitives[i]+ind-1;
                // Swap from start+1 to end
                int numSwap = (end-start+1)/2;
                // Swap vertices, texture coords & normals
                for (int j=0; j < numSwap; j++ )
                {
                    std::swap(vertices[start], vertices[end]);
                    if( tex_coords )
                        std::swap(tex_coords[start], tex_coords[end]);
                    if(normals)
                        std::swap(normals[start], normals[end]);
                    start++;
                    end--;
                }
                ind += primitives[i];
            }
            // Define GeoSet
            gset = new GeoSet;
            gset->setPrimType(GeoSet::TRIANGLE_FAN);
            gset->setPrimLengths(primitives);
        }
        break;
    default:
        
        break;
    };
    
    
    // Add it to the current parent group
    Group *top = parse->GetCurrTop();
    if (gset)
    {
        gset->setCoords(vertices);
        gset->setNumPrims(numPrims);
        if (normals)
            gset->setNormals(normals);
        // Note: Should check number of materials first
        // Note: Should be combining multiple geosets
        ref_ptr<StateSet>  sset = 0L;
        if( local )
            sset = (*parse->GetLocalMaterials())[matId];
        else
           sset = (*parse->GetMaterials())[matId];

        if (tex_coords)
        {
            gset->setTextureCoords(tex_coords);
            gset->setTextureBinding(GeoSet::BIND_PERVERTEX);
        }

        gset->setStateSet(sset.get());
        geode->addDrawable(gset);
        top->addChild(geode);
    }
    return (void *) 1;
}

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
    Group *osg_Group = new Group();
    // Dump this group into the hierarchy
    Group *top = parse->GetCurrTop();
    if (top)
        top->addChild(osg_Group);
    int32 id;
    group.GetID(id);
    parse->AddToGroupList(id,osg_Group);
    return (void *) osg_Group;
}

//----------------------------------------------------------------------------
//
// Attach Reader Class
//
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
    Group *osg_Group = new Group();
    // Dump this group into the hierarchy
    Group *top = parse->GetCurrTop();
    if (top)
        top->addChild(osg_Group);
    int32 id;
    group.GetID(id);
    parse->AddToGroupList(id,osg_Group);
    // This sets the parent ID for the current tile too
    int32 parentID;
    group.GetParentID(parentID);
    parse->SetParentID(parentID);
    return (void *) osg_Group;
}

//----------------------------------------------------------------------------
//
// Billboard Reader Class
//
billboardRead::billboardRead(TrPageParser *in_parse)
{
    parse = in_parse;
}

//----------------------------------------------------------------------------
void* billboardRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgBillboard bill;
    if (!bill.Read(buf))
        return NULL;
    
    Group* osg_Group = new Group;
    int type;
    bill.GetType(type); 
    if( type == trpgBillboard::Group ) 
    {
        // Create a new Performer group
        Billboard* bl = new Billboard();
        int m;
        bill.GetMode(m);
        if( m == trpgBillboard::Eye) 
            bl->setMode(Billboard::POINT_ROT_EYE);
        else if(m == trpgBillboard::World )
            bl->setMode(Billboard::POINT_ROT_WORLD);
        else if(m == trpgBillboard::Axial )
        {
            trpg3dPoint p;
            bill.GetAxis(p); 
            bl->setAxis(Vec3(p.x, p.y, p.z));
            bl->setMode(Billboard::AXIAL_ROT);
        }
        osg_Group->addChild(bl);
    }
    // Dump this group into the hierarchy
    Group *top = parse->GetCurrTop();
    if (top)
        top->addChild(osg_Group);
    int32 id;
    bill.GetID(id);
    parse->AddToGroupList(id,osg_Group);
    
    return (void *) osg_Group;
}

//----------------------------------------------------------------------------
//
// LOD Reader Class
//
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
    double maxRange = MAX(in,out);
    
    // Create a new Performer LOD
    LOD *osg_Lod = new LOD();
    Vec3 osg_Center;
    osg_Center[0] = center.x;  osg_Center[1] = center.y;  osg_Center[2] = center.z;
    osg_Lod->setCenter(osg_Center);
    osg_Lod->setRange(0,minRange);
    osg_Lod->setRange(1,maxRange);
    
    // Our LODs are binary so we need to add a group under this LOD and attach stuff
    //  to that instead of the LOD
    Group *osg_LodG = new Group();
    osg_Lod->addChild(osg_LodG);
    
    // Dump this group into the hierarchy
    Group *top = parse->GetCurrTop();
    if (top)
        top->addChild(osg_Lod);
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
    Node *osg_Model = (*parse->GetModels())[modelID].get();
    // Create the SCS and position the model
    if (osg_Model) {
        Transform *scs = new Transform();
        scs->setMatrix(osg_Mat);
        scs->addChild(osg_Model);
        // Add the SCS to the hierarchy
        Group *top = parse->GetCurrTop();
        if (top)
            top->addChild(scs);
    }
    return (void *) 1;
}

//----------------------------------------------------------------------------
//
// Tile Header Reader Class
//
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


/* ********************************** */

//----------------------------------------------------------------------------
// Constructor for scene graph parser
TrPageParser::TrPageParser(TrPageArchive* parent)
{
    parent_ = parent;
    currTop = NULL;
    top = NULL;
    
    // Register the readers
    AddCallback(TRPG_GEOMETRY,new geomRead(this));
    AddCallback(TRPG_GROUP,new groupRead(this));
    AddCallback(TRPG_ATTACH,new attachRead(this));
    AddCallback(TRPG_BILLBOARD,new billboardRead(this));
    AddCallback(TRPG_LOD,new lodRead(this));
    AddCallback(TRPG_MODELREF,new modelRefRead(this));
    AddCallback(TRPGTILEHEADER,new tileHeaderRead(this));
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
// Parse a buffer and return a (chunk of) Performer
//  scene graph.
Group *TrPageParser::ParseScene(trpgReadBuffer &buf,vector<ref_ptr<StateSet> > &in_mat,vector<ref_ptr<Node> > &in_model)
{
    top = currTop = new Group();
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
    
    Group *ret = top;
    top = currTop = NULL;
    return ret;
}

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
            image_helper.GetImageInfoForLocalMat(&locmat,&mat,&tex,size);

            int texId;
            trpgTextureEnv texEnv;
            mat->GetTexture(0,texId,texEnv);

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
            
            osg_state_set->setAttribute(osg_texenv);
            
            Texture* osg_texture = GetLocalTexture(image_helper,&locmat, tex);

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
                osg_state_set->setAttributeAndModes(osg_texture, StateAttribute::ON);

                int wrap_s, wrap_t;   
                texEnv.GetWrap(wrap_s, wrap_t);
                osg_texture->setWrap(Texture::WRAP_S, wrap_s == trpgTextureEnv::Repeat ? Texture::REPEAT: Texture::CLAMP );
                osg_texture->setWrap(Texture::WRAP_T, wrap_t == trpgTextureEnv::Repeat ? Texture::REPEAT: Texture::CLAMP );
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
            osg_material->setShininess(Material::FRONT_AND_BACK , (float)shinines/128.0);
            
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
            if( alphaFunc != trpgMaterial::None)
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
    Group *node = (Group *)in_node;
    
    currTop = node;
    
    return true;
}

//----------------------------------------------------------------------------
// This is called when the parser hits a pop.
// We'll want to look on the parent list (in trpgSceneParser)
// for the parent above the current one.
// If there isn't one, we'll just stick things in our top group.
bool TrPageParser::EndChildren(void * /* in_node */)
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
        currTop = (Group *)parents[pos];
    }
    return true;
}

//----------------------------------------------------------------------------
// Return the current top node
Group *TrPageParser::GetCurrTop()
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
bool TrPageParser::AddToGroupList(int ID,Group *group)
{
    // Note: check bounds
    groupList[ID] = group;
    
    return true;
}

//----------------------------------------------------------------------------
// Initialize the group list
void TrPageParser::SetMaxGroupID(int maxGroupID)
{
    notify(WARN) << "trpgFPParser: max group ID = " << maxGroupID << std::endl;
    // Initialize the group list with -1's
    groupList.resize(0);
    // Note: Fix this
    for (int i=0;i<maxGroupID;i++)
        groupList.push_back(NULL);
}
