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

/* trpage_warchive.cpp
	This source file contains the implementations of trpgwArchive and trpgwGeomHelper.
	The Write Archive is used to write TerraPage archives.  All its important methods
	 are virtual, so you shouldn't need to modify any of this code.  Simply subclass
	 and override.
	The Geometry Helper is a class that's used to sort out polygons and build
	 trpgGeometry objects, containing triangle strips and fans out of them.  The one
	 contained here is fairly simple, but all its important methods are virtual.  So
	 again, subclass and override if you need to change them.
	*/

#include <trpage_geom.h>
#include <trpage_write.h>
#include <trpage_compat.h>
#include <trpage_read.h>

// Constructor
trpgwArchive::trpgwArchive(trpgEndian inNess,TileMode inTileMode,int inVersion)
{
	version = inVersion;
	if (version != 1 && version != 2)
	    throw 1;
	fp = NULL;
	strcpy(dir,".");
	ness = inNess;
	tileMode = inTileMode;
	cpuNess = trpg_cpu_byte_order();
	tileFile = NULL;
	tileFileCount = 0;
	isRegenerate = false;
	maxTileFileLen = -1;

	firstHeaderWrite = true;
}

// Constructor for regenerate
trpgwArchive::trpgwArchive(char *inDir,char *inFile,trpg2dPoint &sw, trpg2dPoint &ne)
{
	maxTileFileLen = -1;
	version = TRPG_VERSION_MAJOR;
	fp = NULL;
	strcpy(dir,inDir);
	cpuNess = trpg_cpu_byte_order();
	tileFile = NULL;
	tileFileCount = 0;
	isRegenerate = true;
	errMess[0] = '\0';

	// Open a Read Archive to get the rest of the info we need
	trpgr_Archive *inArch = new trpgr_Archive();
	inArch->SetDirectory(inDir);
	if (!inArch->OpenFile(inFile)) {
	    delete inArch;
	    throw 1;
	}
	// Get the header (this is what we need)
	if (!inArch->ReadHeader()) {
	    delete inArch;
	    throw 1;
	}

	ness = inArch->GetEndian();
	const trpgHeader *inHeader = inArch->GetHeader();

	// Expand the coverage
	trpg2dPoint newSW,newNE;
	trpg2dPoint oldSW,oldNE;
	inHeader->GetExtents(oldSW,oldNE);
	newSW.x = MIN(sw.x,oldSW.x);
	newSW.y = MIN(sw.y,oldSW.y);
	newNE.x = MAX(ne.x,oldNE.x);
	newNE.y = MAX(ne.y,oldNE.y);

	// Decide what the offset should be for new tiles
	{
		trpg2dPoint blockSize;
		inHeader->GetTileSize(0,blockSize);
		double dx = (oldSW.x - newSW.x)/blockSize.x + 10e-10;
		double dy = (oldSW.y - newSW.y)/blockSize.y + 10e-10;
		addOffset.x = (int)dx;
		addOffset.y = (int)dy;
		if (dx - addOffset.x > 10e-4 ||
			dy - addOffset.y > 10e-4) {
			delete inArch;
			throw 1;
		}
	}

	// Header can mostly stay the same
	header = *inHeader;
	// Update to the new MBR and tile grid sizes
	header.SetExtents(newSW,newNE);
	header.GetNumLods(numLod);
	for (int i=0;i<numLod;i++) {
		// Figure out the tile grid size
		trpg2dPoint tileSize;
		inHeader->GetTileSize(i,tileSize);
		trpg2iPoint newTileExt;
		newTileExt.x = (int)((newNE.x - newSW.x)/tileSize.x + 10e-5);
		newTileExt.y = (int)((newNE.y - newSW.y)/tileSize.y + 10e-15);
		header.SetLodSize(i,newTileExt);
	}

	// These tables we can copy straight over
	matTable = *inArch->GetMaterialTable();
	texTable = *inArch->GetTexTable();
	modelTable = *inArch->GetModelTable();

	lightTable = *inArch->GetLightTable();
	rangeTable = *inArch->GetRangeTable();
	textStyleTable = *inArch->GetTextStyleTable();
	supportStyleTable = *inArch->GetSupportStyleTable();
	labelPropertyTable = *inArch->GetLabelPropertyTable();

	// Need to resize the tile table
	const trpgTileTable *oldTiles = inArch->GetTileTable();
	trpgTileTable::TileMode tileTableMode;
	oldTiles->GetMode(tileTableMode);
	tileTable.SetMode(tileTableMode);
	tileTable.SetNumLod(numLod);
	for (int lod=0;lod<numLod;lod++) {
		// Size the output tile table
		trpg2iPoint tileSize;
		header.GetLodSize(lod,tileSize);
		tileTable.SetNumTiles(tileSize.x,tileSize.y,lod);

		// Copy over individual tiles
		trpg2iPoint levelOffset;
		levelOffset.x = addOffset.x*(lod+1);
		levelOffset.y = addOffset.y*(lod+1);
		trpg2iPoint oldTileSize;
		inHeader->GetLodSize(lod,oldTileSize);
		for (int ix=0;ix<oldTileSize.x;ix++) {
			for (int iy=0;iy<oldTileSize.y;iy++) {
				trpgwAppAddress addr;
				float zmin,zmax;
				oldTiles->GetTile(ix,iy,lod,addr,zmin,zmax);
				tileTable.SetTile(ix+addOffset.x,iy+addOffset.y,lod,addr,zmin,zmax);
			}
		}
	}

	// Continue to work in the mode the original database is in
	tileMode = (tileTableMode == trpgTileTable::Local) ? TileLocal : TileExternal;

	// That's it for the read archive
	delete inArch;
}

