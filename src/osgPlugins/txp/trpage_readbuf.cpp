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
#include <math.h>

/* trpage_readbuf.cpp
	Methods for the trpgReadBuffer and trpgMemReadBuffer classes.
	trpgReadBuffer is a virtual base class with a few utility functions.
	It's used as generic interface for reading data out of.
	trpgMemReadBuffer is a subclass of that which implements methods for
	 reading out of a chunk of memory.  Data is read off of disk and then
	 dumped into a read buffer for parsing.
	If you wanted to read directly from disk, for example, you could 
	 implement a trpgDiskReadBuffer as a subclass of trpgReadBuffer.
	 */

#include <trpage_io.h>
#include <trpage_swap.h>

/* **********************
   Read buffer base class functions
   **********************
   */

// Basic get functions
bool trpgReadBuffer::Get(int32 &ret)
{
    int32 val;

    if (!GetData((char *)&val,sizeof(int32)))  return false;
    if (ness != cpuNess)
	ret = trpg_byteswap_int(val);
    else
	ret = val;

    return true;
}
bool trpgReadBuffer::Get(int64 &ret)
{
    int64 val;

    if (!GetData((char *)&val,sizeof(int64)))  return false;
	// trpg_byteswap_llong is defined to be 64 bit
    if (ness != cpuNess)
		ret = trpg_byteswap_llong(val);
    else
	ret = val;

    return true;
}
bool trpgReadBuffer::Get(char *ret,int retLen)
{
    int32 len;

    // Get the length first
    if (!Get(len))  return false;

    // Read what we can
    int rlen = MIN(len,retLen-1);
    if (!GetData(ret,rlen))  return false;
    ret[rlen] = 0;

    // Skip the rest
    if (!Skip(len-rlen)) return false;

    return true;
}
bool trpgReadBuffer::Get(std::string &str)
{
    int32 len;

    // Get the length first
    if (!Get(len)) return false;

    if (len < 0)
	return false;
    // Note: Should fix this
    char *tmpStr = new char[len+1];

    // Read it
    if (!GetData(tmpStr,len))  return false;
    tmpStr[len] = 0;

    str = tmpStr;

    return true;
}
bool trpgReadBuffer::Get(float32 &ret)
{
    char cval[4];

    if (!GetData(cval,sizeof(float32)))  return false;
    try {
	if (ness == cpuNess)
	    memcpy(&ret,cval,4);
	else
	    ret = trpg_byteswap_4bytes_to_float(cval);
    }
    catch (...) {
    }

    return true;
}
bool trpgReadBuffer::Get(float64 &ret)
{
    char cval[8];

    if (!GetData(cval,sizeof(float64)))  return false;
    try {
	if (ness == cpuNess)
	    memcpy(&ret,cval,8);
	else
	    ret = trpg_byteswap_8bytes_to_double(cval);
    }
    catch (...) {
    }

    return true;
}
bool trpgReadBuffer::Get(uint8 &ret)
{
    uint8 val;

    if (!GetData((char *)&val,sizeof(uint8)))  return false;
    // No byte swapping needed
    ret = val;

    return true;
}

//#if (bool != int32)
bool trpgReadBuffer::Get(bool &ret)
{
    uint8 val;

    if (!GetData((char *)&val,sizeof(uint8)))  return false;
    // No byte swapping needed
    ret = (val == 0) ? false : true;

    return true;
}
//#endif

#if (trpgDiskRef != int64)
bool trpgReadBuffer::Get(trpgDiskRef &ret)
{
    trpgDiskRef val;

    if (!GetData((char *)&val,sizeof(trpgDiskRef)))  return false;
    if (ness == cpuNess)
	ret = val;
    else
	ret = trpg_byteswap_llong(val);

    return true;
}
#endif

bool trpgReadBuffer::Get(trpgToken &ret)
{
    trpgToken val;

    if (!GetData((char *)&val,sizeof(trpgToken)))  return false;
    if (ness == cpuNess)
	ret = val;
    else
	ret = trpg_byteswap_short(val);

    return true;
}

