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
#include <osg/StateSet>
#include <vector>
#include <trpage_read.h>

namespace txp
{
    class TrPageArchive;
    
	// Group ID Info
	// Used to keep track of which groups are which IDs for parents
	typedef struct {
		osg::Group *group;
		int id;
	} GroupIDInfo;

    class TrPageParser : public trpgSceneParser
    {
    public:
        TrPageParser(TrPageArchive* parent);
        ~TrPageParser();
        
        osg::Group *ParseScene(trpgReadBuffer & buf,
            std::vector<osg::ref_ptr<osg::StateSet> >  & materials_,
            std::vector<osg::ref_ptr<osg::Node> > & node );
        
        // Return the parent of a recently parsed tile
        int GetParentID() { return parentID; }
        void SetParentID(int id) { parentID = id; }
        // Return a reference to the tile header (after a tile has been read)
        trpgTileHeader *GetTileHeaderRef();
        
        // Return the current top node (used during parsing)
        osg::Group *GetCurrTop();
        
        // Return the current material list (passed in to ParseScene())
        std::vector<osg::ref_ptr<osg::StateSet> >* GetMaterials() { return materials; }
        // new to TerraPage 2.0 - local materials
        std::vector<osg::ref_ptr<osg::StateSet> >* GetLocalMaterials() { return &local_materials; }
        std::vector<osg::ref_ptr<osg::Node> >*     GetModels()    { return models; }
        
        // Add the Group to the current group list
        bool AddToGroupList(int id,osg::Group *);
        
        // Return the group list
        std::vector< osg::Group *> *GetGroupList() { return &groupList; }
        
        /// TXP 2.0  - local materials
        void LoadLocalMaterials();
     
		void SetMaxGroupID(int maxGroupID);
		
    protected:
        bool StartChildren(void *);
        bool EndChildren(void *);
        
    protected:
        TrPageArchive* parent_;
        osg::Group *currTop;            // Current parent group
        osg::Group *top;                // Top group
        trpgTileHeader tileHead;        // Dump tile header here
        // If there was an attach node, this is
        // the tile's parent ID.  -1 otherwise
        int parentID;                   
        std::vector<osg::ref_ptr<osg::StateSet> >* materials;
        std::vector<osg::ref_ptr<osg::StateSet> >  local_materials;
        std::vector<osg::Group *>   groupList;
        std::vector<osg::ref_ptr<osg::Node> >*    models;
    };

    osg::Texture2D* GetLocalTexture(trpgrImageHelper& image_helper, trpgLocalMaterial* locmat, const trpgTexture* tex);

    //! callback functions for various scene graph elements
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
    
} // namespace txp
#endif
