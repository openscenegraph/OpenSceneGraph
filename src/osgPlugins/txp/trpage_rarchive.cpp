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

/* trpage_rarchive.cpp
    This source file implements the methods for a trpgr_Archive.
    The Read Archive is used to read a paging archive from disk.
    */

#include <trpage_read.h>
#include <trpage_compat.h>

// Constructor
trpgr_Archive::trpgr_Archive()
{
    fp = NULL;
    ness = LittleEndian;
    strcpy(dir,".");
    tileCache = NULL;
}

// Destructor
trpgr_Archive::~trpgr_Archive()
{
    if (fp)
        fclose(fp);
    fp = NULL;
    if (tileCache)
        delete tileCache;
}

// Set the directory where the archive is
void trpgr_Archive::SetDirectory(const char *in_dir)
{
    strncpy(dir,in_dir,1024);
}

// Open File
// Open the given file and look for the file specific info
bool trpgr_Archive::OpenFile(const char *name)
{
    char file[1024];
    sprintf(file,"%s" PATHSEPARATOR "%s",dir,name);

    CloseFile();

    if (!(fp = fopen(file,"rb")))
        return false;

    // Look for a magic # and endianness
    int32 magic;
    if (fread(&magic,sizeof(int32),1,fp) != 1)
        return false;

    headerRead = false;

    // Figure out the endianness from the magic number
    trpgEndian cpuNess = trpg_cpu_byte_order();
    if (magic == TRPG_MAGIC) {
        ness = cpuNess;
        return true;
    }
    if (trpg_byteswap_int(magic) == TRPG_MAGIC) {
        if (cpuNess == LittleEndian)
            ness = BigEndian;
        else
            ness = LittleEndian;
        return true;
    }
    if (magic != TRPG_MAGIC)
        return false;

    // Not one of our files
    return false;
}

// Close File
// Close the currently open file
void trpgr_Archive::CloseFile()
{
    if (fp)
        fclose(fp);
    fp = NULL;
    if (tileCache)
        delete tileCache;
    tileCache = NULL;
}

// Read Header
// Run through the rest of the header information
bool trpgr_Archive::ReadHeader()
{
    if (!fp || headerRead)
        return false;

    headerRead = true;

    // Next int64 should be the header size
    trpgEndian cpuNess = trpg_cpu_byte_order();
    int32 headerSize;
    if (fread(&headerSize,sizeof(int32),1,fp) != 1) return false;
    if (ness != cpuNess)
        headerSize = trpg_byteswap_int(headerSize);
    int headLen = headerSize;
    if (headLen < 0)  return false;

    // Read in the header whole
    trpgMemReadBuffer buf(ness);
    buf.SetLength(headLen);
    char *data = buf.GetDataPtr();
    if (fread(data,1,headLen,fp) != static_cast<unsigned int>(headLen))  return false;

    // Set up a parser
    // Catch the tables we need for the archive
    trpgMatTable1_0 oldMatTable;
    trpgTexTable1_0 oldTexTable;
    trpgr_Parser parser;
    parser.AddCallback(TRPGHEADER,&header);
    parser.AddCallback(TRPGMATTABLE,&materialTable);    // Went back to oldest style for 2.0
    parser.AddCallback(TRPGMATTABLE2,&oldMatTable);     // Added 11-14-98 (1.0 material table)
    parser.AddCallback(TRPGTEXTABLE,&oldTexTable);
    parser.AddCallback(TRPGTEXTABLE2,&texTable);        // Added for 2.0
    parser.AddCallback(TRPGMODELTABLE,&modelTable);
    parser.AddCallback(TRPGLIGHTTABLE,&lightTable);        // Added for 2.0
    parser.AddCallback(TRPGRANGETABLE,&rangeTable);        // Added for 2.0

    // Don't read the tile table for v1.0 archives
    // It's only really used for 2.0 archives
    parser.AddCallback(TRPGTILETABLE2,&tileTable);

    // Parse the buffer
    if (!parser.Parse(buf))
        return false;

    // 1.0 Compatibility
    // If we see an older style material table, convert it to the new style
    // This isn't terribly memory efficient, but it does work
    if (oldMatTable.isValid())
        materialTable = oldMatTable;
    if (oldTexTable.isValid())
        texTable = oldTexTable;

    // Set up a tile cache, if needed
    trpgTileTable::TileMode tileMode;
    tileTable.GetMode(tileMode);
    if (tileMode == trpgTileTable::Local) {
        if (tileCache)  delete tileCache;
        char fullBase[1024];
        sprintf(fullBase,"%s" PATHSEPARATOR "tileFile",dir);
        tileCache = new trpgrAppFileCache(fullBase,"tpf");
    }

    valid = true;

    return true;
}