// Array Get functions
bool trpgReadBuffer::GetArray(int len,float32 **arr)
{
    if (!GetDataRef((char **)arr,sizeof(float32)*len))
	return false;
    // Byteswap in place if necessary
    if (ness != cpuNess) {
	char *ptr;
	int pos;
	for (pos=0,ptr = (char *)*arr;pos<len;pos++,ptr+=4)
	    trpg_swap_four(ptr,ptr);
    }

    return true;
}
bool trpgReadBuffer::GetArray(int len,float64 **arr)
{
    if (!GetDataRef((char **)arr,sizeof(float64)*len))
	return false;
    // Byteswap in place if necessary
    if (ness != cpuNess) {
	char *ptr;
	int pos;
	for (pos=0,ptr = (char *)*arr;pos<len;pos++,ptr+=8)
	    trpg_swap_eight(ptr,ptr);
    }
    return true;
}
bool trpgReadBuffer::GetArray(int len,int32 **arr)
{
    if (!GetDataRef((char **)arr,sizeof(int32)*len))
	return false;
    // Byteswap in place if necessary
    if (ness != cpuNess) {
	char *ptr;
	int pos;
	for (pos=0,ptr = (char *)*arr;pos<len;pos++,ptr+=4)
	    trpg_swap_four(ptr,ptr);
    }
    return true;
}
bool trpgReadBuffer::GetArray(int len,trpgColor **arr)
{
    if (!GetDataRef((char **)arr,sizeof(trpgColor)*len))
	return false;
    // Byteswap in place if necessary
    if (ness != cpuNess) {
	char *ptr;
	int pos;
	for (pos=0,ptr = (char *)*arr;pos<len;pos++,ptr+=8)
	    trpg_swap_four(ptr,ptr);
    }
    return true;
}
bool trpgReadBuffer::GetArray(int len,char **arr)
{
    return GetDataRef((char **)arr,sizeof(char)*len);
}

// Utility Get functions - Just call the others
bool trpgReadBuffer::Get(trpg2iPoint &pt)
{
    if (!Get(pt.x) || !Get(pt.y))
	return false;
    return true;
}
bool trpgReadBuffer::Get(trpg2dPoint &pt)
{
    if (!Get(pt.x) || !Get(pt.y))
	return false;
    return true;
}
bool trpgReadBuffer::Get(trpg3dPoint &pt)
{
    if (!Get(pt.x) || !Get(pt.y) || !Get(pt.z))
	return false;
    return true;
}
bool trpgReadBuffer::Get(trpgColor &color)
{
    if (!Get(color.red) || !Get(color.green) || !Get(color.blue))
	return false;
    return true;
}

// Get both a token and it's length, since that's fairly common
bool trpgReadBuffer::GetToken(trpgToken &tok,int32 &len)
{
    if (!Get(tok) || !Get(len))
	return false;

    return true;
}

/* Limit Handling functions
   These impose arbitrary lenght limits on the read buffer.
   This keeps us from reading pased a token group and parsing
   random data within an archive.
*/
// Push Limit
// Add another limit to the top of the stack
void trpgReadBuffer::PushLimit(int limit)
{
    limits.push_back(limit);
}

// Pop Limit
// Remove the current limit from the stack
void trpgReadBuffer::PopLimit()
{
    int len = limits.size();

    if (len > 0)
	limits.resize(len-1);
}

// Skip To Limit
// Skip to the end of the current limit.
// This happens when we bag the rest of the current token
bool trpgReadBuffer::SkipToLimit()
{
    int len=0;

    if (limits.size() != 0)
	len = limits[limits.size()-1];

    if (len > 0)
	return Skip(len);

    return true;
}

// Test Limit
// See if the next read is going to blow the limits
bool trpgReadBuffer::TestLimit(int len)
{
    for (unsigned int i=0;i<limits.size();i++)
	if (len > limits[i])
	    return false;

    return true;
}

// Update Limits
// We just read a few bytes.  Update the limits
void trpgReadBuffer::UpdateLimits(int len)
{
    for (unsigned int i=0;i<limits.size();i++)
	limits[i] -= len;
}

/* *************************
   Memory Read Buffer
   *************************
   */
trpgMemReadBuffer::trpgMemReadBuffer(trpgEndian in_ness)
{
    data = NULL;
    len = totLen = pos = 0;
    ness = in_ness;
    cpuNess = trpg_cpu_byte_order();
}
trpgMemReadBuffer::~trpgMemReadBuffer()
{
    if (data)
	delete [] data;
}

