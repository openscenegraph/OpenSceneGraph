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

#ifndef _TRPAGEPARSER_H_
#define _TRPAGEPARSER_H_

#include <osg/Vec3>
#include <osg/Vec2>
#include <osg/StateSet>
#include <osg/ref_ptr>
#include <osg/Texture2D>
#include <osg/Group>
#include <osg/Geode>
#include <osg/StateSet>
#include <vector>
#include "trpage_read.h"


namespace txp
{
    class TrPageArchive;
    struct DefferedLightAttribute;
    
	// Group ID Info
	// Used to keep track of which groups are which IDs for parents
	typedef struct {
		osg::Group *group;
		int id;
	} GroupIDInfo;

	// This is group that will has geode node
	// It is better since all the geometry children will be
	// added into one Geode node as drawables, then having one
	// geode node per child
	// Means, instad of having
	// Group
	//	+-----------
	//	|			|
	//	Geode		Geode
	//	|			|
	//	Drawable	Drawable
	// we will have
	// Group
	//	|
	//	Geode
	//	+-----------
	//	|			|
	//	Drawable	Drawable
	// nick@terrex.com
	class GeodeGroup : public osg::Group
	{
	protected:
		osg::Geode*	_geode;
	public:
		GeodeGroup() : osg::Group(), _geode(NULL) {};
		GeodeGroup(const GeodeGroup& gg,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
			: osg::Group(gg, copyop), _geode(gg._geode) {};
		META_Node(txp, GeodeGroup);
		osg::Geode* GetGeode()
		{ 
			if (!_geode)
			{
				_geode = new osg::Geode();
				addChild(_geode);
			}
			return _geode;
		};
		void SetGeode(osg::Geode* geode)
		{
			if ( _geode )
			{
				// I assume ref_ptr will destroy it
				removeChild(_geode);
			}
			_geode = geode;
			addChild(_geode);
		}
	};

	// same as above, we need this to identify the node as Layer node
	class Layer : public GeodeGroup
	{
	public:
		Layer() : GeodeGroup() {};
		Layer(const Layer& lr,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
				: GeodeGroup(lr, copyop) {};
		META_Node(txp, Layer);
	};

    class TrPageParser : public trpgSceneParser
    {
    public:
        TrPageParser(TrPageArchive* parent);
        ~TrPageParser();
        
		// Scene parser
        osg::Group *ParseScene(trpgReadBuffer & buf,
            std::vector<osg::ref_ptr<osg::StateSet> > & materials_,
            std::vector<osg::ref_ptr<osg::Node> > & node );
        
        // Return the parent of a recently parsed tile
        int GetParentID() { return parentID; }
        void SetParentID(int id) { parentID = id; }
        // Return a reference to the tile header (after a tile has been read)
        trpgTileHeader *GetTileHeaderRef();
        
        // Return the current top node (used during parsing)
        GeodeGroup *GetCurrTop();

		// Return the current layer node (also used during parsing)
		Layer *GetCurrLayer();
        
        // Return the current material list (passed in to ParseScene())
        std::vector<osg::ref_ptr<osg::StateSet> >* GetMaterials() { return materials; }

        // new to TerraPage 2.0 - local materials
        std::vector<osg::ref_ptr<osg::StateSet> >* GetLocalMaterials() { return &local_materials; }
        std::vector<osg::ref_ptr<osg::Node> >*     GetModels()    { return models; }
        
        // Add the Group to the current group list
        bool AddToGroupList(int id,GeodeGroup*);

		// Use this to add nodes into the scenegraph
		void AddIntoSceneGraph(osg::Node* node);
        
        // Return the group list
        std::vector< GeodeGroup *> *GetGroupList() { return &groupList; }
        
        /// TXP 2.0  - local materials
        void LoadLocalMaterials();
     
		// Initialize the group list
		void SetMaxGroupID(int maxGroupID);

		// Methods for tracking whether we are under a billboard in the scene graph
		bool inBillboard() const { return in_billboard; }
		void setBillboardType(int type) { billboard_type = type; }
		int  getBillboardType() { return billboard_type; }
		void setBillboardCenter(trpg3dPoint center) { billboard_center = center; }
		osg::Vec3 getBillboardCenter() { return osg::Vec3(billboard_center.x, billboard_center.y, billboard_center.z); }
	
		DefferedLightAttribute& GetLightAttribute(int attr_index);

		// Sets the info about the tile that is being parsed
		inline void SetTile(int x, int y, int lod)
		{
			_tileX = x; _tileY = y; _tileLOD = lod;
		}

		// Sets a group as potentinal PagedLOD - see below
		inline void SetPotentionalPagedLOD(osg::Group* group)
		{
			_pagedLods[group] = 1;
		}

    protected:
		// Called on start children
        bool StartChildren(void *);

		// Called on end children
        bool EndChildren(void *);

		// LOD parents
		// These will help us to find the "LOD Bridges" in the scene graph. "LOD Bridge" is a
		// group that holds two LOD nodes: one is the parent of the "current" LOD implementation of
		// a tile, the other is parent of the quad of the higher-res LOD implementation of the same
		// tile - it has four higher-res tiles. After a tile is loaded, we replace the "LOD bridge"
		// with PagedLOD node
		// nick@terrex.com
		std::map<osg::Group*,int> _pagedLods;

		// Converts to PagedLOD
		// If the given group is "LOD Bridge" this method will convert it into appropriate PagedLOD
		void ConvertToPagedLOD(osg::Group* group);

		// Current tile that is being loaded
		int _tileX;
		int _tileY;
		int _tileLOD;
		double _tileRange;