// Destructor
trpgwArchive::~trpgwArchive()
{
	if (fp)
		fclose(fp);
	if (tileFile) {
		delete tileFile;
		tileFile = NULL;
	}
}

// IsValid()
// Verifies that our file is open
bool trpgwArchive::isValid() const
{
	if (!fp)
	{
		strcpy(errMess, "File object do not exist");
		return false;
	}

	return true;
}

const char *trpgwArchive::getErrMess() const
{
	if(errMess[0] == '\0')
		return 0;
	else
		return &errMess[0];
}

// Set the maximum advised size for a tile file

void trpgwArchive::SetMaxTileFileLength(int max)
{
    maxTileFileLen = max;
}

/* Set Functions
	These just copy tables and the header from the input.
	If these aren't set, then empty ones are written.
	*/
bool trpgwArchive::SetHeader(const trpgHeader &head)
{
	header = head;
	return true;
}
bool trpgwArchive::SetMaterialTable(const trpgMatTable &mat)
{
	matTable = mat;
	return true;
}
bool trpgwArchive::SetTextureTable(const trpgTexTable &tex)
{
	texTable = tex;
	return true;
}
bool trpgwArchive::SetModelTable(const trpgModelTable &models)
{
	modelTable = models;
	return true;
}
bool trpgwArchive::SetLightTable(const trpgLightTable &lights)
{
	lightTable = lights;
	return true;
}
bool trpgwArchive::SetRangeTable(const trpgRangeTable &ranges)
{
	rangeTable = ranges;
	return true;
}

bool trpgwArchive::SetTextStyleTable(const trpgTextStyleTable &styles)
{
	textStyleTable = styles;
	return true;
}

bool trpgwArchive::SetLabelPropertyTable(const trpgLabelPropertyTable &properties)
{
	labelPropertyTable = properties;
	return true;
}
bool trpgwArchive::SetSupportStyleTable(const trpgSupportStyleTable &styles)
{

	supportStyleTable = styles;
	return true;
}


/* Get Methods
	Used in regenerate.
 */
trpgHeader *trpgwArchive::GetHeader()
{
    return &header;
}
trpgMatTable *trpgwArchive::GetMatTable()
{
    return &matTable;
}
trpgTexTable *trpgwArchive::GetTextureTable()
{
    return &texTable;
}
trpgModelTable *trpgwArchive::GetModelTable()
{
    return &modelTable;
}
trpgLightTable *trpgwArchive::GetLightTable()
{
    return &lightTable;
}
trpgRangeTable *trpgwArchive::GetRangeTable()
{
	return &rangeTable;
}
trpgTextStyleTable *trpgwArchive::GetTextStyleTable()
{
	return &textStyleTable;
}
trpgSupportStyleTable *trpgwArchive::GetSupportStyleTable()
{
	return &supportStyleTable;
}
trpgLabelPropertyTable *trpgwArchive::GetLabelPropertyTable()
{
	return &labelPropertyTable;
}

// OpenFile
// Same as above, only gets a basename as well
bool trpgwArchive::OpenFile(const char *in_dir,const char *name)
{
	char filename[1024];

	strncpy(dir,in_dir,1023);

	sprintf(filename,"%s" PATHSEPERATOR "%s",dir,name);

	if (!(fp = fopen(filename,"wb")))
		return false;

	return true;
}

// CloseFile
// Close the open file
void trpgwArchive::CloseFile()
{
	if (fp)
		fclose(fp);

	fp = NULL;
}

/* Write Header
	Flush out the header (checkpoint) and return.
 */
bool trpgwArchive::WriteHeader()
{
	bool ret = CheckpointHeader();

	if (tileFile) {
		delete tileFile;
		tileFile=NULL;
	}

	return ret;
}

/* CheckpointHeader
	The header lives in its own file, so we can write it at any point we
	have a valid archive.
	This includes all the tables, as well as basic header info.
		*/
