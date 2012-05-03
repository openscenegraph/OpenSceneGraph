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

#include <osgDB/FileUtils>

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

int32 trpgr_Archive::GetHeaderData(char *dataPtr, int length, FILE *filehandle)
{
    return fread(dataPtr,1,length,filehandle);
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
    sprintf(file,"%s" PATHSEPERATOR "%s",dir,name);

    CloseFile();

    if (!(fp = osgDB::fopen(file,"rb")))
        return false;

    // Look for a magic # and endianness
    int32 magic;
    if (fread(&magic,sizeof(int32),1,fp) != 1)
        return false;

    headerRead = false;

    // Figure out the endianness from the magic number
    trpgEndian cpuNess = trpg_cpu_byte_order();
    if (magic == GetMagicNumber()) {
        ness = cpuNess;
        return true;
    }
    if (trpg_byteswap_int(magic) == GetMagicNumber()) {
        if (cpuNess == LittleEndian)
            ness = BigEndian;
        else
            ness = LittleEndian;
        return true;
    }
    if (magic != GetMagicNumber())
        return false;

    // Not one of our files
    return false;
}

// Get new reading app file cache
trpgrAppFileCache* trpgr_Archive::GetNewRAppFileCache(const char *fullBase, const char *ext)
{
    return new trpgrAppFileCache(fullBase,ext);
}

