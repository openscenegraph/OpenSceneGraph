#include "TrPageArchive.h"
#include "TrPageParser.h"

#include <osg/Group>
#include <osg/Image>
#include <osg/Texture>
#include <osg/Material>
#include <osg/TexEnv>
#include <osg/Transparency>
#include <osg/CullFace>
#include <osg/Light>
#include <osg/StateSet>
#include <osg/Notify>
#include <osgDB/FileUtils>
#include <iostream>

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>

#include "trpage_geom.h"
#include "trpage_read.h"
#include "trpage_write.h"
#include "trpage_scene.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace txp;
using namespace osg;

TrPageArchive::TrPageArchive()
: trpgr_Archive()
, parse(new TrPageParser()) 
{
}

TrPageArchive::~TrPageArchive()
{
}

bool TrPageArchive::OpenFile(const std::string &file)
{
    m_alternate_path = osgDB::getFilePath(file);
    std::string name = osgDB::getSimpleFileName(file);
    
    if(m_alternate_path.empty())
        SetDirectory(".");
    else
    {
        osgDB::setFilePath(m_alternate_path.c_str());
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

// load textures and materials
// TODO : multitexturing 
void TrPageArchive::LoadMaterials()
{
    int n_textures;
    texTable.GetNumTextures(n_textures);
    
    m_textures.resize(n_textures);
    
    // these extra braces are workaroud for annoying bug in MSVC 
    // for( int i = ....) and i is visible outside the loop
    {
        for (int i=0; i < n_textures ; i++)
        {
            trpgTexture *tex;
            tex = texTable.GetTextureRef(i);
            char texName[1024];  texName[0] = 0;
            tex->GetName(texName,1023);
            // Create a texture by name.
            Texture *osg_texture = new Texture();
            
            // Load Texture and Create Texture State
            std::string filename = osgDB::getSimpleFileName(texName);
            std::string path(getDir());
            std::string theFile = path + '/'+ filename ;
            ref_ptr<Image> image = osgDB::readImageFile(theFile);
            if (image.valid())
            {
                osg_texture->setImage(image.get());
            }
            m_textures[i] = osg_texture;
        }
    }
    
    // create materials
    int n_materials;
    materialTable.GetNumMaterial(n_materials);
    
    {
        for (int i = 0; i < n_materials; i++)
        {
            StateSet* osg_state_set = new StateSet;
            
            trpgMaterial *mat;
            mat = materialTable.GetMaterialRef(0,i);
            // Set texture
            int numMatTex;
            mat->GetNumTexture(numMatTex);
            
            // TODO : multitextuting
            // also note that multitexturing in terrapage can came from two sides
            // - multiple textures per material, and multiple materials per geometry
            if(numMatTex)
            {
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
                
                /*
                int alphaFunc;
                mat->GetAlphaFunc(alphaFunc);
                */
                
                // transparency
                Transparency *osg_transparency = new Transparency;
                osg_transparency->setFunction(Transparency::SRC_ALPHA, Transparency::ONE_MINUS_SRC_ALPHA);
                osg_state_set->setAttributeAndModes(osg_transparency, StateAttribute::ON);
                
                int wrap_s, wrap_t;   
                texEnv.GetWrap(wrap_s, wrap_t);
                
                Texture* osg_texture = m_textures[texId].get();
                osg_texture->setWrap(Texture::WRAP_S, wrap_s == trpgTextureEnv::Repeat ? Texture::REPEAT: Texture::CLAMP );
                osg_texture->setWrap(Texture::WRAP_T, wrap_t == trpgTextureEnv::Repeat ? Texture::REPEAT: Texture::CLAMP );
                osg_state_set->setAttributeAndModes(osg_texture, StateAttribute::ON);
                
                if(osg_texture->getImage())
                { 
                    switch (osg_texture->getImage()->pixelFormat())
                    {
                    case GL_LUMINANCE_ALPHA:
                    case GL_RGBA:
                        osg_state_set->setMode(GL_BLEND,StateAttribute::ON);
                        osg_state_set->setRenderingHint(StateSet::TRANSPARENT_BIN);
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
          m_gstates.push_back(osg_state_set);
        }
    }
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

Group* TrPageArchive::LoadTile(int x,int y,int lod,int &parentID)
{
   trpgMemReadBuffer buf(GetEndian());

   // Read the tile data in, but don't parse it
   if (!ReadTile(x,y,lod,buf))
      return NULL;

   // Call the parser
   Group *tile = parse->ParseScene(buf, m_gstates , m_models);
   if (tile)
   {
      parentID = parse->GetParentID();
      // This is where you would page in textures and models
   }

   // That's it
   return tile;
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
   std::vector<Group *> *groupList = parse->GetGroupList();

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
