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

#ifndef _trpage_io_h_
#define _trpage_io_h_

/* trpage_io.h
    Token definitions and basic classes.
  */

#include <trpage_sys.h>

#ifndef TeAttrH
/* was unsigned long, but it is used as if it was signed in the constructor
 * and code that set the handle are using a signed value.
 * 01-10-2006 David Fries
 */
typedef long TeAttrHdl;
#endif

// Macros we may need
#ifndef MIN
// {secret}
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef MAX
// {secret}
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

// File header Magic Number
#define TRPG_MAGIC            9480372

// Current TerraPage major version
#define TRPG_VERSION_MAJOR 2
// Current TerraPage minor version
#define TRPG_VERSION_MINOR 2

// defined values for the version that doesn't need
// a merge
#define TRPG_NOMERGE_VERSION_MAJOR 2
#define TRPG_NOMERGE_VERSION_MINOR 2

// Non-existent token
// {secret}
#define TRPG_NOTOKEN        0

// 16 bit token definition value.  These are values such as TRPGTEXTABLE or TRPG_ATTACH, etc...
typedef short trpgToken;

// Tokens for paging data structures
// Update the MINTOKEN and MAXTOKEN when you add tokens
// {secret}
#define TRPG_MINTOKEN        100
// {secret}
#define TRPG_PUSH            100
// {secret}
#define TRPG_POP            101

// {secret}
#define TRPGHEADER            200
// {secret}
#define TRPGHEAD_LODINFO    201

// {secret}
#define TRPGMATTABLE        300
// Added 11/14/98 - New material table
// {secret}
#define TRPGMATTABLE2        301
// Added 11/14/98
// {secret}
#define TRPGSHORTMATTABLE    302

// {secret}
#define TRPGMATERIAL        400
// {secret}
#define TRPGMAT_BASIC        401
// {secret}
#define TRPGMAT_SHADE        402
// {secret}
// {secret}
#define TRPGMAT_SIZES        403
// {secret}
#define TRPGMAT_CULL        404
// {secret}
#define TRPGMAT_ALPHA        405
// {secret}
#define TRPGMAT_NORMAL        406
// {secret}
#define TRPGMAT_TEXTURE        407
// {secret}
#define TRPGMAT_BUMP        408
// {secret}
#define TRPGMAT_ATTR        409
// {secret}
#define TRPGMAT_HANDLE        410

// {secret}
#define TRPGMAT_TEXENV        500
// {secret}
#define TRPGMAT_TXENV_MODE    501
// {secret}
#define TRPGMAT_TXENV_FILTER 502
// {secret}
#define TRPGMAT_TXENV_WRAP    503
// {secret}
#define TRPGMAT_TXENV_BORDER 504

// {secret}
#define TRPGTEXTABLE        600
// {secret}
#define TRPGTEXTABLE2        601
// {secret}
#define TRPGTEXTURE            650

// {secret}
#define TRPGMODELREF        700
// {secret}
#define TRPGMODELREF2        701
// {secret}
#define TRPGMODELTABLE        800

// {secret}
#define TRPGTILETABLE        900
// {secret}
#define TRPG_TILE_ENTRY        901
// {secret}
#define TRPGTILETABLE2        902

// {secret}
#define TRPGTILEHEADER        1000
// {secret}
#define TRPG_TILE_MATLIST    1001
// {secret}
#define TRPG_TILE_MODELLIST    1002
// {secret}
#define TRPG_TILE_DATE        1003
// {secret}
#define TRPGLOCALMATERIAL    1004
// {secret}
#define TRPG_TILE_LOCMATLIST    1005


// Lights support added by Nick
// {secret}
#define TRPGLIGHTTABLE                1100
// {secret}
#define TRPGLIGHTATTR                1150
// {secret}
#define TRPGLIGHTATTR_BASIC            1151
// {secret}
#define TRPGLIGHTATTR_RASCAL        1152
// {secret}
#define TRPGLIGHTATTR_CALLIGRAPHIC    1153
// {secret}
#define TRPGLIGHTATTR_PERFORMER        1154
// {secret}
#define TRPGLIGHTATTR_ANIMATION        1155
// {secret}
#define TRPGLIGHTATTR_COMMENT        1156
// {secret}
#define TRPGLIGHTATTR_HANDLE        1157
// {secret}
#define TRPGLIGHT                    1160
// {secret}
#define TRPG_LIGHT                    1160

