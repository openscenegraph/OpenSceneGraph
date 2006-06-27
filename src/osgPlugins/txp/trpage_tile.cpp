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


/* trpage_tile.cpp
	This source file contains the implementation of trpgTileTable and trpgTileHeader.
	You'll need to edit these if you want to add something to the Tile Table (at
	 the front of an archive) or the Tile Header (at the beginning of each tile).
	 */

#include <trpage_geom.h>
#include <trpage_read.h>

/* Write Tile Table
	Keeps track of tiles written to disk.
	*/

// Constructor
trpgTileTable::trpgTileTable()
{
	localBlock = false;
	Reset();
}

// Reset function
void trpgTileTable::Reset()
{
	errMess[0] = '\0';
	mode = External;
	lodInfo.resize(0);
	valid = true;
	currentRow = -1;
	currentCol = -1;
   
}

// Destructor
trpgTileTable::~trpgTileTable()
{
}

// Set functions

void trpgTileTable::SetMode(TileMode inMode)
{
	Reset();
	mode = inMode;
}

void trpgTileTable::SetNumLod(int numLod)
{
	lodInfo.resize(numLod);
}


void trpgTileTable::SetNumTiles(int nx,int ny,int lod)
{
	
	if(localBlock) {
		LodInfo &li = lodInfo[lod];
		li.numX = nx;  li.numY = ny;
		li.addr.resize(1);
		li.elev_min.resize(1,0.0);
		li.elev_max.resize(1,0.0);
		valid = true;
		// no need to do anything else if we only have one block.
		return;
	}
	if (nx <= 0 || ny <= 0 || lod < 0 || lod >= static_cast<int>(lodInfo.size()))
		return;

	// Got a table we need to maintain
	if (mode == Local || mode == ExternalSaved) {
		// If there's a pre-existing table, we need to preserve the entries
		LodInfo oldLodInfo = lodInfo[lod];

		LodInfo &li = lodInfo[lod];
		li.numX = nx;  li.numY = ny;
		int numTile = li.numX*li.numY;
		li.addr.resize(numTile);
		li.elev_min.resize(numTile,0.0);
		li.elev_max.resize(numTile,0.0);

		// Copy pre-existing data if it's there
		if (oldLodInfo.addr.size() > 0) {
		    for (int x=0;x<oldLodInfo.numX;x++) {
			for (int y=0;y<oldLodInfo.numY;y++) {
			    int oldLoc = y*oldLodInfo.numX + x;
			    int newLoc = y*li.numX + x;
			    li.addr[newLoc] = oldLodInfo.addr[oldLoc];
			    li.elev_min[newLoc] = oldLodInfo.elev_min[oldLoc];
			    li.elev_max[newLoc] = oldLodInfo.elev_max[oldLoc];
			}
		    }
		}
	}
	valid = true;
}
void trpgTileTable::SetTile(int x,int y,int lod,trpgwAppAddress &ref,float32 zmin,float32 zmax)
{
	if (lod < 0 || lod >= static_cast<int>(lodInfo.size())) 
		return;
	if (mode == External)
		return;
	LodInfo &li = lodInfo[lod];
	int loc;
	if(localBlock) {
		loc = 0;
	}
	else {
		if (x < 0 || x >= li.numX || y < 0 || y >= li.numY)
			return;
		loc = y*li.numX + x;
	}
	li.addr[loc] = ref;
	li.elev_min[loc] = zmin;
	li.elev_max[loc] = zmax;
}

bool trpgTileTable::isValid() const
{
	return valid;
}

// Get methods

bool trpgTileTable::GetMode(TileMode &outMode) const
{
	if (!isValid())  return false;

	outMode = mode;
	return true;
}

bool trpgTileTable::GetTile(int x,int y,int lod,trpgwAppAddress &ref,float32 &zmin,float32 &zmax) const
{
	if (!isValid()) return false;

	if (lod < 0 || lod >= static_cast<int>(lodInfo.size())) return false;
	if (mode == External)
		return false;

	const LodInfo &li = lodInfo[lod];
	int loc;
	if(localBlock) {
		loc = 0;			
	}
	else {
		if (x < 0 || x >= li.numX || y < 0 || y >= li.numY)
			return false;
		loc = y*li.numX + x;
	}
	
	ref = li.addr[loc];
	zmin = li.elev_min[loc];
	zmax = li.elev_max[loc];

	return true;
}

// Write tile table
bool trpgTileTable::Write(trpgWriteBuffer &buf)
{
	if (!isValid())
		return false;

	buf.Begin(TRPGTILETABLE2);
	
	// Write the mode
	buf.Add(mode);

	// Depending on the mode we'll have a lot or a little data
	if (mode == Local || mode == ExternalSaved) {
		// The lod sizing is redundant, but it's convenient here
		int numLod = lodInfo.size();
		buf.Add(numLod);

		// Write each terrain LOD set
		for (int i=0;i<numLod;i++) {
			LodInfo &li = lodInfo[i];
			if(localBlock) {
				// only one x and one y in a local archive
				buf.Add(1);
				buf.Add(1);
				// local blocks always use index 0
				trpgwAppAddress &ref = li.addr[0];
				buf.Add((int32)ref.file);
				buf.Add((int32)ref.offset);

				buf.Add(li.elev_min[0]);
				buf.Add(li.elev_max[0]);
			}
			else {
				buf.Add(li.numX);
				buf.Add(li.numY);
				// Now for the interesting stuff
				unsigned int j;
				for (j=0;j<li.addr.size();j++) {
					trpgwAppAddress &ref = li.addr[j];
					buf.Add((int32)ref.file);
					buf.Add((int32)ref.offset);
				}
				for (j=0;j<li.elev_min.size();j++) {
					buf.Add(li.elev_min[j]);
					buf.Add(li.elev_max[j]);
				}
			}
		}
	}

	buf.End();

	return true;
}

