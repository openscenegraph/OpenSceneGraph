#include <stdlib.h>

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
#include <osg/ApplicationUsage>
#include <osgText/Text>

#include <osgUtil/Optimizer>

#include "TXPParser.h"
#include "TXPArchive.h"

using namespace txp;

#include <sstream>

static osg::ApplicationUsageProxy TXP_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_TXP_DEFAULT_MAX_ANISOTROPY \"<value> [<value>]\"","1.0 | 2.0 | 4.0 | 8.0 | 16.0");


class LayerGroup : public osg::Group
{
public:
    LayerGroup() : osg::Group() {}
    
    LayerGroup(const LayerGroup& gg,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
        osg::Group(gg, copyop)
    {};
    
    META_Node(txp, LayerGroup);
protected:
    virtual ~LayerGroup() {}
};

class LayerVisitor : public osg::NodeVisitor
{
public:
    LayerVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
    virtual void apply(osg::Group& node)
    {
        LayerGroup *lg = dynamic_cast<LayerGroup*>(&node);
        if (lg)
        {
            for (unsigned int i=1; i < lg->getNumChildren(); ++i)
            {
                osg::Node *child = lg->getChild(i);
                osg::StateSet *sset = child->getOrCreateStateSet();
                osg::PolygonOffset* polyoffset = new osg::PolygonOffset;
                polyoffset->setFactor(-1.0f);
                polyoffset->setUnits(-200.0f*i);
                sset->setAttributeAndModes(polyoffset,osg::StateAttribute::ON);
            }
        }
        traverse(node);
    }
};

TXPParser::TXPParser():
    _archive(0),
    _currentTop(0),
    _root(0),
    _underBillboardSubgraph(false),
    _numBillboardLevels(0),
    _underLayerSubgraph(false),
    _numLayerLevels(0),
    _layerGeode(0),
    _defaultMaxAnisotropy(1.0f),
    _realMinRange(0.0),
    _realMaxRange(0.0),
    _usedMaxRange(0.0),
    _childRefCB(0)
{
    AddCallback(TRPG_ATTACH,    new attachRead(this));
    AddCallback(TRPG_CHILDREF,  new childRefRead(this));
    AddCallback(TRPG_GEOMETRY,  new geomRead(this));
    AddCallback(TRPG_GROUP,     new groupRead(this));
    AddCallback(TRPG_LOD,       new lodRead(this));
    AddCallback(TRPG_MODELREF,  new modelRefRead(this));
    AddCallback(TRPG_BILLBOARD, new billboardRead(this));
    AddCallback(TRPG_LIGHT,     new lightRead(this));
    AddCallback(TRPG_LAYER,     new layerRead(this));
    AddCallback(TRPG_LABEL,     new labelRead(this));
    AddCallback(TRPGTILEHEADER, new tileHeaderRead(this));

    _childRefCB = dynamic_cast<childRefRead *>(GetCallback(TRPG_CHILDREF));
    
    if (getenv("OSG_TXP_DEFAULT_MAX_ANISOTROPY"))
    {
        _defaultMaxAnisotropy = osg::asciiToFloat(getenv("OSG_TXP_DEFAULT_MAX_ANISOTROPY"));
    }
    
}

TXPParser::~TXPParser()
{
}

osg::Group *TXPParser::parseScene(
    trpgReadBuffer &buf, 
    std::map<int,osg::ref_ptr<osg::StateSet> > &materials,
    std::map<int,osg::ref_ptr<osg::Node> > &models,
    double realMinRange, double realMaxRange, double usedMaxRange)
{
    if (_archive == 0) return NULL;

    if(_childRefCB)
       _childRefCB->Reset();

    _root = new osg::Group();
    _currentTop = _root.get();

    _materialMap = &materials;
    _localMaterials.clear();
    _models = &models;

    _underBillboardSubgraph = false;
    _numBillboardLevels = 0;
    _underLayerSubgraph = false;
    _numLayerLevels = 0;

    _realMinRange = realMinRange;
    _realMaxRange = realMaxRange;
    _usedMaxRange = usedMaxRange;

    _tileCenter = osg::Vec3(0.f,0.f,0.f);

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

    try
    {
       LayerVisitor lv;
       _root->accept(lv);

       //modified by Brad Anderegg May-27-08
       //running the optimizer on the terrain fixes some major preformance issues, unfortunately the texture atlas builder seems to get messed up 
       //on some of the textures (usually around buildings) and the tri stripper seems to occasionally crash and also mess up the indices on certain buildings.
       osgUtil::Optimizer opt;
       opt.optimize(_root.get(), (osgUtil::Optimizer::ALL_OPTIMIZATIONS ^ osgUtil::Optimizer::TEXTURE_ATLAS_BUILDER) ^ osgUtil::Optimizer::TRISTRIP_GEOMETRY);
    }
    catch (...)
    {
       osg::notify(osg::NOTICE) << "txp::TXPParser::parseScene(): exception thrown in the osg::Optimizer" << std::endl;
    }


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

            _tileCenter = loLOD->getCenter();

            group->addChild(loLOD->getChild(0));
            group->removeChild(loLOD);
            group->removeChild(hiLOD);
        }
    }
}