// {secret}
#define TRPGRANGETABLE                1200
// {secret}
#define TRPG_RANGE                    1201

// Label and style support added 12-02
// {secret}
#define TRPG_TEXT_STYLE_TABLE        1300
// {secret}
#define TRPG_TEXT_STYLE                1301
// {secret}
#define TRPG_TEXT_STYLE_BASIC        1302

// {secret}
#define TRPG_SUPPORT_STYLE_TABLE    1310
// {secret}
#define TRPG_SUPPORT_STYLE          1311
// {secret}
#define TRPG_SUPPORT_STYLE_BASIC    1312

// {secret}
#define TRPG_LABEL_PROPERTY_TABLE   1320
// {secret}
#define TRPG_LABEL_PROPERTY         1321
// {secret}
#define TRPG_LABEL_PROPERTY_BASIC   1322


// {secret}
#define TRPG_LABEL                    1330


// {secret}
#define TRPG_GROUP            2001
// {secret}
#define TRPG_BILLBOARD        2002
// {secret}
#define TRPG_LOD            2003
// {secret}
#define TRPG_TRANSFORM        2004
// {secret}
#define TRPG_MODELREF        2005
// {secret}
#define TRPG_LAYER            2006

// {secret}
#define TRPG_GEOMETRY        3000
// {secret}
#define TRPG_GEOM_PRIM        3001
// {secret}
#define TRPG_GEOM_MATERIAL    3002
// {secret}
#define TRPG_GEOM_VERT32    3003
// {secret}
#define TRPG_GEOM_VERT64    3004
// {secret}
#define TRPG_GEOM_NORM32    3005
// {secret}
#define TRPG_GEOM_NORM64    3006
// {secret}
#define TRPG_GEOM_COLOR        3007
// {secret}
#define TRPG_GEOM_TEX32        3008
// {secret}
#define TRPG_GEOM_TEX64        3009
// {secret}
#define TRPG_GEOM_EFLAG        3010

// {secret}
#define TRPG_ATTACH            4000

// {secret}
#define TRPG_CHILDREF        5000

// {secret}
#define TRPG_MAXTOKEN        5000

// Basic data types

/* 64 bit disk reference value. */
typedef trpgllong trpgDiskRef;
// {secret}
typedef int trpgMatRef;

/* Double precision 2 dimensional point. */
TX_EXDECL class TX_CLDECL trpg2dPoint {
public:
    double x, y;
    trpg2dPoint (void) { };
    trpg2dPoint (double in_x,double in_y) { x = in_x; y = in_y; };
    bool operator==(const trpg2dPoint& pt ) const {
        if ( x != pt.x ) return false;
        if ( y != pt.y ) return false;
        return true;
    };
    bool operator!=(const trpg2dPoint& pt ) const { return !operator==(pt); };
};
/* Integer 2 dimensional point.  This is used primarily as a 2D index value. */
TX_EXDECL class TX_CLDECL trpg2iPoint {
public:
    int x,y;
    trpg2iPoint (void) { };
    trpg2iPoint (int in_x,int in_y) {x = in_x; y = in_y;};
};
/* Double precision 3 dimensional point. */
TX_EXDECL class TX_CLDECL trpg3dPoint {
public:
    double x,y,z;
    trpg3dPoint(void) { };
    trpg3dPoint(double in_x,double in_y,double in_z) {x = in_x; y = in_y; z = in_z;}
    bool operator==(const trpg3dPoint& pt ) const {
        if ( x != pt.x ) return false;
        if ( y != pt.y ) return false;
        if ( z != pt.z ) return false;
        return true;
    };
    bool operator!=(const trpg3dPoint& pt ) const { return !operator==(pt); };
};
/* Simple red, green, blue color definition */
TX_EXDECL class TX_CLDECL trpgColor {
public:
    trpgColor(float64 r,float64 g,float64 b) {red = r; green = g; blue = b;}
    trpgColor(void) { };
    bool operator==(const trpgColor& color) {
        if ( color.red != red )        return false;
        if ( color.green != green ) return false;
        if ( color.blue != blue )    return false;
        return true;
    };
    bool operator!=(const trpgColor& color) { return !operator==(color); };
    float64 red,green,blue;
};

