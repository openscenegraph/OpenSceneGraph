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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* trpage_tile.cpp
	This source file contains the implementation of trpgTileTable and trpgTileHeader.
	You'll need to edit these if you want to add something to the Tile Table (at
	 the front of an archive) or the Tile Header (at the beginning of each tile).
	 */

#include "trpage_geom.h"
#include "trpage_read.h"

/* Write Tile Table
	Keeps track of tiles written to disk.
	*/

// Constructor
trpgTileTable::trpgTileTable()
{
//	numX = numY = numLod = 0;
	numLod = 0;
	baseName = NULL;
	type = External;
}

// Reset function
void trpgTileTable::Reset()
{
	if (baseName)
		delete baseName;
//	center.resize(0);
//	tiles.resize(0);
//	numX = numY;
	numLod = 0;
	baseName = NULL;
	type = External;
}

// Destructor
trpgTileTable::~trpgTileTable()
{
	Reset();
}

// Set functions
void trpgTileTable::SetNumTiles(int nx,int ny)
{
	if (nx <= 0 || ny <= 0)
		return;

	SetNumTiles(nx,ny,1);
}
void trpgTileTable::SetNumTiles(int nx,int ny,int nl)
{
	if (nx <= 0 || ny <= 0 || nl <= 0 || nl >= numLod)
		return;

	lodSizes[nl] = trpg2iPoint(nx,ny);

//	tiles.resize(nx*ny*nl,0);
}
void trpgTileTable::SetTile(int nx,int ny,int nl,trpgDiskRef ref)
{
	if (nx < 0 || nx >= numX ||
		ny < 0 || ny >= numY ||
		nl < 0 || nl >= numLod)
		return;

	type = Local;

//	tiles[nl*(numX*numY)+ny*numX+nx] = ref;
}
void trpgTileTable::SetTile(int nx,int ny,trpgDiskRef ref)
{
	SetTile(nx,ny,0,ref);
}
void trpgTileTable::SetBaseName(const char *name)
{
	if (baseName)
		delete baseName;

	baseName = new char[(name ? strlen(name) : 0)+1];
	strcpy(baseName,name);
	type = External;
}
void trpgTileTable::SetCenter(int nx,int ny,int nl,const trpg3dPoint &pt)
{
	if (nx < 0 || nx >= numX ||
		ny < 0 || ny >= numY ||
		nl < 0 || nl >= numLod)
		return;

//	center[nl*(numX*numY)+ny*numX+nx] = pt;
}
void trpgTileTable::SetCenter(int nx,int ny,const trpg3dPoint &pt)
{
	SetCenter(nx,ny,0,pt);
}

// Need the basename when writing an archive
const char *trpgTileTable::GetBaseName() const
{
	return baseName;
}

// Validity check
bool trpgTileTable::isValid() const
{
//	if (numX == 0 || numY == 0 || numLod == 0)
//		return false;

	return true;
}

// Write tile table
bool trpgTileTable::Write(trpgWriteBuffer &buf)
{
	if (!isValid())
		return false;

	buf.Begin(TRPGTILETABLE);
#if 0
	numTiles = tiles.size();
	buf.Add(numX);
	buf.Add(numY);
	buf.Add(numLod);
	for (unsigned int i=0;i<tiles.size();i++) {
		buf.Add(tiles[i]);
		buf.Add(center[i]);
	}
#endif
	buf.Add(baseName);
	buf.End();

	return true;
}

/*	**************
	Tile Table Read methods
	**************
	*/

// Get methods
bool trpgTileTable::GetNumTiles(int &x,int &y,int &l) const
{
//	if (!isValid()) return false;
	x = numX;
	y = numY;
	l = numLod;
	return true;
}
bool trpgTileTable::GetType(int &t) const
{
	if (!isValid()) return false;
	t = type;
	return true;
}
bool trpgTileTable::GetBaseName(char *str,int strLen) const
{
	if (!isValid()) return false;
	if (type != External)  return false;
	int len = (baseName ? strlen(baseName) : 0);
	strncpy(str,baseName,MIN(len,strLen)+1);
	return true;
}
bool trpgTileTable::GetTile(int x,int y,int lod,trpgDiskRef &ref) const
{
	if (!isValid() || type != Local) return false;
	// Note: fill this in
	return false;
}
bool trpgTileTable::GetTile(int x,int y,int lod,char *str,int strLen) const
{
	if (!isValid() || type != External) return false;
	sprintf(str,"%s\\tile_%d_%d_%d.tpt",baseName,x,y,lod);
	return true;
}
bool trpgTileTable::GetCenter(int x,int y,int lod,trpg3dPoint &pt) const
{
	if (!isValid() || x < 0 || x >= numX || y < 0 || y >= numY ||
			lod < 0 || lod >= numLod)
			return false;
//	pt = center[lod*(numX*numY)+y*numX+x];
	return true;
}