unsigned int TXPParser::GetNbChildrenRef() const
{
   if(_childRefCB)
         return _childRefCB->GetNbChildrenRef();
      else
         return 0;
}

const trpgChildRef* TXPParser::GetChildRef(unsigned int idx) const
{
   if(_childRefCB)
      return _childRefCB->GetChildRef(idx);
   else
      return 0;
}

bool TXPParser::StartChildren(void * /*in*/)
{

    bool pushParent = true;
    if (_underBillboardSubgraph )
    {
        if (_numBillboardLevels > 0) pushParent = false;
        _numBillboardLevels++;
    }
    else
    if (_underLayerSubgraph)
    {
        if (_numLayerLevels > 0) pushParent = false;
        _numLayerLevels++;
    }
    if (pushParent)
    {
        _parents.push(_currentTop);
        _currentTop = _currentNode->asGroup();
    }
    
    return true;
}

bool TXPParser::EndChildren(void *)
{
    bool popParent = true;
    if (_underLayerSubgraph)
    {
        _numLayerLevels--;
        if (_numLayerLevels == 0)
        {
            _underLayerSubgraph = false;
        }
        else
            popParent = false;
    }
    else
    if (_underBillboardSubgraph)
    {
        _numBillboardLevels--;
        if (_numBillboardLevels == 0)
        {
            _underBillboardSubgraph = false;
        }
        else
            popParent = false;
    }
    if (popParent)
    {
        if (_parents.size())
        {
            _currentTop = _parents.top();
            _parents.pop();
        }
        else
            _currentTop = _root.get();
    }

    return true;
}