// Empty check
bool trpgMemReadBuffer::isEmpty()
{
    if (!data)  return true;

    if (pos >= len)
	return true;

    // Also test the limits
    for (unsigned int i=0;i<limits.size();i++)
	if (limits[i] == 0)  return true;

    return false;
}

// Set Length
// Allocate the given space
void trpgMemReadBuffer::SetLength(int newLen)
{
    if (newLen > totLen) {
	if (data)
	    delete [] data;
	data = new char[newLen];
	totLen = newLen;
    }
    len = newLen;
    pos = 0;
}

// Get Data Ptr
// Return a pointer to our data so it can be written to
char *trpgMemReadBuffer::GetDataPtr()
{
    return data;
}

// Get Data
// Protected method for actually retrieving a piece of data
bool trpgMemReadBuffer::GetData(char *ret,int rlen)
{
    if (rlen < 0)
	return false;

    // Test against limits imposed from without
    if (!TestLimit(rlen))  throw 1;

    // See if we've actually got the data
    if (pos+rlen > len)  throw 1;

    // Copy into the return buffer
    memcpy(ret,&data[pos],rlen);

    // Update any limits we might have
    UpdateLimits(rlen);

    pos += rlen;

    return true;
}

// Get Reference to Data
// Protected method that retrieves a reference to the given amoutn of data
bool trpgMemReadBuffer::GetDataRef(char **ret,int rlen)
{
    if (rlen < 0)  return false;

    // Test against limits
    if (!TestLimit(rlen)) throw 1;
    if (pos + rlen > len)  throw 1;

    // Set up reference
    *ret = &data[pos];

    UpdateLimits(rlen);
    pos += rlen;

    return true;
}

// Skip
// Same as read except we're not, uh, reading
bool trpgMemReadBuffer::Skip(int rlen)
{
    if (rlen == 0)
	return true;

    if (rlen < 0)
	return false;

    // Test against limits
    if (!TestLimit(rlen))  return false;
    if (pos + rlen > len)  return false;

    UpdateLimits(rlen);

    pos += rlen;

    return true;
}

/* Appendable File read class
 */

trpgrAppFile::trpgrAppFile(trpgEndian inNess,const char *fileName)
{
    Init(inNess,fileName);
}

void trpgrAppFile::Init(trpgEndian inNess,const char *fileName)
{
    valid = false;
    ness = inNess;
    cpuNess = trpg_cpu_byte_order();

    if (!(fp = fopen(fileName,"rb")))
	return;

    valid = true;
}

trpgrAppFile::~trpgrAppFile()
{
    if (fp)
	fclose(fp);

    valid = false;
}

bool trpgrAppFile::isValid(void) const
{
    return valid;
}

// Read a section of data from the given file
//  and dump it into the given buffer
bool trpgrAppFile::Read(trpgMemReadBuffer *buf,int32 offset)
{
    if (!valid)  return false;

    // Seek to the right location
    if (fseek(fp,offset,SEEK_SET))
	return false;

    // Read a length
    int32 len;
    if (fread(&len,sizeof(int32),1,fp) != 1) {
	valid = false;
	return false;
    }

    // Byteswap if necessary
    if (ness != cpuNess)
	len = trpg_byteswap_int(len);

    if (len < 0) {
	valid = false;
	return false;
    }

    buf->SetLength(len);
    char *data = buf->GetDataPtr();
    if (!data) {
	valid = false;
	return false;
    }

	if (fread(data,sizeof(char),len,fp) != (uint32)len) {
	valid = false;
	return false;
    }

    return true;
}