// Used to specify machine endianess
typedef enum {LittleEndian,BigEndian} trpgEndian;

/*     This is a base class for an abstract buffer type.
    It contains the virtual methods for writing
    data to an abstract device.  The device implementation is up
    to the subclass.  trpgReadBuffer performs the similar function
    for reading.
    {group:Low Level I/O}
     */
TX_EXDECL class TX_CLDECL trpgWriteBuffer {
public:
    virtual ~trpgWriteBuffer(void) { };
    /* The add functions must be filled in by the child class
        They add data of the appropriate type to the current buffer. */
    virtual void Add(int32) = 0;
    virtual void Add(int64) = 0;
    virtual void Add(const char *) = 0;
    virtual void Add(float32) = 0;
    virtual void Add(float64) = 0;
//#if (bool != int32)
    virtual void Add(bool) = 0;
//#endif
    virtual void Add(uint8) = 0;
#if (trpgDiskRef != int64)
    virtual void Add(trpgDiskRef) = 0;
#endif
    virtual void Add(trpgToken) = 0;
    /* Child class method.  Returns the buffer to an original state. */
    virtual void Reset(void) = 0;
    // See trpgMemWriteBuffer for details
    virtual void Begin(trpgToken) = 0;
    // See trpgMemWriteBuffer for details
    virtual void End(void) = 0;
    // See trpgMemWriteBuffer for details
    virtual void Push(void) = 0;
    // See trpgMemWriteBuffer for details
    virtual void Pop(void) = 0;

    // Some add functions are helpers for composite values that call the basic add functions
    virtual void Add(const trpg2iPoint &);
    virtual void Add(const trpg2dPoint &);
    virtual void Add(const trpg3dPoint &);
    virtual void Add(const trpgColor &);
    virtual void Add(const std::string &);

    /* Endianness is something the child class buffer type must set and use.
        This function returns the endiannes of the current buffer. */
    virtual trpgEndian GetEndian(void) { return ness; }

protected:
    // Target endianness of the buffer.  This should be set by the subclass on creation.
    trpgEndian ness;
    // Endianness of the machine we're running on.
    trpgEndian cpuNess;
};

/* The Memory Write Buffer is an implementation of the Write Buffer that
    uses chunks of memory.  It contains implementations of the all the virtual
    methods straight to memory.  This is used primarily in writing archives and
    tiles.
    {group:Low Level I/O}
    */
