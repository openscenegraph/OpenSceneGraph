#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "TrPageArchive.h"
#include "TrPageParser.h"

#include <osg/AlphaFunc>
#include <osg/Group>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/Material>
#include <osg/TexEnv>
#include <osg/CullFace>
#include <osg/Light>
#include <osg/StateSet>
#include <osg/Notify>
#include <osgDB/FileUtils>

#include <iostream>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>

#include <osgSim/Sector>
#include <osgSim/LightPoint>
#include <osgSim/LightPointNode>
#include <osgSim/BlinkSequence>
#include <osg/Point>
#include <osg/BlendFunc>

#include "trpage_geom.h"
#include "trpage_read.h"
#include "trpage_write.h"
#include "trpage_scene.h"
#include "trpage_managers.h"


using namespace txp;
using namespace osg;

// disable 'this' used in base pointer initilialization..
#if defined(WIN32) && !defined(__GNUC__)
    #pragma warning( disable : 4355 )
#endif


TrPageArchive::TrPageArchive()
: trpgr_Archive()
, parse(new TrPageParser(this))
, buf(GetEndian())
{
}

TrPageArchive::~TrPageArchive()
{
}

bool TrPageArchive::OpenFile(const char* file)
{
    m_alternate_path = osgDB::getFilePath(file);
    std::string name = osgDB::getSimpleFileName(file);
    
    if(m_alternate_path.empty())
        SetDirectory(".");
    else
    {
        // push the path to the front of the list so that all subsequenct
        // files get loaded relative to this if possible.
        osgDB::getDataFilePathList().push_front(m_alternate_path);
        SetDirectory(m_alternate_path.c_str());
    }
    
    if (!trpgr_Archive::OpenFile(name.c_str()))
    {
        notify(WARN) << "TrPageArchive::OpenFile() error: "
            << "couldn't open archive: " << file << std::endl;
        return false;
    }
    if (!ReadHeader())
    {
        notify(WARN) << "TrPageArchive::OpenFile() error: "
            << "couldn't read header for archive: " << file
            << std::endl;
        return false;
    }
    
    // Set the max group ID here because it won't change
    const trpgHeader *head = GetHeader();
    int maxID;
    head->GetMaxGroupID(maxID);
    parse->SetMaxGroupID(maxID);
    
    return true;
}

/* Calculate the center of the database (including Z)
 */
void TrPageArchive::GetCenter(Vec3 &center)
{
	trpg2dPoint sw,ne;
    const trpgHeader *head = GetHeader();
	const trpgTileTable *tileTable = GetTileTable();
	head->GetExtents(sw,ne);
	trpg2dPoint tileSize;
	head->GetTileSize(0,tileSize);

	center[0] = (ne.x+sw.x)/2.0;
	center[1] = (ne.y+sw.y)/2.0;

	trpg2iPoint loc;
	loc.x = int((center[0]-sw.x)/tileSize.x);
	loc.y = int((center[1]-sw.y)/tileSize.y);
	trpgwAppAddress foo;
	float zmin,zmax;
	tileTable->GetTile(loc.x, loc.y,0,foo,zmin,zmax);
	center[2] = (zmin+zmax)/2.0;
}

