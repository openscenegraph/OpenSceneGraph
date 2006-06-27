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
#include <stdexcept>

#include <trpage_sys.h>
#include <trpage_geom.h>
#include <trpage_read.h>
#include <trpage_print.h>
#include <trpage_managers.h>

/* Managed Tile class.
	Check the header file for details.
 */

trpgManagedTile::trpgManagedTile()
{
    isLoaded = false;
    location.x = location.y = -1;
    location.lod = -1;
    localData = NULL;
}

void trpgManagedTile::Reset()
{
    // Null out the local material data
    for (unsigned int i=0;i<localMatData.size();i++)
	localMatData[i] = NULL;
    groupIDs.resize(0);

    isLoaded = false;
    location.x = location.y = -1;
    location.lod = -1;
    localData = NULL;

    childLocationInfo.clear();

}

bool trpgManagedTile::ParseTileHeader(trpgReadBuffer &buf)
{
    isLoaded = false;
    if (!tileHead.Read(buf))
	return false;

    int numLoc;
    tileHead.GetNumLocalMaterial(numLoc);
    localMatData.resize(numLoc);

    // The tile considers itself loaded with just
    //  the header.  This isn't true as far as the paging
    //  manager is concerned.
    isLoaded = true;
    return true;
}

bool trpgManagedTile::IsLoaded()
{
    return isLoaded;
}

bool trpgManagedTile::SetTileLoc(int inX,int inY,int inLod)
{
    location.x = inX;
    location.y = inY;
    if (inLod < 0)
	return false;
    location.lod = inLod;

    return true;
}

bool trpgManagedTile::GetTileLoc(int &retx,int &rety,int &retLod) const
{
    retx = location.x;  rety = location.y;  retLod = location.lod;

    return true;
}

void trpgManagedTile::SetTileAddress(const trpgwAppAddress& gAddr)
{
    location.addr = gAddr;
}
void trpgManagedTile::SetTileAddress(int32 file, int32 offset)
{
    location.addr.file = file;
    location.addr.offset = offset;
}
const trpgwAppAddress& trpgManagedTile::GetTileAddress() const
{
    return location.addr;
}
bool trpgManagedTile::SetChildLocationInfo(int childIdx, int x, int y, const trpgwAppAddress& addr)
{
    if(childIdx < 0)
	throw std::invalid_argument(
            "trpgManagedTile::SetChildLocationInfo(): index argument out of bound.");

    int size = childLocationInfo.size();
    if(childIdx < size)
	childLocationInfo[childIdx] = TileLocationInfo(x, y, location.lod+1, addr);
    else if(childIdx == size)
	childLocationInfo.push_back(TileLocationInfo(x, y, location.lod+1, addr));
    else
    {
	childLocationInfo.resize(childIdx+1);
	childLocationInfo[childIdx] = TileLocationInfo(x, y, location.lod+1, addr);
    }

    return true;
}
bool trpgManagedTile::SetChildLocationInfo(int childIdx, const TileLocationInfo& info)
{
    if(childIdx < 0)
	throw std::invalid_argument(
            "trpgManagedTile::SetChildLocationInfo(): index argument out of bound.");

    int size = childLocationInfo.size();
    if(childIdx < size)
	childLocationInfo[childIdx] = info;
    else if(childIdx == size)
	childLocationInfo.push_back(info);
    else
    {
	childLocationInfo.resize(childIdx+1);
	childLocationInfo[childIdx] = info;
    }

    return true;
}
const TileLocationInfo& trpgManagedTile::GetChildLocationInfo(int childIdx) const
{
    if(childIdx < 0 || childIdx >= (int)childLocationInfo.size())
	throw std::invalid_argument(
            "trpgManagedTile::GetChildLocationInfo(): index argument out of bound.");
   

    return childLocationInfo[childIdx];
}

bool trpgManagedTile::GetChildTileLoc(int childIdx, int &x,int &y,int &lod) const
{
    if(childIdx < 0 || childIdx >= (int)childLocationInfo.size())
	throw std::invalid_argument(
            "trpgManagedTile::GetChildTileLoc(): index argument out of bound.");
    TileLocationInfo const& info = childLocationInfo[childIdx];

    x = info.x;  
    y = info.y;  
    lod = info.lod;

    return true;
}
   
const trpgwAppAddress& trpgManagedTile::GetChildTileAddress(int childIdx) const
{
    if(childIdx < 0 || childIdx >= (int)childLocationInfo.size())
	throw std::invalid_argument(
            "trpgManagedTile::GetChildTileAddress(): index argument out of bound.");
    return childLocationInfo[childIdx].addr;
}

const trpgTileHeader *trpgManagedTile::GetTileHead()
{
    return &tileHead;
}

