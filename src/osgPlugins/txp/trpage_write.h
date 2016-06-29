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

#ifndef _txpage_write_h_
// {secret}
#define _txpage_write_h_

/* trpage_write.h
    Classes that are used to write paging archives.
    */

#include <trpage_sys.h>
#include <trpage_io.h>
#include <trpage_swap.h>
#include <trpage_read.h>

/* Geometry Stats
    Used with a Geometry Helper to keep track of what go built.
    {group:Archive Writing}
    */
TX_EXDECL class TX_CLDECL trpgwGeomStats {
public:
    trpgwGeomStats(void);
    ~trpgwGeomStats(void);

    int totalTri;  // Total # of triangles

    int totalQuad;  // Total # of quads

    // Add up to totalTri
    int totalStripTri;  // triangles in strips
    int totalFanTri;    // triangles in fans
    int totalBagTri;    // loose triangles

    int numStrip;       // Number of distinct strips
    int numFan;            // Number of distinct fans

    int stripStat[15];  // Strip length stats
    int fanStat[15];    // Fan length stats

    int stripGeom;        // Number of separate trpgGeometry nodes for strips
    int fanGeom;        // Same for fans
    int bagGeom;        // Same for bags

    int stateChanges;    // Number of distinct material switches

    // Helper functions
    inline void AddStripStat(int val) { stripStat[MIN(14,val)]++; totalStripTri += val; totalTri += val; numStrip++;}
    inline void AddFanStat(int val) { fanStat[MIN(14,val)]++; totalFanTri += val; totalTri += val; numFan++;}
    inline void AddBagStat(int val) { totalBagTri += val; totalTri += val;}
    inline void AddQuadStat(int val) { totalQuad += val; }
};

/* Geometry Helper
    Collects up geometry and tries to form triangle strips, fans,
     and groups of triangles.
    Right now this looks for a very careful ordering.  If that ordering
     isn't there you won't get useful tristrips or fans.  You can, however
     use this class as a starting point and build something more akin
     to the geometry builder in Performer.
    {group:Archive Writing}
 */
TX_EXDECL class TX_CLDECL trpgwGeomHelper {
public:
    trpgwGeomHelper(void);
    virtual ~trpgwGeomHelper(void);
    enum {UseDouble,UseFloat};
    trpgwGeomHelper(trpgWriteBuffer *,int dataType=UseDouble);
    void init(trpgWriteBuffer *,int dataType=UseDouble);
    virtual void SetMode(int);  // Takes a trpgGeometry primitive type (triangle by default)
    virtual void Reset(void);
    // Start/End polygon definition
    virtual void StartPolygon(void);
    virtual void EndPolygon(void);
    virtual void ResetPolygon(void);  // If you change your mind about the current poly
    // Set the current state
    // Note: Currently you *must* set all of these
    virtual void SetColor(trpgColor &);
    virtual void SetTexCoord(trpg2dPoint &);
    virtual void AddTexCoord(trpg2dPoint &); // for multiple textures
    virtual void SetNormal(trpg3dPoint &);
    virtual void SetMaterial(int32);
    virtual void AddMaterial(int32); // for multiple textures
    // Pull the state info together and add a vertex
    virtual void AddVertex(trpg3dPoint &);

    // Dump whatever we're doing and move on
    virtual void FlushGeom(void);

    // Get the Min and Max Z values
    virtual void GetZMinMax(double &min,double &max);

    // Get statistics for whatever we built
    trpgwGeomStats *GetStats(void) { return &stats; }
protected:
    int mode;
    int dataType;
    trpgWriteBuffer *buf;

    /* Builds strips and fans from the triangle array.
        We (TERREX) are assuming a certain ordering in our vertex array
         because we do this optimization elsewhere.  This won't work well
         for anyone else.  What you will need to do if you want good
         performance is to implement a more generic form of this method.
         All you should have to do is override Optimize().  You've
         got the triangle arrays and a guarantee that the triangles
         have the same material.  All you really need is a decent fan/strip
         algorithm.
         */
    virtual void Optimize(void);

    // Reset Triangle arrays
    virtual void ResetTri(void);

    // Collections of geometry
    trpgGeometry strips,fans,bags;

    // Temporary data arrays for triangles/quads
    std::vector<int32> matTri;
    std::vector<trpg2dPoint> tex;
    std::vector<trpg3dPoint> norm,vert;
    // Data arrays for a polygon
    std::vector<int32> matPoly;
    std::vector<trpg2dPoint> polyTex;
    std::vector<trpg3dPoint> polyNorm,polyVert;
    // Single points
    std::vector<trpg2dPoint> tmpTex;
    trpg3dPoint tmpNorm;
    trpgColor tmpCol;

    // Geometry status built up as we go
    trpgwGeomStats stats;

    // Keeps track of min and max z values
    double zmin,zmax;
};

