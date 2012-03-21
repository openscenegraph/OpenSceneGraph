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

/* trpage_header.cpp
    Source for trpgHeader methods.
    The only reason to change this is if you want to add something
     to the header definition.
     */

#include <trpage_geom.h>
#include <trpage_read.h>

/* Write Header class
    Fill it in and write it out.
    */

// Constructor
trpgHeader::trpgHeader()
{
    Reset();
}
trpgHeader::~trpgHeader()
{
}

// Validity check
bool trpgHeader::isValid() const
{
    // We also need to check that this is a 'master' archive here.
    if((verMajor>=TRPG_NOMERGE_VERSION_MAJOR) && (verMinor>=TRPG_NOMERGE_VERSION_MINOR))
    {
        return true;
    }
    else {
        if (numLods <= 0)
        {
            strcpy(errMess, "Number of LOD <= 0");
            return false;
        }
        if (sw.x == ne.x && sw.y == ne.y)
        {
            strcpy(errMess, "Mbr is invalid");

            return false;
        }
    }

    return true;
}

// Reset contents
void trpgHeader::Reset()
{
    // Initialize to a default state
    verMinor = TRPG_VERSION_MINOR;
    verMajor = TRPG_VERSION_MAJOR;
    dbVerMinor = 0;
    dbVerMajor = 0;
    origin = trpg3dPoint(0,0,0);
    sw = ne = trpg2dPoint(0,0);
    tileType = DatabaseLocal;

    numLods = 0;
    lodSizes.resize(0);
    lodRanges.resize(0);
    tileSize.resize(0);
    maxGroupID = -1;
    flags = 0;
    errMess[0] = '\0';
    cols = -1;
    rows = -1;
}

// Set functions
void trpgHeader::SetVersion(int32 vmaj,int32 vmin)
{
    verMinor = vmin;
    verMajor = vmaj;
}
void trpgHeader::SetDbVersion(int32 vmaj,int32 vmin)
{
    dbVerMinor = vmin;
    dbVerMajor = vmaj;
}
void trpgHeader::SetTileSize(int id,const trpg2dPoint &pt)
{
    if (id < 0 || id >= (int)tileSize.size())  return;
    tileSize[id] = pt;
}
void trpgHeader::SetOrigin(const trpg3dPoint &pt)
{
    origin = pt;
}
void trpgHeader::SetExtents(const trpg2dPoint &in_sw,const trpg2dPoint &in_ne)
{
    sw = in_sw;
    ne = in_ne;
}
void trpgHeader::SetTileOriginType(trpgTileType type)
{
    tileType = type;
}
void trpgHeader::SetNumLods(int no)
{
    if (no < 0)  return;
    numLods = no;

    lodSizes.resize(no);
    lodRanges.resize(no);
}
void trpgHeader::SetLodSize(int no,const trpg2iPoint &pt)
{
    if (no < 0 || no >= numLods)
        return;

    lodSizes[no] = pt;
}
void trpgHeader::SetLodSize(const trpg2iPoint *pt)
{
    for (int i=0;i<numLods;i++)
        lodSizes[i] = pt[i];
}
void trpgHeader::SetLodRange(int no,float64 r)
{
    if (no < 0 || no >= numLods)
        return;

    lodRanges[no] = r;
}
void trpgHeader::SetLodRange(const float64 *r)
{
    for (int i=0;i<numLods;i++)
        lodRanges[i] = r[i];
}
void trpgHeader::AddLod(const trpg2iPoint &pt,const trpg2dPoint &sz,float64 r)
{
    lodRanges.push_back(r);
    lodSizes.push_back(pt);
    tileSize.push_back(sz);
    numLods++;
}
void trpgHeader::SetLod(const trpg2iPoint &pt,const trpg2dPoint &sz,float64 r,unsigned int lod)
{
    if (lodRanges.size()<=lod)
        lodRanges.resize(lod+1);
    lodRanges[lod]=r;
    if (lodSizes.size()<=lod)
        lodSizes.resize(lod+1);
    lodSizes[lod]=pt;
    if (tileSize.size()<=lod)
        tileSize.resize(lod+1);
    tileSize[lod]=sz;
    if (numLods<=static_cast<int>(lod))
        numLods=lod+1;
}
void trpgHeader::SetMaxGroupID(int id)
{
    maxGroupID = id;
}
int trpgHeader::AddGroupID(void)
{
    maxGroupID++;
    return maxGroupID;
}

