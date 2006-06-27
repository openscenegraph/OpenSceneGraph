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

#ifndef trpage_print_h_
#define trpage_print_h_

#include <trpage_read.h>
#include <trpage_managers.h>
#include <stack>

/* Print Buffer for TerraPage.  Subclasses of this object
	are used to print out to stdout or disk (or whatever).
	You won't create one of these directly, instead you'll create
	something which inherits from it.
 */
TX_EXDECL class TX_CLDECL trpgPrintBuffer
{
public:
    trpgPrintBuffer(void);
    virtual ~trpgPrintBuffer(void) { };

    // Check if print buffer is valid
    virtual bool isValid(void) { return true; }

    // The main print function.  Subclasses must fill this in.
    virtual bool prnLine(char *str=NULL)=0;

    // This increases the current indentation by the amount given (defaults to one)
    virtual void IncreaseIndent(int amount=1);
    // Decreases the current indentation by the amount given (defaults to one)
    virtual void DecreaseIndent(int amount=1);
 protected:
    void updateIndent(void);
    int curIndent;
    char indentStr[200];
};

/* File print buffer for TerraPage.  The file print buffer writes
   debugging output to a file.
*/
TX_EXDECL class TX_CLDECL trpgFilePrintBuffer : public trpgPrintBuffer {
 public:
    // This class can be constructed with either a FILE pointer or a file name
    trpgFilePrintBuffer(FILE *);
    trpgFilePrintBuffer(char *);
    ~trpgFilePrintBuffer(void);

    // Check if file print buffer is valid (i.e. if file was opened)
    bool isValid(void) { return valid; };

    // For a file printer buffer, this writes a string out to a file
    bool prnLine(char *str = NULL);
 protected:
    bool valid;
    bool isMine;
    FILE *fp;
};

/* The Print Graph Parser is a scene graph parser that
   prints out the scene graph as it goes.  It's simpler
   than the scene example in trpage_scene.cpp since it
   isn't trying to build up a working scene graph.
*/
TX_EXDECL class TX_CLDECL trpgPrintGraphParser : public trpgSceneParser
{
 public:
    trpgPrintGraphParser(trpgr_Archive *,trpgrImageHelper *,trpgPrintBuffer *);
    virtual ~trpgPrintGraphParser(void) { };

    // Clear all list and free associated pointer
    void Reset();

    // After parsing this will return the number of trpgChildRef node found.
    unsigned int GetNbChildrenRef() const;
    // This will return the trpgChildRef node pointer associated with the index.
    // Will return 0 if index is out of bound
    const trpgChildRef* GetChildRef(unsigned int idx) const;

    /* The read helper class is the callback for all the various
       token (node) types.  Normally we would use a number of
       these, probably one per token.  However, since we're just
       printing we can use a switch statement instead.
    */
    class ReadHelper : public trpgr_Callback
    {
    public:
	// typedef std::vector<const trpgChildRef> ChildRefList;
	// The const in the template parameter was removed because it causes GCC to
	// freak out.  I am of the opinion that const doesn't make sense in a template
	// parameter for std::vector anyway... const prevents you from changing the
	// value, so what exactly is the point?  How does one add entries to the vector
	// without giving them a value?  -ADS
	typedef std::vector<trpgChildRef> ChildRefList;

	ReadHelper(trpgPrintGraphParser *inPG,trpgPrintBuffer *inBuf): pBuf(inBuf), parse(inPG) {}
	    ~ReadHelper() { Reset();}

	    void *Parse(trpgToken,trpgReadBuffer &buf);
	    void Reset();
	    // After parsing this will return the number of trpgChildRef node found.
	    unsigned int GetNbChildrenRef() const;
	    // This will return the trpgChildRef node associated with the index.
	    // this will retrun 0 if idx is out of bound
	    const trpgChildRef* GetChildRef(unsigned int idx) const;
    protected:
	    trpgPrintBuffer *pBuf;
	    trpgPrintGraphParser *parse;

    private:
      
	    ChildRefList childRefList;


    };

    // Fetch the archive associated with this print
    trpgr_Archive *GetArchive() {return archive; };
    trpgrImageHelper *GetImageHelp() {return imageHelp; };
	
 protected:
    bool StartChildren(void *);
    bool EndChildren(void *);

    trpgPrintBuffer *printBuf;
    trpgr_Archive *archive;
    trpgrImageHelper *imageHelp;
   
    ReadHelper *childRefCB;
   
};

// Print utitility for while archive

#define TRPGPRN_ALL -1
#define TRPGPRN_HEADER (1<<0)
#define TRPGPRN_BODY   (1<<1)
TX_CPPDECL bool trpgPrintArchive(char *filename,trpgPrintBuffer &pBuf,int flags=TRPGPRN_ALL);
TX_CPPDECL bool trpgPrintArchive(trpgr_Archive *,trpgPrintBuffer &pBuf,int flags=TRPGPRN_ALL);

#endif
