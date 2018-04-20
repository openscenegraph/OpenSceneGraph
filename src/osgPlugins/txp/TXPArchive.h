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
 * based on/modified  sgl (Scene Graph Library) loader by Bryan Walsh
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
#include "trpage_geom.h"

#include <osg/Referenced>
#include <osg/BoundingBox>
#include <osg/Texture2D>
#include <osg/StateSet>
#include <osg/Node>
#include <osg/PagedLOD>
#include <osg/Array>
#include <osgSim/LightPointNode>
#include <osgText/Font>

#include <OpenThreads/Mutex>

namespace txp
{
    // this one handles different placement of light direction in osg and terrapage
    struct DeferredLightAttribute
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
        bool loadTexture(int i);

        // Load the models from the archive
        bool loadModels();
        bool loadModel(int ix);

        // Load the light attribs from the archive
        bool loadLightAttributes();

        // Load the text styles from the archive
        bool loadTextStyles();
        inline std::map<int, osg::ref_ptr<osgText::Font> >& getStyles()
        {
            return _fonts;
        }
        inline std::map<int, osg::Vec4 >& getTextColors()
        {
            return _fcolors;
        }

        // Add light attrib
        void addLightAttribute(osgSim::LightPointNode* lpn, osg::StateSet* fallback , const osg::Vec3& attitude,int handle);

        int getNumLightAttributes()
        {
            return _lights.size();
        }

        // Get light attrib
        inline DeferredLightAttribute& getLightAttribute(unsigned int i)
        {
            return _lights[i];
        };

        // Gets some information for a given tile
        struct TileInfo
        {
            osg::Vec3           center;
            double              minRange;
            double              maxRange;
            double              lod0Range;
            float               radius;
            osg::Vec3           size;
            osg::BoundingBox    bbox;
        };
        struct TileLocationInfo
        {
            TileLocationInfo() : x( -1 ), y( -1 ), lod( -1 ), zmin(0.0f), zmax(0.0f)
            {}
            TileLocationInfo(int gx, int gy, int glod, const trpgwAppAddress& gaddr, float gzmin = 0.0f, float gzmax = 0.0f):
                    x( gx ), y( gy ), lod( glod ), addr( gaddr ), zmin( gzmin ), zmax( gzmax )
            {}
            int x, y, lod;
            trpgwAppAddress addr;
            float zmin, zmax;
        };

        bool getTileInfo(int x, int y, int lod, TileInfo& info);
        bool getTileInfo(const TileLocationInfo& loc, TileInfo& info);

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
        // FIXME - Needs to change for databases that aren't flat-earth
        void getExtents(osg::BoundingBox& extents);
    //     {
    //         extents.set(_swExtents.x,_swExtents.y,0.0f,_neExtents.x,_neExtents.y,0.0f);
    //     }

        // Returns the origin of the archive
        inline void getOrigin(double& x, double& y)
        {
            x=_swExtents.x;
            y=_swExtents.y;
        }

        // Returns global texture
        inline osg::Texture2D* getGlobalTexture(int id)
        {
            return GetTexMapEntry(id).get();
        }

        // Returns scenegraph representing the Tile.
        // For version 2.1 and over this function can only be call
        // with lod = 0, since the archive tile table will contain
        // only tiles with lod = 0
        osg::Group* getTileContent(
                int x,
                int y,
                int lod,
                double realMinRange,
                double realMaxRange,
                double usedMaxRange,
                osg::Vec3& tileCenter,
                std::vector<TileLocationInfo>& childInfoList);

        //  To be used for Version 2.1 with lod > 0
        osg::Group* getTileContent(
                const TileLocationInfo& loc,
                double realMinRange,
                double realMaxRange,
                double usedMaxRange,
                osg::Vec3& tileCenter,
                std::vector<TileLocationInfo>& childInfoList);

        // Get the number of tiles for given LOD
        bool getLODSize(int lod, int& x, int& y);

        void GetVersion(int& majorVer, int& minorVer) const
        {
            majorVer = _majorVersion;
            minorVer = _minorVersion;
        }

        //////////////////////////////////////////////////////////////////
        // This section brought to you by A. Danklefsen and the team @
        // Alion Science And Technology 2/12/07
        //
        // This will allow you to have smc / fid / swc / stp values and
        // places them on the userdata of the state set. this way your own
        // terrain loader / parser can know these values
        void SetUserDataToMaterialAttributes(osg::StateSet& osg_state_set, const trpgMaterial& mat)
        {
            if(!_loadMaterialsToStateSet)
                return;

            int attr_values = 0;
            osg::ref_ptr<osg::IntArray> ourValueArray = new osg::IntArray();
            for(int attrIter = 0 ; attrIter < 4; ++attrIter)
            {
                mat.GetAttr(attrIter, attr_values);
                ourValueArray->push_back(attr_values);
            }
            osg_state_set.setUserData(ourValueArray.get());
        }

        void SetMaterialAttributesToStateSetVar(bool value) {_loadMaterialsToStateSet = value;}

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
        typedef std::map<int,osg::ref_ptr<osg::Texture2D> > OSGTexMapType;
        OSGTexMapType _texmap;

        void SetTexMap(int key,osg::ref_ptr<osg::Texture2D> ref);
        osg::ref_ptr<osg::Texture2D> GetTexMapEntry(int key);


        // States
        typedef std::map<int,osg::ref_ptr<osg::StateSet> > OSGStatesMapType;
        OSGStatesMapType _statesMap;

        void SetStatesMap(int key,osg::ref_ptr<osg::StateSet> ref);
        osg::ref_ptr<osg::StateSet> GetStatesMapEntry(int key);

        // Models
        typedef std::map<int,osg::ref_ptr<osg::Node> > OSGModelsMapType;
        OSGModelsMapType      _models;

        // Light attributes vector
        std::map<int, DeferredLightAttribute>           _lights;

        // Text styles / Fonts
        std::map<int, osg::ref_ptr<osgText::Font> >    _fonts;

        // Text colors
        std::map<int, osg::Vec4 >            _fcolors;

        //
        OpenThreads::Mutex  _mutex;

        // Cache those: TerraPage version
        int _majorVersion, _minorVersion;

        bool _isMaster;

        bool _loadMaterialsToStateSet;

    };

} // namespace

#endif // __TXPARCHIVE_H_