// load textures and materials
// TODO : multitexturing 
void TrPageArchive::LoadMaterials()
{
    trpgrImageHelper image_helper(this->GetEndian(),getDir(),materialTable,texTable);

    int n_textures;

    texTable.GetNumTextures(n_textures);
    
    m_textures.resize(n_textures);
    
    // these extra braces are workaroud for annoying bug in MSVC 
    // for( int i = ....) and i is visible outside the loop
    {
        for (int i=0; i < n_textures ; i++)
        {
            const trpgTexture *tex;
            tex = texTable.GetTextureRef(i);
            trpgTexture::ImageMode mode;
            tex->GetImageMode(mode);
            if(mode == trpgTexture::External)
            {
                char texName[1024];  texName[0] = 0;
                tex->GetName(texName,1023);
                // Create a texture by name.
                ref_ptr<Texture2D> osg_texture = new Texture2D();
                
                // Load Texture and Create Texture State
                std::string filename = osgDB::getSimpleFileName(texName);
                std::string path(getDir());
#ifdef _WIN32
                const char _PATHD = '\\';
#elif defined(macintosh)
                const char _PATHD = ':';
#else
                const char _PATHD = '/';
#endif
                if( path == "." ) 
                    path = "";
                else
                    path += _PATHD ;
                
                std::string theFile = path + filename ;
                Image* image = osgDB::readImageFile(theFile);
                if (image)
                {
                    osg_texture->setImage(image);
                }
				else
				{
			        notify(WARN) << "TrPageArchive::LoadMaterials() error: "
					  << "couldn't open image: " << filename << std::endl;
				}
                m_textures[i] = osg_texture;
            }
            else if( mode == trpgTexture::Local )
            {
                m_textures[i] = GetLocalTexture(image_helper,tex);
            }
            else if( mode == trpgTexture::Template )
            {
                m_textures[i] = 0L; //GetTemplateTexture(image_helper,0, tex);
            }
            else
            {
                m_textures[i] = 0;
            }
        }
    }

    int n_materials;
    materialTable.GetNumMaterial(n_materials);
    {
        m_gstates.resize(n_materials);
        for (int i = 0; i < n_materials; i++)
        {
            StateSet* osg_state_set = new StateSet;
            
            const trpgMaterial *mat;
            mat = materialTable.GetMaterialRef(0,i);
            // Set texture
            int numMatTex;
            mat->GetNumTexture(numMatTex);
            
            // TODO : multitextuting
            // also note that multitexturing in terrapage can came from two sides
            // - multiple textures per material, and multiple materials per geometry
			// Note: Only in theory.  The only type you'll encounter is multiple
			//		 materials per polygon.
            if( numMatTex )
            {
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
                
                for (int ntex = 0; ntex < numMatTex; ntex ++ )
                {
                    int texId;
                    trpgTextureEnv texEnv;
                    mat->GetTexture(ntex,texId,texEnv);
                
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
                
                    osg_state_set->setTextureAttribute(ntex,osg_texenv);

                    int wrap_s, wrap_t;   
                    texEnv.GetWrap(wrap_s, wrap_t);

                    Texture2D* osg_texture = m_textures[texId].get();
                    if(osg_texture)
                    {

                        osg_texture->setWrap(Texture2D::WRAP_S, wrap_s == trpgTextureEnv::Repeat ? Texture2D::REPEAT: Texture2D::CLAMP );
                        osg_texture->setWrap(Texture2D::WRAP_T, wrap_t == trpgTextureEnv::Repeat ? Texture2D::REPEAT: Texture2D::CLAMP );

                        // -----------
                        // Min filter
                        // -----------
                        int32 minFilter;
                        texEnv.GetMinFilter(minFilter);
                        switch (minFilter)
                        {
                        case trpgTextureEnv::Point:
                        case trpgTextureEnv::Nearest:
                            osg_texture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::NEAREST);
                            break;
                        case trpgTextureEnv::Linear:
                            osg_texture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::LINEAR);
                            break;
                        case trpgTextureEnv::MipmapPoint:
                            osg_texture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::NEAREST_MIPMAP_NEAREST);
                            break;
                        case trpgTextureEnv::MipmapLinear:
                            osg_texture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::NEAREST_MIPMAP_LINEAR);
                            break;
                        case trpgTextureEnv::MipmapBilinear:
                            osg_texture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::LINEAR_MIPMAP_NEAREST);
                            break;
                        case trpgTextureEnv::MipmapTrilinear:
                            osg_texture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::LINEAR_MIPMAP_LINEAR);
                            break;
                        default:
                            osg_texture->setFilter(osg::Texture2D::MIN_FILTER, Texture2D::LINEAR);
                            break;
                        }


                        // -----------
                        // Mag filter
                        // -----------
                        int32 magFilter;
                        texEnv.GetMagFilter(magFilter);
                        switch (magFilter)
                        {
                        case trpgTextureEnv::Point:
                        case trpgTextureEnv::Nearest:
                            osg_texture->setFilter(osg::Texture2D::MAG_FILTER,Texture2D::NEAREST);
                            break;
                        case trpgTextureEnv::Linear:
                        default:
                            osg_texture->setFilter(osg::Texture2D::MAG_FILTER, Texture2D::LINEAR);
                            break;
                        }

                        // pass on to the stateset.                
                        osg_state_set->setTextureAttributeAndModes(ntex,osg_texture, StateAttribute::ON);

                        if(osg_texture->getImage() &&  osg_texture->getImage()->isImageTranslucent())
                        { 
                            osg_state_set->setMode(GL_BLEND,StateAttribute::ON);
                            osg_state_set->setRenderingHint(StateSet::TRANSPARENT_BIN);
                        }
                    }        
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
          }
          m_gstates[i] = osg_state_set;
        }
    }
}