DeferredLightAttribute& TXPParser::getLightAttribute(int ix)
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

    FindEmptyGroupsVisitor& operator = (const FindEmptyGroupsVisitor&) { return *this; }

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
    bool separateGeo = false;
    int majorVer,minorVer;
    this->getArchive()->GetVersion(majorVer,minorVer);
    if((majorVer >= TRPG_NOMERGE_VERSION_MAJOR) && (minorVer>=TRPG_NOMERGE_VERSION_MINOR)) {
    separateGeo = true;
    }
    // new to 2.0 LOCAL materials
    trpgrImageHelper image_helper(
        _archive->GetEndian(),
        _archive->getDir(),
        *_archive->GetMaterialTable(),
        *_archive->GetTexTable(),
        separateGeo
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
            
            const trpgMaterial* mat = 0;
            const trpgTexture *tex = 0;

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
                    osg_texture->setWrap(osg::Texture2D::WRAP_S, wrap_s == trpgTextureEnv::Repeat ? osg::Texture2D::REPEAT: osg::Texture2D::CLAMP_TO_EDGE );
                    osg_texture->setWrap(osg::Texture2D::WRAP_T, wrap_t == trpgTextureEnv::Repeat ? osg::Texture2D::REPEAT: osg::Texture2D::CLAMP_TO_EDGE );
                    
                    // by default is anisotropic filtering.
                    osg_texture->setMaxAnisotropy(_defaultMaxAnisotropy);
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

            _archive->SetUserDataToMaterialAttributes(*osg_state_set, *mat);

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

bool TXPParser::requestModel(int ix) 
{
    return _archive->loadModel(ix); 
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
    osg::ref_ptr<GeodeGroup> osgLodG = new GeodeGroup;

    osgLod->addChild(osgLodG.get());

    osg::Vec3 osgCenter;
    osgCenter[0] = center.x;
    osgCenter[1] = center.y;
    osgCenter[2] = center.z;
    osgLod->setCenter(osgCenter);
#if 1
    osgLod->setRange(0,minRange, maxRange );
#else
    osgLod->setRange(
        0,
        _parse->checkAndGetMinRange(minRange), 
        _parse->checkAndGetMaxRange(maxRange)
    );
#endif
    
    // Our LODs are binary so we need to add a group under this LOD and attach stuff
    // to that instead of the LOD
    // Add it into the scene graph
    _parse->setCurrentNode(osgLodG.get());
    _parse->getCurrTop()->addChild(osgLod.get());
    _parse->setPotentionalTileGroup(_parse->getCurrTop());

    return (void*)1;
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
    std::map<int,osg::ref_ptr<osg::Node> >*modelList = _parse->getModels();
    //if( modelList->size() )
    {
       osg_Model = (*modelList)[modelID].get();

       if (osg_Model==NULL)
       {
           _parse->requestModel(modelID);
           osg_Model = (*modelList)[modelID].get();
       }

        // Create the SCS and position the model
        if (osg_Model)
        {
            osg::MatrixTransform *scs = new osg::MatrixTransform();
            scs->setMatrix(osg_Mat);
            scs->addChild(osg_Model);

            // Add the SCS to the hierarchy
            _parse->setCurrentNode(scs);
            _parse->getCurrTop()->addChild(scs);
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
        GeodeGroup* grp = new GeodeGroup;
        _parse->setCurrentNode(grp);
        _parse->getCurrTop()->addChild(grp);

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

    if (_parse->underLayerSubgraph()) return (void*)1;

    osg::ref_ptr<GeodeGroup> osgGroup = new GeodeGroup();
    _parse->setCurrentNode(osgGroup.get());
    _parse->getCurrTop()->addChild(osgGroup.get());
    return (void*)1;
}

//----------------------------------------------------------------------------
//
// Attach Reader Class
//
//----------------------------------------------------------------------------
void* attachRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgAttach group;
    if (!group.Read(buf)) return NULL;

    // Create a new group
    osg::ref_ptr<osg::Group> osgGroup = new osg::Group();
    _parse->setCurrentNode(osgGroup.get());
    _parse->getCurrTop()->addChild(osgGroup.get());
    return (void*)1;
}

void* childRefRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
   
   // This object contribute nothing to the scenegraph, except
   // where the children tile should connect.
   // It only contain location info of the children tile
   childRefList.push_back(trpgChildRef());
   trpgReadWriteable& obj = childRefList.back();

   if(obj.Read(buf))
      return &obj;
    else
      return 0;

}
void childRefRead::Reset()
{
   childRefList.clear();
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

    uint32 nvert;
    light.GetNumVertices(nvert);

    const trpgLightTable *lt = _parse->getArchive()->GetLightTable();
    trpgLightAttr *ref = const_cast<trpgLightAttr*>(lt->GetLightAttrRef(attr_index));
    if (!ref)
    {
        osg::notify(osg::NOTICE) << "NULL LightAttr " << attr_index << std::endl;
        return (void*)1;
    }

    osgSim::LightPointNode *lpNode = new osgSim::LightPointNode();

    trpgColor col;
    ref->GetFrontColor(col);

    float64 inten;
    ref->GetFrontIntensity(inten);

    trpgLightAttr::PerformerAttr perfAttr;
    ref->GetPerformerAttr(perfAttr);

    lpNode->setMaxPixelSize(perfAttr.maxPixelSize);
    lpNode->setMinPixelSize(perfAttr.minPixelSize);    

    trpg3dPoint norm;
    ref->GetNormal(norm);

    trpgLightAttr::LightDirectionality direc;
    ref->GetDirectionality(direc);

    for ( unsigned i=0; i < nvert; i++ )
    {
        trpg3dPoint pt;
        light.GetVertex(i, pt);

        osgSim::LightPoint lp( 
            true, 
            osg::Vec3(pt.x,pt.y,pt.z), 
            osg::Vec4(col.red, col.green,col.blue, 1.0), 
            inten
        );

        switch (direc)
        {
        case trpgLightAttr::trpg_Unidirectional:
            {
                float lobeVert=0.f, lobeHorz=0.f, lobeRoll=0.f;
                float64 tmp;

                ref->GetHLobeAngle(tmp);
                lobeHorz = osg::DegreesToRadians( tmp );
                
                ref->GetVLobeAngle(tmp);
                lobeVert = osg::DegreesToRadians( tmp );
                
                ref->GetLobeRollAngle(tmp);
                lobeRoll = osg::DegreesToRadians( tmp );

                osg::Vec3 normal(norm.x,norm.y,norm.z);
                lp._sector = new osgSim::DirectionalSector( normal, lobeHorz, lobeVert, lobeRoll );
            }
            break;
        case trpgLightAttr::trpg_Bidirectional:
            {
                float lobeVert=0.f, lobeHorz=0.f, lobeRoll=0.f;
                float64 tmp;

                ref->GetHLobeAngle(tmp);
                lobeHorz = osg::DegreesToRadians( tmp );
                
                ref->GetVLobeAngle(tmp);
                lobeVert = osg::DegreesToRadians( tmp );
                
                ref->GetLobeRollAngle(tmp);
                lobeRoll = osg::DegreesToRadians( tmp );

                osg::Vec3 normal(norm.x,norm.y,norm.z);
                lp._sector = new osgSim::DirectionalSector( normal, lobeHorz, lobeVert, lobeRoll );

                ref->GetBackColor(col);
                ref->GetBackIntensity(inten);

                osgSim::LightPoint lp2( 
                    true, 
                    osg::Vec3(pt.x,pt.y,pt.z), 
                    osg::Vec4(col.red, col.green,col.blue, 1.0), 
                    inten
                );

                lp2._sector = new osgSim::DirectionalSector( -normal, lobeHorz, lobeVert, lobeRoll );
                lpNode->addLightPoint( lp2 );
            }
            break;
        default:
            ;
        }

        lpNode->addLightPoint( lp);

    }

    _parse->setCurrentNode(lpNode);
    _parse->getCurrTop()->addChild(lpNode);

#if 0

    DefferedLightAttribute& dla = _parse->getLightAttribute(attr_index); 
    osgSim::LightPointNode* node = dla.lightPoint.get();

    uint32 nvert;
    light.GetNumVertices(nvert);

    if( node->getLightPoint(0)._sector.valid() ) // osgSim::LightPoint is a must
    {
        for(unsigned int i = 0; i < nvert; i++)
        {
            trpg3dPoint pt;
            light.GetVertex(i, pt);
            osg::Matrix matrix;
        //        matrix.makeTranslate(pt.x,pt.y,pt.z);
            matrix.makeRotate(osg::Quat(0.0,dla.attitude));
            matrix.setTrans(pt.x,pt.y,pt.z);
            osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform();
            trans->setMatrix(matrix);
            trans->addChild(node);

            _parse->setCurrentNode(trans.get());
            _parse->getCurrTop()->addChild(trans.get());
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

        osg::Geode* g = new osg::Geode;
        g->addDrawable(geom.get());
        _parse->setCurrentNode(g);
        _parse->getCurrTop()->addChild(g);
    }
#endif

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

    osg::ref_ptr<LayerGroup> osgGroup = new LayerGroup();
    _parse->setCurrentNode(osgGroup.get());
    _parse->getCurrTop()->addChild(osgGroup.get());
    return (void*)1;
}

//----------------------------------------------------------------------------
//
// Label Reader Class
//
//----------------------------------------------------------------------------
void* labelRead::Parse(trpgToken /*tok*/,trpgReadBuffer &buf)
{
    trpgLabel label;
    if (!label.Read(buf)) return NULL;

    const std::string *labelText = label.GetText();
    if (!labelText) return (void*)1;

    osg::Vec3 pos(label.GetLocation().x, label.GetLocation().y, label.GetLocation().z);

    osg::ref_ptr< osg::Geode > textGeode = new osg::Geode;
    osg::ref_ptr< osgText::Text > text = new osgText::Text;

    // Text
    std::ostringstream os;
    std::string::size_type nl;
    std::string lb = *labelText;
    while ( (nl=lb.find_first_of('\\')) != std::string::npos)
    {
        std::string sub = lb.substr(0,nl);
        switch (lb[nl+1])
        {
        case 'n':
            lb.erase(0,nl+2);
            if (sub.length()) os << sub << std::endl;
            break;
        case 't':
            lb.erase(0,nl+2);
            os << sub << "     ";//'\t';
            break;
        default:
            lb.erase(0,nl+1);
            os << '\\' << sub;
            break;
        }
        
    }
    if (lb.length()) os << lb;
    text->setText(os.str());

    // Position
    text->setPosition(pos);
    // Alignment
    switch (label.GetAlignment())
    {
    case trpgLabel::Left:
        text->setAlignment(osgText::Text::LEFT_BOTTOM);
        break;
    case trpgLabel::Right:
        text->setAlignment(osgText::Text::RIGHT_BOTTOM);
        break;
    default:
        text->setAlignment(osgText::Text::CENTER_BOTTOM);
    }
    // Axis alignment
    text->setAxisAlignment(osgText::Text::XZ_PLANE);

    const trpgLabelPropertyTable *labelPropertyTable = _parse->getArchive()->GetLabelPropertyTable();
    const trpgLabelProperty *labelProperty = labelPropertyTable ? 
        labelPropertyTable->GetPropertyRef(label.GetProperty()) : 0;
    
    bool addTextGeodeIntoSceneGraph = true;
    if (labelProperty)
    {
        const trpgTextStyleTable *textStyleTable = _parse->getArchive()->GetTextStyleTable();
        if (!textStyleTable) return (void*)1;

        const trpgTextStyle *textStyle = textStyleTable->GetStyleRef(labelProperty->GetFontStyle());
        if (!textStyle) return (void*)1;

        // Size
        text->setCharacterSize(textStyle->GetCharacterSize()*label.GetScale()*2);
        text->setCharacterSizeMode(osgText::Text::OBJECT_COORDS);

        // Font
        text->setFont(_parse->getArchive()->getStyles()[labelProperty->GetFontStyle()].get());

        // Color
        text->setColor(_parse->getArchive()->getTextColors()[labelProperty->GetFontStyle()]);

        // Cube
        osg::ref_ptr<osg::ShapeDrawable> cube = 0;

        // Type
        switch (labelProperty->GetType())
        {
        case trpgLabelProperty::Billboard:
            text->setAxisAlignment(osgText::Text::XY_PLANE);
            text->setAutoRotateToScreen(true);
            break;
        case trpgLabelProperty::VertBillboard:
            addTextGeodeIntoSceneGraph = false;
            {
                osg::ref_ptr< osg::Billboard > billboard = new osg::Billboard;
                text->setPosition(osg::Vec3(0.f,0.f,0.f));
                billboard->addDrawable(text.get());
                billboard->setAxis(osg::Vec3(0.0f,0.0,1.0f) );
                billboard->setNormal(osg::Vec3(0.0f,-1.0,0.0f));
                billboard->setMode(osg::Billboard::AXIAL_ROT);
                billboard->setPosition(0,pos);

                _parse->getCurrTop()->addChild(billboard.get());
            }
            break;
        case trpgLabelProperty::Cube:
            addTextGeodeIntoSceneGraph = false;
            {
                osg::Group* group = new osg::Group;

                osg::BoundingBox box = text->getBound();
                float shift = box.radius()+1.f;

                // front
                text->setAlignment(osgText::Text::CENTER_CENTER);

                // back
                osg::ref_ptr<osgText::Text> backText = new osgText::Text(*text);
                backText->setPosition(osg::Vec3(pos.x(),pos.y()+shift,pos.z()));
                backText->setAxisAlignment(osgText::Text::REVERSED_XZ_PLANE);

                // top
                osg::ref_ptr<osgText::Text> topText = new osgText::Text(*text);
                topText->setPosition(osg::Vec3(pos.x(),pos.y(),pos.z()+shift));
                topText->setAxisAlignment(osgText::Text::XY_PLANE);

                // bottom
                osg::ref_ptr<osgText::Text> bottomText = new osgText::Text(*text);
                bottomText->setPosition(osg::Vec3(pos.x(),pos.y(),pos.z()-shift));
                bottomText->setAxisAlignment(osgText::Text::REVERSED_XY_PLANE);

                // left
                osg::ref_ptr<osgText::Text> leftText = new osgText::Text(*text);
                leftText->setPosition(osg::Vec3(pos.x()-shift,pos.y(),pos.z()));
                leftText->setAxisAlignment(osgText::Text::REVERSED_YZ_PLANE);

                // right
                osg::ref_ptr<osgText::Text> rightText = new osgText::Text(*text);
                rightText->setPosition(osg::Vec3(pos.x()+shift,pos.y(),pos.z()));
                rightText->setAxisAlignment(osgText::Text::YZ_PLANE);

                text->setPosition(osg::Vec3(pos.x(),pos.y()-shift,pos.z()));

                osg::TessellationHints* hints = new osg::TessellationHints;
                hints->setDetailRatio(0.5f);
                cube = new osg::ShapeDrawable(new osg::Box(pos,2*shift),hints);

                osg::ref_ptr<osg::PolygonOffset> polyoffset = new osg::PolygonOffset;
                polyoffset->setFactor(10.0f);
                polyoffset->setUnits(10.0f);
                osg::ref_ptr<osg::StateSet> ss = cube->getOrCreateStateSet();
                ss->setAttributeAndModes(polyoffset.get(),osg::StateAttribute::ON);
                cube->setStateSet(ss.get());

                textGeode->addDrawable(cube.get());
                textGeode->addDrawable(text.get());
                textGeode->addDrawable(backText.get());
                textGeode->addDrawable(topText.get());
                textGeode->addDrawable(bottomText.get());
                textGeode->addDrawable(leftText.get());
                textGeode->addDrawable(rightText.get());

                group->addChild(textGeode.get());

                _parse->getCurrTop()->addChild(group);
            }
            break;
        default:
            break;
        }

         const std::vector<trpg3dPoint> *supports = label.GetSupports();
         if (supports && supports->size())
         {
             osg::ref_ptr<osg::Geode> supGeode = new osg::Geode;

             int supId = labelProperty->GetSupport();
             const trpgSupportStyleTable *supTable = _parse->getArchive()->GetSupportStyleTable();
             const trpgSupportStyle *supStyle = supTable ? supTable->GetStyleRef(supId) : 0;
             if (supStyle)
             {
                int matId = supStyle->GetMaterial();

                osg::Vec4 supLineColor(1.f,1.f,1.f,1.f);
                 _parse->loadMaterial(matId);
                osg::ref_ptr<osg::StateSet>  sset = (*_parse->getMaterials())[matId];

                if (cube.get())
                {
                    osg::StateSet* ss = cube->getOrCreateStateSet();
                    ss->merge(*sset);
                }

                const trpgMatTable* matTable = _parse->getArchive()->GetMaterialTable();
                if (matTable)
                {
                    const trpgMaterial* mat = matTable->GetMaterialRef(0,matId);
                    if (mat)
                    {
                        trpgColor faceColor;
                        mat->GetColor(faceColor);

                        float64 alpha;
                        mat->GetAlpha(alpha);

                        supLineColor = osg::Vec4(faceColor.red, faceColor.green, faceColor.blue, alpha );
                    }
                }
                
                 switch (supStyle->GetType())
                 {
                 case trpgSupportStyle::Line:
                     {
                        osg::Geometry* linesGeom = new osg::Geometry();
                        osg::Vec3Array* vertices = new osg::Vec3Array(supports->size()*2);

                        int cnt = 0;
                        for (unsigned int i = 0; i < supports->size(); i++)
                        {
                            const trpg3dPoint& supPt = (*supports)[i];
                            (*vertices)[cnt++].set(pos);
                            (*vertices)[cnt++].set(osg::Vec3(supPt.x,supPt.y,supPt.z));
                        }

                        linesGeom->setVertexArray(vertices);

                        osg::Vec4Array* colors = new osg::Vec4Array;
                        colors->push_back(supLineColor);
                        linesGeom->setColorArray(colors);
                        linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
                        
                        osg::Vec3Array* normals = new osg::Vec3Array;
                        normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
                        linesGeom->setNormalArray(normals);
                        linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);


                        linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,supports->size()*2));
                        supGeode->addDrawable(linesGeom);
                     }

                     _parse->getCurrTop()->addChild(supGeode.get());
                     break;
                 case trpgSupportStyle::Cylinder:
                     {
                        osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
                        hints->setDetailRatio(0.5f);

                        for (unsigned int i = 0; i < supports->size(); i++)
                        {
                            const trpg3dPoint& supPt = (*supports)[i];
                            
                            osg::Vec3 supPos(supPt.x,supPt.y,supPt.z);
                            osg::Vec3 supCenter = (supPos+pos)/2.f;
                            float supHeight = (supPos-pos).length();

                            osg::Vec3 d = pos-supPos;
                            d.normalize();
                            osg::Quat r;

                            r.makeRotate(osg::Vec3(0.f,0.f,1.f),d);

                            osg::Cylinder* cylinder = new osg::Cylinder(supCenter,10.f,supHeight);
                            cylinder->setRotation(r);

                            osg::ShapeDrawable* cylinderDrawable = new osg::ShapeDrawable(cylinder,hints.get());
                            osg::StateSet* ss = cylinderDrawable->getOrCreateStateSet();
                            ss->merge(*sset);

                            supGeode->addDrawable(cylinderDrawable);

                        }
                        
                        _parse->getCurrTop()->addChild(supGeode.get());
                     }
                     break;
                 default:
                    break;
                 }

                 
             }
         }
    }
    if (addTextGeodeIntoSceneGraph)
    {
        _parse->getCurrTop()->addChild(textGeode.get());
        textGeode->addDrawable(text.get());
    }

    return (void*)1;
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
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(numVert);
    geom.GetVertices((float *)&(vertices->front()));
    
    // Turn the trpgGeometry into something osg can understand
    osg::ref_ptr<osg::Geometry> geometry;
    
    // Get texture coordinates
    ;
    int num_tex;
    geom.GetNumTexCoordSets(num_tex);
    std::vector< osg::ref_ptr<osg::Vec2Array> > tex_coords(num_tex);
    for (int texno = 0; texno < num_tex; texno++)
    {
        const trpgTexData* td = geom.GetTexCoordSet(texno);
        if (td)
        {
            tex_coords[texno] = new osg::Vec2Array(numVert);
            const float* sourcePtr = &(td->floatData[0]); 
            float* destinationPtr = (float*)&((*tex_coords[texno])[0]);
            for (int i=0 ;i < numVert; ++i)
            {
                *destinationPtr++ = *sourcePtr++;
                *destinationPtr++ = *sourcePtr++;
            }
        }
    }
   
    // The normals
    osg::ref_ptr<osg::Vec3Array> normals;
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
    geom.GetPrimLengths(reinterpret_cast<int*>(&(dal->front())));
    geometry->addPrimitiveSet(dal);
    }
    break;
    case trpgGeometry::TriFans:
    {
    geometry = new osg::Geometry;
    osg::DrawArrayLengths* dal = new osg::DrawArrayLengths(osg::PrimitiveSet::TRIANGLE_FAN,0,numPrims);
    geom.GetPrimLengths(reinterpret_cast<int*>(&(dal->front())));
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
            if( tex_coords[texno].valid() )
            {
            std::swap((*tex_coords[texno])[start], (*tex_coords[texno])[end]);
            }
        }
        if(normals.valid())
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

    if (geometry.valid() && top)
    {

       //modifed by Brad Anderegg on May-27-08
       //using display lists actually increases our framerate by 
       //a fair amount, on certain laptops it increased by as much as 1000%
       geometry->setUseDisplayList(true);

        geometry->setVertexArray(vertices.get());
        if (normals.valid())
        {
            geometry->setNormalArray(normals.get());
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
            {
                _parse->loadMaterial(matId);
                tmp_ss = (*_parse->getMaterials())[matId];
            }
            if(sset.valid()) 
            {
                if(tmp_ss.valid())
                  {
                    osg::StateAttribute* texenv0 = tmp_ss->getTextureAttribute(0,osg::StateAttribute::TEXENV);
                    if(texenv0) 
                        sset->setTextureAttribute(n_mat,texenv0);
                    osg::StateAttribute* tex0 = tmp_ss->getTextureAttribute(0,osg::StateAttribute::TEXTURE);
                    if(tex0) 
                        sset->setTextureAttributeAndModes(n_mat,tex0,osg::StateAttribute::ON);
                    // submitted by a. danklefsen
                    // Alion science and Technology 2/12/07
                    // copy fid/smc codes over to this new state set from the prev state set.
                    sset->setUserData(tmp_ss->getUserData());
                }
//                sset->merge(*tmp_ss.get());
            }
            else 
                sset = tmp_ss;
        }

        if (!tex_coords.empty())
        {
            for (int texno = 0; texno < num_tex; texno++)
            {
                geometry->setTexCoordArray( texno, tex_coords[texno].get());
            }
        }

        if (_parse->underBillboardSubgraph())
        {
            geometry->setStateSet(sset.get());

            TXPParser::TXPBillboardInfo info;
            _parse->getLastBillboardInfo(info);

            osg::ref_ptr<osg::Billboard> billboard = new osg::Billboard;
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

                billboard->addDrawable(geometry.get());
                billboard->setPosition(0, center);
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

                billboard->addDrawable(geometry.get());
                billboard->setPosition(0, center);
            }
            break;
            default:
                billboard->addDrawable(geometry.get());
                osg::notify(osg::WARN) << "TerraPage loader: fell through case: " <<  __FILE__ << " " << __LINE__  << ".\n";
                break;
            }

            top->addChild(billboard.get());
        }
        else