trpgrImageHelper* trpgr_Archive::GetNewRImageHelper(trpgEndian ness,char *dir,const trpgMatTable &matTable,const trpgTexTable &texTable)
{
    bool separateGeo = false;
    int majorVer,minorVer;
    GetHeader()->GetVersion(majorVer,minorVer);
    if((majorVer >= TRPG_NOMERGE_VERSION_MAJOR) && (minorVer>=TRPG_NOMERGE_VERSION_MINOR)) {
        separateGeo = true;
    }
    return new trpgrImageHelper(ness,dir,matTable,texTable,separateGeo);
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

/**
 * Read a sub block from a 2.2 TXP database. This can be called any time after ReadHeader is called
 * if ReadHeader is called with the false parameter to specify not to read all the sub-archives.
 * This can make a huge improvement in startup time for loading a very large archive with many blocks.
 **/
bool trpgr_Archive::ReadSubArchive(int row, int col, trpgEndian cpuNess)
{
    int ret;
    trpgHeader blockHeader;
    trpgr_Parser bparser;

    char blockpath[1024];
    //open the block archive
    // the block archive will be in the base dir + \\cols\\row\\archive.txp
    sprintf(blockpath,"%s%s%d%s%d%sarchive.txp",dir,PATHSEPERATOR,col,PATHSEPERATOR,row,PATHSEPERATOR);
    FILE *bfp = osgDB::fopen(blockpath,"rb");
    if(!bfp) {
        return false;
    }
    // Look for a magic # and endianness
    int32 bmagic;
    if (fread(&bmagic,sizeof(int32),1,bfp) != 1)
        return false;
    // The block archive will always be the same endianness as the master
    if ( (bmagic != GetMagicNumber()) && (trpg_byteswap_int(bmagic) != GetMagicNumber()) )
        return false;

    int32 bheaderSize=0;
    if (fread(&bheaderSize,sizeof(int32),1,bfp) != 1)
        return false;
    if (ness != cpuNess)
        bheaderSize = trpg_byteswap_int(bheaderSize);
    int bheadLen = bheaderSize;
    if (bheadLen < 0)
        return false;

    // Read in the header whole
    trpgMemReadBuffer bbuf(ness);
    bbuf.SetLength(bheadLen);
    char *bdata = bbuf.GetDataPtr();
    if ((ret = GetHeaderData(bdata,bheadLen,bfp)) != bheadLen)
        return false;
    //keep track of where this came from in the master table.
    tileTable.SetCurrentBlock(row,col,true);
    texTable.SetCurrentBlock(row,col);

    bparser.AddCallback(TRPGHEADER,&blockHeader);
    bparser.AddCallback(TRPGMATTABLE,&materialTable);    // Went back to oldest style for 2.0
    //if(!headerHasTexTable) {
    bparser.AddCallback(TRPGTEXTABLE2,&texTable);            // Added for 2.0
    //}
    bparser.AddCallback(TRPGMODELTABLE,&modelTable);
    bparser.AddCallback(TRPGLIGHTTABLE,&lightTable);                // Added for 2.0
    bparser.AddCallback(TRPGRANGETABLE,&rangeTable);                // Added for 2.0
    bparser.AddCallback(TRPG_TEXT_STYLE_TABLE,&textStyleTable);                // Added for 2.1
    bparser.AddCallback(TRPG_SUPPORT_STYLE_TABLE,&supportStyleTable);
    bparser.AddCallback(TRPG_LABEL_PROPERTY_TABLE,&labelPropertyTable);
    // Don't read the tile table for v1.0 archives
    // It's only really used for 2.0 archives
    bparser.AddCallback(TRPGTILETABLE2,&tileTable);

    // Parse the buffer
    if (!bparser.Parse(bbuf))
        return false;
    //close the block archive
    fclose(bfp);

    tileTable.SetCurrentBlock(-1,-1,false);

    return true;
}

bool trpgr_Archive::ReadHeader()
{
    return ReadHeader(true);
}

// Read Header
// Run through the rest of the header information
bool trpgr_Archive::ReadHeader(bool readAllBlocks)
{
    int ret;

    if (!fp || headerRead)
        return false;

    headerRead = true;

    // Next int64 should be the header size
    trpgEndian cpuNess = trpg_cpu_byte_order();
    int32 headerSize;
    if (fread(&headerSize,sizeof(int32),1,fp) != 1)
        return false;
    if (ness != cpuNess)
        headerSize = trpg_byteswap_int(headerSize);
    int headLen = headerSize;
    if (headLen < 0)
        return false;

    // Read in the header whole
    trpgMemReadBuffer buf(ness);
    buf.SetLength(headLen);
    char *data = buf.GetDataPtr();
    if ((ret = GetHeaderData(data,headLen,fp)) != headLen)
        return false;

    // Set up a parser
    // Catch the tables we need for the archive
    trpgMatTable1_0 oldMatTable;
    trpgTexTable1_0 oldTexTable;
    trpgr_Parser parser;
    parser.AddCallback(TRPGHEADER,&header);
    parser.AddCallback(TRPGMATTABLE,&materialTable);    // Went back to oldest style for 2.0
    parser.AddCallback(TRPGMATTABLE2,&oldMatTable);     // Added 11-14-98 (1.0 material table)
    parser.AddCallback(TRPGTEXTABLE,&oldTexTable);
    parser.AddCallback(TRPGTEXTABLE2,&texTable);            // Added for 2.0
    parser.AddCallback(TRPGMODELTABLE,&modelTable);
    parser.AddCallback(TRPGLIGHTTABLE,&lightTable);                // Added for 2.0
    parser.AddCallback(TRPGRANGETABLE,&rangeTable);                // Added for 2.0
    parser.AddCallback(TRPG_TEXT_STYLE_TABLE,&textStyleTable);                // Added for 2.1
    parser.AddCallback(TRPG_SUPPORT_STYLE_TABLE,&supportStyleTable);
    parser.AddCallback(TRPG_LABEL_PROPERTY_TABLE,&labelPropertyTable);
    // Don't read the tile table for v1.0 archives
    // It's only really used for 2.0 archives
    parser.AddCallback(TRPGTILETABLE2,&tileTable);

    // Parse the buffer
    if (!parser.Parse(buf))
        return false;

    if(header.GetIsMaster())
    {
        // bool firstBlock = true;
        //if the master has textures, we want to use them instead of the tables in the
        //block archives

        // int numTiles = 0;
        //tileTable.
        int totalrows,totalcols;
        trpg2dPoint mhdr_swExtents;
        trpg2dPoint mhdr_neExtents;
        trpg3dPoint mhdr_Origin;
        // integrate header information from the block header.
        header.GetExtents(mhdr_swExtents,mhdr_neExtents);
        header.GetOrigin(mhdr_Origin);
        header.GetBlocks(totalrows,totalcols);
        if(readAllBlocks) {
            for(int row=0;row<totalrows;row++) {
                for(int col=0;col<totalcols;col++) {
                    // Read each block -- Warning, this can take a while!!!
                    ReadSubArchive( row, col, cpuNess);
                }
            }
        }
        else {
            ReadSubArchive( 0, 0, cpuNess);//Get the first archive!
        }

    }
    tileTable.SetCurrentBlock(-1,-1,false);

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
        sprintf(fullBase,"%s" PATHSEPERATOR "tileFile",dir);
        tileCache = GetNewRAppFileCache(fullBase,"tpf");
    }

    valid = true;

    return true;
}