const std::vector<trpgLocalMaterial> *trpgManagedTile::GetLocMatList() const
{
    return tileHead.GetLocalMaterialList();
}

const trpgLocalMaterial *trpgManagedTile::GetLocMaterial(int id) const
{
    const std::vector<trpgLocalMaterial> *matList;
    matList = tileHead.GetLocalMaterialList();

    if (id <0 || id >= (int)matList->size())
	return NULL;
    
    return &(*matList)[id];
}

void trpgManagedTile::SetLocalData(void *inData)
{
    localData = inData;
}

void *trpgManagedTile::GetLocalData() const
{
    return localData;
}

bool trpgManagedTile::SetMatData(int id,void *info)
{
    if (id < 0 || id >= (int)localMatData.size())
	return false;

    localMatData[id] = info;

    return true;
}

void *trpgManagedTile::GetMatData(int id) const
{
    if (id < 0 || id >= (int)localMatData.size())
	return NULL;

    return localMatData[id];
}

void trpgManagedTile::AddGroupID(int id)
{
    groupIDs.push_back(id);
}

const std::vector<int> *trpgManagedTile::GetGroupIDs() const
{
    return &groupIDs;
}

void trpgManagedTile::Print(trpgPrintBuffer &buf)
{
    char line[1024];
    sprintf(line,"x = %d, y = %d, lod = %d",location.x, location.y, location.lod);  buf.prnLine(line);
    // Note: Should print the rest too, probably.
}
 
/*  Page Manager LOD Page Info class.
    Used by the page manager to keep track of paging information
    for a single terrain LOD.  See the header file for details.
*/

trpgPageManager::LodPageInfo::LodPageInfo()
{
    valid = false;
    pageDist = 0.0;
    cell.x = cell.y = -100;
}

trpgPageManager::LodPageInfo::~LodPageInfo()
{
    Clean();
}

void trpgPageManager::LodPageInfo::Clean()
{
    // Clean up managed tile structures
    unsigned int i;
    for (i=0;i<load.size();i++)
	if (load[i])
	    delete load[i];
    load.resize(0);
    for (i=0;i<unload.size();i++)
	if (unload[i])
	    delete unload[i];
    unload.resize(0);
    for (i=0;i<current.size();i++)
	if (current[i])
	    delete current[i];
    current.resize(0);
    for (i=0;i<freeList.size();i++)
	delete freeList[i];
    freeList.resize(0);
    activeLoad = false;
    activeUnload = false;
}

bool trpgPageManager::LodPageInfo::Init(trpgr_Archive *archive, int myLod, double scale, int freeListDivider)
{
    Clean();

    lod = myLod;
    // In theory we could have a negative scale, but I don't
    //  really want to deal with that.
    if (scale < 0)  scale = 0.0;

    tileTable = archive->GetTileTable();

    // Need some size and shape info about our terrain LOD
    const trpgHeader *head = archive->GetHeader();
    head->GetTileSize(lod,cellSize);
    head->GetLodRange(lod,pageDist);
    head->GetLodSize(lod,lodSize);
    pageDist *= scale;

    head->GetVersion(majorVersion, minorVersion);
   

    // Area of interest size (in cells)
    aoiSize.x = (int)(pageDist/cellSize.x);
    aoiSize.y = (int)(pageDist/cellSize.y);

    /* Make a guess as to how many tiles we might have loaded
       in at any given time.  Give ourselves 15% extra room.
       From the area of interest in cells, we can guess the max
       number of tiles (aka cells) we'll have loaded in at once.
       Note that the AOI size is only ahead, so it must be doubled.

       Version 2.1 now support variable lods, it might be overkill to
       allocate a free list by supposing that the tiles exist.
       So only for version 2.1 an over we will use the divider to allocate less
    */ 
    maxNumTiles = (int)(1.15*(2*aoiSize.x+1)*(2*aoiSize.y+1));
    if(majorVersion == 2 && minorVersion >= 1)
	maxNumTiles = (int)(1.15*(2*aoiSize.x+1)*(2*aoiSize.y+1)/freeListDivider);
    else
	maxNumTiles = (int)(1.15*(2*aoiSize.x+1)*(2*aoiSize.y+1));


   

    // Allocate 'em
    for (int i=0;i<maxNumTiles;i++) {
	trpgManagedTile *tile = new trpgManagedTile();
	freeList.push_back(tile);
    }

    // We still don't have a position yet
    valid = true;

    return true;
}