/*	**************
	Tile Table Read method
	**************
	*/


bool trpgTileTable::Read(trpgReadBuffer &buf)
{
	valid = false;

	try {
		int imode;
		buf.Get(imode);  mode = (TileMode)imode;
		if (mode != External && mode != Local && mode != ExternalSaved)  throw 1;
		if (mode == Local || mode == ExternalSaved) {
			int numLod;
			buf.Get(numLod);
			if (numLod <= 0) throw 1;
			lodInfo.resize(numLod);

			for (int i=0;i<numLod;i++) {

				LodInfo &li = lodInfo[i];
				if(localBlock) {
					if(li.addr.size()==0) {
						li.addr.resize(1);
						li.elev_min.resize(1,0.0);
						li.elev_max.resize(1,0.0);
					}
					int32 x,y;
					buf.Get(x);
					buf.Get(y);
					int pos = (currentRow * li.numX) + currentCol;
					int32 file,offset;
					buf.Get(file);
					buf.Get(offset);
					trpgwAppAddress &ref = li.addr[pos];
					ref.file = file;
					ref.offset = offset;
					ref.col = currentCol;
					ref.row = currentRow;
										
					float emin,emax;
					buf.Get(emin);
					buf.Get(emax);

					li.elev_max[pos] = emax;
					li.elev_min[pos] = emin;
				}
				else {
					buf.Get(li.numX);
					buf.Get(li.numY);
					if (li.numX <= 0 || li.numY <= 0)  
						throw 1;
					int numTile = li.numX*li.numY;
					li.addr.resize(numTile);
					li.elev_min.resize(numTile);
					li.elev_max.resize(numTile);
					int j;
					for (j=0;j<numTile;j++) {
						int32 file,offset;
						buf.Get(file);
						buf.Get(offset);
						trpgwAppAddress &ref = li.addr[j];
						ref.file = file;
						ref.offset = offset;
					}
					for (j=0;j<numTile;j++) {
						float emin,emax;
						buf.Get(emin);
						buf.Get(emax);
						li.elev_max[j] = emax;
						li.elev_min[j] = emin;
					}
				}
			}
		}

		valid = true;
	}
	catch (...) {
		printf("Caught an exception\n");
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
	col = -1;
	row = -1;
}
trpgTileHeader::~trpgTileHeader()
{
}

void trpgTileHeader::Reset()
{
	matList.resize(0);
	modelList.resize(0);
	locMats.resize(0);
	col = -1;
	row = -1;
}

// Set functions
void trpgTileHeader::SetMaterial(int no,int id)
{
	if (no < 0 || no >= static_cast<int>(matList.size()))
		return;
	matList[no] = id;
}
void trpgTileHeader::SetModel(int no,int id)
{
	if (no < 0 || no >= static_cast<int>(modelList.size()))
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

// Local material methods

void trpgTileHeader::AddLocalMaterial(trpgLocalMaterial &locMat)
{
    locMats.push_back(locMat);
}

bool trpgTileHeader::GetNumLocalMaterial(int32 &retNum) const
{
    if (!isValid()) return false;
    retNum = locMats.size();

    return true;
}

bool trpgTileHeader::GetLocalMaterial(int32 id,trpgLocalMaterial &retMat) const
{
    if (id < 0 || id >= static_cast<int>(locMats.size()))
	return false;

    retMat = locMats[id];

    return true;
}

const std::vector<trpgLocalMaterial> *trpgTileHeader::GetLocalMaterialList() const
{
    if (!isValid())  return NULL;

    return &locMats;
}

// Get methods
bool trpgTileHeader::GetNumMaterial(int32 &no) const
{
	if (!isValid()) return false;
	no = matList.size();
	return true;
}
bool trpgTileHeader::GetMaterial(int32 id,int32 &mat) const
{
	if (!isValid() || id < 0 || id >= static_cast<int>(matList.size()))
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
	if (!isValid() || id < 0 || id >= static_cast<int>(modelList.size()))
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
	for (i=0;i<locMats.size();i++)
	    if (!locMats[i].isValid())
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
	buf.Begin(TRPG_TILE_LOCMATLIST);
	buf.Add((int32)locMats.size());
	for (i=0;i<locMats.size();i++)
		locMats[i].Write(buf);
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
		case TRPG_TILE_LOCMATLIST:
			{
			    int32 numLocMat;
			    buf.Get(numLocMat);
			    if (numLocMat < 0)  throw 1;
			    std::vector<trpgLocalMaterial> *locMats;
			    locMats = const_cast<std::vector<trpgLocalMaterial> *> (head->GetLocalMaterialList());
			    locMats->resize(numLocMat);
			    for (i=0;i<numLocMat;i++) {
					trpgToken matTok;
					int32 len;
					buf.GetToken(matTok,len);
					if (matTok != TRPGLOCALMATERIAL)  throw 1;
					buf.PushLimit(len);
					trpgLocalMaterial &locMat = (*locMats)[i];
					locMat.Read(buf);
					// Set the row/col for later finding
					trpgwAppAddress addr;
					locMat.GetAddr(addr);
					head->GetBlockNo(addr.row,addr.col);
					locMat.SetAddr(addr);

					buf.PopLimit();
			    }
			}
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
	// New for 2.0
	parse.AddCallback(TRPG_TILE_LOCMATLIST,&tcb,false);
	parse.Parse(buf);

	return isValid();
}
