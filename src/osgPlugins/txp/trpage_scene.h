/* ************************
   Copyright Terrain Experts Inc.
   Terrain Experts Inc (TERREX) reserves all rights to this source code
   unless otherwise specified in writing by the Chief Operating Officer
   of TERREX.
   This copyright may be updated in the future, in which case that version
   supercedes this one.
   -------------------
   Terrex Experts Inc.
   84 West Santa Clara St., Suite 380
   San Jose, CA 95113
   info@terrex.com
   Tel: (408) 293-9977
   ************************
   */

#ifndef _txpage_scene_h_
// {secret}
#define _txpage_scene_h_

/* trpage_scene.h
    Scene Graph definition.
    This is a small scene graph we use for testing.
    It's not intended to replace the scene graph you may already be using.
    You do not need to translate from this scene graph structure to your own,
     at run-time.  Instead, use this file and trpage_scene.cpp as a guideline
     for how to read TerraPage format into your own scene graph.
   */

#include "trpage_geom.h"

/*
    {group:Demonstration Scene Graph}
    */
TX_EXDECL class TX_CLDECL trpgMBR {
public:
    trpgMBR(void);
    ~trpgMBR(void) { };
    bool isValid() const;
    void Reset();
    void AddPoint(const trpg3dPoint &);
    void AddPoint(double,double,double);
    void GetMBR(trpg3dPoint &ll,trpg3dPoint &ur) const;
    trpg3dPoint GetLL() const;
    trpg3dPoint GetUR() const;
    void Union(const trpgMBR &);
//    bool Overlap(const trpgMBR &) const;
    bool Overlap(const trpg2dPoint &ll, const trpg2dPoint &ur) const;
//    bool Within(const trpg3dPoint &) const
    bool Within(const trpg2dPoint &) const;
protected:
    inline bool inRange(double minv,double maxv,double val) const { return (val >= minv && val <= maxv); }
    bool valid;
    trpg3dPoint ll,ur;
};

// Read Node
// Simple Scenegraph node used for read testing
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgReadNode {
public:
    virtual ~trpgReadNode() { };
    virtual bool isGroupType() = 0;
    virtual int GetType() { return type; }
    virtual trpgMBR GetMBR() const { return trpgMBR(); }
protected:
    int type;
};

// Read Group Base
// Base class for all group nodes
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgReadGroupBase : public trpgReadNode {
public:
    virtual ~trpgReadGroupBase();
    void AddChild(trpgReadNode *);
    bool isGroupType() { return true; }
    int GetNumChildren() { return children.size(); }
    trpgReadNode *GetChild(int i) { return children[i]; }
    trpgMBR GetMBR() const;
    void unRefChild(int i);
    void unRefChildren();
protected:
    trpgMBR mbr;
    void DeleteChildren();
    vector<trpgReadNode *> children;
};

// Read Geometry
// The leaf for this scene graph
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgReadGeometry : public trpgReadNode {
public:
    trpgReadGeometry() { type = TRPG_GEOMETRY; }
    ~trpgReadGeometry() { };
    bool isGroupType() { return false; }
    trpgGeometry *GetData() { return &data; }
    trpgMBR GetMBR() const;
protected:
    trpgMBR mbr;
    trpgGeometry data;
};

// Read Tile Header
// One per tile.  Info about what materials and models are used
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgReadTileHeader : public trpgReadNode {
public:
    trpgReadTileHeader() { type = TRPGTILEHEADER; }
    ~trpgReadTileHeader() { };
    bool isGroupType() { return false; }
    trpgTileHeader *GetData() { return &data; }
    trpgMBR GetMBR() const { trpgMBR mbr;  return mbr; };
protected:
    trpgTileHeader data;
};

// Read Group
// Simple group structure
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgReadGroup : public trpgReadGroupBase {
public:
    trpgReadGroup() { type = TRPG_GROUP; }
    ~trpgReadGroup() { };
    trpgGroup *GetData() { return &data; }
protected:
    trpgGroup data;
};

// Read Attach
// Should be the top of a higher LOD tile
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgReadAttach : public trpgReadGroupBase {
public:
    trpgReadAttach() { type = TRPG_ATTACH; }
    ~trpgReadAttach() { };
    trpgAttach *GetData() { return &data; }
protected:
    trpgAttach data;
};

// Read billboard
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgReadBillboard : public trpgReadGroupBase {
public:
    trpgReadBillboard() { type = TRPG_BILLBOARD; }
    ~trpgReadBillboard() { };
    trpgBillboard *GetData() { return &data; }
protected:
    trpgBillboard data;
};

// Read LOD
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgReadLod : public trpgReadGroupBase {
public:
    trpgReadLod() { type = TRPG_LOD; }
    ~trpgReadLod() { };
    trpgLod *GetData() { return &data; }
protected:
    trpgLod data;
};

// Read Layer
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgReadLayer : public trpgReadGroupBase {
public:
    trpgReadLayer() { type = TRPG_LAYER; }
    ~trpgReadLayer() { };
    trpgLayer *GetData() { return &data; }
protected:
    trpgLayer data;
};

// Read Transform
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgReadTransform : public trpgReadGroupBase {
public:
    trpgReadTransform() { type = TRPG_TRANSFORM; }
    ~trpgReadTransform() { };
    trpgTransform *GetData() { return &data; }
protected:
    trpgTransform data;
};

// Read Model Reference
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgReadModelRef : public trpgReadGroupBase {
public:
    trpgReadModelRef() { type = TRPG_MODELREF; }
    ~trpgReadModelRef() { };
    trpgModelRef *GetData() { return &data; }
protected:
    trpgModelRef data;
};

/* Scene Graph Parser
    Parses a read buffer and returns a full scenegraph.
    You don't want to use this if you're reading into your own scenegraph.
    Instead, you'll want to sublcass trpgSceneParser, which is a helper
     class to keep track of pushes and pops and implement the same functionality
     that trpgSceneGraphParser has for your own scene graph.
    */
//     {group:Demonstration Scene Graph}
TX_EXDECL class TX_CLDECL trpgSceneGraphParser : public trpgSceneParser {
public:
#if defined(_WIN32) && !defined(__GNUC__)
    typedef map<int,trpgReadGroupBase *> GroupMap;
#else
    typedef map<int,trpgReadGroupBase *,less<int> > GroupMap;
#endif
    trpgSceneGraphParser();
    ~trpgSceneGraphParser() { };
    // Call this instead of Parse()
    // Deleting it is your responsibility
    trpgReadNode *ParseScene(trpgReadBuffer &,GroupMap &);
    trpgReadGroupBase *GetCurrTop();  // Get the current parent object
    trpgReadTileHeader *GetTileHeaderRef();

    // For use by the helpers only
    GroupMap *GetGroupMap();
protected:
    bool StartChildren(void *);
    bool EndChildren(void *);
    trpgReadNode *currTop;            // Current parent group
    trpgReadNode *top;                // Top of everything
    GroupMap *gmap;
    trpgReadTileHeader tileHead;    // Tile header gets read into here
};

/*  Test Archive
    Utility function that loads and tests all tiles.
    The only reason you'd want to call this is to test a TerraPage archive
     you'd written.
    */
//     {group:Demonstration Scene Graph}
TX_CPPDECL bool trpgTestArchive(trpgr_Archive &);

#endif
