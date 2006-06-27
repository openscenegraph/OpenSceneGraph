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

#ifndef _txpage_read_h_
// {secret}
#define _txpage_read_h_

/* txpage_read.h
   Classes used to represent read objects for paging files.
*/

#include <trpage_sys.h>

#include <trpage_geom.h>

/* Callback base class
   Called when a given token is found.
   {group:Archive Reading}
*/
TX_EXDECL class TX_CLDECL trpgr_Callback
{
 public:
    virtual ~trpgr_Callback(void) { };
    virtual void *Parse(trpgToken,trpgReadBuffer &) { return (void *)1; };
};

/* Paging Token
   Stores callback info associated with a given token.
   {group:Archive Reading}
*/
TX_EXDECL class TX_CLDECL trpgr_Token
{
 public:
    trpgr_Token(void);
    trpgr_Token(int,trpgr_Callback *,bool destroy=true);
    ~trpgr_Token(void);
    void init(int,trpgr_Callback *,bool destroy=true);
    int Token;			// Constant token value
    trpgr_Callback *cb; // Callback when we hit this token
    bool destroy;       // Should we call delete on the callback or not
    void Destruct(void);    // Not quite like delete
};

/* Parse class for paging data structures.
   This executes callbacks
   {group:Archive Reading}
*/
TX_EXDECL class TX_CLDECL trpgr_Parser
{
 public:
    trpgr_Parser(void);
    virtual ~trpgr_Parser(void);
    bool isValid(void) const;

    // Add and remove token callbacks
    virtual void AddCallback(trpgToken,trpgr_Callback *,bool destroy = true);
    virtual void AddCallback(trpgToken,trpgReadWriteable *);
    virtual const trpgr_Callback *GetCallback(trpgToken tok) const;
    virtual trpgr_Callback *GetCallback(trpgToken tok);
    virtual void RemoveCallback(trpgToken);
    virtual void SetDefaultCallback(trpgr_Callback *,bool destroy = true);
    // Parse a read buffer
    virtual bool Parse(trpgReadBuffer &);
    virtual bool TokenIsValid(trpgToken);  // Check token validity
 protected:
    void *lastObject;
 private:
    // Note: Just how slow is a map<> anyway?
    //		 This usage is self-contained and could be replaced with an array
#if defined(_WIN32)
    typedef std::map<trpgToken,trpgr_Token> tok_map;
#else
    typedef std::map< trpgToken,trpgr_Token,std::less<trpgToken> > tok_map;
#endif
    tok_map tokenMap;
    trpgr_Token defCb;     // Call this when no others are called
};

/* Image Read Helper.
   Used to help read Local and Tile Local textures into
   memory (in OpenGL format).  You're on your own for External
   textures.
   If you want to add additional ways to read textures, feel free
   to subclass this object.
*/
class trpgwImageHelper;
TX_EXDECL class TX_CLDECL trpgrImageHelper
{
 public:
    trpgrImageHelper() {;};
    trpgrImageHelper(trpgEndian ness,char *dir,const trpgMatTable &,const trpgTexTable &,bool separateGeoTyp);
    // real construction is here
    virtual void Init(trpgEndian ness,char *dir,const trpgMatTable &,const trpgTexTable &,bool separateGeoTyp);
    virtual ~trpgrImageHelper(void);

    /* Fetch the bytes for the given texture.
       This is only valid for Local textures.
    */
    virtual bool GetLocalGL(const trpgTexture *,char *data,int32 dataSize);

    /* Fetch the bytes for the given mip level of a given texture.
       This is only valid for Local textures.
    */
    virtual bool GetMipLevelLocalGL(int miplevel, const trpgTexture *,char *data,int32 dataSize);

    /* Do the lookups to figure out the correct material
       and Template (or Local) texture for a given Local Material.
       You'll need this for sizes (among other things).
       This routine also calculates the total size, including mipmaps if they're there.
    */
    virtual bool GetImageInfoForLocalMat(const trpgLocalMaterial *locMat,
					 const trpgMaterial **retMat,const trpgTexture **retTex,
					 int &totSize);

    /* Same as above, but gets info for nth image associated with this local material
     */
    virtual bool GetNthImageInfoForLocalMat(const trpgLocalMaterial *locMat, int index,
					    const trpgMaterial **retMat,const trpgTexture **retTex,
					    int &totSize);

    /* Fetch the bytes for the given Local Material (and
       associated texture).  This is for Tile Local and
       Global textures.
       Data is a pre-allocated buffer for the data and
       dataSize is the size of that buffer.
    */
    virtual bool GetImageForLocalMat(const trpgLocalMaterial *locMat,char *data,int dataSize);

    /* Same as above, but gets nth image associated with this local material
     */
    virtual bool GetNthImageForLocalMat(const trpgLocalMaterial *locMat, int index, char *data,int dataSize);

    /* Same as the one above, just fetch single mip levels
     */
    virtual bool GetMipLevelForLocalMat(int miplevel, const trpgLocalMaterial *locMat,char *data,int dataSize);

    /* Get mip levels for one of multiple images
     */
    virtual bool GetNthImageMipLevelForLocalMat(int miplevel, const trpgLocalMaterial *locMat, int index, char *data,int dataSize);

    /* Determine the full path of the image in the given
       trpgTexture class.
       Only useful for External images.
    */
    virtual bool GetImagePath(const trpgTexture *,char *,int len);

    virtual trpgrAppFileCache* GetNewRAppFileCache(const char *fullBase,const char *ext);

    trpgrAppFileCache *GetGeoTypCache()
    {
	return geotypCache;
    }
    void SetTexTable(trpgTexTable *texTable)
    {
	this->texTable = texTable;
    }
 protected:
    char dir[1024];
    trpgEndian ness;
    const trpgMatTable *matTable;
    const trpgTexTable *texTable;

    trpgrAppFileCache *texCache;
    trpgrAppFileCache *geotypCache;
    bool separateGeoTyp;
};

