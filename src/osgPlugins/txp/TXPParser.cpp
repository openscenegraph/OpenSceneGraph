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
#include <osg/ShapeDrawable>

#include "TXPParser.h"
#include "TXPArchive.h"
using namespace txp;

TXPParser::TXPParser():
_archive(0),
_currentTop(0),
_root(0),
_underBillboardSubgraph(false),
_numBillboardLevels(0),
_underLayerSubgraph(false),
_numLayerLevels(0),
_layerGeode(0)
{
    AddCallback(TRPG_GEOMETRY,new geomRead(this));
    AddCallback(TRPG_GROUP,new groupRead(this));
    AddCallback(TRPG_LOD,new lodRead(this));
    AddCallback(TRPG_MODELREF,new modelRefRead(this));
    AddCallback(TRPG_BILLBOARD,new billboardRead(this));
    AddCallback(TRPG_LIGHT,new lightRead(this));
    AddCallback(TRPG_LAYER,new layerRead(this));
    AddCallback(TRPGTILEHEADER,new tileHeaderRead(this));
}

TXPParser::~TXPParser()
{
}

osg::Group *TXPParser::parseScene(
    trpgReadBuffer &buf, 
    std::vector<osg::ref_ptr<osg::StateSet> > &materials,
    std::vector<osg::ref_ptr<osg::Node> > &models)
{
    if (_archive == 0) return NULL;

    _root = new osg::Group();
    _currentTop = _root.get();

    _materials = &materials;
    _localMaterials.clear();
    _models = &models;

    _underBillboardSubgraph = false;
    _numBillboardLevels = 0;
    _underLayerSubgraph = false;
    _numLayerLevels = 0;

    if (!Parse(buf))
    {
        osg::notify(osg::NOTICE) << "txp::TXPParser::parseScene(): failed to parse the given tile" << std::endl;
        return NULL;
    }

    for (std::map<osg::Group*,int>::iterator i = _tileGroups.begin(); i != _tileGroups.end(); i++)
    {
        replaceTileLod((*i).first);
    }
    _tileGroups.clear();

    return _root.get();
}

void TXPParser::replaceTileLod(osg::Group* group)
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

            group->addChild(loLOD->getChild(0));
            group->removeChild(loLOD);
            group->removeChild(hiLOD);
        }
    }
}

bool TXPParser::StartChildren(void *in)
{
    if (_underBillboardSubgraph )
    {
        _numBillboardLevels++;
    }
    else
    if (_underLayerSubgraph)
    {
        _numLayerLevels++;
    }
    else
    if (in && (in != (void*)1))
    {
        osg::Group *parent = (osg::Group*)in;
        _parents.push(parent);
        _currentTop = parent;
    }
    return true;
}

bool TXPParser::EndChildren(void *)
{
    if (_underLayerSubgraph)
    {
        _numLayerLevels--;
        if (_numLayerLevels == 0)
        {
            _underLayerSubgraph = false;
        }
    }
    else
    if (_underBillboardSubgraph)
    {
        _numBillboardLevels--;
        if (_numBillboardLevels == 0)
        {
            _underBillboardSubgraph = false;
        }
    }
    else
    {
        _currentTop = 0;
        if (_parents.size())
        {
            _parents.pop();
            if (_parents.size())
            {
                _currentTop = _parents.top();
            }
        }
    }

    return true;
}

DefferedLightAttribute& TXPParser::getLightAttribute(int ix)
{ 
    return _archive->getLightAttribute(ix); 
}

class FindEmptyGroupsVisitor : public osg::NodeVisitor
{
public:
    FindEmptyGroupsVisitor(osg::NodeList& nl):
      osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _nl(nl) {};

    virtual void apply(osg::Group& group)
    {
        if (group.getNumChildren()==0)
        {
            _nl.push_back(&group);
        }
        traverse(group);
    }
protected:
    osg::NodeList& _nl;
};

void TXPParser::removeEmptyGroups()
{
    if (_root.get() && _root->getNumChildren())
    {
        osg::NodeList nl;
        FindEmptyGroupsVisitor fegv(nl);

        _root->accept(fegv);

        for (unsigned int i = 0; i < nl.size(); i++)
        {
            osg::Node* node = nl[i].get();
            if (node == NULL) continue;

            osg::Node::ParentList parents = node->getParents();
            for (unsigned int j = 0; j < parents.size(); j++)
            {
                osg::Group* parent = parents[j];
                if (parent) parent->removeChild(node);
            }
        }
    }
}