#if 0
        if (_parse->underLayerSubgraph())
        {
            osg::Geode* layer = _parse->getLayerGeode();

            if (layer->getNumDrawables())
            {
                osg::StateSet* poStateSet = new osg::StateSet;
                osg::PolygonOffset* polyoffset = new osg::PolygonOffset;

                poStateSet->merge(*sset.get());
                polyoffset->setFactor(-1.0f);//*layer->getNumDrawables());
                polyoffset->setUnits(-20.0f);//*layer->getNumDrawables());
                poStateSet->setAttributeAndModes(polyoffset,osg::StateAttribute::ON);


                geometry->setStateSet(poStateSet);
            }
            else
            {
                geometry->setStateSet(sset.get());
            }

            layer->addDrawable(geometry.get());
        }
        else
#else
        {
            geometry->setStateSet(sset.get());
            if (geodeTop)
            {
                geodeTop->getGeode()->addDrawable(geometry.get());
                _parse->setCurrentNode(geodeTop->getGeode());
            }
            else
            {
                osg::Geode* geode = new osg::Geode;
                geode->addDrawable(geometry.get());

                _parse->setCurrentNode(geode);
                _parse->getCurrTop()->addChild(geode);
            }
        }
#endif        
        
    }
    else
    {
        osg::notify(osg::WARN)<<"Detected potential memory leak in TXPParser.cpp"<<std::endl;
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
        case trpgTexture::trpg_MCM5:
        case trpgTexture::trpg_MCM6R:
        case trpgTexture::trpg_MCM6A:
        case trpgTexture::trpg_MCM7RA:
        case trpgTexture::trpg_MCM7AR:
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

        // make sure the Texture unref's the Image after apply, when it is no longer needed.                
        osg_texture->setUnRefImageDataAfterApply(true);

        osg::Image* image = new osg::Image;
        char* data = 0L;

        bool bMipmap;
        tex->GetIsMipmap(bMipmap);
        int32 num_mipmaps = bMipmap  ?  tex->CalcNumMipmaps() : 1; // this is currently line 130

        // osg::Image do their own mipmaps
        if(num_mipmaps <= 1)
        {
            int32 size = tex->CalcTotalSize();
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

            image->setMipmapLevels(mipmaps);

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

        // make sure the Texture unref's the Image after apply, when it is no longer needed.                
        osg_texture->setUnRefImageDataAfterApply(true);

        osg::Image* image = new osg::Image;
        char* data = 0L;

        bool bMipmap;
        tex->GetIsMipmap(bMipmap);
        int32 num_mipmaps = bMipmap  ?  tex->CalcNumMipmaps() : 1; // this is currently line 130

        // osg::Image do their own mipmaps
        if(num_mipmaps <= 1)
        {
            int32 size = tex->CalcTotalSize();
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

            image->setMipmapLevels(mipmaps);

        }

        osg_texture->setImage(image);
    }
    return osg_texture;
}