// Write out to a buffer
bool trpgHeader::Write(trpgWriteBuffer &buf)
{

    if (!isValid())
        return false;

    buf.Begin(TRPGHEADER);
    buf.Add((int32)verMajor);
    buf.Add((int32)verMinor);
    buf.Add((int32)dbVerMajor);
    buf.Add((int32)dbVerMinor);
    buf.Add(origin);
    buf.Add(sw);
    buf.Add(ne);
    buf.Add((uint8)tileType);

    buf.Add((int32)numLods);

    buf.Begin(TRPGHEAD_LODINFO);
    for (int i=0;i<numLods;i++) {
        buf.Add(lodSizes[i]);
        buf.Add(lodRanges[i]);
        buf.Add(tileSize[i]);
    }
    buf.End();

    buf.Add(maxGroupID);

    if((verMajor >= TRPG_NOMERGE_VERSION_MAJOR) && (verMinor >=TRPG_NOMERGE_VERSION_MINOR)) {
        buf.Add(flags);
        buf.Add(rows);
        buf.Add(cols);
    }

    buf.End();

    return true;
}

/* ********
   Read Header class.
   */

// Get Functions
bool trpgHeader::GetVersion(int32 &vmaj,int32 &vmin) const
{
    if (!isValid()) return false;
    vmin = verMinor;
    vmaj = verMajor;
    return true;
}
bool trpgHeader::GetDbVersion(int32 &vmaj,int32 &vmin) const
{
    if (!isValid()) return false;
    vmaj = dbVerMajor;
    vmin = dbVerMinor;
    return true;
}
bool trpgHeader::GetTileSize(int id,trpg2dPoint &pt) const
{
    if (!isValid()) return false;
    if (id < 0 || id >= (int)tileSize.size())  return false;
    pt = tileSize[id];
    return true;
}
bool trpgHeader::GetOrigin(trpg3dPoint &pt) const
{
    if (!isValid()) return false;
    pt = origin;
    return true;
}
bool trpgHeader::GetTileOriginType(trpgTileType &type) const
{
    if (!isValid()) return false;
    type = tileType;
    return true;
}
bool trpgHeader::GetNumLods(int32 &no) const
{
    if (!isValid()) return false;
    no = numLods;
    return true;
}
bool trpgHeader::GetLodSize(int32 id,trpg2iPoint &pt) const
{
    if (!isValid() || (id < 0 || id >= numLods)) return false;
    pt = lodSizes[id];
    return true;
}
bool trpgHeader::GetLodRange(int32 id,float64 &range) const
{
    if (!isValid() || (id < 0 || id >= numLods)) return false;
    range = lodRanges[id];
    return true;
}
bool trpgHeader::GetExtents(trpg2dPoint &osw,trpg2dPoint &one) const
{
    if (!isValid()) return false;
    osw = sw;
    one = ne;
    return true;
}
bool trpgHeader::GetMaxGroupID(int &id) const
{
    id = maxGroupID;
    return true;
}

// Read in the header
bool trpgHeader::Read(trpgReadBuffer &buf)
{
    uint8 i8;
    trpgToken tok;
    bool status;
    int32 len;

    try {
        buf.Get(verMajor);
        buf.Get(verMinor);
        buf.Get(dbVerMajor);
        buf.Get(dbVerMinor);
        buf.Get(origin);
        buf.Get(sw);
        buf.Get(ne);
        buf.Get(i8);  tileType = (trpgTileType)i8;
        buf.Get(numLods);
        if (numLods < 0) throw 1;

        // Read in the LOD range info
        buf.GetToken(tok,len);
        if (tok != TRPGHEAD_LODINFO)  throw 1;
        buf.PushLimit(len);
        status = ReadLodInfo(buf);
        buf.PopLimit();
        if (!status) throw 1;

        // Added after the first version (but still in 1.0)
        buf.Get(maxGroupID);
        if((verMajor >= TRPG_NOMERGE_VERSION_MAJOR) && (verMinor >=TRPG_NOMERGE_VERSION_MINOR)) {
            buf.Get(flags);
            buf.Get(rows);
            buf.Get(cols);
        }
    }

    catch (...) {
        return false;
    }

    return isValid();
}

// Read the LOD info (separate token)
bool trpgHeader::ReadLodInfo(trpgReadBuffer &buf)
{
    float64 range;
    trpg2iPoint pt;
    trpg2dPoint sz;

    try {
        for (int i=0;i<numLods;i++) {
            buf.Get(pt);
            buf.Get(range);
            buf.Get(sz);
            lodSizes.push_back(pt);
            lodRanges.push_back(range);
            tileSize.push_back(sz);
        }
    }
    catch (...) {
        return false;
    }

    return true;
}