void TrPageArchive::LoadLightAttributes()
{
	int num;
	lightTable.GetNumLightAttrs(num);
	for ( int attr_num = 0; attr_num < num; attr_num++	){

		trpgLightAttr* ref = const_cast<trpgLightAttr*>(lightTable.GetLightAttrRef(attr_num));

		osgSim::LightPointNode* osgLight = new osgSim::LightPointNode();

		osg::Point* osgPoint = new osg::Point();

		osgSim::LightPoint lp ;
		lp._on = true;

		trpgColor col;
		ref->GetFrontColor(col);
		lp._color = osg::Vec4(col.red, col.green,col.blue, 1.0);

		float64 inten;
		ref->GetFrontIntensity(inten);
		lp._intensity = inten;

		trpgLightAttr::PerformerAttr perfAttr;
		ref->GetPerformerAttr(perfAttr);

		// point part
		osgPoint->setSize(perfAttr.actualSize);
		osgPoint->setMaxSize(perfAttr.maxPixelSize);
		osgPoint->setMinSize(perfAttr.minPixelSize);	
        osgPoint->setFadeThresholdSize(perfAttr.transparentFallofExp);
        //numbers that are going to appear are "experimental"
        osgPoint->setDistanceAttenuation(osg::Vec3(0.0001, 0.0005, 0.00000025));
//        osgPoint->setDistanceAttenuation(osg::Vec3(1.0, 0.0, 1.0));

		osg::StateSet* stateSet = new osg::StateSet();
	    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	    stateSet->setMode(GL_POINT_SMOOTH, osg::StateAttribute::ON);
        stateSet->setAttributeAndModes(osgPoint, osg::StateAttribute::ON );
        stateSet->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::ON);

		osgLight->setMaxPixelSize(perfAttr.maxPixelSize);
		osgLight->setMinPixelSize(perfAttr.minPixelSize);	

//		float64 clamp;
//		ref->GetPerformerTpClamp(clamp);
//		osgLight->setMaxVisibleDistance2(clamp);

		trpg3dPoint normal;
		ref->GetNormal(normal);

//		lp._radius = clamp;

		trpgLightAttr::LightDirectionality direc;
		ref->GetDirectionality(direc);
		if( direc == trpgLightAttr::trpg_Unidirectional){
			osgSim::AzimElevationSector*  sec = new osgSim::AzimElevationSector();
			float64 tmp;
			ref->GetHLobeAngle(tmp);
			float64 tmpfade;
			ref->GetLobeFalloff(tmpfade);
			sec->setAzimuthRange(-tmp/2.0,tmp/2.0,tmpfade);

			ref->GetVLobeAngle(tmp);
			sec->setElevationRange(0,tmp, tmpfade);

			lp._sector = sec;
			osgLight->addLightPoint(lp);
		}
		else if( direc == trpgLightAttr::trpg_Bidirectional ){
			osgSim::AzimElevationSector*  front = new osgSim::AzimElevationSector();
			float64 tmp;
			ref->GetHLobeAngle(tmp);
			float64 tmpfade;
			ref->GetLobeFalloff(tmpfade);
			front->setAzimuthRange(-tmp/2.0,tmp/2.0,tmpfade);

			ref->GetVLobeAngle(tmp);
			front->setElevationRange(0,tmp, tmpfade);

			lp._sector = front;
			osgLight->addLightPoint(lp);

			osgSim::AzimElevationSector*  back = new osgSim::AzimElevationSector();
			back->setAzimuthRange(osg::PI-tmp/2.0,osg::PI+tmp/2.0,tmpfade);
			back->setElevationRange(0,tmp, tmpfade);
			lp._sector = back;
			osgLight->addLightPoint(lp);
		} 
		else{
			osgLight->addLightPoint(lp);
		}

		AddLightAttribute(osgLight, stateSet, osg::Vec3(normal.x,normal.y,normal.z));
	}
}

void TrPageArchive::AddLightAttribute(osgSim::LightPointNode* lpn, osg::StateSet* fallback, const osg::Vec3& att)
{
	DefferedLightAttribute la;
	la.lightPoint = lpn;
	la.fallback = fallback;
	la.attitude = att;
	lightAttrTable.push_back(la);
}


bool TrPageArchive::LoadModels()
{
   int numModel;
   modelTable.GetNumModels(numModel);

   // Iterate over the models
   for (int i=0; i< numModel; i++)
   {
      trpgModel *mod = modelTable.GetModelRef(i);
      int type;
      mod->GetType(type);

      // Only dealing with external models currently
      if (type == trpgModel::External)
      {
         char name[1024];
         mod->GetName(name,1023);

         // Load the model.  It's probably not TerraPage
         Node *osg_model = osgDB::readNodeFile(name);
         if (!osg_model)
         {
            notify(WARN) << "TrPageArchive::LoadModels() error: "
                              << "failed to load model: "
                              << name << std::endl;
         }
         // Do this even if it's NULL
         m_models.push_back(osg_model);
      }
/*
      else
      {
          trpgMemReadBuffer buf(GetEndian());
          mod->Read(buf);
          Group *osg_model = parse->ParseScene(buf, m_gstates , m_models);
          m_models.push_back(osg_model);  
      }
*/
  }
   return true;
}