bool trpgwArchive::CheckpointHeader()
{
	trpgMemWriteBuffer buf(ness);

	if (!isValid())
		return false;

	if (!header.isValid())
	{
		if(header.getErrMess())
			strcpy(errMess, header.getErrMess());
		return false;
	}

	// This will close the appendable files
	if (tileFile) {
		tileFile->Flush();
	}

	/* Build a Tile Table
		We need to build one from scratch here.  However,
		we have all the relevant information collected during
		the WriteTile calls.
	 */
	if (tileMode == TileExternal) {
		// External tiles are easy
		tileTable.SetMode(trpgTileTable::External);
	} else {
	    if (!isRegenerate && firstHeaderWrite) {
			// Local tiles require more work
			tileTable.SetMode(trpgTileTable::Local);
			// Set up the sizes
			int32 numLod;
			header.GetNumLods(numLod);
			tileTable.SetNumLod(numLod);
			for (int i=0;i<numLod;i++) {
				trpg2iPoint lodSize;
				header.GetLodSize(i,lodSize);
				tileTable.SetNumTiles(lodSize.x,lodSize.y,i);
			}
			firstHeaderWrite = false;
	    }

	    // Now set the individual tile locations
	    for (unsigned int i=0;i<tileFiles.size();i++) {
		    TileFile &tf = tileFiles[i];
		    for (unsigned int j=0;j<tf.tiles.size();j++) {
			    TileFileEntry &te = tf.tiles[j];
			    trpgwAppAddress addr;
			    addr.file = tf.id;
			    addr.offset = te.offset;
			    tileTable.SetTile(te.x,te.y,te.lod,addr,te.zmin,te.zmax);
		    }

			tf.tiles.clear();
	    }
	}

    // Write all the headers into a buffer
    if (!header.Write(buf))
		return false;

    // Do the mat table and texture table
    // These can be different depending on the version
    switch (version) {
    case 1:
	{
	    trpgMatTable1_0 matTable1_0(matTable);
	    trpgTexTable1_0 texTable1_0(texTable);
	    trpgTileTable1_0 tileTable1_0(tileTable);

	    if (!matTable1_0.Write(buf) ||
		!texTable1_0.Write(buf) ||
		!modelTable.Write(buf) ||
		!tileTable1_0.Write(buf) ||
		!lightTable.Write(buf) ||
		!rangeTable.Write(buf))
		return false;
	}
	break;
    case 2:
		if (!matTable.Write(buf))
		{
			strcpy(errMess, "Error writing material table");
			if(matTable.getErrMess())
			{
				strcat(errMess, ": ");
				strcat(errMess, matTable.getErrMess());
				return false;
			}
		}
		if(!texTable.Write(buf))
		{
			strcpy(errMess, "Error writing texture table");
			if(texTable.getErrMess())
			{
				strcat(errMess, ": ");
				strcat(errMess, texTable.getErrMess());
				return false;
			}
		}
		
	    if(!modelTable.Write(buf) )
		{
			strcpy(errMess, "Error writing model table");
			if(modelTable.getErrMess())
			{
				strcat(errMess, ": ");
				strcat(errMess, modelTable.getErrMess());
				return false;
			}
		}
	    if(!tileTable.Write(buf))
		{
			strcpy(errMess, "Error writing tile table");
			if(tileTable.getErrMess())
			{
				strcat(errMess, ": ");
				strcat(errMess, tileTable.getErrMess());
				return false;
			}
		}
	    if(!lightTable.Write(buf))
		{
			strcpy(errMess, "Error writing light table");
			if(lightTable.getErrMess())
			{
				strcat(errMess, ": ");
				strcat(errMess, lightTable.getErrMess());
				return false;
			}
		}
		if(!rangeTable.Write(buf))
		{
			strcpy(errMess, "Error writing range table");
			if(rangeTable.getErrMess())
			{
				strcat(errMess, ": ");
				strcat(errMess, rangeTable.getErrMess());
				return false;
			}
		}
		if (!textStyleTable.Write(buf))
		{
			strcpy(errMess,"Error writing text style table");
			if (textStyleTable.getErrMess())
			{
				strcat(errMess, ": ");
				strcat(errMess, textStyleTable.getErrMess());
				return false;
			}
		}
		if (!supportStyleTable.Write(buf))
		{
			strcpy(errMess,"Error writing support style table");
			if (supportStyleTable.getErrMess())
			{
				strcat(errMess, ": ");
				strcat(errMess, supportStyleTable.getErrMess());
				return false;
			}
		}
		if (!labelPropertyTable.Write(buf))
		{
			strcpy(errMess,"Error writing label property table");
			if (labelPropertyTable.getErrMess())
			{
				strcat(errMess, ": ");
				strcat(errMess, labelPropertyTable.getErrMess());
				return false;
			}
		}

	break;
    }

    // Write the disk header
    int32 magic = TRPG_MAGIC;
    if (ness != cpuNess)
	    magic = trpg_byteswap_int(magic);
    if (fwrite(&magic,sizeof(int32),1,fp) != 1)
	{
		strcpy(errMess, "Could not write the magic number");
	    return false;
	}

    // Write the header length
    int32 headerSize = buf.length();
    int headLen = headerSize;
    if (ness != cpuNess)
	    headerSize = trpg_byteswap_int(headerSize);
    if (fwrite(&headerSize,1,sizeof(int32),fp) != sizeof(int32))
	{
		strcpy(errMess, "Could not write the header size");
		return false;
	}

    // Write the buffer
    const char *data = buf.getData();

    if (fwrite(data,sizeof(char),headLen,fp) != (unsigned int)headLen)
	{
		strcpy(errMess, "Could not write the buffer");
	    return false;
	}

	// Note: Not sure what this is
    char space[40];
    if (fwrite(space,1,4,fp) != 4)
	    return false;

	// Flush output
	fflush(fp);
	// Head back to the start of the file
	rewind(fp);

    return true;
}