// Read Tile
// Read a tile into a read buffer
bool trpgr_Archive::ReadTile(uint32 x,uint32 y,uint32 lod,trpgMemReadBuffer &buf)
{
    if (!isValid()) return false;

    // Reality check the address
    int32 numLods;
    header.GetNumLods(numLods);
    if (lod >= static_cast<unsigned int>(numLods)) return false;
    trpg2iPoint lodSize;
    header.GetLodSize(lod,lodSize);
    if (x >= static_cast<unsigned int>(lodSize.x) || y >= static_cast<unsigned int>(lodSize.y)) return false;

    trpgTileTable::TileMode tileMode;
    tileTable.GetMode(tileMode);

    if (tileMode == trpgTileTable::External) {
        // Figure out the file name
        // Note: This assumes External tiles
        char filename[1024];
        sprintf(filename,"%s" PATHSEPARATOR "tile_%d_%d_%d.tpt",dir,x,y,lod);

        // Open the file and read the contents
        FILE *fp=NULL;
        try {
            if (!(fp = fopen(filename,"rb")))  throw 1;
            // Find the file end
            if (fseek(fp,0,SEEK_END))  throw 1;
            // Note: This means tile is capped at 2 gigs
            long pos = ftell(fp);
            if (fseek(fp,0,SEEK_SET)) throw 1;
            // Now we know the size.  Read the whole file
            buf.SetLength(pos);
            char *data = buf.GetDataPtr();
            if (fread(data,pos,1,fp) != 1) throw 1;
            fclose(fp); fp = NULL;
        }
        catch (...) {
            if (fp)
                fclose(fp);
            return false;
        }
    } else {
        // Local tile.  Figure out where it is (which file)
        trpgwAppAddress addr;
        float zmin,zmax;
        if (!tileTable.GetTile(x,y,lod,addr,zmin,zmax))
            return false;

        // Fetch the appendable file from the cache
        trpgrAppFile *tf = tileCache->GetFile(ness,addr.file);
        if (!tf)  return false;

        // Fetch the tile
        if (!tf->Read(&buf,addr.offset))
            return false;
    }

    return true;
}

// Get methods
const trpgHeader *trpgr_Archive::GetHeader() const
{
    return &header;
}
const trpgMatTable *trpgr_Archive::GetMaterialTable() const
{
    return &materialTable;
}
const trpgTexTable *trpgr_Archive::GetTexTable() const
{
    return &texTable;
}
const trpgModelTable *trpgr_Archive::GetModelTable() const
{
    return &modelTable;
}
const trpgTileTable *trpgr_Archive::GetTileTable() const
{
    return &tileTable;
}
const trpgLightTable *trpgr_Archive::GetLightTable() const
{
    return &lightTable;
}
const trpgRangeTable *trpgr_Archive::GetRangeTable() const
{
    return &rangeTable;
}
trpgEndian trpgr_Archive::GetEndian() const
{
    return ness;
}

// Utility MBR routine
bool trpgr_Archive::trpgGetTileMBR(uint32 x,uint32 y,uint32 lod,trpg3dPoint &ll,trpg3dPoint &ur) const
{
    if (!header.isValid())
        return false;
    int32 numLod;
    header.GetNumLods(numLod);
    trpg2iPoint maxXY;
    header.GetLodSize(lod,maxXY);
    if (x >= static_cast<unsigned int>(maxXY.x) || y>= static_cast<unsigned int>(maxXY.y))
        return false;

    trpg3dPoint origin;
    header.GetOrigin(origin);
    trpg2dPoint size;
    header.GetTileSize(lod,size);

    ll.x = origin.x + size.x*x;
    ll.y = origin.y + size.y*y;
    ur.x = origin.x + size.x*(x+1);
    ur.y = origin.y + size.y*(y+1);

    // If the tiles are local, we should have Z information
    trpgwAppAddress addr;
    float elev_min=0.0,elev_max=0.0;
    tileTable.GetTile(x,y,lod,addr,elev_min,elev_max);
    ll.z = elev_min;  ur.z = elev_max;

    return true;
}

/* *****************
   Read Image Helper
   *****************
 */

trpgrImageHelper::trpgrImageHelper(trpgEndian inNess,char *inDir,
                                   const trpgMatTable &inMatTable,const trpgTexTable &inTexTable)
{
    ness = inNess;
    strcpy(dir,inDir);
    matTable = &inMatTable;
    texTable = &inTexTable;

    // Set up the texture cache
    // It doesn't do anything until it's called anyway
    char fullBase[1024];
    sprintf(fullBase,"%s" PATHSEPARATOR "texFile",dir);
    texCache = new trpgrAppFileCache(fullBase,"txf");
}

trpgrImageHelper::~trpgrImageHelper()
{
    if (texCache)
        delete texCache;
}

