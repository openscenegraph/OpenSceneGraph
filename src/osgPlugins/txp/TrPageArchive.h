/* **************************************************************************
 * OpenSceneGraph loader for Terrapage format database
 * by Boris Bralo 2002
 *
 * based on/modifed  sgl (Scene Graph Library) loader by Bryan Walsh
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

#ifndef _TRPAGEARCHIVE_H_
#define _TRPAGEARCHIVE_H_



#include "trpage_sys.h"
#include "trpage_read.h"
#include "trpage_managers.h"

#include "TrPageParser.h"

#include <osgSim/LightPointNode>

#include <string>
#include <vector>
#include <memory> // for auto_ptr

namespace txp
{
    // this one handles different placement of light direction in osg and terrapage
    struct DefferedLightAttribute{
        // light point at (0,0,0) looking in (0,0,0) direction
        osg::ref_ptr<osgSim::LightPointNode> lightPoint;
        osg::ref_ptr<osg::StateSet> fallback;
        osg::Vec3 attitude;
    };
    /// main class for loading terrapage archives  
    class TrPageArchive : public trpgr_Archive
    {
    public:
        TrPageArchive();
        
        ~TrPageArchive();
        
        // open archive file
        virtual bool OpenFile(const char* file);
        
        /// Load and create textures and materials
        void LoadMaterials();

        /// Load and create models, usualy OpenFlight models
        bool LoadModels();

        void LoadLightAttributes();

        void AddLightAttribute(osgSim::LightPointNode* lpn, osg::StateSet* fallback , const osg::Vec3& attitude);

        DefferedLightAttribute& GetLightAttribute(unsigned int i) {
            return lightAttrTable[i];
        };
 
        /** Load a TXP tile and 
        @param x Tile location input - x dimension.
        @param y Tile location input - y dimension.
        @param lod Tile LOD level input.
        @return The parent ID of this tile to let you hook it into the scene
                graph.

        x, y dimensions are not coordinates, they are tile numbers. For example,
        for combination 10, 1 and lod number 2 terrapage opens file tile_10_1_2.tpt 
        in directory of the archive. This is THE method which shoud be used once 
        paging is implemented.

        */
        osg::Group *LoadTile(int x,int y,int lod,int &parent);

        /* This version is used during the paging and takes a Managed Tile
            instead of location.  These are used to keep track of what to
            page in and out.
         */
        osg::Group *LoadTile(osg::Group *rootNode,trpgPageManager *,trpgManagedTile *,osg::Group **parentNode=NULL);

        /* Unload Tile
            This is called to get rid of a tile from the scenegraph
         */
        bool UnLoadTile(trpgPageManager *,trpgManagedTile *);
        
        /** Load all the tiles . No paging.
        @return The parent of the complete scene graph.
        */
        osg::Group *LoadAllTiles();

        // Calculate the center
        void GetCenter(osg::Vec3 &center);

        osg::Texture2D* getGlobalTexture(int id) 
        {
            return m_textures[id].get();
        }

    protected:
        /// This class does most of the actual parsing. 
        std::auto_ptr<TrPageParser> parse;
        //  Texture, material, and model lists.
        std::vector< osg::ref_ptr<osg::Texture2D> >   m_textures;
        std::vector< osg::ref_ptr<osg::StateSet> >  m_gstates;
        std::vector< osg::ref_ptr<osg::Node> >      m_models;
        // light attributes vector
        std::vector<DefferedLightAttribute>    lightAttrTable;

        std::string   m_alternate_path;
        trpgMemReadBuffer buf;
    };
}; // end namespace

#endif