/* Increment Tile File.
	Close the current tile file (if any) and open the next one.
	Also update the records we're keeping of which tiles went in
	which files.
 */
bool trpgwArchive::IncrementTileFile()
{
	if (tileMode != TileLocal)
		return false;

	// Closes the current tile file
	if (tileFile) {
		delete tileFile;
		tileFile=NULL;
	}

	// Open the next one
	char filename[1024];
	sprintf(filename,"%s" PATHSEPERATOR "tileFile_%d.tpf",dir,tileFileCount++);
	tileFile = new trpgwAppFile(ness,filename);
	if (!tileFile->isValid())
		return false;

	// Add another TileFiles entry
	tileFiles.resize(tileFiles.size()+1);
	tileFiles[tileFiles.size()-1].id = tileFiles.size()-1;

	return true;
}

/* Designate Tile File
	Close the current tile file (if any) and open one with the
	given base name.  This is used for regenerate.
 */
bool trpgwArchive::DesignateTileFile(int id)
{
    if (tileMode != TileLocal)
	return false;

    // Close the current tile file
    if (tileFile) {
		delete tileFile;
		tileFile=NULL;
	}

    // Open a named on
    char filename[1024];
    sprintf(filename,"%s" PATHSEPERATOR "tileFile_%d.tpf",dir,id);
    tileFile = new trpgwAppFile(ness,filename);
    if (!tileFile->isValid())
	return false;

    // Add another TileFiles entry
    tileFiles.resize(tileFiles.size()+1);
    tileFiles[tileFiles.size()-1].id = id;

    return true;
}

/* WriteTile.
	Write the given tile (x,y,lod) in the appropriate mode (Local or External).
	The tile header is given separately from the rest of the tile, but they are
	appended together to the file.
	*/
bool trpgwArchive::WriteTile(unsigned int x,unsigned int y,unsigned int lod, float zmin, float zmax,
							 const trpgMemWriteBuffer *head,const trpgMemWriteBuffer *buf)
{
	FILE *tfp=NULL;

	if (!isValid())
		return false;

	// External tiles get their own individual files
	if (tileMode == TileExternal) {
		// Make a new filename
		char filename[1024];
		// Note: Windows specific
		sprintf(filename,"%s" PATHSEPERATOR "tile_%d_%d_%d.tpt",dir,x,y,lod);
		if (!(tfp = fopen(filename,"wb")))
			return false;

		// Write the header first
		unsigned int len;
		const char *data;
		if (head) {
			data = head->getData();
			len = head->length();
			if (fwrite(data,sizeof(char),len,tfp) != len) {
				fclose(tfp);
				return false;
			}
		}

		// Write the buffer out
		data = buf->getData();
		len = buf->length();
		if (fwrite(data,sizeof(char),len,tfp) != len) {
			fclose(tfp);
			return false;
		}
		fclose(tfp);
	} else {
		// Local tiles get appended to a tile file
		if (!tileFile) {
			if (!IncrementTileFile())
				return false;
		} else {
		    // See if we've exceeded the maximum advised size for a tile file
		    if (maxTileFileLen > 0 && tileFile->GetLengthWritten() > maxTileFileLen)
			if (!IncrementTileFile())
			    return false;
		}

		int32 pos = static_cast<int32>(tileFile->Pos());
		if (!tileFile->Append(head,buf))
			return false;
		// Keep track of the fact that this went here
		TileFile &tf = tileFiles[tileFiles.size()-1];
		TileFileEntry te;
		te.x = x;  te.y = y;  te.lod = lod;
		te.zmin = zmin;  te.zmax = zmax;  te.offset = pos;
		tf.tiles.push_back(te);
	}

	return true;
}

/*  ****************
	Geometry Stats
	Used by the Geometry Helper
	****************
	*/
trpgwGeomStats::trpgwGeomStats()
{
	totalTri = totalStripTri = totalFanTri = totalBagTri = 0;
	for (int i=0;i<15;i++) {
		stripStat[i] = fanStat[i] = 0;
	}
	stripGeom = fanGeom = bagGeom = 0;
	stateChanges = 0;
	numStrip = numFan = 0;
	totalQuad = 0;
}
trpgwGeomStats::~trpgwGeomStats()
{
}

/*  ****************
	Geometry Helper
	Here, since it's used with a write archive.
	****************
	*/
trpgwGeomHelper::trpgwGeomHelper()
{
	buf = NULL;
	mode = trpgGeometry::Triangles;
}
trpgwGeomHelper::~trpgwGeomHelper()
{
}
void trpgwGeomHelper::SetMode(int m)
{
	if (m == trpgGeometry::Triangles || m == trpgGeometry::Quads)
		mode = m;
}
trpgwGeomHelper::trpgwGeomHelper(trpgWriteBuffer *ibuf, int dtype)
{
	init(ibuf,dtype);
}
void trpgwGeomHelper::init(trpgWriteBuffer *ibuf,int dtype)
{
	buf = ibuf;
	dataType = dtype;
	zmin = 1e12;
	zmax = -1e12;
}
// Reset back to a clean state (except for the buffer)
void trpgwGeomHelper::Reset()
{
	ResetTri();
	ResetPolygon();
	zmin = 1e12;
	zmax = -1e12;
}