osg::Geode* TXPParser::createBoundingBox(int x,int y, int lod)
{
    TXPArchive::TileInfo info;

    _archive->getTileInfo(x,y,lod,info);

    osg::Geode* geode = new osg::Geode();

    osg::TessellationHints* hints = new osg::TessellationHints;
    hints->setDetailRatio(0.5f);
    osg::ShapeDrawable* sd = new osg::ShapeDrawable(
            new osg::Box(
                info.center,
                info.bbox.xMax()-info.bbox.xMin(),
                info.bbox.yMax()-info.bbox.yMin(),
                1
            ),
            hints
        );
    
    if (lod==0)
    {
        sd->setColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    }
    else
    if (lod==1)
    {
        sd->setColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    }
    else
    if (lod==2)
    {
        sd->setColor(osg::Vec4(0.0f,1.0f,0.0f,1.0f));
    }
    else
    if (lod==3)
    {
        sd->setColor(osg::Vec4(0.0f,0.0f,1.0f,1.0f));
    }
    else
    if (lod==4)
    {
        sd->setColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    }
    geode->addDrawable( sd );

    return geode;
}

void TXPParser::loadLocalMaterials()
{
    // new to 2.0 LOCAL materials
    trpgrImageHelper image_helper(
        _archive->GetEndian(),
        _archive->getDir(),
        *_archive->GetMaterialTable(),
        *_archive->GetTexTable()
    );
    trpgTileHeader* tile_head = getTileHeaderRef();

    int n_materials;
    tile_head->GetNumLocalMaterial(n_materials);

    int n_mat;
    tile_head->GetNumMaterial(n_mat);

    _localMaterials.clear();
    _localMaterials.resize(n_materials);
    {
        for (int i = 0; i < n_materials; i++)
        {
            osg::StateSet* osg_state_set = new osg::StateSet;
            
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
                osg::TexEnv       *osg_texenv       = new osg::TexEnv();
                int32 te_mode;
                texEnv.GetEnvMode(te_mode);
                switch( te_mode )
                {
                case trpgTextureEnv::Alpha :
                    osg_texenv->setMode(osg::TexEnv::REPLACE);
                break;
                case trpgTextureEnv::Decal:
                    osg_texenv->setMode(osg::TexEnv::DECAL);
                    break;
                case trpgTextureEnv::Blend :
                    osg_texenv->setMode(osg::TexEnv::BLEND);
                    break;
                case trpgTextureEnv::Modulate :
                    osg_texenv->setMode(osg::TexEnv::MODULATE);
                    break;
                }
            
                osg_state_set->setTextureAttribute(texNo,osg_texenv);

                image_helper.GetNthImageInfoForLocalMat(&locmat, texNo, &mat,&tex,size);
            
                trpgTexture::ImageMode mode;
                tex->GetImageMode(mode);
                osg::Texture2D* osg_texture = 0L;
                if(mode == trpgTexture::Template) 
                    osg_texture = getTemplateTexture(image_helper,&locmat, tex, texNo);
                else if(mode == trpgTexture::Local) 
                    osg_texture = getLocalTexture(image_helper,tex);
                else if(mode == trpgTexture::Global)
                    osg_texture = _archive->getGlobalTexture(texId);

                if(osg_texture)
                {
                    if(osg_texture->getImage())
                    {
                        GLenum gltype = osg_texture->getImage()->getPixelFormat();
                        if( gltype == GL_RGBA || gltype == GL_LUMINANCE_ALPHA )
                        {
                            osg_state_set->setMode(GL_BLEND,osg::StateAttribute::ON);
                            osg_state_set->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                        }
                    }
                    else
                    {
                        osg::notify(osg::WARN) << "No image\n";
                    }
                    osg_state_set->setTextureAttributeAndModes(texNo,osg_texture, osg::StateAttribute::ON);

                    int wrap_s, wrap_t;   
                    texEnv.GetWrap(wrap_s, wrap_t);
                    osg_texture->setWrap(osg::Texture2D::WRAP_S, wrap_s == trpgTextureEnv::Repeat ? osg::Texture2D::REPEAT: osg::Texture2D::CLAMP );
                    osg_texture->setWrap(osg::Texture2D::WRAP_T, wrap_t == trpgTextureEnv::Repeat ? osg::Texture2D::REPEAT: osg::Texture2D::CLAMP );
                    
                    // by default is anisotropic filtering.
                    osg_texture->setMaxAnisotropy(4.0f);
                }
                else
                {
                    osg::notify(osg::WARN) << "No texture\n";
                }
            }

            osg::Material     *osg_material     = new osg::Material;
            
            float64 alpha;
            mat->GetAlpha(alpha);
            
            trpgColor color;
            mat->GetAmbient(color);
            osg_material->setAmbient( osg::Material::FRONT_AND_BACK , 
                osg::Vec4(color.red, color.green, color.blue, alpha));
            mat->GetDiffuse(color);
            osg_material->setDiffuse(osg::Material::FRONT_AND_BACK , 
                osg::Vec4(color.red, color.green, color.blue, alpha));
            
            mat->GetSpecular(color);
            osg_material->setSpecular(osg::Material::FRONT_AND_BACK , 
                osg::Vec4(color.red, color.green, color.blue, alpha));
            mat->GetEmission(color);
            osg_material->setEmission(osg::Material::FRONT_AND_BACK , 
                osg::Vec4(color.red, color.green, color.blue, alpha));
            
            float64 shinines;
            mat->GetShininess(shinines);
            osg_material->setShininess(osg::Material::FRONT_AND_BACK , (float)shinines);
            
            osg_material->setAlpha(osg::Material::FRONT_AND_BACK ,(float)alpha);
            osg_state_set->setAttributeAndModes(osg_material, osg::StateAttribute::ON);
            
            if( alpha < 1.0f )
            {
                osg_state_set->setMode(GL_BLEND,osg::StateAttribute::ON);
                osg_state_set->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
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
                osg::AlphaFunc *osg_alpha_func = new osg::AlphaFunc;
                osg_alpha_func->setFunction((osg::AlphaFunc::ComparisonFunction)alphaFunc,(float)ref);
                osg_state_set->setAttributeAndModes(osg_alpha_func, osg::StateAttribute::ON);
            }

            int cullMode;
            mat->GetCullMode(cullMode);
            
            // Culling mode in txp means opposite from osg i.e. Front-> show front face
            if( cullMode != trpgMaterial::FrontAndBack)
            {
                osg::CullFace* cull_face = new osg::CullFace;
                switch (cullMode)
                {
                case trpgMaterial::Front:
                    cull_face->setMode(osg::CullFace::BACK);
                    break;
                case trpgMaterial::Back:
                    cull_face->setMode(osg::CullFace::FRONT);
                    break;
                }
                osg_state_set->setAttributeAndModes(cull_face, osg::StateAttribute::ON);
            }
            _localMaterials[i] = osg_state_set;
        }
     }
}

