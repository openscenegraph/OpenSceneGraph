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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* trpage_rarchive.cpp
	This source file implements the methods for a trpgr_Archive.
	The Read Archive is used to read a paging archive from disk.
	*/

#include "trpage_read.h"

// Constructor
trpgr_Archive::trpgr_Archive()
{
	fp = NULL;
	ness = LittleEndian;
	strcpy(dir,".");
}

// Destructor
trpgr_Archive::~trpgr_Archive()
{
	if (fp)
		fclose(fp);
	fp = NULL;
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
	sprintf(file,"%s/%s",dir,name);

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
}

// Read Header
// Run through the rest of the header information
bool trpgr_Archive::ReadHeader()
{
	if (!fp || headerRead)
		return false;

	headerRead = true;

	// Eat some bytes
//	char stuff[20];
//	if (fread(stuff,1,20,fp) != 20) return false;

//	if (fread(stuff,1,sizeof(trpgllong),fp) != sizeof(trpgllong)) return false;

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
	if (fread(data,1,headLen,fp) != (unsigned int)headLen)  return false;

	// Set up a parser
	// Catch the tables we need for the archive
	trpgr_Parser parser;
	parser.AddCallback(TRPGHEADER,&header);
	parser.AddCallback(TRPGMATTABLE,&materialTable);
	parser.AddCallback(TRPGMATTABLE2,&materialTable);  // Added 11-14-98
	parser.AddCallback(TRPGTEXTABLE,&texTable);
	parser.AddCallback(TRPGMODELTABLE,&modelTable);
	parser.AddCallback(TRPGTILETABLE,&tileTable);

	// Parse the buffer
	if (!parser.Parse(buf))
		return false;

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
	if (/*lod < 0 ||*/ (int)lod >= numLods) return false;
	trpg2iPoint lodSize;
	header.GetLodSize(lod,lodSize);
	if (/*x < 0 ||*/ (int)x >= lodSize.x || /*y < 0 ||*/ (int)y >= lodSize.y) return false;

	// Figure out the file name
	// Note: This assumes External tiles
	const char *base = tileTable.GetBaseName();
	char filename[1024];
	sprintf(filename,"%s/%s/tile_%d_%d_%d.tpt",dir,base,x,y,lod);

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
trpgEndian trpgr_Archive::GetEndian() const
{
	return ness;
}

// Utility MBR routine
bool trpgr_Archive::trpgGetTileMBR(uint32 x,uint32 y,uint32 lod,trpg2dPoint &ll,trpg2dPoint &ur) const
{
	if (!header.isValid())
		return false;
	int32 numLod;
	header.GetNumLods(numLod);
	trpg2iPoint maxXY;
	header.GetLodSize(lod,maxXY);
	if (/*x < 0 ||*/ (int)x>= maxXY.x || /*y < 0 ||*/ (int)y>= maxXY.y)
		return false;

	trpg3dPoint origin;
	header.GetOrigin(origin);
	trpg2dPoint size;
	header.GetTileSize(lod,size);

	ll.x = origin.x + size.x*x;
	ll.y = origin.y + size.y*y;
	ur.x = origin.x + size.x*(x+1);
	ur.y = origin.y + size.y*(y+1);

	return true;
}
