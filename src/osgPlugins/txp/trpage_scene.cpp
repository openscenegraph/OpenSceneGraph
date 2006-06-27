/* ************************
   Copyright Terrain Experts Inc.
   Terrain Experts Inc (TERREX) reserves all rights to this source code
   unless otherwise specified in writing by the President of TERREX.
   This copyright may be updated in the future, in which case that version
   supercedes this one.
   -------------------
   Terrex Experts Inc.
   4400 East Broadway #314
   Tucson, AZ  85711
   info@terrex.com
   Tel: (520) 323-7990
   ************************
   */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* trpage_scene.cpp
	This file implements a bunch of stuff, all of it optional.  See trpage_scene.h
	 for more information.
	Scene Graph nodes -
	 All the methods for the simple scene graph are here.
	trpgSceneGraphParser -
	 This is a subclass of trpgSceneParser.  It uses that utility class to keep track
	  of pushes and pops.  It also registers an interest in all the node types it
	  knows about (Geometry,Group,LOD,ModelRef).  When one of those is encountered
	  by the trpgr_Parser (which it's also a subclass of) it reads it into the
	  appropriate trpgRead* type.
	 Unless you're reading into the scene graph defined in trpage_scene.h, you won't
	  use this class directly.  Instead, copy it and use it as a template for how
	  to read into a scene graph.  You'll need to replace the helpers, primarily.
	*/

#include <trpage_read.h>
#include <trpage_scene.h>

/*  ****************
	MBR Calculation and handling
	****************
	*/
trpgMBR::trpgMBR()
{
	valid = false;
}
bool trpgMBR::isValid() const
{
	return valid;
}
void trpgMBR::Reset()
{
	valid = false;
}
trpg3dPoint trpgMBR::GetLL() const
{
	return ll;
}
trpg3dPoint trpgMBR::GetUR() const
{
	return ur;
}
void trpgMBR::AddPoint(const trpg3dPoint &pt)
{
	if (valid) {
		ll.x = MIN(pt.x,ll.x);
		ll.y = MIN(pt.y,ll.y);
		ll.z = MIN(pt.z,ll.z);
		ur.x = MAX(pt.x,ur.x);
		ur.y = MAX(pt.y,ur.y);
		ur.z = MAX(pt.z,ur.z);
	} else {
		valid = true;
		ll = ur = pt;
	}
}
void trpgMBR::AddPoint(double x,double y,double z)
{
	AddPoint(trpg3dPoint(x,y,z));
}
void trpgMBR::GetMBR(trpg3dPoint &oll,trpg3dPoint &our) const
{
	oll = ll;
	our = ur;
}
// Add the input MBR to this one
void trpgMBR::Union(const trpgMBR &in)
{
	if (valid) {
		if (in.isValid()) {
			AddPoint(in.GetLL());
			AddPoint(in.GetUR());
		}
	} else {
		valid = true;
		*this = in;
	}
}
// See if there's any overlap between the two MBRs
bool trpgMBR::Overlap(const trpg2dPoint &ill, const trpg2dPoint &iur) const
{
	if (!isValid()) return false;

	trpg2dPoint ilr = trpg2dPoint(iur.x,ill.y);
	trpg2dPoint iul = trpg2dPoint(ill.x,iur.y);

	// B MBR falls within A
	if (Within(ill) || Within(iur) || Within(ilr) || Within(iul))
		return true;

	// A MBR falls within B
	if ((inRange(ill.x,iur.x,ll.x) && inRange(ill.y,iur.y,ll.y)) ||
		(inRange(ill.x,iur.x,ur.x) && inRange(ill.y,iur.y,ll.y)) ||
		(inRange(ill.x,iur.x,ur.x) && inRange(ill.y,iur.y,ur.y)) ||
		(inRange(ill.x,iur.x,ll.x) && inRange(ill.y,iur.y,ur.y)))
		return true;

	if ((inRange(ll.x,ur.x,ill.x) && ill.y < ll.y && iur.y > ur.y) ||
		(inRange(ll.y,ur.y,ill.y) && ill.x < ll.x && iur.x > ur.x))
		return true;

	return false;
}
// Check if a given 2d point is within the MBR
bool trpgMBR::Within(const trpg2dPoint &pt) const
{
	if (inRange(ll.x,ur.x,pt.x) && inRange(ll.y,ur.y,pt.y))
		return true;
	return false;
}

/*	****************
	Read Group Base
	Base class for all group structures.
	****************
	*/

// Destructor
trpgReadGroupBase::~trpgReadGroupBase()
{
	DeleteChildren();
}

// Delete all children
void trpgReadGroupBase::DeleteChildren()
{
	for (unsigned int i=0;i<children.size();i++)
		if (children[i])
			delete children[i];
}

// Add a child to the list
void trpgReadGroupBase::AddChild(trpgReadNode *n)
{
	children.push_back(n);
}

// Unref a child (but don't delete it)
void trpgReadGroupBase::unRefChild(int id)
{
	if (id < 0 || id >= (int)children.size())
		return;
	children[id] = NULL;
}

// Unref all the children (they've probably been moved elsewhere)
void trpgReadGroupBase::unRefChildren()
{
	for (unsigned int i=0;i<children.size();i++)
		unRefChild(i);
}

// Calculate an MBR
trpgMBR trpgReadGroupBase::GetMBR() const
{
	if (mbr.isValid())
		return mbr;
	else {
		// Calculate and cache a new MBR
		trpgMBR *cmbr = const_cast<trpgMBR *>(&mbr);
		trpgMBR kmbr;
		// Ask the kids
		for (unsigned int i=0;i<children.size();i++) {
			kmbr = children[i]->GetMBR();
			cmbr->Union(kmbr);
		}
		return *cmbr;
	}
}

/*  ****************
	Read Geometry
	****************
	*/
// Calculate an MBR
trpgMBR trpgReadGeometry::GetMBR() const
{
	if (mbr.isValid())
		return mbr;

	trpgMBR *pmbr = const_cast<trpgMBR *>(&mbr);

	int numVert,i;
	trpg3dPoint pt;
	data.GetNumVertex(numVert);
	numVert /= 3;
	for (i=0;i<numVert;i++) {
		data.GetVertex(i,pt);
		pmbr->AddPoint(pt);
	}

	return mbr;
}

/*	****************
	Scene Graph Parser
	****************
	*/

/* Scene Graph Parser Helpers
	Each of these classes reads a certain kind of data (e.g. a group)
	 and creates the appropriate trpgrRead* form and returns that.
	 */

/* This is a helper registered by trpgSceneGraphParser that readers trpgGeometry
	nodes and adds them to the current scene graph.  trpgGeometry nodes are
	always leaves so there should be no pushes after this node.  The Parse method
	also adds the new node as a child to any existing (e.g. top) group.
	{group:Demonstration Scene Graph}
 */
class trpgReadGeometryHelper : public trpgr_Callback {
public:
	trpgReadGeometryHelper(trpgSceneGraphParser *in_parse) { parse = in_parse;}
	void *Parse(trpgToken /*tok*/,trpgReadBuffer &buf) {
		trpgReadGeometry *geom = new trpgReadGeometry();
		trpgGeometry *data = geom->GetData();
		if (!data->Read(buf)) {
			delete geom;
			return NULL;
		}
		trpgReadGroupBase *top = parse->GetCurrTop();
		if (top)
			top->AddChild(geom);
		else
			delete geom;

		return geom;
	}
protected:
	trpgSceneGraphParser *parse;
};

/* This helper is registered by trpgSceneGraphParser.  It reads a trpgGroup
	from the trpgReadBuffer.  It then adds it to our current scene graph.
	It also adds an index corresponding to the group's group ID in our group
	mapping in trpgSceneGraphParser.  The new group becomes the top one
	after returning from the Parse call.
	{group:Demonstration Scene Graph}
 */
class trpgReadGroupHelper : public trpgr_Callback {
public:
	trpgReadGroupHelper(trpgSceneGraphParser *in_parse) { parse = in_parse; }
	void *Parse(trpgToken /*tok*/,trpgReadBuffer &buf) {
		trpgReadGroup *group = new trpgReadGroup();
		trpgGroup *data = group->GetData();
		if (!data->Read(buf)) {
			delete group;
			return NULL;
		}
		trpgReadGroupBase *top = parse->GetCurrTop();
		if (top)
			top->AddChild(group);
		else
			delete group;
		// Add to the group map
		int id;
		data->GetID(id);
		trpgSceneGraphParser::GroupMap *gmap = parse->GetGroupMap();
		(*gmap)[id] = group;
		return group;
	}
protected:
	trpgSceneGraphParser *parse;
};
class trpgReadBillboardHelper : public trpgr_Callback {
public:
	trpgReadBillboardHelper(trpgSceneGraphParser *in_parse) { parse = in_parse; }
	void *Parse(trpgToken /*tok*/,trpgReadBuffer &buf) {
		trpgReadBillboard *group = new trpgReadBillboard();
		trpgBillboard *data = group->GetData();
		if (!data->Read(buf)) {
			delete group;
			return NULL;
		}
		trpgReadGroupBase *top = parse->GetCurrTop();
		if (top)
			top->AddChild(group);
		else
			delete group;
		// Add to the group map
		int id;
		data->GetID(id);
		trpgSceneGraphParser::GroupMap *gmap = parse->GetGroupMap();
		(*gmap)[id] = group;
		return group;
	}
protected:
	trpgSceneGraphParser *parse;
};
class trpgReadAttachHelper : public trpgr_Callback {
public:
	trpgReadAttachHelper(trpgSceneGraphParser *in_parse) { parse = in_parse; }
	void *Parse(trpgToken /*tok*/,trpgReadBuffer &buf) {
		trpgReadAttach *attach = new trpgReadAttach();
		trpgAttach *data = attach->GetData();
		if (!data->Read(buf)) {
			delete attach;
			return NULL;
		}
		trpgReadGroupBase *top = parse->GetCurrTop();
		if (top)
			top->AddChild(attach);
		else
			delete attach;
		// Add to the group map
		int id;
		data->GetID(id);
		trpgSceneGraphParser::GroupMap *gmap = parse->GetGroupMap();
		(*gmap)[id] = attach;
		return attach;
	}
protected:
	trpgSceneGraphParser *parse;
};

class trpgReadChildRefHelper : public trpgr_Callback {
public:
	trpgReadChildRefHelper(trpgSceneGraphParser *in_parse) { parse = in_parse; }
	void *Parse(trpgToken /*tok*/,trpgReadBuffer &buf) {
		trpgReadChildRef *childRef = new trpgReadChildRef();
		trpgChildRef *data = childRef->GetData();
		if (!data->Read(buf)) {
			delete childRef;
			return NULL;
		}
		trpgReadGroupBase *top = parse->GetCurrTop();
		// NOTE: this is bad, we delete the pointer then we save it.
      //       this is done everywhere and should be corrected
      if (top)
			top->AddChild(childRef);
		else
			delete childRef;
	
		return childRef;
	}
protected:
	trpgSceneGraphParser *parse;
};
class trpgReadLodHelper : public trpgr_Callback {
public:
	trpgReadLodHelper(trpgSceneGraphParser *in_parse) { parse = in_parse; }
	void *Parse(trpgToken /*tok*/,trpgReadBuffer &buf) {
		trpgReadLod *lod = new trpgReadLod();
		trpgLod *data = lod->GetData();
		if (!data->Read(buf)) {
			delete lod;
			return NULL;
		}
		trpgReadGroupBase *top = parse->GetCurrTop();
		if (top)
			top->AddChild(lod);
		else
			delete lod;
		// Add to the group map
		int id;
		data->GetID(id);
		trpgSceneGraphParser::GroupMap *gmap = parse->GetGroupMap();
		(*gmap)[id] = lod;
		return lod;
	}
protected:
	trpgSceneGraphParser *parse;
};
class trpgReadModelRefHelper : public trpgr_Callback {
public:
	trpgReadModelRefHelper(trpgSceneGraphParser *in_parse) { parse = in_parse; }
	void *Parse(trpgToken /*tok*/,trpgReadBuffer &buf) {
		trpgReadModelRef *mod = new trpgReadModelRef();
		trpgModelRef *data = mod->GetData();
		if (!data->Read(buf)) {
			delete mod;
			return NULL;
		}
		trpgReadGroupBase *top = parse->GetCurrTop();
		if (top)
			top->AddChild(mod);
		else
			delete mod;
		return mod;
	}
protected:
	trpgSceneGraphParser *parse;
};
class trpgReadTileHeaderHelper : public trpgr_Callback {
public:
	trpgReadTileHeaderHelper(trpgSceneGraphParser *in_parse) { parse = in_parse; }
	void *Parse(trpgToken /*tok*/,trpgReadBuffer &buf) {
		trpgReadTileHeader *th = parse->GetTileHeaderRef();
		trpgTileHeader *data = th->GetData();
		if (!data->Read(buf))
			return NULL;
		return th;
	}
protected:
	trpgSceneGraphParser *parse;
};

/* The Scene Graph Parser constructor does two things.  First, it sets
	up any internal variables like a normal constructor.  Then it registers
	an interest in all the node types it knows how to parse.  It does this
	by calling AddCallback, which is a method of its parent.  It passes in
	a token representing the node type (see trpg_io.h) and an object that
	is capable of parsing the given type.

	The objects we pass in here are called helpers.  They parse specific
	objects and add them to the user defined scene graph.  Examples include
	trpgReadGeometryHelper, trpgReadGroupHelper, trpgReadAttachHelper,
	trpgReadBillboardHelper, trpgReadLodHelper, trpgReadModelRefHelper,
	trpgReadTileHeaderHelper.  These are all derived from trpgr_Callback.
	You should not use any of these yourself. Instead look at these classes
	as examples of how to implement your own subclass of trpgSceneParser.
	*/
trpgSceneGraphParser::trpgSceneGraphParser()
{
	top = currTop = NULL;

	// Register the readers
	AddCallback(TRPG_GEOMETRY,new trpgReadGeometryHelper(this));
	AddCallback(TRPG_GROUP,new trpgReadGroupHelper(this));
	AddCallback(TRPG_ATTACH,new trpgReadAttachHelper(this));
   AddCallback(TRPG_CHILDREF,new trpgReadChildRefHelper(this));
	AddCallback(TRPG_BILLBOARD,new trpgReadBillboardHelper(this));
	AddCallback(TRPG_LOD,new trpgReadLodHelper(this));
//	AddCallback(TRPG_TRANSFORM,new trpgReadTransformHelper(this));
	AddCallback(TRPG_MODELREF,new trpgReadModelRefHelper(this));
//	AddCallback(TRPG_LAYER,new trpgReadLayerHelper(this));
	AddCallback(TRPGTILEHEADER,new trpgReadTileHeaderHelper(this));
}

// Get Current Top node
trpgReadGroupBase *trpgSceneGraphParser::GetCurrTop()
{
	if (!currTop)
		return NULL;
	if (currTop->isGroupType())
		return (trpgReadGroupBase *)currTop;

	return NULL;
}

// Return a pointer to the tile header record
trpgReadTileHeader *trpgSceneGraphParser::GetTileHeaderRef()
{
	return &tileHead;
}

// Parse Scene
// Parse a buffer and return the resulting scene graph
trpgReadNode *trpgSceneGraphParser::ParseScene(trpgReadBuffer &buf,GroupMap &inGmap)
{
	gmap = &inGmap;
	trpgTileHeader *data = tileHead.GetData();
	data->Reset();

	// Always put a group up top, since there might be more than
	//  one node at the top level in the file.
	top = currTop = new trpgReadGroup();

	// All the setup for tokens is handled in the constructor
	// Just call parse
	if (!Parse(buf)) {
		// Failed to parse correctly.  Give up.
		delete top;
		return NULL;
	}

	return top;
}

// Start Children
// This is called when the parser hits a push.
// We'll want to make the node it's handing us the "top" node
bool trpgSceneGraphParser::StartChildren(void *in_node)
{
	trpgReadNode *node = (trpgReadNode *)in_node;

	if (!node || !node->isGroupType()) {
		// Looks like there's a push in the wrong place
		// Make the current "top" NULL.
		// This will drop all node until we pop back above
		currTop = NULL;
	} else {
		// This node is our new "top"
		currTop = node;
	}

	return true;
}

/* This is called whent he parser hits a pop.
   We'll want to look on the parent list (in trpgSceneParser)
    for the parent above the current one.
   If there isn't one, we'll just stick things in our top group.
   */
bool trpgSceneGraphParser::EndChildren(void* /*in_node*/)
{
	// We don't need it here, but this is the node we just
	//  finished putting children under.  If you need to close
	//  it out in some way, do that here
	//trpgReadNode *node = (trpgReadNode *)in_node;

	// Get the parent above the current one
	int pos = parents.size()-2;
	if (pos < 0)
		// Nothing above the current one.  Fall back on our top group
		currTop = top;
	else
		currTop = (trpgReadNode *)parents[pos];

	return true;
}

// Return group map (for use by helpers)
trpgSceneGraphParser::GroupMap *trpgSceneGraphParser::GetGroupMap()
{
	return gmap;
}

/*  ***********
	Test functions
	***********
	*/

// Test all the tiles in an archive
bool trpgTestArchive(trpgr_Archive &archive)
{
	int numLod;
	trpg2iPoint tileSize;
	trpgSceneGraphParser parse;
	trpgReadNode *scene;
	trpgSceneGraphParser::GroupMap gmap;

	if (!archive.isValid()) return false;

	const trpgHeader *head = archive.GetHeader();
	head->GetNumLods(numLod);

	// Iterate over the lods
	int nl,x,y;
	trpgMemReadBuffer buf(archive.GetEndian());
	trpg3dPoint ll,ur;
	for (nl = 0;nl < numLod;nl++) {
		head->GetLodSize(nl,tileSize);
		// Iterate over the tiles within those
		for (x = 0; x < tileSize.x; x++)
			for (y = 0; y < tileSize.y; y++) {
				archive.trpgGetTileMBR(x,y,nl,ll,ur);
				if (archive.ReadTile(x,y,nl,buf)) {
					// Parse it
					scene = parse.ParseScene(buf,gmap);
					if (scene)
						delete scene;
				}
			}
	}

	return true;
}