/* Read a section of data from the given file
   and dump it into the given memory.  Sanity
   check the length against the size of the memory
   passed into dataSize.
*/
bool trpgrAppFile::Read(char *data,int32 baseOffset,int32 objOffset,int32 dataSize)
{
    if (!valid)  return false;

    // Seek to the right place
    int result;
    if ((result = fseek(fp,baseOffset,SEEK_SET))) {
	valid = false;
	return false;
    }

    // Read the total object length
    int32 len;
    if (fread(&len,sizeof(int32),1,fp) != 1) {
	valid = false;
	return false;
    }

    // Byteswap if necessary
    if (ness != cpuNess)
	len = trpg_byteswap_int(len);

    if (len < 0) {
	valid = false;
	return false;
    }

    // It's all right to read less than the whole data block
    if (objOffset+dataSize > len)
	return false;

    // Skip to the object offset
    if (fseek(fp,objOffset,SEEK_CUR)) {
	valid = false;
	return false;
    }

    // Read the raw data
    // Note: What about byte swapping?
	if (fread(data,sizeof(char),dataSize,fp) != (uint32)dataSize) {
	valid = false;
	return false;
    }

    return true;
}

/* App File Cache
   This class manages a group of appendable files with
   the same base name and extension.  It will keep a certain
   number of them open to facilitate caching.
*/

trpgrAppFileCache::trpgrAppFileCache(const char *inPre,const char *inExt,int noFiles)
{
    Init(inPre,inExt,noFiles);
}

void trpgrAppFileCache::Init(const char *inPre,const char *inExt,int noFiles)
{
    strcpy(baseName,inPre);
    strcpy(ext,inExt);

    files.resize(noFiles);
    timeCount = 0;
}

trpgrAppFileCache::~trpgrAppFileCache()
{
    unsigned int len = files.size();
    for (unsigned int i=0;i<len;i++) {
	if (files[i].afile) {
	    delete files[i].afile;
	    files[i].afile = NULL;
	}
    }
}

trpgrAppFile *trpgrAppFileCache::GetNewRAppFile(trpgEndian ness, const char *fileName)
{
    return new trpgrAppFile(ness,fileName);
}

trpgrAppFile *trpgrAppFileCache::GetFile(trpgEndian ness,int id)
{
    return GetFile(ness,id,-1,-1);
}

/*
 */
trpgrAppFile *trpgrAppFileCache::GetFile(trpgEndian ness,int id,int col,int row)
{
    // Look for it already here
    int foundID = -1;
    unsigned int i;
    for (i=0;i<files.size();i++) {
	if ((files[i].id == id)&&
	    (files[i].col == col) &&
	    (files[i].row == row)) {
	    foundID = i;
	    break;
	}
    }

    // Found it in cache, just return
    if (foundID != -1) {
	OpenFile &of = files[foundID];

	if (of.afile->isValid())
	{
	    of.lastUsed = timeCount;
	    return of.afile;
	}
	else
	{
	    if (of.afile)
		delete of.afile;
	    of.afile = NULL;
	}
    }

    // Didn't find it.  Need to reclaim one
    // Look for the oldest used
    int oldTime=-1,oldID=-1;
    for (i=0;i<files.size();i++) {
	OpenFile &of = files[i];
	if (!of.afile || (oldTime == -1) || (of.lastUsed < oldTime)) {
	    oldID = i;
	    oldTime = of.lastUsed;
	    if (!of.afile)
		break;
	}
    }


    // Reclaim this one
    OpenFile &of = files[oldID];
    if (of.afile)
	delete of.afile;
    char fileName[1024];
    if(col==-1) {
	sprintf(fileName,"%s_%d.%s",baseName,id,ext);
    }
    else {
	char dir[1024];
	char filebase[1024];
	//this is very ugly, but it avoids radical API changes
	// find the last PATHSEPERATOR in the baseName string
	int len = strlen(baseName);
	while(--len > 0) {
	    if(baseName[len]==PATHSEPERATOR[0]) {
		strcpy(filebase,&baseName[len+1]);
		strcpy(dir,baseName);
		dir[len]='\0';
		break;
	    }
	}
	sprintf(fileName,"%s" PATHSEPERATOR "%d" PATHSEPERATOR "%d" PATHSEPERATOR "%s_%d.%s",
		dir,col,row,filebase,id,ext);
    }
    of.afile = GetNewRAppFile(ness,fileName);
    of.id = id;
    of.row = row;
    of.col = col;
    of.lastUsed = timeCount;

    timeCount++;

    return of.afile;
}

// Constructor for OpenFile class
trpgrAppFileCache::OpenFile::OpenFile()
{
    afile = NULL;
    lastUsed = 0;
    id = -1;
}