// Group* TrPageArchive::LoadTile(int x,int y,int lod,int &parentID,vector<GroupIDInfo> **groupList)
Group* TrPageArchive::LoadTile(int x,int y,int lod,int &parentID)
{
   trpgMemReadBuffer buf(GetEndian());

   // Read the tile data in, but don't parse it
   if (!ReadTile(x,y,lod,buf))
      return NULL;

   parse->SetTile(x,y,lod);
   Group *tile = parse->ParseScene(buf, m_gstates , m_models);
   if (tile)
   {
      parentID = parse->GetParentID();
      // This is where you would page in textures and models
   }

   // Also return the list of IDs and their groups we read in for this tile
//   *groupList = parse->GetGroupList();

   // That's it
   return tile;
}

// Group* TrPageArchive::LoadTile(Group *rootNode,trpgPageManager *pageManage,trpgManagedTile *tile,vector<)
Group* TrPageArchive::LoadTile(Group *rootNode,
		trpgPageManager * /*pageManage*/,
		trpgManagedTile *tile,osg::Group **parentNode)
{
	int x,y,lod;
	tile->GetTileLoc(x,y,lod);
	std::vector<GeodeGroup *> *groupList = parse->GetGroupList();

	// Fetch the tile data (but don't parse it)
	if (!ReadTile(x,y,lod,buf))
		return NULL;

	// Now parse it
	parse->SetTile(x,y,lod);
	Group *gTile = parse->ParseScene(buf, m_gstates, m_models);
	if (gTile && rootNode) {
		// Hook it into its parent
		int parentID = parse->GetParentID();
		if (parentID >= 0) {
			(*groupList)[parentID]->addChild(gTile);
		} else
			rootNode->addChild(gTile);

		// Note: Need to page in the textures
		//		They're being forced in during the parse right now
	}
	if (parentNode) {
		int parentID = parse->GetParentID();
		if (parentID >= 0)
			*parentNode = (*groupList)[parentID];
		else
			*parentNode = rootNode;
	}

	// Add the tile to the local data in the managed tile
	tile->SetLocalData(gTile);

	return gTile;
}

bool TrPageArchive::UnLoadTile(trpgPageManager * /*pageManage*/,
		  	       trpgManagedTile *tile)
{
	// The local tile data will have the group we need
	Group *gTile = (Group *)tile->GetLocalData();
	if (gTile) {
		// Remove it from the parent list
		// We're only expecting on parent
		const Group::ParentList &pList = gTile->getParents();
		if (pList.size() != 1)
			return false;
		pList[0]->removeChild(gTile);
	} else
		return false;

	return true;
}

//----------------------------------------------------------------------------
Group* TrPageArchive::LoadAllTiles()
{
   // Size information comes out of the header
   const trpgHeader *head = GetHeader();
   // Create one group as the top
   Group *topGroup = new Group;

   int32 numLod;
   head->GetNumLods(numLod);

   //  Iterate over the LODs.  Lower res LODs must be loaded
   //  first, otherwise there's nothing to hook the higher res
   //  LODs into.
   trpg2iPoint tileSize;

   // The group list is used to map parent IDs to pfGroup nodes
   //std::vector<Group *> groupList;
   std::vector<GeodeGroup *> *groupList = parse->GetGroupList();

   for (int nl=0;nl<numLod;nl++)
   {
      // The number of tiles in X and Y changes per LOD
      head->GetLodSize(nl,tileSize);
      for (int x=0; x < tileSize.x; x++)
      {
         for (int y=0; y < tileSize.y; y++)
         {
            int parentID;
            Group *tile = LoadTile(x,y,nl,parentID);
            if (!tile)
            {
                notify(WARN)<< "TrPageArchive::LoadAllTiles error: "
                                 << "failed to load tile ("
                                 << x << "," << y << "," << nl << ")"
                                 << std::endl;
            }
            else
            {
               if (parentID == -1)
               {
                  // Get added to the top level node
                  topGroup->addChild(tile);
               }
               else
               {
                  // Added below some other node
                   (*groupList)[parentID]->addChild(tile);
               }
            }
         }
      }
   }
   return topGroup;
}