bool trpgPageManager::LodPageInfo::SetLocation(trpg2dPoint &loc)
{
    // Calculate the cell this falls into
    trpg2iPoint newCell;
    newCell.x = (int)(loc.x/cellSize.x);
    newCell.y = (int)(loc.y/cellSize.y);

    // Snap to the database border
    if (newCell.x < 0)  newCell.x = 0;
    if (newCell.y < 0)  newCell.y = 0;
    if (newCell.x >= lodSize.x)  newCell.x = lodSize.x-1;
    if (newCell.y >= lodSize.y)  newCell.y = lodSize.y-1;

    // Nothing to page.  Done.
    if (newCell.x == cell.x && newCell.y == cell.y)
	return false;

    // Cell has changed.  Update.
    cell = newCell;

    Update();

    return true;
}

trpgManagedTile *trpgPageManager::LodPageInfo::GetNextLoad()
{
    // Can only load one tile at a time
    if (activeLoad)
	return NULL;

    // Clear any NULLs at the beginning
    while (load.size() && !load[0])
	load.pop_front();

    if (load.size() > 0) {
	activeLoad = true;
	return load[0];
    }

    return NULL;
}

void trpgPageManager::LodPageInfo::AckLoad()
{
    if (activeLoad) {
	current.push_back(load[0]);
	load.pop_front();
    }
    activeLoad = false;
}

trpgManagedTile *trpgPageManager::LodPageInfo::GetNextUnload()
{
    // Can only unload one tile at a time
    if (activeUnload)
	return NULL;

    // Clear any NULLs at the beginning
    while (unload.size() && !unload[0])
	unload.pop_front();

    if (unload.size() > 0) {
	activeUnload = true;
	return unload[0];
    }

    return NULL;
}

void trpgPageManager::LodPageInfo::AckUnload()
{
    if (activeUnload) {
	trpgManagedTile *tile = unload[0];
	tile->Reset();
	freeList.push_back(tile);
	unload.pop_front();
    }
    activeUnload = false;
}

bool trpgPageManager::LodPageInfo::isWithin(trpgManagedTile *tile,trpg2iPoint &sw,trpg2iPoint &ne)
{
    int tileX,tileY,tileLod;
    tile->GetTileLoc(tileX,tileY,tileLod);
    if (tileX >= sw.x && 
        tileX <= ne.x &&
	tileY >= sw.y && 
        tileY <= ne.y)
	return true;
    else
	return false;
}

bool trpgPageManager::LodPageInfo::Stop()
{
    // Empty the load list
    unsigned int i;
    for (i=0;i<load.size();i++)
	freeList.push_back(load[i]);
    load.resize(0);

    // Move the current tiles to the unload list
    for (i=0;i<current.size();i++)
	if (current[i])
	    unload.push_back(current[i]);
    current.resize(0);

    return (unload.size() > 0);
}
void trpgPageManager::LodPageInfo::AddChildrenToLoadList(std::vector<trpgManagedTile*>& parentList)
{
    if(parentList.size() == 0)
	return;

    // Area coverage, in cell unit
    trpg2iPoint sw,ne;
    sw.x = cell.x - aoiSize.x;
    sw.y = cell.y - aoiSize.y;
    ne.x = cell.x + aoiSize.x;  
    ne.y = cell.y + aoiSize.y;
    sw.x = MAX(0,sw.x);		    
    sw.y = MAX(0,sw.y);
    ne.x = MIN(lodSize.x-1,ne.x);   
    ne.y = MIN(lodSize.y-1,ne.y);

    int dx = ne.x - sw.x +1;
    int dy = ne.y - sw.y +1;

    // Mark the one that are already there
    tmpCurrent.resize(dx*dy);
    std::fill(tmpCurrent.begin(), tmpCurrent.end(), false);
    for (unsigned int i = 0; i<current.size(); i++) {
	trpgManagedTile *tile = current[i];
	if (tile) {
	    int tileX,tileY,tileLod;
	    tile->GetTileLoc(tileX,tileY,tileLod);
	    tmpCurrent[(tileY-sw.y)*dx + (tileX-sw.x)] = true;
	}
    }
    for (unsigned int i=0;i<load.size();i++) {
	trpgManagedTile *tile = load[i];
	if (tile) {
	    int tileX,tileY,tileLod;
	    tile->GetTileLoc(tileX,tileY,tileLod);
	    tmpCurrent[(tileY-sw.y)*dx + (tileX-sw.x)] = true;
	}
    }

    for(unsigned int parentIdx = 0; parentIdx < parentList.size(); ++parentIdx) {
      
	trpgManagedTile* parentTile = parentList[parentIdx];
	unsigned int nbChildren = parentTile->GetNbChildren();

	for(unsigned int childIdx = 0; childIdx < nbChildren; ++childIdx) {
         
	    const TileLocationInfo& childLoc = parentTile->GetChildLocationInfo(childIdx);

	    // a sanity check: if the lod is not the same then this
	    // parent is not at the right place
	    if(childLoc.lod != lod)
		break;

	    // Make sure it is within
	    if (childLoc.x >= sw.x && 
		childLoc.x <= ne.x &&
		childLoc.y >= sw.y && 
		childLoc.y <= ne.y)   {
           
		// Is it alread there ?
		if(!tmpCurrent[(childLoc.y - sw.y)*dx + (childLoc.x - sw.x)]) {
              
		    // Not there, add it
		    AddToLoadList(childLoc.x, childLoc.y, childLoc.addr);
		}
	    }
	}
    }
}
  