//----------------------------------------------------------------------------
//
// LOD Reader Class
//
//----------------------------------------------------------------------------
void* lodRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgLod lod;
    if (!lod.Read(buf)) return NULL;

    // Pull out the LOD data we'll need
    trpg3dPoint center;
    lod.GetCenter(center);

    double in,out,width;
    lod.GetLOD(in,out,width);

    double minRange = MIN(in,out);
    double maxRange = MAX(in,out+width);

    // Create a new osg LOD
    osg::ref_ptr<osg::LOD> osgLod = new osg::LOD();

    osg::Vec3 osgCenter;
    osgCenter[0] = center.x;
    osgCenter[1] = center.y;
    osgCenter[2] = center.z;
    osgLod->setCenter(osgCenter);
    osgLod->setRange(0,minRange, maxRange );
    
    // Our LODs are binary so we need to add a group under this LOD and attach stuff
    //  to that instead of the LOD
    osg::ref_ptr<GeodeGroup> osgLodG = new GeodeGroup();
    osgLod->addChild(osgLodG.get());

    // Add it into the scene graph
    osg::Group *top = _parse->getCurrTop();
    if (top)
    {
        top->addChild(osgLod.get());
        _parse->setPotentionalTileGroup(top);
        return (void *) osgLodG.get();
    }
    else
    {
        return (void*) 1;
    }
}

