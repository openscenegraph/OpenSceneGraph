/*************************************************************************** 
 * December 2003
 *
 * This TerraPage loader was re-written in a fashion to use PagedLOD 
 * to manage paging entirely, also includes a version of Terrex's smart mesh 
 * adapted to work with PagedLOD. The essential code by Boris Bralo is still present, 
 * slight modified.
 * nick at terrex dot com
 * 
 * Ported to PagedLOD technology by Trajce Nikolov (Nick) & Robert Osfield
 *****************************************************************************/

/***************************************************************************
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
#ifndef __TXPARCHIVE_H_
#define __TXPARCHIVE_H_

#include "trpage_sys.h"
#include "trpage_read.h"

#include <osg/Referenced>
#include <osg/BoundingBox>
#include <osg/Texture2D>
#include <osg/StateSet>
#include <osg/Node>
#include <osg/PagedLOD>
#include <osgSim/LightPointNode>

namespace txp
{
// this one handles different placement of light direction in osg and terrapage
struct DefferedLightAttribute
{
    // light point at (0,0,0) looking in (0,0,0) direction
    osg::ref_ptr<osgSim::LightPointNode> lightPoint;
    osg::ref_ptr<osg::StateSet> fallback;
    osg::Vec3 attitude;
};

class TXPParser;
class TXPArchive : public trpgr_Archive, public osg::Referenced
{
public:
    // Constructor
    TXPArchive();
    
    // Opens the archive file and reads the header
    bool openFile(const std::string& archiveName);
    
    // Load the materials from the archve
    bool loadMaterials();
    bool loadMaterial(int ix);
    
    // Load the models from the archive
    bool loadModels();
    bool loadModel(int ix);
    
    // Load the light attribs from the archive
    bool loadLightAttributes();
    
    // Add light attrib
    void addLightAttribute(osgSim::LightPointNode* lpn, osg::StateSet* fallback , const osg::Vec3& attitude);
    
    // Get light attrib
    inline DefferedLightAttribute& getLightAttribute(unsigned int i)
    {
        return _lights[i];
    };
    
    // Gets some informations for a given tile
    struct TileInfo
    {
        osg::Vec3			center;
        double              minRange;
        double              maxRange;
        float               radius;
        osg::BoundingBox    bbox;
    };
    bool getTileInfo(int x, int y, int lod, TileInfo& info);
    
    // Set/Get the archive id
    inline void setId(int id)
    {
        _id = id;
    }
    inline const int& getId() const
    {
        return _id;
    }
    
    // Returns the number of LODs for this archive
    inline const int& getNumLODs() const
    {
        return _numLODs;
    }
    
    // Returns the extents of the archive
    inline void getExtents(osg::BoundingBox& extents)
    {
        extents.set(_swExtents.x,_swExtents.y,0.0f,_neExtents.x,_neExtents.y,0.0f);
    }
    
    // Returns the origin of the archive
    inline void getOrigin(double& x, double& y)
    {
        x=_swExtents.x;
        y=_swExtents.y;
    }
    
	// Returns global texture
    inline osg::Texture2D* getGlobalTexture(int id)
    {
        return _textures[id].get();
    }
    
	// Returns scenegraph representing the Tile
    osg::Group* getTileContent(
        int x,
        int y,
        int lod,
        double realMinRange,
        double realMaxRange,
        double usedMaxRange);
        
protected:

    // Destructor
    virtual ~TXPArchive();
    
    // Id of the archive
    int _id;
    
    // Number of the LODs
    int _numLODs;
    
    // Archive extents
    trpg2dPoint _swExtents;
    trpg2dPoint _neExtents;
    
    // Terra Page Parser
    osg::ref_ptr<TXPParser>    _parser;
    
    // Textures
    std::vector< osg::ref_ptr<osg::Texture2D> >        _textures;
    
    // States
    std::vector< osg::ref_ptr<osg::StateSet> >        _gstates;
    
    // Models
    std::vector< osg::ref_ptr<osg::Node> >            _models;
    
    // Light attributes vector
    std::vector<DefferedLightAttribute>                _lights;
    
};

} // namespace

#endif // __TXPARCHIVE_H_