// Read Tile
// Read a tile into a read buffer
// For version 2.1 only  tile with lod=0 are stored in the tile table, so an
// error will be returned if you try to use the table with a differrent lod.
bool trpgr_Archive::ReadTile(uint32 x,uint32 y,uint32 lod,trpgMemReadBuffer &buf)
{
    if (!isValid())
        return false;

    // Reality check the address
    int32 numLods;
    header.GetNumLods(numLods);
    if (static_cast<int>(lod) >= numLods)
        return false;
    trpg2iPoint lodSize;
    header.GetLodSize(lod,lodSize);
    if (static_cast<int>(x) >= lodSize.x || static_cast<int>(y) >= lodSize.y)
        return false;

    trpgTileTable::TileMode tileMode;
    tileTable.GetMode(tileMode);

    bool status = true;
    if (tileMode == trpgTileTable::External || tileMode == trpgTileTable::ExternalSaved) {
        status = ReadExternalTile(x, y, lod, buf);

    } else {
        // Local tile.  Figure out where it is (which file)
        int majorVersion, minorVersion;
        header.GetVersion(majorVersion, minorVersion);
        if(majorVersion == 2 && minorVersion >=1)
        {
            // Version 2.1
            // Tile table contains only lod 0 tiles
            if(lod != 0)
                status = false;
        }

        if(status)
        {
            trpgwAppAddress addr;
            float zmin,zmax;
            status = tileTable.GetTile(x,y,lod,addr,zmin,zmax);

            if(status)
                status = ReadTile(addr, buf);
        }
    }

    return status;
}