bool trpgPageManager::LodPageInfo::AddToLoadList(int x, int y, const trpgwAppAddress& addr)
{
    trpg2iPoint sw,ne;

    // Figure out the lower left and upper right corners
    //  in cell coordinates
    sw.x = cell.x - aoiSize.x;  
    sw.y = cell.y - aoiSize.y;
    ne.x = cell.x + aoiSize.x;  
    ne.y = cell.y + aoiSize.y;
    sw.x = MAX(0,sw.x);		    
    sw.y = MAX(0,sw.y);
    ne.x = MIN(lodSize.x-1,ne.x);   
    ne.y = MIN(lodSize.y-1,ne.y);

    if (x >= sw.x &&
	x <= ne.x &&
	y >= sw.y &&
	y <= ne.y)   {
      
	trpgManagedTile *tile = 0;
	if(freeList.size() > 0){

	    tile = freeList[0];
	    freeList.pop_front();
	}
	else
	    tile = new trpgManagedTile();
	tile->SetTileLoc(x, y, lod);
	tile->SetTileAddress(addr);
	load.push_back(tile);

	return true;
    }
    else
	return false;
}

/* Update does the major work of figuring out what to load and
   what to unload.
*/

void trpgPageManager::LodPageInfo::Update()
{
    trpg2iPoint sw,ne;

    // Figure out the lower left and upper right corners
    //  in cell coordinates
    sw.x = cell.x - aoiSize.x;  sw.y = cell.y - aoiSize.y;
    ne.x = cell.x + aoiSize.x;  ne.y = cell.y + aoiSize.y;
    sw.x = MAX(0,sw.x);		    sw.y = MAX(0,sw.y);
    ne.x = MIN(lodSize.x-1,ne.x);   ne.y = MIN(lodSize.y-1,ne.y);

    /* Load list -
       Some of the tiles we're supposed to load may now be
       out of range.  Take them off the load list.
    */
    unsigned int i;
    for (i=0;i<load.size();i++) {
	if (load[i] && !isWithin(load[i],sw,ne)) {
	    freeList.push_back(load[i]);
	    load[i] = NULL;
	}
    }

    /* Unload list -
       Some of the tiles we were planning on unloading may now
       be in view again.  Move them back to current.
    */
    for (i=0;i<unload.size();i++) {
	if (unload[i] && isWithin(unload[i],sw,ne)) {
	    current.push_back(unload[i]);
	    unload[i] = NULL;
	}
    }

    /* Current list -
       We need to figure out a few things here.
       1) What's in the current list that should be paged out.
       2) What's already paged, sorted into tmpCurrent.
       3) What's missing from tmpCurrent and should be paged in.
    */

    // Look for tiles to page out
    // Move them to the unload list
    for (i=0;i<current.size();i++) {
	if (current[i] && !isWithin(current[i],sw,ne)) {
	    unload.push_back(current[i]);
	    current[i] = NULL;
	}
    }
    // Clean the NULLs out of the current list
    int curPos = 0;
    for (i=0;i<current.size();i++) {
	if (current[i]) {
	    current[curPos] = current[i];
	    curPos++;
	}
    }
    current.resize(curPos);
    
    bool doUpdate = true;
    if(majorVersion == 2 && minorVersion >= 1)
    {
	// Version 2.1, we update only lod 0 since the tile table
	// will only contain lod 0. All tiles from other lod must be
	// update through the trpgPageManager::AckLoad(tile info)
	if(lod != 0)
	    doUpdate = false;
    }
    if(doUpdate)
    {
	// Sort the currently loaded stuff into a spatial array
	//  so we can figure out what needs to be loaded in addition.
	int dx,dy;
	dx = ne.x - sw.x+1;  dy = ne.y - sw.y+1;
	tmpCurrent.resize(dx*dy);
	for (i=0;i<tmpCurrent.size();i++)  tmpCurrent[i] = false;
	for (i=0;i<current.size();i++) {
	    trpgManagedTile *tile = current[i];
	    if (tile) {
		int tileX,tileY,tileLod;
		tile->GetTileLoc(tileX,tileY,tileLod);
		tmpCurrent[(tileY-sw.y)*dx + (tileX-sw.x)] = true;
	    }
	}

    // Now figure out which ones are missing and add them
    //  to the load list
	for (int x=0;x<dx;x++) {
	    for (int y=0;y<dy;y++) {
		if (!tmpCurrent[y*dx + x]) {
		    // Allocate a new tile
		    trpgManagedTile *tile = NULL;
		    if (freeList.size() > 0) {
			tile = freeList[0];
			freeList.pop_front();
		    } else
			tile = new trpgManagedTile();
		    tile->SetTileLoc(x+sw.x,y+sw.y,lod);
		    trpgwAppAddress addr;
		    float32 zmin, zmax;

		    if(tileTable && tileTable->GetTile(x+sw.x, y+sw.y, lod, addr, zmin, zmax))
			tile->SetTileAddress(addr);

		    load.push_back(tile);
		}
	    }
	}
    }

    // That's it.  All the rest is handled by the caller
    //  iterating through the tiles to load and unload.
}