// Reset triangle arrays (usually after a flush)
void trpgwGeomHelper::ResetTri()
{
	strips.Reset();
	fans.Reset();
	bags.Reset();

	tex.resize(0);
	norm.resize(0);
	vert.resize(0);
}

// Start a polygon definition
void trpgwGeomHelper::StartPolygon()
{
	ResetPolygon();
}

// Finish a polygon definition
void trpgwGeomHelper::EndPolygon()
{
	// See if we can add it to the current triangle arrays
	if (vert.size() && (matTri != matPoly)) {
		// Couldn't flush geometry and move on
		FlushGeom();
	}

	// Turn the polygon into triangles
	// Note: Only dealing with convex here
	matTri = matPoly;
	unsigned int numMats=matTri.size();

	switch (mode) {
	case trpgGeometry::Triangles:
		{
		int num = polyVert.size() - 2;
		int id1,id2;
		for (int i=0;i<num;i++) {
			// Note: Handle color

			/* Swap 1 and 2 positions
			   This lets the Optimizer pick up on triangle fans
			   If you're not using our optimizer this will do very weird things
			   Probably it will screw up backface culling.
			   */
	// Note: turned this off because it was broken.  Put it back
#if 0
			id1 = i+1;
			id2 = i+2;
			if (num > 1) {
				id1 = i+2;  id2 = i+1;
			}
#else
			id1 = i+1;
			id2 = i+2;
#endif

			// Define the triangle
			vert.push_back(polyVert[0]);
			vert.push_back(polyVert[id1]);
			vert.push_back(polyVert[id2]);

			norm.push_back(polyNorm[0]);
			norm.push_back(polyNorm[id1]);
			norm.push_back(polyNorm[id2]);

			// multiple textures
			unsigned int loop;
			for (loop=0;loop<numMats;loop++) tex.push_back(polyTex[loop]);
			for (loop=0;loop<numMats;loop++) tex.push_back(polyTex[numMats*id1+loop]);
			for (loop=0;loop<numMats;loop++) tex.push_back(polyTex[numMats*id2+loop]);
		}
		}
		break;
	case trpgGeometry::Quads:
		{
			int num = polyVert.size();
			if (polyVert.size() == 4) {
				for (int i=0;i<num;i++) {
					vert.push_back(polyVert[i]);
					norm.push_back(polyNorm[i]);
					// multiple textures
					for (unsigned int loop=0;loop<numMats;loop++) tex.push_back(polyTex[numMats*i+loop]);
				}
			}
		}
		break;
	}

	ResetPolygon();
}

// Clean out the polygon arrays
void trpgwGeomHelper::ResetPolygon()
{
	tmpTex.resize(0);
	matPoly.resize(0);
	polyTex.resize(0);
	polyNorm.resize(0);
	polyVert.resize(0);
}

// Set the current color
// Note: Required
void trpgwGeomHelper::SetColor(trpgColor& /*col*/)
{
//	tmpColor = col;
}

// Set the current texture coord
// Note: Required
void trpgwGeomHelper::SetTexCoord(trpg2dPoint &pt)
{
	tmpTex.resize(0);
	tmpTex.push_back(pt);
}

void trpgwGeomHelper::AddTexCoord(trpg2dPoint &pt)
{
	tmpTex.push_back(pt);
}

// Set the current normal
// Note: required
void trpgwGeomHelper::SetNormal(trpg3dPoint &pt)
{
	tmpNorm = pt;
}

// Set the current material
// Note: required
void trpgwGeomHelper::SetMaterial(int32 imat)
{
	matPoly.resize(0);
	matPoly.push_back(imat);
}

void trpgwGeomHelper::AddMaterial(int32 imat)
{
	matPoly.push_back(imat);
}

// Get the Z min/max we've collected so far
void trpgwGeomHelper::GetZMinMax(double &outZmin,double &outZmax)
{
	outZmin = zmin;
	outZmax = zmax;
}

// Collect the current vertex data and add a new whole vertex
// Note: Deal with color
void trpgwGeomHelper::AddVertex(trpg3dPoint &pt)
{
	polyTex.insert(polyTex.end(),tmpTex.begin(),tmpTex.end());
	polyNorm.push_back(tmpNorm);
// Note: Turn this back on.  It's not right currently, though
#if 0
    if (buf->GetEndian() != trpg_cpu_byte_order())
    {
        trpg3dPoint tmpVert;
        tmpVert.x = trpg_byteswap_8bytes_to_double ((char *)&pt.x);
        tmpVert.y = trpg_byteswap_8bytes_to_double ((char *)&pt.y);
        tmpVert.z = trpg_byteswap_8bytes_to_double ((char *)&pt.z);
	    polyVert.push_back(tmpVert);
    }
    else
#endif
	    polyVert.push_back(pt);

	// Update min/max
	zmin = MIN(pt.z,zmin);
	zmax = MAX(pt.z,zmax);
}