bool trpgr_Archive::ReadExternalTile(uint32 x,uint32 y,uint32 lod,trpgMemReadBuffer &buf)
{
    // Figure out the file name
    char filename[1024];
    int majorVer,minorVer;
    header.GetVersion(majorVer,minorVer);
    if((majorVer >= TRPG_NOMERGE_VERSION_MAJOR) && (minorVer >= TRPG_NOMERGE_VERSION_MINOR)) {
        int blockx,blocky;
        unsigned int denom = (1 << lod); // this should work up to lod 31
        blockx = x/denom;
        blocky = y/denom;
        sprintf(filename,"%s" PATHSEPERATOR "%d" PATHSEPERATOR "%d" PATHSEPERATOR "tile_%d_%d_%d.tpt",
                dir,blockx,blocky,x,y,lod);
    }
    else {
        sprintf(filename,"%s" PATHSEPERATOR "tile_%d_%d_%d.tpt",dir,x,y,lod);
    }
    // Open the file and read the contents
    FILE *fp= 0;
    try {
        if (!(fp = osgDB::fopen(filename,"rb")))  {

            throw 1;
        }
        // Find the file end
        if (fseek(fp,0,SEEK_END))
            throw 1;
        // Note: This means tile is capped at 2 gigs
        long pos = ftell(fp);
        if (fseek(fp,0,SEEK_SET))
            throw 1;
        // Now we know the size.  Read the whole file
        buf.SetLength(pos);
        char *data = buf.GetDataPtr();
        if (fread(data,pos,1,fp) != 1)
            throw 1;
        fclose(fp);
        fp = NULL;
    }
    catch (...) {
        if (fp)
            fclose(fp);
        return false;
    }

    return true;
}
bool trpgr_Archive::ReadTile(const trpgwAppAddress& addr, trpgMemReadBuffer &buf)
{
    // Fetch the appendable file from the cache
    trpgrAppFile *tf = tileCache->GetFile(ness,addr.file,addr.col,addr.row);
    if (!tf)
        return false;

    // Fetch the tile
    if (!tf->Read(&buf,addr.offset))
        return false;
    else
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
trpgTexTable *trpgr_Archive::GetTexTable()
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
const trpgTextStyleTable *trpgr_Archive::GetTextStyleTable() const
{
    return &textStyleTable;
}
const trpgSupportStyleTable *trpgr_Archive::GetSupportStyleTable() const
{
    return &supportStyleTable;
}
const trpgLabelPropertyTable *trpgr_Archive::GetLabelPropertyTable() const
{
    return &labelPropertyTable;
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
    if (static_cast<int>(x) >= maxXY.x || static_cast<int>(y)>= maxXY.y)
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
                                   const trpgMatTable &inMatTable,const trpgTexTable &inTexTable,bool separateGeoTyp)
{
    Init(inNess,inDir,inMatTable,inTexTable,separateGeoTyp);
}

void trpgrImageHelper::Init(trpgEndian inNess,char *inDir,
                            const trpgMatTable &inMatTable,const trpgTexTable &inTexTable,bool separateGeoTyp)
{
    ness = inNess;
    strcpy(dir,inDir);
    this->separateGeoTyp = separateGeoTyp;
    matTable = &inMatTable;
    texTable = &inTexTable;

    // Set up the texture cache
    // It doesn't do anything until it's called anyway
    char fullBase[1024];
    sprintf(fullBase,"%s" PATHSEPERATOR "texFile",dir);
    texCache = GetNewRAppFileCache(fullBase,"txf");
    if(separateGeoTyp) {
        sprintf(fullBase,"%s" PATHSEPERATOR "geotypFile",dir);
        geotypCache = GetNewRAppFileCache(fullBase,"txf");
    }
    else {
        geotypCache = texCache;
    }

}

trpgrImageHelper::~trpgrImageHelper()
{
    if (texCache) {
        delete texCache;
        texCache = NULL;
    }
    if(separateGeoTyp && geotypCache) {
        delete geotypCache;
        geotypCache = NULL;
    }
}

trpgrAppFileCache* trpgrImageHelper::GetNewRAppFileCache(const char *fullBase,const char* /*ext*/)
{
    return new trpgrAppFileCache(fullBase,"txf");
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
    trpgrAppFile *af = geotypCache->GetFile(ness,addr.file,addr.col,addr.row);
    if (!af)
        return false;
    if (!af->Read(data,addr.offset,0,size))
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
    trpgrAppFile *af = texCache->GetFile(ness,addr.file,addr.col,addr.row);
    if (!af)
        return false;

    int level_offset = (const_cast<trpgTexture*>(tex))->MipLevelOffset(miplevel);
    if (!af->Read(data,addr.offset,level_offset,dataSize))
        return false;

    return true;
}


bool trpgrImageHelper::GetImageInfoForLocalMat(const trpgLocalMaterial *locMat,
                                               const trpgMaterial **retMat,const trpgTexture **retTex,int &totSize)
{
    return GetNthImageInfoForLocalMat(locMat, 0, retMat, retTex, totSize);
}

bool trpgrImageHelper::GetNthImageInfoForLocalMat(const trpgLocalMaterial *locMat, int index,
                                                  const trpgMaterial **retMat,const trpgTexture **retTex,int &totSize)
{
    // Get the base material for the Local Material
    int32 matSubTable,matID;
    locMat->GetBaseMaterial(matSubTable,matID);
    // For right now, force the subtable number to match the index.
    // Eventually, either store multiple base materials for each local material,
    // or overhaul this in some other fashion.
    int numTables;
    if (!matTable->GetNumTable(numTables))
        return false;
    if (index>=numTables)
        return false;
    if (index>0) matSubTable=index; // otherwise, leave it alone - could be nonzero
    const trpgMaterial *mat = matTable->GetMaterialRef(matSubTable,matID);
    if (!mat)
        return false;

    // Now get the texture (always the first one)
    trpgTextureEnv texEnv;
    int32 texID;
    if (!mat->GetTexture(0,texID,texEnv))
        return false;
    const trpgTexture *tex = texTable->GetTextureRef(texID);
    if (!tex)
        return false;

    totSize = tex->CalcTotalSize();

    *retTex = tex;
    *retMat = mat;
    return true;
}

bool trpgrImageHelper::GetImageForLocalMat(const trpgLocalMaterial *locMat,char *data,int dataSize)
{
    return GetNthImageForLocalMat(locMat, 0, data, dataSize);
}

bool trpgrImageHelper::GetNthImageForLocalMat(const trpgLocalMaterial *locMat,int index, char *data,int dataSize)
{
    if (!locMat->isValid())
        return false;

    const trpgMaterial *mat;
    const trpgTexture *tex;
    int totSize;
    if (!GetNthImageInfoForLocalMat(locMat,index,&mat,&tex,totSize))
        return false;

    // Determine the type
    trpgTexture::ImageMode imageMode;
    tex->GetImageMode(imageMode);
    switch (imageMode) {
    case trpgTexture::Template:
    {
        // Read the image data out of the Local image (in an archive somewhere)
        trpgwAppAddress addr;
        if (!locMat->GetNthAddr(index,addr)) return false;
        trpgrAppFile *af = texCache->GetFile(ness,addr.file,addr.col,addr.row);
        if (!af)
            return false;
        if (!af->Read(data,addr.offset,0,dataSize))
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
    return GetNthImageMipLevelForLocalMat(miplevel, locMat, 0, data, dataSize);
}

bool trpgrImageHelper::GetNthImageMipLevelForLocalMat(int miplevel, const trpgLocalMaterial *locMat, int index, char *data,int dataSize)
{
    if (index>0) return false; // not yet, folks, if ever.  index>1 means sensors for now.
    if (!locMat->isValid()) return false;

    const trpgMaterial *mat;
    const trpgTexture *tex;
    int totSize;
    if (!GetNthImageInfoForLocalMat(locMat,index,&mat,&tex,totSize))
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
        if (!locMat->GetNthAddr(index,addr)) return false;
        trpgrAppFile *af = texCache->GetFile(ness,addr.file,addr.col,addr.row);
        if (!af)  return false;

        int level_offset = (const_cast<trpgTexture*>(tex))->MipLevelOffset(miplevel);
        if (!af->Read(data,addr.offset,level_offset,dataSize))
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

    if (static_cast<int>(strlen(dir)) + nameLen + 2 > pathLen)
        return false;

    sprintf(fullPath,"%s" PATHSEPERATOR "%s",dir,name);

    return true;
}