/* Paging Archive (read version)
   This just reads the first bits of the file (and the header)
   and lets you parse from there.
   {group:Archive Reading}
*/
TX_EXDECL class TX_CLDECL trpgr_Archive : public trpgCheckable
{
 public:
    trpgr_Archive(void);
    virtual ~trpgr_Archive(void);

    virtual void SetDirectory(const char *);
    virtual bool OpenFile(const char *);	// Open File
    virtual void CloseFile(void);
    virtual bool ReadHeader(void);		// Read header (materials, tile table. etc..)
    //overload that lets you specify if you want to read all the blocks now,
    //or defer reading them for later.
    virtual bool ReadHeader(bool readAllBlocks); 
    bool ReadSubArchive(int row, int col, trpgEndian cpuNess);
    // In version 2.1, only tile at lod 0 are fetchable via the tile table
    virtual bool ReadTile(uint32 x, uint32 y, uint32 lod,trpgMemReadBuffer &);
    virtual bool ReadTile(const trpgwAppAddress& addr, trpgMemReadBuffer &buf);
    virtual bool ReadExternalTile(uint32 x,uint32 y,uint32 lod,trpgMemReadBuffer &buf);

    // Get access to header info
    virtual const trpgHeader *GetHeader(void) const;
    virtual const trpgMatTable *GetMaterialTable(void) const;
    virtual trpgTexTable *GetTexTable(void) ;
    virtual const trpgModelTable *GetModelTable(void) const;
    virtual const trpgTileTable *GetTileTable(void) const;
    virtual const trpgLightTable *GetLightTable(void) const;
    virtual const trpgRangeTable *GetRangeTable(void) const;
    virtual const trpgLabelPropertyTable *GetLabelPropertyTable() const;
    virtual const trpgTextStyleTable *GetTextStyleTable() const;
    virtual const trpgSupportStyleTable *GetSupportStyleTable() const;

    // Utility routine to calculate the MBR of a given tile
    virtual bool trpgGetTileMBR(uint32 x,uint32 y,uint32 lod,
				trpg3dPoint &ll,trpg3dPoint &ur) const;

    trpgEndian GetEndian(void) const;
    char* getDir(void){return dir;};
    virtual trpgrImageHelper* GetNewRImageHelper(trpgEndian ness,char *dir,const trpgMatTable &matTable,const trpgTexTable &texTable);
    virtual trpgrAppFileCache* GetNewRAppFileCache(const char *fullBase, const char *ext);
    virtual int32 GetHeaderData(char *dataPtr,int32 length,FILE *filehandle);
    virtual int32 GetMagicNumber() {return TRPG_MAGIC;};
 protected:
    bool headerRead;
    trpgEndian ness;
    FILE *fp;
    int fid;
    // Header info
    char dir[1024];
    trpgHeader header;
    trpgMatTable materialTable;
    trpgTexTable texTable;
    trpgModelTable modelTable;
    trpgTileTable tileTable;
    trpgLightTable lightTable;
    trpgRangeTable rangeTable;
    trpgTextStyleTable textStyleTable;
    trpgSupportStyleTable supportStyleTable;
    trpgLabelPropertyTable labelPropertyTable;

    trpgrAppFileCache *tileCache;
};

class trpgSceneHelperPush;
class trpgSceneHelperPop;
class trpgSceneHelperDefault;
/* Scene Parser
   This class assists in parsing a scene graph structure (tiles and models).
   To use it, do an archive ReadTile and pass the resulting Read Buffer to this
   parser.
   {group:Archive Reading}
*/
TX_EXDECL class TX_CLDECL trpgSceneParser : public trpgr_Parser
{
    friend class trpgSceneHelperPush;
    friend class trpgSceneHelperPop;
    friend class trpgSceneHelperDefault;
 public:
    trpgSceneParser(void);
    virtual ~trpgSceneParser(void);
 protected:
    // Start defining children for the given object
    virtual bool StartChildren(void *) { return true;};
    virtual bool EndChildren(void *) { return true;};

    // List of objects whose children we're working on
    std::vector<void *> parents;
};

#endif