// Get the list of currently loaded tiles that falls wiithin the area calculted from
// the given paging distance
void trpgPageManager::LodPageInfo::GetLoadedTileWithin(double pagingDistance, std::vector<trpgManagedTile*>& tileList)
{
    trpg2iPoint aoi_size((int)(pagingDistance/cellSize.x) +1,
			 (int)(pagingDistance/cellSize.y) +1);

    // Calculate the area that we must check, in parent cell coordinate
    trpg2iPoint sw, ne;
    sw.x = cell.x - aoi_size.x;  
    sw.y = cell.y - aoi_size.y;
    ne.x = cell.x + aoi_size.x;  
    ne.y = cell.y + aoi_size.y;
    sw.x = MAX(0,sw.x);		    
    sw.y = MAX(0,sw.y);
    ne.x = MIN(lodSize.x-1,ne.x);   
    ne.y = MIN(lodSize.y-1,ne.y);

    tileList.clear();

    for (unsigned i=0; i <current.size(); i++) {
	if (current[i] && isWithin(current[i],sw,ne)) 
	    tileList.push_back(current[i]);
    }
}
void trpgPageManager::LodPageInfo::Print(trpgPrintBuffer &buf)
{
    char line[1024];
    unsigned int i;

    sprintf(line,"lod = %d,  valid = %s",lod,(valid ? "yes" : "no")); buf.prnLine(line);
    sprintf(line,"pageDist = %f,  maxNumTiles = %d",pageDist,maxNumTiles);  buf.prnLine(line);
    sprintf(line,"cellSize = (%f,%f)",cellSize.x,cellSize.y);  buf.prnLine(line);
    sprintf(line,"cell = (%d,%d),  aoiSize = (%d,%d),  lodSize = (%d,%d)",cell.x,cell.y,aoiSize.x,aoiSize.y,lodSize.x,lodSize.y);  buf.prnLine(line);

    sprintf(line,"Loads:  (activeLoad = %s)",(activeLoad ? "yes" : "no"));  buf.prnLine(line);
    buf.IncreaseIndent();
    for (i=0;i<load.size();i++)
	if (load[i])
	    load[i]->Print(buf);
    buf.DecreaseIndent();

    sprintf(line,"Unloads:  (activeUnload = %s)",(activeUnload ? "yes" : "no"));  buf.prnLine(line);
    buf.IncreaseIndent();
    for (i=0;i<unload.size();i++)
	if (unload[i])
	    unload[i]->Print(buf);
    buf.DecreaseIndent();

    buf.prnLine("Currently loaded:");
    buf.IncreaseIndent();
    for (i=0;i<current.size();i++)
	if (current[i])
	    current[i]->Print(buf);
    buf.DecreaseIndent();

    sprintf(line,"Free list size = %d", (int)freeList.size());  buf.prnLine(line);
}

/* Page Manager methods
   These are methods of the main trpgPageManager.  Check the header
   file for more detailed information.
*/

trpgPageManager::trpgPageManager()
{
    scale = 1.0;
    valid = false;
    // This should be sufficiently unlikely
    pagePt.x = -1e20;  pagePt.y = -1e20;
}

trpgPageManager::~trpgPageManager()
{
}