bool trpgTileTable::Read(trpgReadBuffer &buf)
{
	// trpg3dPoint pt;
	char tmpStr[1024];

	try {
		type = External;  // Note: Read this in
#if 0
		buf.Get(numX);
		buf.Get(numY);
		buf.Get(numLod);
		if (numTiles < 0)  throw 1;
		for (int i=0;i<numTiles;i++) {
			buf.Get(diskRef);
			buf.Get(pt);
			tiles.push_back(diskRef);
			center.push_back(pt);
		}
#endif
		buf.Get(tmpStr,1023);
		SetBaseName(tmpStr);
	}
	catch (...) {
		return false;
	}

	return isValid();
}

/* Tile Header
	Each distinct tile (or model) must have a header
	 which tells you what models and materials are
	 referenced in that tile.
	 */
// Constructor
trpgTileHeader::trpgTileHeader()
{
}
trpgTileHeader::~trpgTileHeader()
{
}

void trpgTileHeader::Reset()
{
	matList.resize(0);
	modelList.resize(0);
}

// Set functions
void trpgTileHeader::SetMaterial(int no,int id)
{
	if (no < 0 || no >= (int)matList.size())
		return;
	matList[no] = id;
}
void trpgTileHeader::SetModel(int no,int id)
{
	if (no < 0 || no >= (int)modelList.size())
		return;
	modelList[no] = id;
}

// Set functions
void trpgTileHeader::AddMaterial(int id)
{
	// Look for it first
	for (unsigned int i=0;i<matList.size();i++)
		if (matList[i] == id)
			return;
	// Didn't find it, add it.
	matList.push_back(id);
}
void trpgTileHeader::AddModel(int id)
{
	for (unsigned int i=0;i<modelList.size();i++)
		if (modelList[i] == id)
			return;
	modelList.push_back(id);
}
void trpgTileHeader::SetDate(int32 d)
{
	date = d;
}

// Get methods
bool trpgTileHeader::GetNumMaterial(int32 &no) const
{
	if (!isValid()) return false;
	no = (int32)matList.size();
	return true;
}
bool trpgTileHeader::GetMaterial(int32 id,int32 &mat) const
{
	if (!isValid() || id < 0 || id >= (int32)matList.size())
		return false;
	mat = matList[id];
	return true;
}
bool trpgTileHeader::GetNumModel(int32 &no) const
{
	if (!isValid()) return false;
	no = modelList.size();
	return true;
}
bool trpgTileHeader::GetModel(int32 id,int32 &m) const
{
	if (!isValid() || id < 0 || id >= (int32)modelList.size())
		return false;
	m = modelList[id];
	return true;
}
bool trpgTileHeader::GetDate(int32 &d) const
{
	if (!isValid()) return false;
	d = date;
	return true;
}

// Validity check
bool trpgTileHeader::isValid() const
{
	return true;
}

// Write to a buffer
bool trpgTileHeader::Write(trpgWriteBuffer &buf)
{
	unsigned int i;

	if (!isValid())
		return false;

	buf.Begin(TRPGTILEHEADER);
	buf.Begin(TRPG_TILE_MATLIST);
	buf.Add((int32)matList.size());
	for (i=0;i<matList.size();i++)
		buf.Add(matList[i]);
	buf.End();
	buf.Begin(TRPG_TILE_MODELLIST);
	buf.Add((int32)modelList.size());
	for (i=0;i<modelList.size();i++)
		buf.Add(modelList[i]);
	buf.End();
	buf.Begin(TRPG_TILE_DATE);
	buf.Add(date);
	buf.End();
	buf.End();

	return true;
}

// Tile Header CB
// Used to aid in parsing tile header
// We want the tile header to be expandable, so be careful here
class tileHeaderCB : public trpgr_Callback {
public:
	void * Parse(trpgToken,trpgReadBuffer &);
	trpgTileHeader *head;
};

void * tileHeaderCB::Parse(trpgToken tok,trpgReadBuffer &buf)
{
	int32 no,id,date,i;

	try {
		switch (tok) {
		case TRPG_TILE_MATLIST:
			buf.Get(no);
			if (no < 0)  throw 1;
			for (i = 0;i < no; i++) {
				buf.Get(id);
				head->AddMaterial(id);
			}
			break;
		case TRPG_TILE_MODELLIST:
			buf.Get(no);
			if (no < 0)  throw 1;
			for (i=0;i<no;i++) {
				buf.Get(id);
				head->AddModel(id);
			}
			break;
		case TRPG_TILE_DATE:
			buf.Get(date);
			head->SetDate(date);
			break;
		default:
			// Don't care
			break;
		}
	}
	catch (...) {
		return NULL;
	}

	return head;
}

// Read tile header
bool trpgTileHeader::Read(trpgReadBuffer &buf)
{
	tileHeaderCB tcb;
	trpgr_Parser parse;

	tcb.head = this;
	parse.AddCallback(TRPG_TILE_MATLIST,&tcb,false);
	parse.AddCallback(TRPG_TILE_MODELLIST,&tcb,false);
	parse.AddCallback(TRPG_TILE_DATE,&tcb,false);
	parse.Parse(buf);

	return isValid();
}