TX_EXDECL class TX_CLDECL trpgMemWriteBuffer : public trpgWriteBuffer {
public:
    /* The constructor takes an endianness for this buffer.
        Data will automatically be converted to that as it goes in. */
    trpgMemWriteBuffer(trpgEndian);
    virtual ~trpgMemWriteBuffer(void);
    // Return the current length of buffer
    virtual int length(void) const;
    // Return the raw data (if you want to write to disk, for example)
    virtual const char *getData(void) const;
    // Allocate the given amount of space for the buffer
    virtual void setLength(unsigned int);

    // Add a 32 bit integer to the buffer
    virtual void Add(int32);
    // Add a 64 bit integer to the buffer
    virtual void Add(int64);
    /* Add an arbitrary length string to the buffer.
        This writes both the length and the string itself.
        */
    virtual void Add(const char *);
    // Same as const char * version
    virtual void Add(std::string &);
    // Add a 32 bit float to the buffer
    virtual void Add(float32);
    // Add a 64 bit float to the buffer
    virtual void Add(float64);
//#if (bool != int32)
    // Add a boolean value to the buffer.  It will become at least one byte.
    virtual void Add(bool);
//#endif
    // Add an unsigned character to the buffer
    virtual void Add(uint8);
#if (trpgDiskRef != int64)
    // Add a 64 bit disk reference to the buffer
    virtual void Add(trpgDiskRef);
#endif
    // Add a token (16 bit) to the buffer
    virtual void Add(trpgToken);
    // Reset this buffer.  This will set the current length to zero, but will not deallocate memory
    virtual void Reset(void);
    /* Start defining an tokenized object.  The token is put into the buffer stream
       and the position of a size value following it is kept.  When End() is called
       the buffer will rewind to that value and save the size.  This method ensures
       that token data can be skipped if necessary. */
    virtual void Begin(trpgToken);
    /* This method is called at the end of a tokenized object.  See Begin for details. */
    virtual void End(void);
    /* Adds the TRPG_PUSH token to the current buffer.  This is done by objects
       that have children as they're being written.  See Pop as well. */
    virtual void Push(void);
    /* Adds the TRPG_POP token to the current buffer.  This is done by objects
       that have defined children.  See Push. */
    virtual void Pop(void);
   /* Take out the pop from the end of the buffer, if there is one */
   /* Will return true if a pop was actually taken out */
   virtual bool UnPop();
  /* Take out the push from the end of the buffer, if there is one */
   /* Will return true if a push was actually taken out */
   virtual bool UnPush();

protected:
    virtual void append(unsigned int,const char *);
    virtual void set(unsigned int pos,unsigned int len,const char *);
    int curLen;
    int totLen;
    char *data;
    std::vector<int> lengths;
};

/* This is a virtual base class for reading data from a device.
    The device implementation is left as an excercise to the sub class.
    This class contains methods for getting data that must be filled in
    as well as helper methods that call those.
    {group:Low Level I/O}
      */
TX_EXDECL class TX_CLDECL trpgReadBuffer {
public:
    virtual ~trpgReadBuffer(void) { };
    /* The Get methods are utility routines that all call the GetData method.
        Only that method need be filled in by a subclass. */
    virtual bool Get(int32 &);
    virtual bool Get(int64 &);
    virtual bool Get(char *,int);
    virtual bool Get(std::string &);
    virtual bool Get(float32 &);
    virtual bool Get(float64 &);
//#if (bool != int32)
    virtual bool Get(bool &);
//#endif
    virtual bool Get(uint8 &);
#if (trpgDiskRef != int64)
    virtual bool Get(trpgDiskRef &);
#endif
    virtual bool Get(trpgToken &);

    /* These methods return references to arrays of data of the given types.
        These are all utility routines and depend upon GetDataRef. */
    virtual bool GetArray(int,float32 **);
    virtual bool GetArray(int,float64 **);
    virtual bool GetArray(int,int32 **);
    virtual bool GetArray(int,trpgColor **);
    virtual bool GetArray(int,char **);

    virtual bool Get(trpg2iPoint &);
    virtual bool Get(trpg2dPoint &);
    virtual bool Get(trpg3dPoint &);
    virtual bool Get(trpgColor &);
    virtual bool GetToken(trpgToken &,int32 &);

    /* Force the buffer to only allow the next N bytes to be read.
        The limits are stack based.  That is, this limit is the current one
        until it's popped off the stack.  Then it reverts to the previous one.
        Any bytes read in the mean time count against all limits. */
    virtual void PushLimit(int);
    /* Revert to the limit before this one.  Typically this would happen when
       a tokenized object has been read. */
    virtual void PopLimit(void);
    /* Skip to the end of the current limit.  This is done by a parser when
        reading a tokenized object from the buffer to make sure that the next
        object can be safely read even if the current one wasn't. */
    virtual bool SkipToLimit(void);

    // Buffer is empty
    virtual bool isEmpty(void) = 0;

    // MD: making this public to unravel trpgModel read problem.
    /* A utility function for subclasses to use to see if they would exceed an
        outside imposed limit by reading the given number of bytes. */
    virtual bool TestLimit(int);

protected:
    trpgEndian ness;                     // Endianness of the source we're reading
    trpgEndian cpuNess;                     // Endiannees of the CPU
    /* Virtual raw data retrieval function that must be implemented by a subclass.
        It must return a given number of bytes in the array passed in. */
    virtual bool GetData(char *,int)=0;
    /* Virtual raw data reference retrieval function.  The difference between
        this method and GetData is that this is supposed to return a pointer
        to a given amount of bytes.  This assumes some sort of memory caching
        mechanism in the subclass. */
    virtual bool GetDataRef(char **,int)=0;
    /* This virtual method must be filled in by the subclass so that SkipToLimit
        will work correctly. */
    virtual bool Skip(int) = 0;
    /* Utility function that must be called after a successfull read to update
        the outside imposed read limits. */
    virtual void UpdateLimits(int);
    std::vector<int> limits;
};