/* Image Write Helper.
    Used to manage textures being added to a TerraPage archive.
    It can write Local and Tile Local textures and also manages
    the names of External textures (but you have to write those yourself).
 */
TX_EXDECL class TX_CLDECL trpgwImageHelper {
public:
    trpgwImageHelper() {;};
    trpgwImageHelper(trpgEndian ness,char *dir,trpgTexTable &,bool separateGeoTypical);
    // construction is really here
    virtual void Init(trpgEndian ness,char *dir,trpgTexTable &,bool separateGeoTypical);

    virtual ~trpgwImageHelper(void);

    // Adds an entry to the texture table for an external texture
    virtual bool AddExternal(char *name,int &texID,bool lookForExisting=true);

    /* Adds an entry to the texture table for a local texture and
        writes the data for that texture out to one of our texture
        archive files.
     */
    virtual bool AddLocal(char *name,trpgTexture::ImageType type,int sizeX,int sizeY,bool isMipmap,char *data,int &texID,bool deferWrite);

    /* Replaces texture table information for a local texture and
        writes the data for that texture out to one of our texture
        archive files.
        Up to you to ensure data is appropriate for the texture.
     */
    virtual bool ReplaceLocal(char *data,int &texID);

    /* Write a Tile Local texture out to one of our texture archive files.
        Also creates a texture template, if necessary.
        Caller is responsible for creating the Tile Local material and
        placing it in the appropriate tile.
     */
    virtual bool AddTileLocal(char *name,trpgTexture::ImageType type,int sizeX,int sizeY,bool isMipmap,char *data, int &texID,trpgwAppAddress &addr);

    /* Sets the maximum advised length for a texture archive file.
        Once the length is exceeded, the image write helper will move
        on to the next tex file.
     */
    virtual void SetMaxTexFileLength(int len);

    /* Texture archive files are managed by this class and will
        be created as needed.  This method will increment to
        the next texture file.
        Note: This may create more files than we really need.
     */
    virtual trpgwAppFile * IncrementTextureFile(bool geotyp);

    /* Close the current texture file and go on to one with the
        given base name.  This is used for regenerate.
     */
    virtual bool DesignateTextureFile(int);

    // Flush current texture output files
    virtual bool Flush(void);

    // Get a new appendable file
    virtual trpgwAppFile* GetNewWAppFile(trpgEndian inNess,const char *fileName,bool reuse=false);

    // Write the given texture data into one our local archives
    bool WriteToArchive(const trpgTexture &tex,char *data,trpgwAppAddress &addr,bool geotyp=false);
    // Merge block textable into a master.

protected:


    trpgEndian ness;
    char dir[1024];
    trpgTexTable *texTable;
    std::vector<int> texFileIDs;
    trpgwAppFile *texFile;
    std::vector<int> geotypFileIDs;
    trpgwAppFile *geotypFile;
    bool separateGeoTypical;
    int maxTexFileLen;
};

/* Paging Archive
    This is a writeable paging archive.
    It organizes where things get written and how.
    {group:Archive Writing}
    */