void trpgPageManager::Init(trpgr_Archive *inArch)
{
    archive = inArch;

    // We're resetting everything.  In general, Init should only
    //  be called once, but it'll work fine if you call it more than once.
    lastLoad = None;
    lastTile = NULL;
    lastLod = -1;

    // Need to know the number of terrain LODs
    const trpgHeader *head = archive->GetHeader();
    int numLod;
    head->GetNumLods(numLod);
    head->GetVersion(majorVersion, minorVersion);
    
    // Reset the terrain LOD paging classes.
    valid = true;
    pageInfo.resize(numLod);
    for (int i=0;i<numLod;i++) {
	if(i > 3)
	    pageInfo[i].Init(archive,i,scale, 4);
	else
	    pageInfo[i].Init(archive,i,scale);
    }
}

// We might want to init only to support the tile table content.
// For Version 2.1 the tile table contain only lod 0.
// We might want to do that to use another paging scheme for
// lower lod
void trpgPageManager::Init(trpgr_Archive *inArch, int maxLod)
{
    archive = inArch;

    // We're resetting everything.  In general, Init should only
    //  be called once, but it'll work fine if you call it more than once.
    lastLoad = None;
    lastTile = NULL;
    lastLod = -1;

    // Need to know the number of terrain LODs
    const trpgHeader *head = archive->GetHeader();
    int numLod;
    head->GetNumLods(numLod);
    head->GetVersion(majorVersion, minorVersion);

    if(maxLod > numLod)
	maxLod = numLod;
    
    // Reset the terrain LOD paging classes.
    valid = true;
    pageInfo.resize(maxLod);
    for (int i=0;i<maxLod;i++) {
	if(i > 3)
	    pageInfo[i].Init(archive,i,scale, 4);
	else
	    pageInfo[i].Init(archive,i,scale);
    }
}

bool trpgPageManager::SetPageDistFactor(double inFact)
{
    // A scaling factor less than 1 will break the archive display.
    if (inFact <= 1.0)
	return false;

    scale = inFact;

    return true;
}

bool trpgPageManager::SetLocation(trpg2dPoint &pt)
{
    // Do a basic sanity check
    if (!valid || (pagePt.x == pt.x && pagePt.y == pt.y))
	return false;
    pagePt = pt;

    // Call each terrain LOD and let if figure out if something
    //  has changed.
    bool change = false;
    for (unsigned int i=0;i<pageInfo.size();i++) {
	if (pageInfo[i].SetLocation(pt))
	    change = true;
    }

    if(majorVersion == 2 && minorVersion >= 1 && change) {

	// Version 2.1 and over
	// Since we don't have a tile table for lod > 0, 
	// we must rely on the parent to see if children 
	// tiles have to be loaded:
	// First we get the list of parent tiles that falls
	// in the area of interest of the children,
	// Second we add to the load list all the parent's 
	// children that are no already part of the list

	for(unsigned int lodIdx = 1; lodIdx < pageInfo.size(); ++lodIdx) {

	    LodPageInfo& parentInfo = pageInfo[lodIdx -1];
	    LodPageInfo& childInfo = pageInfo[lodIdx];

	    // Get the list of parents tile that are currently
	    // loaded in the children aoi
	    std::vector<trpgManagedTile*> parentList;
	    parentInfo.GetLoadedTileWithin(childInfo.GetPageDistance(), parentList);
      
	    // Add the children of those parents to the load list
	    // of the children info, if they are not already there
	    childInfo.AddChildrenToLoadList(parentList);
	}

    }

    return change;
}

trpgManagedTile *trpgPageManager::GetNextLoad()
{
    // If we're already doing something, let them know about it
    if (lastLoad != None)
	throw 1;

    // Look for anything that needs loaded
    // Start with lowest LOD, work up to highest
    trpgManagedTile *tile = NULL;
    for (unsigned int i=0;i<pageInfo.size();i++) {
	LodPageInfo &info = pageInfo[i];
	if ((tile = info.GetNextLoad()))
	    break;
    }

    // Found one.  Now the user has to load it.
    if (tile) {
	lastLoad = Load;
	lastTile = tile;
	lastLod = tile->location.lod;
    }

    return tile;
}

void trpgPageManager::AckLoad()
{
    std::vector<TileLocationInfo> children;
    AckLoad(children);
}
void trpgPageManager::AckLoad(std::vector<TileLocationInfo> const& children)
{
    // If we're not in the middle of a load, register our displeasure
    if (lastLoad != Load)
	throw 1;

    if(majorVersion >= 2 && minorVersion >=1)
    {
	if(children.size() > 0)
	{
	    LodPageInfo& childInfo = pageInfo[lastLod+1];
	    for(unsigned int idx = 0; idx < children.size(); ++idx)
	    {
		TileLocationInfo const&childLocInfo = children[idx]; 
		if(childLocInfo.lod != lastLod+1)
		    continue; // Something wrong here
		childInfo.AddToLoadList(childLocInfo.x, childLocInfo.y, childLocInfo.addr);

		// Also save info in parent tile
		lastTile->SetChildLocationInfo(idx, childLocInfo);

	    }
	}
    }
  
    pageInfo[lastLod].AckLoad();
    lastLoad = None;
    lastTile = NULL;
}