/* This class implements a read buffer that uses a chunk of memory.
    Typically, raw data will be dumped into this class, then it will be
    passed to a parser for object based reading.
    {group:Low Level I/O}
    */
TX_EXDECL class TX_CLDECL trpgMemReadBuffer : public trpgReadBuffer {
public:
    // Memory read buffers must be initialized with an endianness
    trpgMemReadBuffer(trpgEndian);
    ~trpgMemReadBuffer(void);
    // Return true if we're out of data
    bool isEmpty(void);
    // Sets the size of this read buffer.
    void SetLength(int);
    // Gets the size of the buffer.
    int GetLength(void) { return len; }
    /* Return a pointer to the raw data cache for this object.  Data will
        be dumped straight into here (from disk, for example) and then parsed
        by something that takes a trpgReadBuffer as input. */
    char *GetDataPtr(void);
protected:
    bool GetData(char *,int);            // Retrieve the given amount of data
    bool GetDataRef(char **,int);        // Retrieve a pointer to the given array
    bool Skip(int);                        // Skip over the given amount
    int len;                            // Data Length
    int totLen;                            // Total allocated length
    int pos;                            // Current position
    char *data;
};

/* A Checkable is purely a base class used by other classes that
    need validity checks associated with them.  By default, the
    checkable will return false for isValid unless the valid flag is set.
    */
TX_EXDECL class TX_CLDECL trpgCheckable {
public:
    trpgCheckable(void);
    virtual ~trpgCheckable(void);
    // Returns the state of the valid flag, or can be overriden by a subclass to do a more complex check.
    bool isValid(void) const;

    virtual TeAttrHdl GetHandle() const {
        return handle;
    }
    virtual void SetHandle(TeAttrHdl hdl) {
        writeHandle = true;
        handle = hdl;
    }

protected:
    /* Set this flag to true if your checkable structure doesn't have a complex
        check it needs to do. */
    bool valid;
    TeAttrHdl handle;
    bool writeHandle;
};

class trpgPrintBuffer;
/* The Read/Writeable is a class that knows how to read itself from a trpgReadBuffer
    and write itself to a trpgWriteBuffer.  This includes all the node and header
    data in TerraPage.  These classes are intended as marshalling points for reading
    and writing data, not as data containers in and of themselves.  If you find
    yourself keeping a lot of classes derived from trpgReadWriteable around, you're
    probably misusing them.

    The classes derived from this one will have a lot of methods that begin with
    "Set", "Get", and "Add".  These will almost always return a bool value.  This
    is used to indicate whether the given call succeeded.  In the case of "Set" and "Add" calls
    this should always work if it possibly can.  An out of range index might make it
    fail, for example.  "Get" calls will always fail if the object you're getting from
    is not valid.  Be sure to do an isValid check as soon as you've read or filled out
    one of these objects.
    {group:Read/Write Classes}
    */