// Flush the current set of geometry and move on
void trpgwGeomHelper::FlushGeom()
{
	bool hadGeom = false;

	switch (mode) {
	case trpgGeometry::Triangles:
		{
			Optimize();

			// Write only if we've got something
			int numPrim;
			if (strips.GetNumPrims(numPrim) && numPrim) {
				strips.Write(*buf);
				stats.stripGeom++;
				hadGeom = true;
			}
			if (fans.GetNumPrims(numPrim) && numPrim) {
				fans.Write(*buf);
				stats.fanGeom++;
				hadGeom = true;
			}
			if (bags.GetNumPrims(numPrim) && numPrim) {
				bags.Write(*buf);
				stats.bagGeom++;
				hadGeom = true;
			}
		}
		break;
	case trpgGeometry::Quads:
		{
			unsigned int numVert = vert.size();
			unsigned int numMat = matTri.size();
			unsigned int loop;

			// Make sure we've got quads
			if (numVert % 4 == 0) {
				int dtype = (dataType == UseDouble ? trpgGeometry::DoubleData : trpgGeometry::FloatData);
				// Just dump the quads into single geometry node
				trpgGeometry quads;
				quads.SetPrimType(trpgGeometry::Quads);
				for (loop=0;loop<numMat;loop++) quads.AddTexCoords(trpgGeometry::PerVertex);
				for (unsigned int i=0;i<numVert;i++) {
					quads.AddVertex((trpgGeometry::DataType)dtype,vert[i]);
					quads.AddNormal((trpgGeometry::DataType)dtype,norm[i]);
					for (loop=0;loop<numMat;loop++) quads.AddTexCoord((trpgGeometry::DataType)dtype,tex[i*numMat+loop],loop);
				}
				quads.SetNumPrims(numVert/4);
				for (loop=0;loop<numMat;loop++) quads.AddMaterial(matTri[loop]);

				quads.Write(*buf);
				stats.totalQuad++;
				hadGeom = true;
			}
		}
		break;
	}

	if (hadGeom)
		stats.stateChanges++;
	ResetTri();
}

/* Optimize
	Form triangle strips and fans and dump the rest into a "bag"
	of triangles.
	This works for TERREX, but won't do a whole lot for anyone else.
	It will produce valid output, but not particularly optimized.
	*/