    protected:
        TrPageArchive*	parent_;		 // The archive
        GeodeGroup		*currTop;        // Current parent group
        GeodeGroup		*top;            // Top group
        trpgTileHeader	tileHead;        // Dump tile header here
        // If there was an attach node, this is
        // the tile's parent ID.  -1 otherwise
        int parentID;    
		// Materials
        std::vector<osg::ref_ptr<osg::StateSet> >*	materials;
		// Local materials
        std::vector<osg::ref_ptr<osg::StateSet> >	local_materials;
		// Group list. Probably needed for keepinh tracks of the IDs (?)
        std::vector<GeodeGroup *>					groupList;
		// Model list
        std::vector<osg::ref_ptr<osg::Node> >*		models;
		// track whether we are under a billboard in the scene graph
		bool		in_billboard;
		int			billboard_type;
		trpg3dPoint billboard_center;
		// Keeps track how deep are we into the layer subtree
		int layerDepth;
		// The current layer we are processing nodes below
		Layer*										currLayer;
		// Nodes that are part of the label subtree and will never get
		// into the sceen graph.
		// Consider this subtree
		// Layer0
		//	+-------------------
		//	|					|					
		// Group0(layer0)	Group4(layer1)
		//	|					|
		// Group1			Geometry3(for layer1)
		//	+---------------
		//	|				|
		// Geometry0		Layer1	
		//					+---------------
		//					|				|
		//					Group2			Group3
		//					|				|
		//					Geometry1		Geometry2
		//
		// The idea is to have all the geometries below Layer0 into one Geode
		// and set the right PolygonOffset. That means the rest of the nodes will go away
		// One problem that can occur is, if there is trpgAttach existing that attach something
		// to a trpgGroup from the Layer0 subtree ( or LODs - but I dont think any writer will do
		// produce such an archive ). The right way to do this probably is by using
		// targettable bin numbers, but I was not able to make it work
		// Here's Layer0 in osg representation
		// Layer0 (it's GeodeGroup, it has it's own Geode)
		//	|
		// Geode
		//	+-----------------------------------
		//	|			|			|			|
		//	Geometry0	Geometry1	Geometry2	Geometry3
		//
		//	Geometry1-3 will have PolygonState truned on
		// p.s. I have no archive at present to test this against. Usualy, the layers are
		// coming out of TerraVista in such a tree as follows:
		// Layer
		//	+---------------
		//	|				|
		// Group(layer0)	Group(layer1)
		//	|				|
		// Geometry			Geometry
		// It is tested on this kind of archive and it works. 
		// nick@terrex.com
		std::vector<osg::Node*>			deadNodes;
    };

	// Gets local texture via the image helper
    osg::Texture2D* GetLocalTexture(trpgrImageHelper& image_helper, const trpgTexture* tex);
	osg::Texture2D* GetTemplateTexture(trpgrImageHelper& image_helper, trpgLocalMaterial* locmat, const trpgTexture* tex, int index=0);

    //! callback functions for various scene graph elements
	//----------------------------------------------------------------------------
    class geomRead : public trpgr_Callback {
    public:
        geomRead(TrPageParser *in_parse);
        ~geomRead();
        void *Parse(trpgToken tok,trpgReadBuffer &buf);
    protected:
        TrPageParser *parse;
    };
    
    //----------------------------------------------------------------------------
    class groupRead : public trpgr_Callback {
    public:
        groupRead(TrPageParser *in_parse);
        void *Parse(trpgToken tok,trpgReadBuffer &buf);
    protected:
        TrPageParser *parse;
    };
    
    //----------------------------------------------------------------------------
    class attachRead : public trpgr_Callback {
    public:
        attachRead(TrPageParser*in_parse);
        void *Parse(trpgToken tok,trpgReadBuffer &buf);
    protected:
        TrPageParser*parse;
    };
    
    //----------------------------------------------------------------------------
    class billboardRead : public trpgr_Callback {
    public:
        billboardRead(TrPageParser*in_parse);
        void *Parse(trpgToken tok,trpgReadBuffer &buf);
    protected:
        TrPageParser*parse;
    };
    
    //----------------------------------------------------------------------------
    class lodRead : public trpgr_Callback {
    public:
        lodRead(TrPageParser*in_parse);
        void *Parse(trpgToken tok,trpgReadBuffer &buf);
    protected:
        TrPageParser*parse;
    };
    
    //----------------------------------------------------------------------------
    class modelRefRead : public trpgr_Callback {
    public:
        modelRefRead(TrPageParser*in_parse);
        void *Parse(trpgToken tok,trpgReadBuffer &buf);
    protected:
        TrPageParser*parse;
    };
    
    //----------------------------------------------------------------------------
    class tileHeaderRead : public trpgr_Callback {
    public:
        tileHeaderRead(TrPageParser*in_parse);
        void *Parse(trpgToken tok,trpgReadBuffer &buf);
    protected:
        TrPageParser*parse;
    };

	//----------------------------------------------------------------------------
    class layerRead: public trpgr_Callback {
    public:
        layerRead(TrPageParser*in_parse);
        void *Parse(trpgToken tok,trpgReadBuffer &buf);
    protected:
        TrPageParser*parse;
    };

	//----------------------------------------------------------------------------
    class lightRead: public trpgr_Callback {
    public:
        lightRead(TrPageParser*in_parse);
        void *Parse(trpgToken tok,trpgReadBuffer &buf);
    protected:
        TrPageParser*parse;
    };

    
} // namespace txp
#endif