void trpgPageManager::AddGroupID(trpgManagedTile *tile,int groupID,void *data)
{
    groupMap[groupID] = data;
    tile->AddGroupID(groupID);
}

void *trpgPageManager::GetGroupData(int groupID)
{
    ManageGroupMap::const_iterator p = groupMap.find(groupID);
    if (p != groupMap.end())
	return (*p).second;

    return NULL;
}

trpgManagedTile *trpgPageManager::GetNextUnload()
{
    // If we're already doing something, let them know about it
    if (lastLoad != None)
	throw 1;

    // Look for anything that needs unloaded
    // Start with highest LOD, work down to lowest
    trpgManagedTile *tile = NULL;
    for (int i=pageInfo.size()-1;i>=0;i--) {
	LodPageInfo &info = pageInfo[i];
	if ((tile = info.GetNextUnload()))
	    break;
    }

    // Found one.  Now the user has to unload it.
    if (tile) {
	lastLoad = Unload;
	lastTile = tile;
	lastLod = tile->location.lod;
    }

    return tile;
}

void trpgPageManager::AckUnload()
{
    // If we're not in the middle of an unload, let 'em know.
    if (lastLoad != Unload)
	throw 1;

    // Remove this tile's group IDs from the map
    const std::vector<int> *groupIDs = lastTile->GetGroupIDs();
    for (unsigned int i=0;i<groupIDs->size();i++) {
	ManageGroupMap::iterator p = groupMap.find((*groupIDs)[i]);
	if (p != groupMap.end())
	    groupMap.erase(p);
    }

    LodPageInfo &info = pageInfo[lastLod];
    info.AckUnload();
    lastLoad = None;
    lastTile = NULL;
}

bool trpgPageManager::Stop()
{
    bool res=false;
    for (unsigned int i=0;i<pageInfo.size();i++)
	res |= pageInfo[i].Stop();

    lastLoad = None;

    return res;
}

void trpgPageManager::Print(trpgPrintBuffer &buf)
{
    char line[1024];
    sprintf(line,"Paging pos = (%f,%f),  scale = %f",pagePt.x,pagePt.y,scale);  buf.prnLine(line);
    buf.prnLine("Terrain LODs:");

    for (unsigned int i=0;i<pageInfo.size();i++) {
	sprintf(line,"----Terrain lod %d---",i);  buf.prnLine(line);
        buf.IncreaseIndent();
	pageInfo[i].Print(buf);
	buf.DecreaseIndent();
    }
}

void *trpgr_ChildRefCB::Parse(trpgToken tok, trpgReadBuffer& rbuf)
{
    // It should be a tile ref token, if not then something is wrong
    if(tok == TRPG_CHILDREF)
    {
	childList.push_back(trpgChildRef());
	trpgChildRef &childRef = childList.back();
	if(childRef.Read(rbuf))
	    return &childRef;
	else
	    return false;

    }
    else
	return 0;
}
void trpgr_ChildRefCB::Reset()
{
    childList.clear();
}

unsigned int trpgr_ChildRefCB::GetNbChildren() const
{
    return childList.size();
}
const trpgChildRef& trpgr_ChildRefCB::GetChildRef(unsigned int idx) const
{
    if(idx >= childList.size())
	throw std::invalid_argument(
            "trpgPageManageTester::ChildRefCB::GetChild(): index argument out of bound.");
    else
	return childList[idx];
}

/* Page Manager Tester
   These methods are used to test the Paging Manager.
*/

trpgPageManageTester::trpgPageManageTester()
{
    manager = NULL;
    archive = NULL;
}

trpgPageManageTester::~trpgPageManageTester()
{
}

void trpgPageManageTester::Init(trpgPrintBuffer *pBuf,trpgPageManager *pMan,trpgr_Archive *inArch)
{
    archive = inArch;
    manager = pMan;
    printBuf = pBuf;

    if (!archive->isValid())
	throw 1;

    const trpgHeader *header = archive->GetHeader();
    header->GetVersion(majorVersion, minorVersion);

    tileParser.AddCallback(TRPG_CHILDREF, &childRefCB, false);

    // Start up the paging manager
    manager->Init(archive);
}