//----------------------------------------------------------------------------
//
// Tile Header Reader Class
//
//----------------------------------------------------------------------------
void* tileHeaderRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgTileHeader *tileHead = _parse->getTileHeaderRef();
    if (!tileHead->Read(buf))
        return NULL;
    _parse->loadLocalMaterials();
    return (void *) 1;
}

//----------------------------------------------------------------------------
//
// Model Reference Reader Class
//
//----------------------------------------------------------------------------
void *modelRefRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgModelRef model;
    if (!model.Read(buf)) return NULL;

    // Get the matrix and pfNode for the model
    int modelID;
    model.GetModel(modelID);
    float64 mat[16];
    model.GetMatrix(mat);
    osg::Matrix osg_Mat(
        (float)mat[0], (float)mat[1], (float)mat[2], (float)mat[3],
        (float)mat[4], (float)mat[5], (float)mat[6], (float)mat[7],
        (float)mat[8], (float)mat[9], (float)mat[10],(float)mat[11],
        (float)mat[12],(float)mat[13],(float)mat[14],(float)mat[15]
        );
    
    // Note: Array check before you do this
    osg::Node *osg_Model = NULL;
    std::vector<osg::ref_ptr<osg::Node> >*modelList = _parse->getModels();
    if( modelList->size() > size_t(modelID) )
    {
       osg_Model = (*modelList)[modelID].get();

        // Create the SCS and position the model
        if (osg_Model)
        {
            osg::MatrixTransform *scs = new osg::MatrixTransform();
            scs->setMatrix(osg_Mat);
            scs->addChild(osg_Model);
            // Add the SCS to the hierarchy
            osg::Group *top = _parse->getCurrTop();
            if (top)
            {
                top->addChild(scs);
            }
        }
    }
    return (void *) 1;
}

//----------------------------------------------------------------------------
//
// Billboard Reader Class
//
//----------------------------------------------------------------------------
void* billboardRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    // Read in the txp billboard
    trpgBillboard bill;
    if (!bill.Read(buf)) return NULL;

    if (_parse->underBillboardSubgraph())
    {
        // we don't allow anything under a billboard except geometry
        osg::notify(osg::WARN) << "TerraPage loader: can only have geometry nodes beneath a billboard.\n";
    }
    else
    {
        TXPParser::TXPBillboardInfo info;
        if (bill.GetType(info.type) && bill.GetMode(info.mode) && bill.GetCenter(info.center) && bill.GetAxis(info.axis))
        {
            // save this state for processing of the geometry node(s)
            _parse->setLastBillboardInfo(info);
            _parse->setUnderBillboardSubgraph(true);
        }
    }

    return (void *)1;
}

//----------------------------------------------------------------------------
//
// Group Reader Class
//
//----------------------------------------------------------------------------
void* groupRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgGroup group;
    if (!group.Read(buf)) return NULL;

    // Create a new group
    osg::ref_ptr<GeodeGroup> osgGroup = new GeodeGroup();

    // Add it into the scene graph
    osg::Group *top = _parse->getCurrTop();
    if (top)
    {
        top->addChild(osgGroup.get());
        _parse->setPotentionalTileGroup(top);
        return (void *) osgGroup.get();
    }
    else
    {
        return (void*) 1;
    }
}