#define ADDVERT(dest,vt) { \
	dest.AddVertex((trpgGeometry::DataType)dtype,vt.v); \
	dest.AddNormal((trpgGeometry::DataType)dtype,vt.n); \
	dest.AddTexCoord((trpgGeometry::DataType)dtype,vt.tex); \
}
class optVert {
public:
	optVert() { valid = false; }
	optVert(trpg3dPoint &iv,trpg3dPoint &in,trpg2dPoint &itex) { v = iv; n = in; tex.resize(0); tex.push_back(itex); valid = true;}
	optVert(trpg3dPoint &iv,trpg3dPoint &in,std::vector<trpg2dPoint> &itex) { v = iv; n = in; tex=itex; valid = true;}
	optVert(int numMat, int vid, std::vector<trpg3dPoint> &iv, std::vector<trpg3dPoint> &in, std::vector<trpg2dPoint> &itex);
	trpg3dPoint v;
	trpg3dPoint n;
	std::vector<trpg2dPoint> tex;
	bool valid;
	int operator == (const optVert &in) const { return (v == in.v && n == in.n && tex == in.tex); }
};
optVert::optVert(int numMat, int vid, std::vector<trpg3dPoint> &iv, std::vector<trpg3dPoint> &in, std::vector<trpg2dPoint> &itex)
{
	v=iv[vid];
	n=in[vid];
	tex.resize(0);
	for (unsigned int loop=0; loop < (unsigned int)numMat; loop++) tex.push_back(itex[vid*numMat+loop]);
}
void trpgwGeomHelper::Optimize()
{
	int dtype = (dataType == UseDouble ? trpgGeometry::DoubleData : trpgGeometry::FloatData);

	// Potentially writing to all of these
	strips.SetPrimType(trpgGeometry::TriStrips);
	fans.SetPrimType(trpgGeometry::TriFans);
	bags.SetPrimType(trpgGeometry::Triangles);
	unsigned int numMat = matTri.size();
	for (unsigned int loop =0; loop < numMat; loop++ ) {
		strips.AddMaterial(matTri[loop]);
		strips.AddTexCoords(trpgGeometry::PerVertex);
		fans.AddMaterial(matTri[loop]);
		fans.AddTexCoords(trpgGeometry::PerVertex);
		bags.AddMaterial(matTri[loop]);
		bags.AddTexCoords(trpgGeometry::PerVertex);
	}

	int numTri = vert.size()/3;

	if (numTri == 0)
		return;

	// Iterate through the triangles
	enum {Strip,Fan,Bag};
	int type,triId;
	optVert a[3],b[3],c[3];
	for (triId = 0; triId<numTri; ) {
		// Triangle A
		int vid = 3*triId;
		a[0] = optVert(numMat,vid,vert,norm,tex);
		a[1] = optVert(numMat,vid+1,vert,norm,tex);
		a[2] = optVert(numMat,vid+2,vert,norm,tex);
		// If we've got two or more triangles to go, try to form something
		if (triId + 1 <numTri) {
			// Triangle B
			b[0] = optVert(numMat,vid+3,vert,norm,tex);
			b[1] = optVert(numMat,vid+4,vert,norm,tex);
			b[2] = optVert(numMat,vid+5,vert,norm,tex);

			// Is it a triangle strip?
			if (a[1] == b[1] && a[2] == b[0])
				type = Strip;
			else {
				// Might be a Fan
				if (a[0] == b[0] && a[1] == b[2])
					type = Fan;
				else
					type = Bag;
			}
		} else
			type = Bag;

		switch (type) {
		case Bag:
			ADDVERT(bags,a[0]);
			ADDVERT(bags,a[1]);
			ADDVERT(bags,a[2]);
			bags.AddPrim();
			triId++;
			stats.AddBagStat(1);
			break;
		case Strip:
			{
				bool isStrip=true,flip=true;
				int primLen = 0;
				// Dump A into the strip
				ADDVERT(strips,a[0]);
				ADDVERT(strips,a[1]);
				ADDVERT(strips,a[2]);
				triId++;
				primLen = 3;
				do {
					// Already checked that B was good on last go-round
					ADDVERT(strips,b[2]);  primLen++;
					triId++;
					vid = 3*triId;

					if (triId < numTri) {
						// B is the new primary, check it against the next
						c[0] = optVert(numMat,vid,vert,norm,tex);
						c[1] = optVert(numMat,vid+1,vert,norm,tex);
						c[2] = optVert(numMat,vid+2,vert,norm,tex);
						if (flip)
							isStrip = (c[0] == b[0] && c[1] == b[2]);
						else
							isStrip = (c[0] == b[2] && c[1] == b[1]);
						b[0] = c[0];  b[1] = c[1];  b[2] = c[2];
					} 
					flip = !flip;
				} while (triId < numTri && isStrip);

				strips.AddPrimLength(primLen);
				stats.AddStripStat(primLen);
			}
			break;
		case Fan:
			{
				bool isFan = true;
				int primLen = 0;

				// Dump A into the Fan
				ADDVERT(fans,a[0]);
				ADDVERT(fans,a[2]);
				ADDVERT(fans,a[1]);
				triId++;
				primLen = 3;
				do {
					// Already know that B is good, add that
					ADDVERT(fans,b[1]);  primLen++;
					triId++;
					vid = 3*triId;

					if (triId < numTri) {
						// B is the new primary, check it agains the next
						c[0] = optVert(numMat,vid,vert,norm,tex);
						c[1] = optVert(numMat,vid+1,vert,norm,tex);
						c[2] = optVert(numMat,vid+2,vert,norm,tex);
						isFan = (c[0] == b[0] && c[2] == b[1]);
						b[0] = c[0];  b[1] = c[1];  b[2] = c[2];
					}
				} while (triId < numTri && isFan);

				fans.AddPrimLength(primLen);
				stats.AddFanStat(primLen);
			}
			break;
		}
	}
}

/* *************************
	Image Write Helper class
   *************************
 */

trpgwImageHelper::trpgwImageHelper(trpgEndian inNess,char *inDir,trpgTexTable &inTable)
{
	ness = inNess;
	strcpy(dir,inDir);
	texTable = &inTable;
	texFile = NULL;
	maxTexFileLen = -1;
}

trpgwImageHelper::~trpgwImageHelper()
{
	if (texFile)
		delete texFile;
}

bool trpgwImageHelper::AddExternal(char *name,int &texID,bool lookForExisting)
{
	trpgTexture tex;
	tex.SetImageMode(trpgTexture::External);
	tex.SetName(name);
	if (lookForExisting)
		texID = texTable->FindAddTexture(tex);
	else
		texID = texTable->AddTexture(tex);

	return (texID != -1);
}

void trpgwImageHelper::SetMaxTexFileLength(int len)
{
    maxTexFileLen = len;
}

// doing this so people don't have to modify thier code, for both TXP users out there
bool trpgwImageHelper::AddLocal(char *name,trpgTexture::ImageType type,int sizeX,int sizeY,
								bool isMipmap,char *data,int &texID)
{
	int depth;
	switch (type) {
	case trpgTexture::trpg_INT8:
		depth = 1;
		break;
	case trpgTexture::trpg_INTA8:
		depth = 2;
		break;
	case trpgTexture::trpg_RGB8:
	case trpgTexture::trpg_FXT1:
	case trpgTexture::trpg_DXT1:
	case trpgTexture::trpg_DXT3:
	case trpgTexture::trpg_DXT5:
		depth = 3;
		break;
	case trpgTexture::trpg_RGBA8:
		depth = 4;
		break;
	default:
		depth = -1;
		break;
	}

	return AddLocal(name,type,sizeX,sizeY,depth,trpgTexture::trpg_RGBX_Neither,isMipmap,data,texID);
}