TX_EXDECL class TX_CLDECL trpgReadWriteable : public trpgCheckable {
public:

    trpgReadWriteable() { errMess[0] = '\0';}

    /* The Write method is a virtual that must be filled in by the subclass.
        It takes a trpgWriteBuffer and should return true on success. */
    virtual bool        Write(trpgWriteBuffer &) = 0;
    /* The Read method should be overriden by a subclass.  It should read
        the contents of the given trpgReadBuffer up to the current limit
        into itself.  It must return true on success. */
    virtual bool        Read(trpgReadBuffer &) { return false;};
    /* Every read/writeable must be able to reset itself to a pristine state
        so that, for example, multiple objects of the same type can be read into
        it, one after the other.  */
    virtual void        Reset(void) = 0;
    /* The print method is optional.  If it's not there, it won't do anything.
     */
    virtual bool        Print(trpgPrintBuffer &) const { return true; }

    const char *getErrMess() const {if(errMess[0] == '\0') return 0;else return &errMess[0];}

protected:

    mutable char errMess[512];
};

/* Pointer into a trpgwAppFile.  The full name of the file
    is based on the context (e.g. texture vs. tile)
    {group:Archive Writing}
 */
TX_EXDECL class TX_CLDECL trpgwAppAddress {
public:
    trpgwAppAddress() {file = -1; offset = -1; row = -1; col = -1;};
    // Which file
    int32 file;
    // Offset within the file
    // Note: This is not a 64 bit value
    int32 offset;
    // Row and col are used for TerraPage 2.3 archives, so we
    // can know which block to get the appropriate file from
    int32 row;
    int32 col;

};

/* Archive File.
    This class represents an appendable file archive used for
    consolidating tiles and textures.
    {group:Archive Writing}
 */
TX_EXDECL class TX_CLDECL trpgwAppFile {
public:
    trpgwAppFile() {valid=false;};
    trpgwAppFile(trpgEndian,const char *,bool reuse=false);
    virtual ~trpgwAppFile(void);
    virtual bool Append(const trpgMemWriteBuffer *,const trpgMemWriteBuffer *);
    virtual bool Append(const char *,int size);
    virtual int64 Pos(void) const;
    virtual int GetLengthWritten();
    virtual bool Flush(void);
    virtual void Init(trpgEndian,const char *,bool reuse=false);

    bool isValid(void) const;
protected:
    bool valid;
    trpgEndian ness,cpuNess;
    FILE *fp;
    int lengthSoFar;
};

/* Archive File - Read version.
    This class represents an appendable file archive from the
    read perspective.  This is the same type of file written by
    trpgwAppFile.
 */
TX_EXDECL class TX_CLDECL trpgrAppFile {
public:
    trpgrAppFile() {valid=false;};
    trpgrAppFile(trpgEndian,const char *);
    // real construction is here
    virtual void Init(trpgEndian,const char *);
    virtual ~trpgrAppFile(void);
    virtual bool Read(trpgMemReadBuffer *,int32 offset);
    virtual bool Read(char *data,int32 baseOffset,int32 objOffset,int32 dataSize);

    bool isValid(void) const;
protected:
    bool valid;
    trpgEndian ness,cpuNess;
    FILE *fp;
};

/* Archive File Cache.
    This class keeps
 */
TX_EXDECL class TX_CLDECL trpgrAppFileCache {
public:
    trpgrAppFileCache(){;};
    trpgrAppFileCache(const char *prefix,const char *ext,int noFiles=32);
    // real construction is here
    virtual void Init(const char *prefix,const char *ext,int noFiles);
    virtual ~trpgrAppFileCache(void);
    virtual trpgrAppFile *GetFile(trpgEndian ness,int id,int col,int row);
    virtual trpgrAppFile *GetFile(trpgEndian ness,int id);
    virtual trpgrAppFile *GetNewRAppFile(trpgEndian ness, const char *fileName);
protected:
    // Prefix name and extension
    char baseName[1024],ext[20];

    class OpenFile {
    public:
        OpenFile(void);
        int id;  // ID of open file
        int row;
        int col;
        trpgrAppFile *afile;
        int lastUsed;  // When the file was last accessed
    };

    std::vector<OpenFile> files;
    int timeCount;   // Incremented for every access
};

#endif