//----------------------------------------------------------------------------
//
// light Reader Class
//
//----------------------------------------------------------------------------
void* lightRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgLight light;
    if (!light.Read(buf)) return NULL;

    int attr_index;
    light.GetAttrIndex(attr_index);

    DefferedLightAttribute& dla = _parse->getLightAttribute(attr_index); 
    osgSim::LightPointNode* node = dla.lightPoint.get();

    uint32 nvert;
    light.GetNumVertices(nvert);

    osg::Group *top = _parse->getCurrTop();

    if( node->getLightPoint(0)._sector.valid() ) // osgSim::LigthPoint is a must
    {
        for(unsigned int i = 0; i < nvert; i++)
        {
            std::cout<<"LightPoint node"<<std::endl;
            trpg3dPoint pt;
            light.GetVertex(i, pt);
            osg::Matrix matrix;
    //        matrix.makeTranslate(pt.x,pt.y,pt.z);
            matrix.makeRotate(osg::Quat(0.0,dla.attitude));
            matrix.setTrans(pt.x,pt.y,pt.z);
            osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform();
            trans->setMatrix(matrix);
            trans->addChild(node);
            if (top)
            {
                top->addChild(trans.get());
            }
        }
    }
    else
    { //Fall back to osg::Points
        osg::Vec3Array* vertices = new osg::Vec3Array(nvert);
        osg::Vec4Array* colors = new osg::Vec4Array(nvert);

        for(unsigned int i = 0; i < nvert; i++)
        {
            trpg3dPoint pt;
            light.GetVertex(i, pt);
            (*vertices)[i] = osg::Vec3(pt.x,pt.y,pt.z);
            (*colors)[i] = node->getLightPoint(0)._color;
        }

        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
        geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,nvert));
        geom->setVertexArray(vertices);
        geom->setColorArray(colors);
        geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

        geom->setUseDisplayList(false);
        geom->setStateSet(dla.fallback.get());

        if (top)
        {
            osg::Geode* g = new osg::Geode;
            g->addDrawable(geom.get());
            top->addChild(g);

        }
    }

    return (void *) 1;
}

//----------------------------------------------------------------------------
//
// Layer Reader Class
//
//----------------------------------------------------------------------------
void* layerRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgLayer group;
    if (!group.Read(buf)) return NULL;

    if (_parse->underLayerSubgraph()) return (void*)1;

    osg::ref_ptr<GeodeGroup> layer = new GeodeGroup;
    osg::Group* top = _parse->getCurrTop();
    if (top)
    {
        _parse->setLayerGeode(layer->getGeode());
        _parse->setUnderLayerSubgraph(true);
        top->addChild(layer.get());
    }

    return (void *) 1;
}

//----------------------------------------------------------------------------
//
// Geometry Reader Class
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
        else 
        if (type == osg::Drawable::NORMALS)
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

void* geomRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgGeometry geom;
    if (!geom.Read(buf)) return NULL;
    
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
    osg::Vec3Array* vertices = new osg::Vec3Array(numVert);
    geom.GetVertices((float *)&(vertices->front()));
    
    // Turn the trpgGeometry into something osg can understand
    osg::Geometry *geometry = 0L;
    
    // Get texture coordinates
    trpgTexData td;
    int num_tex;
    geom.GetNumTexCoordSets(num_tex);
    osg::Vec2Array** tex_coords = new osg::Vec2Array*[num_tex];
    for (int texno = 0; texno < num_tex; texno++)
    {
        tex_coords[texno] = 0L;
        if (geom.GetTexCoordSet(texno,&td))
        {
            tex_coords[texno] = new osg::Vec2Array(numVert); 
            for (int i=0 ;i < numVert; i++)
            {
                (*(tex_coords[texno]))[i].set(td.floatData[2*i+0],td.floatData[2*i+1]);
            }
        }
    }
   
    // The normals
    osg::Vec3Array* normals = 0L;
    if (numNorm == numVert)
    {
        normals = new osg::Vec3Array(numVert);
        geom.GetNormals((float*)&(normals->front()));
    }
    
    // Set up the primitive type
    switch (primType)
    {
    case trpgGeometry::Triangles:
        {
            geometry = new osg::Geometry;
            geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,numPrims*3));
        }
        break;
    case trpgGeometry::Quads:
        {
            geometry = new osg::Geometry;
            geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,numPrims*4));
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
    osg::Group *top = _parse->getCurrTop();

    // Is this geode group
    GeodeGroup *geodeTop = dynamic_cast<GeodeGroup*>(top);

    if (geometry && top)
    {
        // added this set use display list off since terrapage will
        // be creating and deleting these geometry leaves on the fly
        // so we don't want to be creating short lived display lists either.
        geometry->setUseDisplayList(false);

        geometry->setVertexArray(vertices);
        if (normals)
        {
            geometry->setNormalArray(normals);
            geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        }

        bool local;
        int matId;
        int  matNo;
        geom.GetNumMaterial(matNo);
        osg::ref_ptr<osg::StateSet>  sset = 0L;
        for(int n_mat = 0; n_mat < matNo ; ++n_mat)
        {
            osg::ref_ptr<osg::StateSet> tmp_ss = 0L;
            geom.GetMaterial(n_mat,matId,local);
            if( local )
                tmp_ss = (*_parse->getLocalMaterials())[matId];
            else
               tmp_ss = (*_parse->getMaterials())[matId];
            if(sset.valid()) 
            {
                if(tmp_ss.valid()){
                    osg::StateAttribute* texenv0 = tmp_ss->getTextureAttribute(0,osg::StateAttribute::TEXENV);
                    if(texenv0) 
                        sset->setTextureAttribute(n_mat,texenv0);
                    osg::StateAttribute* tex0 = tmp_ss->getTextureAttribute(0,osg::StateAttribute::TEXTURE);
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
            {
                geometry->setTexCoordArray( texno, tex_coords[texno]);
            }
        }

        if (_parse->underBillboardSubgraph())
        {
            geometry->setStateSet(sset.get());

            TXPParser::TXPBillboardInfo info;
            _parse->getLastBillboardInfo(info);

            osg::Billboard* billboard = new osg::Billboard;
            billboard->setAxis(osg::Vec3(0.0f,0.0,1.0f) );
            billboard->setNormal(osg::Vec3(0.0f,-1.0,0.0f));
            
            geometry->setUseDisplayList(true);

            switch (info.mode)
            {
            case trpgBillboard::World:
                billboard->setMode(osg::Billboard::POINT_ROT_WORLD);
                break;
            case trpgBillboard::Eye:
                billboard->setMode(osg::Billboard::POINT_ROT_EYE);
                break;
            default:
                billboard->setMode(osg::Billboard::AXIAL_ROT);
                break;
            }

            switch (info.type)
            {
            case trpgBillboard::Individual:
            {
                // compute center of billboard geometry
                const osg::BoundingBox& bbox = geometry->getBound();
                osg::Vec3 center ((bbox._min + bbox._max) * 0.5f);

                // make billboard geometry coordinates relative to computed center
                osg::Matrix matrix;
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
                osg::Vec3 center(info.center.x, info.center.y, info.center.z);

                // make billboard geometry coordinates relative to specified center
                osg::Matrix matrix;
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
                osg::notify(osg::WARN) << "TerraPage loader: fell through case: " <<  __FILE__ << " " << __LINE__  << ".\n";
                break;
            }

            top->addChild(billboard);
        }
        else
        if (_parse->underLayerSubgraph())
        {
            osg::Geode* layer = _parse->getLayerGeode();

            if (layer->getNumDrawables())
            {
                osg::StateSet* poStateSet = new osg::StateSet;
                osg::PolygonOffset* polyoffset = new osg::PolygonOffset;

                poStateSet->merge(*sset.get());
                polyoffset->setFactor(-2.0f*layer->getNumDrawables());
                polyoffset->setUnits(-10.0f*layer->getNumDrawables());
                poStateSet->setAttributeAndModes(polyoffset,osg::StateAttribute::ON);

                geometry->setStateSet(poStateSet);
            }
            else
            {
                geometry->setStateSet(sset.get());
            }

            layer->addDrawable(geometry);
        }
        else
        {
            geometry->setStateSet(sset.get());
            if (geodeTop)
            {
                geodeTop->getGeode()->addDrawable(geometry);
            }
            else
            {
                osg::Geode* geode = new osg::Geode;
                geode->addDrawable(geometry);
                top->addChild(geode);
            }
        }
        
        
    }

    return (void *) 1;
}


namespace
{
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
osg::Texture2D* txp::getLocalTexture(trpgrImageHelper& image_helper, const trpgTexture* tex)
{
    osg::Texture2D* osg_texture= 0L;

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
        osg_texture = new osg::Texture2D();

        osg::Image* image = new osg::Image;
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
            osg::Image::MipmapDataType mipmaps;
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
osg::Texture2D* txp::getTemplateTexture(trpgrImageHelper& image_helper, trpgLocalMaterial* locmat, const trpgTexture* tex, int index)
{
    osg::Texture2D* osg_texture= 0L;

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
        osg_texture = new osg::Texture2D();

        osg::Image* image = new osg::Image;
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
            osg::Image::MipmapDataType mipmaps;
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