TX_EXDECL class TX_CLDECL trpgwArchive : public trpgCheckable {
public:
    // Tiles can be stored as individual files (External) or grouped together (Local)
    enum TileMode {TileLocal,TileExternal,TileExternalSaved};

    // real constructor work done in Init(...) now for quasi-virtual ctor action.
    // Add data to an existing archive
    trpgwArchive(char *baseDir,char *name,trpg2dPoint &ll,trpg2dPoint &ur, int majorVer=TRPG_VERSION_MAJOR, int minorVer=TRPG_VERSION_MINOR);
    virtual void Init(char *baseDir,char *name,trpg2dPoint &ll,trpg2dPoint &ur, int majorVer=TRPG_VERSION_MAJOR, int minorVer=TRPG_VERSION_MINOR);
    // Start an archive from scratch.
    trpgwArchive(trpgEndian ness=LittleEndian,TileMode tileMode=TileLocal,int majorVer=TRPG_VERSION_MAJOR, int minorVer=TRPG_VERSION_MINOR);
    virtual void Init(trpgEndian ness=LittleEndian,TileMode tileMode=TileLocal,int majorVer=TRPG_VERSION_MAJOR, int minorVer=TRPG_VERSION_MINOR);
    // dummy constructor, does nothing so subclasses can have more control
    trpgwArchive(int ) {;};
    virtual ~trpgwArchive(void);

    // Set the maximum length for a tile file (if using them)
    // This is only a suggestion for when to stop appending
    virtual void SetMaxTileFileLength(int len);

    // Set functions.  Have to fill all these out before writing
    virtual bool SetHeader(const trpgHeader &);
    virtual bool SetMaterialTable(const trpgMatTable &);
    virtual bool SetTextureTable(const trpgTexTable &);
    virtual bool SetModelTable(const trpgModelTable &);
    virtual bool SetLightTable(const trpgLightTable &);
    virtual bool SetRangeTable(const trpgRangeTable &);
    virtual bool SetLabelPropertyTable(const trpgLabelPropertyTable &);
    virtual bool SetSupportStyleTable(const trpgSupportStyleTable &);
    virtual bool SetTextStyleTable(const trpgTextStyleTable &);

    // Get functions.  If we're doing a regenerate we need to get at these
    virtual trpgHeader *GetHeader();
    virtual trpgMatTable *GetMatTable();
    virtual trpgTexTable *GetTextureTable();
    virtual trpgModelTable *GetModelTable();
    virtual trpgLightTable *GetLightTable();
    virtual trpgRangeTable *GetRangeTable();
    virtual trpgLabelPropertyTable *GetLabelPropertyTable();
    virtual trpgTextStyleTable *GetTextStyleTable();
    virtual trpgSupportStyleTable *GetSupportStyleTable();



    virtual bool IncrementTileFile(void);
    virtual bool DesignateTileFile(int);

    // Write functions.
    // For now, the header is written last.

    virtual bool OpenFile(const char *,const char *);
    virtual void CloseFile(void);
    virtual bool WriteHeader(void);
    virtual bool CheckpointHeader(void);
    virtual bool WriteTile(unsigned int,unsigned int,unsigned int,float zmin,float zmax,
        const trpgMemWriteBuffer *,const trpgMemWriteBuffer *, int32& fileId, int32& fileOffset);
//    virtual bool WriteModel(unsigned int,trpgMemWriteBuffer &);

    bool isValid(void) const;
    const char* getErrMess() const;
    char* getDir(void){return dir;};
    virtual trpgwImageHelper* GetNewWImageHelper(trpgEndian ness,char *dir,trpgTexTable &);
    virtual trpgwAppFile* GetNewWAppFile(trpgEndian inNess,const char *fileName,bool reuse=false);
    virtual trpgr_Archive* GetArchiveReader() {return new trpgr_Archive();};
    virtual int32 WriteHeaderData(const char *dataPtr,int32 length,FILE *filehandle);
    virtual int32 GetMagicNumber() {return TRPG_MAGIC;};
protected:
    // Set if we're adding to an existing archive
    bool isRegenerate;

    // Used to keep track of which tiles are in which file
    class TileFileEntry {
    public:
        int x,y,lod;    // Identifying info for tile
        float zmin,zmax;
        int32 offset;  // Offset into file
    };
    class TileFile {
    public:
        int id;
        std::vector<TileFileEntry> tiles;
    };

    trpgEndian ness,cpuNess;
    int majorVersion, minorVersion;
    // Fed in from the outside
    char dir[1024];       // Directory where we're doing all this

    // These are passed in

    trpgHeader header;
    trpgMatTable matTable;
    trpgTexTable texTable;
    trpgModelTable modelTable;
    trpgLightTable lightTable;
    trpgRangeTable rangeTable;
    trpgTextStyleTable textStyleTable;
    trpgSupportStyleTable supportStyleTable;
    trpgLabelPropertyTable labelPropertyTable;


    trpgTileTable tileTable;

    int numLod;
    TileMode tileMode;

    trpgwAppFile *tileFile;
    int tileFileCount;

    std::vector<TileFile> tileFiles;

    std::vector<TileFileEntry> externalTiles;

    int maxTileFileLen;

    // This offset is used when we're adding to an existing archive
    trpg2iPoint addOffset;

    FILE *fp;

    bool firstHeaderWrite;

    mutable std::string errMess;
};

#endif
