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

#ifndef _txpage_read_h_
// {secret}
#define _txpage_read_h_

/* txpage_read.h
    Classes used to represent read objects for paging files.
    */

#include "trpage_sys.h"

#include "trpage_geom.h"

/* Callback base class
    Called when a given token is found.
    {group:Archive Reading}
    */
TX_EXDECL class TX_CLDECL trpgr_Callback {
public:
    virtual ~trpgr_Callback(void) { };
    virtual void *Parse(trpgToken,trpgReadBuffer &) { return (void *)1; };
};

/* Paging Token
    Stores callback info associated with a given token.
    {group:Archive Reading}
    */
TX_EXDECL class TX_CLDECL trpgr_Token {
public:
    trpgr_Token(void);
    trpgr_Token(int,trpgr_Callback *,bool destroy=true);
    ~trpgr_Token(void);
    void init(int,trpgr_Callback *,bool destroy=true);
    int Token;            // Constant token value
    trpgr_Callback *cb; // Callback when we hit this token
    bool destroy;       // Should we call delete on the callback or not
    void Destruct(void);    // Not quite like delete
};

/* Parse class for paging data structures.
    This executes callbacks
    {group:Archive Reading}
    */
TX_EXDECL class TX_CLDECL trpgr_Parser {
public:
    trpgr_Parser(void);
    virtual ~trpgr_Parser(void);
    bool isValid(void) const;

    // Add and remove token callbacks
    virtual void AddCallback(trpgToken,trpgr_Callback *,bool destroy = true);
    virtual void AddCallback(trpgToken,trpgReadWriteable *);
    virtual void RemoveCallback(trpgToken);
    virtual void SetDefaultCallback(trpgr_Callback *,bool destroy = true);
    // Parse a read buffer
    virtual bool Parse(trpgReadBuffer &);
    virtual bool TokenIsValid(trpgToken);  // Check token validity
protected:
    void *lastObject;
private:
    // Note: Just how slow is a map<> anyway?
    //         This usage is self-contained and could be replaced with an array
#if defined(_WIN32) && !defined(__GNUC__)
        typedef map<trpgToken,trpgr_Token> tok_map;
#else
        typedef map<trpgToken,trpgr_Token,less<trpgToken> > tok_map;
#endif
    tok_map tokenMap;
    trpgr_Token defCb;     // Call this when no others are called
};

/* Paging Archive (read version)
    This just reads the first bits of the file (and the header)
    and lets you parse from there.
    {group:Archive Reading}
     */
TX_EXDECL class TX_CLDECL trpgr_Archive : public trpgCheckable {
public:
    trpgr_Archive(void);
    virtual ~trpgr_Archive(void);

    virtual void SetDirectory(const char *);
    virtual bool OpenFile(const char *);    // Open File
    virtual void CloseFile(void);
    virtual bool ReadHeader(void);        // Read header (materials, tile table. etc..)
    virtual bool ReadTile(uint32 x, uint32 y, uint32 lod,trpgMemReadBuffer &);

    // Get access to header info
    virtual const trpgHeader *GetHeader(void) const;
    virtual const trpgMatTable *GetMaterialTable(void) const;
    virtual const trpgTexTable *GetTexTable(void) const;
    virtual const trpgModelTable *GetModelTable(void) const;
    virtual const trpgTileTable *GetTileTable(void) const;

    // Utility routine to calculate the MBR of a given point
    virtual bool trpgGetTileMBR(uint32 x,uint32 y,uint32 lod,
                                trpg2dPoint &ll,trpg2dPoint &ur) const;

    trpgEndian GetEndian() const;
    char* getDir(){return dir;};
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
TX_EXDECL class TX_CLDECL trpgSceneParser : public trpgr_Parser {
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
    vector<void *> parents;
};

#endif