bool trpgrImageHelper::GetLocalGL(const trpgTexture *tex,char *data,int32 size)
{
    // Make sure the texture is Local
    trpgTexture::ImageMode mode;
    tex->GetImageMode(mode);
    if (mode != trpgTexture::Local)
        return false;

    // Fetch data data
    trpgwAppAddress addr;
    tex->GetImageAddr(addr);
    trpgrAppFile *af = texCache->GetFile(ness,addr.file);
    if (!af)
        return false;
    if (!af->Read(data,addr.offset,size))
        return false;

    return true;
}

bool trpgrImageHelper::GetMipLevelLocalGL(int miplevel, const trpgTexture *tex,char *data,int32 dataSize)
{
    if ( miplevel >= tex->CalcNumMipmaps() || miplevel < 0 )
        return false;

    // Make sure the texture is Local
    trpgTexture::ImageMode mode;
    tex->GetImageMode(mode);
    if (mode != trpgTexture::Local)
        return false;

    // Fetch data data
    trpgwAppAddress addr;
    tex->GetImageAddr(addr);
    trpgrAppFile *af = texCache->GetFile(ness,addr.file);
    if (!af)
        return false;

    int level_offset = (const_cast<trpgTexture*>(tex))->MipLevelOffset(miplevel);
    if (!af->Read(data,addr.offset+level_offset,dataSize))
        return false;

    return true;
}


bool trpgrImageHelper::GetImageInfoForLocalMat(const trpgLocalMaterial *locMat,
                           const trpgMaterial **retMat,const trpgTexture **retTex,int &totSize)
{
    // Get the base material for the Local Material
    int32 matSubTable,matID;
    locMat->GetBaseMaterial(matSubTable,matID);
    const trpgMaterial *mat = matTable->GetMaterialRef(matSubTable,matID);
    if (!mat)  return false;

    // Now get the texture (always the first one)
    trpgTextureEnv texEnv;
    int32 texID;
    if (!mat->GetTexture(0,texID,texEnv)) return false;
    const trpgTexture *tex = texTable->GetTextureRef(texID);
    if (!tex)  return false;

    totSize = tex->CalcTotalSize();

    *retTex = tex;
    *retMat = mat;
    return true;
}

bool trpgrImageHelper::GetImageForLocalMat(const trpgLocalMaterial *locMat,char *data,int dataSize)
{
    if (!locMat->isValid()) return false;

    const trpgMaterial *mat;
    const trpgTexture *tex;
    int totSize;
    if (!GetImageInfoForLocalMat(locMat,&mat,&tex,totSize))
        return false;

    // Determine the type
    trpgTexture::ImageMode imageMode;
    tex->GetImageMode(imageMode);
    switch (imageMode) {
    case trpgTexture::Template:
        {
            // Read the image data out of the Local image (in an archive somewhere)
            trpgwAppAddress addr;
            locMat->GetAddr(addr);
            trpgrAppFile *af = texCache->GetFile(ness,addr.file);
            if (!af)  return false;
            if (!af->Read(data,addr.offset,dataSize))
                return false;
        }
        break;
    case trpgTexture::Global:
        // Note: Not dealing with Global textures yet
        return false;
        break;
    default:
        // This is not a valid Local Material
        return false;
    };

    return true;
}

bool trpgrImageHelper::GetMipLevelForLocalMat(int miplevel, const trpgLocalMaterial *locMat,char *data,int dataSize)
{
    if (!locMat->isValid()) return false;

    const trpgMaterial *mat;
    const trpgTexture *tex;
    int totSize;
    if (!GetImageInfoForLocalMat(locMat,&mat,&tex,totSize))
        return false;

    if ( miplevel >= tex->CalcNumMipmaps() || miplevel < 0 )
        return false;

    // Determine the type
    trpgTexture::ImageMode imageMode;
    tex->GetImageMode(imageMode);
    switch (imageMode) {
    case trpgTexture::Template:
        {
            // Read the image data out of the Local image (in an archive somewhere)
            trpgwAppAddress addr;
            locMat->GetAddr(addr);
            trpgrAppFile *af = texCache->GetFile(ness,addr.file);
            if (!af)  return false;

            int level_offset = (const_cast<trpgTexture*>(tex))->MipLevelOffset(miplevel);
            if (!af->Read(data,addr.offset+level_offset,dataSize))
                return false;
        }
        break;
    case trpgTexture::Global:
        // Note: Not dealing with Global textures yet
        return false;
        break;
    default:
        // This is not a valid Local Material
        return false;
    };

    return true;
}

bool trpgrImageHelper::GetImagePath(const trpgTexture *tex,char *fullPath,int pathLen)
{
    char name[1024];
    int nameLen=1024;
    tex->GetName(name,nameLen);
    nameLen = strlen(name);

    if (strlen(dir) + nameLen + 2 > static_cast<unsigned int>(pathLen))
        return false;

    sprintf(fullPath,"%s" PATHSEPARATOR "%s",dir,name);

    return true;
}