void trpgPageManageTester::RandomTest(int num,int seed)
{
    if (!manager || !archive || !printBuf)
	throw 1;

    // Seed the random number generator so we can replicate runs
    if (seed != -1)
	srand(seed);

    // Need the extents
    trpg2dPoint ll,ur,lod0Size;
    const trpgHeader *head = archive->GetHeader();
    head->GetExtents(ll,ur);
    head->GetTileSize(0,lod0Size);

    // Give ourselves some space around the extents
    ll.x -= lod0Size.x/2.0;  ll.y -= lod0Size.y/2.0;
    ur.x += lod0Size.x/2.0;  ur.y += lod0Size.y/2.0;

    // Jump around
    int i;
    char line[1024];
    for (i=0;i<num;i++) {
	// Generate a point
	double randNum1 = rand()/(double)RAND_MAX;
	double randNum2 = rand()/(double)RAND_MAX;
	trpg2dPoint pt;
	pt.x = (ur.x - ll.x)*randNum1;
	pt.y = (ur.y - ll.y)*randNum2;

	// Jump to the point
	bool changes = manager->SetLocation(pt);
	sprintf(line,"Jumped to (%f,%f).  Tiles to load/unload = %s",pt.x,pt.y,
	        (changes ? "yes" : "no"));  printBuf->prnLine(line);

	// Process the results
	ProcessChanges();
    }

    // Ask the page manager for its final status
    manager->Print(*printBuf);

    manager->Stop();
}

void trpgPageManageTester::Fly_LL_to_UR(double dist)
{
    char line[1024];

    if (!manager || !archive || !printBuf)
	throw 1;

    // Need the extents
    trpg2dPoint ll,ur,lod0Size;
    const trpgHeader *head = archive->GetHeader();
    head->GetExtents(ll,ur);
    head->GetTileSize(0,lod0Size);

    // Give ourselves some space around the extents
    ll.x -= lod0Size.x/2.0;  ll.y -= lod0Size.y/2.0;
    ur.x += lod0Size.x/2.0;  ur.y += lod0Size.y/2.0;


    // Fly the path
    trpg2dPoint loc;   loc = ll;
    do {
	loc.x += dist;  loc.y += dist;

	// Jump to next point
	bool changes = manager->SetLocation(loc);
	sprintf(line,"Moved to (%f,%f).  Tiles to load/unload = %s",loc.x,loc.y,
		(changes ? "yes" : "no"));  printBuf->prnLine(line);

	// Process new location
	ProcessChanges();
    } while (loc.x < ur.x && loc.y < ur.y);

    // Ask the page manager for its final status
    manager->Print(*printBuf);

    manager->Stop();
}

void trpgPageManageTester::ProcessChanges()
{
    char line[1024];
    int x,y,lod;

    // Look for unloads to process
    trpgManagedTile *unloadTile;
    printBuf->prnLine("Tiles to unload:");
    printBuf->IncreaseIndent();
    while ((unloadTile = manager->GetNextUnload())) {
	unloadTile->GetTileLoc(x,y,lod);
	sprintf(line,"x = %d, y = %d, lod = %d",x,y,lod);  printBuf->prnLine(line);
	manager->AckUnload();
    }
    printBuf->DecreaseIndent();

    // Look for loads to process
    trpgManagedTile *loadTile;
    printBuf->prnLine("Tiles to load:");
    printBuf->IncreaseIndent();
    while ((loadTile = manager->GetNextLoad())) {
	loadTile->GetTileLoc(x,y,lod);
	sprintf(line,"x = %d, y = %d, lod = %d",x,y,lod);  printBuf->prnLine(line);

	if(majorVersion == 2 && minorVersion >= 1)
	{
	    // Version 2.1 and over
	    // We need to parse the loaded tile to get all of its children info
	    const trpgwAppAddress& tileAddress = loadTile->GetTileAddress();
	    trpgMemReadBuffer buf(archive->GetEndian());
	    if(archive->ReadTile(tileAddress, buf))
	    {
		childRefCB.Reset();
		if(tileParser.Parse(buf))
		{
		    //  childRefCB should now have alist of trpgChildRef found in the tile
		    unsigned int nbChildRef = childRefCB.GetNbChildren();
		    if(nbChildRef > 0)
		    {
			std::vector<TileLocationInfo> locInfoList;
			for(unsigned int idx = 0; idx < nbChildRef; ++idx)
			{
			    const trpgChildRef& childRef = childRefCB.GetChildRef(idx);
			    locInfoList.push_back(TileLocationInfo());
			    TileLocationInfo& locInfo = locInfoList.back(); 
			    childRef.GetTileLoc(locInfo.x, locInfo.y, locInfo.lod);
			    childRef.GetTileAddress(locInfo.addr);
			}

			manager->AckLoad(locInfoList);
		    }
		    else
			manager->AckLoad();
		}
	    }
	    else
		manager->AckLoad();
	}
	else
	    manager->AckLoad();
    }
    printBuf->DecreaseIndent();
}