bool trpgwImageHelper::AddLocal(char *name,trpgTexture::ImageType type,int sizeX,int sizeY, int sizeZ,
								trpgTexture::ImageOrg org, bool isMipmap,char *data,int &texID)
{
	// Set up the basic texture
	trpgTexture tex;
	tex.SetName(name);
	tex.SetImageMode(trpgTexture::Local);
	tex.SetImageType(type);
	tex.SetImageSize(trpg2iPoint(sizeX,sizeY));
	tex.SetNumLayer(sizeZ);
	tex.SetImageOrganization(org);
	tex.SetIsMipmap(isMipmap);

	// Write the image out to disk
	trpgwAppAddress addr;
	if (!WriteToArchive(tex,data,addr))
		return false;

	// Now add the specifics to the texture table
	tex.SetImageAddr(addr);
	texID = texTable->AddTexture(tex);

	return true;
}

// doing this so people don't have to modify thier code, for both TXP users out there
bool trpgwImageHelper::AddTileLocal(char *name,trpgTexture::ImageType type, int sizeX, int sizeY,
									bool isMipmap,char *data,int &texID,trpgwAppAddress &addr)
{
	int depth;
	switch (type) {
	case trpgTexture::trpg_INT8:
		depth = 1;
		break;
	case trpgTexture::trpg_INTA8:
		depth = 2;
		break;
	case trpgTexture::trpg_RGB8:
	case trpgTexture::trpg_FXT1:
	case trpgTexture::trpg_DXT1:
	case trpgTexture::trpg_DXT3:
	case trpgTexture::trpg_DXT5:
		depth = 3;
		break;
	case trpgTexture::trpg_RGBA8:
		depth = 4;
		break;
	default:
		depth = -1;
		break;
	}

	return AddTileLocal(name,type,sizeX,sizeY,depth,trpgTexture::trpg_RGBX_Neither,isMipmap,data,texID,addr);
}

bool trpgwImageHelper::AddTileLocal(char *name,trpgTexture::ImageType type, int sizeX, int sizeY, int sizeZ,
									trpgTexture::ImageOrg org, bool isMipmap,char *data,int &texID,trpgwAppAddress &addr)
{
	// Set up the texture template and add to the table
	trpgTexture tex;
	tex.SetName(name);
	tex.SetImageMode(trpgTexture::Template);
	tex.SetImageType(type);
	tex.SetImageSize(trpg2iPoint(sizeX,sizeY));
	tex.SetNumLayer(sizeZ);
	tex.SetImageOrganization(org);
	tex.SetIsMipmap(isMipmap);
	texID = texTable->FindAddTexture(tex);

	// Write the specific data out to an archive (return the address)
	if (!WriteToArchive(tex,data,addr))
		return false;

	return true;
}

/* Increment Texture File.
	Close the current texture file (if any) and open the next one.
 */
bool trpgwImageHelper::IncrementTextureFile()
{
	// Closes the current texture file
	if (texFile)  delete texFile;
	texFile = NULL;

	// Open the next one
	char filename[1024];
	sprintf(filename,"%s" PATHSEPERATOR "texFile_%d.txf",dir,texFileIDs.size());
	texFile = new trpgwAppFile(ness,filename);
	if (!texFile->isValid())
		return false;

	texFileIDs.push_back(texFileIDs.size());

	return true;
}

// Flush current texture file (if any)
bool trpgwImageHelper::Flush()
{
	if (texFile)
		texFile->Flush();

	return true;
}

/* Designate Texture File
	Close the curren texture file (if any) and open one with the given
	base name.
 */
bool trpgwImageHelper::DesignateTextureFile(int id)
{
    // Close the current texture file
    if (texFile)  delete texFile;
    texFile = NULL;

    // Open one with the given base name
    char filename[1024];
    sprintf(filename,"%s" PATHSEPERATOR "texFile_%d.txf",dir,id);
    texFile = new trpgwAppFile(ness,filename);
    if (!texFile->isValid())
	return false;

    texFileIDs.push_back(id);

    return true;
}

/* Write To Archive.
	Write the given image data out to an appropriate archive and
	return the address.  This is used for Local and Tile Local textures.
 */
bool trpgwImageHelper::WriteToArchive(const trpgTexture &tex,char *data,trpgwAppAddress &addr)
{
	trpg2iPoint size;
	tex.GetImageSize(size);
	int32 depth;
	tex.GetImageDepth(depth);

	// Get a usable texture archive file
	if (!texFile) {
	    if (!IncrementTextureFile())
		return false;
	} else {
	    // Deal with maximum advised size for texture archive
	    if (maxTexFileLen > 0 && texFile->GetLengthWritten() > maxTexFileLen) {
		if (!IncrementTextureFile())
		    return false;
	    }
	}

	// Get the current address
	addr.file = texFileIDs[texFileIDs.size()-1];
	addr.offset = static_cast<int32>(texFile->Pos());

	// Write the data out to the archive.
	int totSize = tex.CalcTotalSize();
	if (!texFile->Append(data,totSize))
		return false;

	return true;
}
