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

#include "trpage_geom.h"
#include "trpage_write.h"

// Constructor
trpgwArchive::trpgwArchive(trpgEndian inNess)
{
	fp = NULL;
	// Note: only doing external tiles
	tileTable.SetBaseName(".");
	strcpy(dir,".");
	ness = inNess;
	cpuNess = trpg_cpu_byte_order();
}
void trpgwArchive::init(trpgEndian inNess)
{
	ness = inNess;
}

// Destructor
trpgwArchive::~trpgwArchive()
{
	if (fp)
		fclose(fp);
}

// IsValid()
// Verifies that our file is open
bool trpgwArchive::isValid() const
{
	if (!fp)
		return false;

	return true;
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

// OpenFile
// Same as above, only gets a basename as well
bool trpgwArchive::OpenFile(const char *in_dir,const char *name)
{
	char filename[1024];

	strncpy(dir,in_dir,1023);

	sprintf(filename,"%s/%s",dir,name);

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

/* WriteHeader
	For now everything is external, so the header is written last.
	The order is this:
		Header
		Material table
		Texture References
		Model References
		[Future:
			Tile Address Table
			Model Address Table]
		*/
bool trpgwArchive::WriteHeader()
{
	trpgMemWriteBuffer buf(ness);

	if (!isValid())
		return false;

    // Write all the headers into a buffer
    if (!header.Write(buf) ||
        !matTable.Write(buf) ||
        !texTable.Write(buf) ||
        !modelTable.Write(buf) ||
        !tileTable.Write(buf))
        return false;

	// Write the disk header
	int32 magic = TRPG_MAGIC;
	if (ness != cpuNess)
		magic = trpg_byteswap_int(magic);
	if (fwrite(&magic,sizeof(int32),1,fp) != 1)
		return false;

	// Write the header length
	int32 headerSize = buf.length();
	int headLen = headerSize;
	if (ness != cpuNess)
		headerSize = trpg_byteswap_int(headerSize);
	if (fwrite(&headerSize,1,sizeof(int32),fp) != sizeof(int32)) return false;

	// Write the buffer
	const char *data = buf.getData();

	if (fwrite(data,sizeof(char),headLen,fp) != (unsigned int)headLen)
		return false;

	char space[40];
	if (fwrite(space,1,4,fp) != 4)
		return false;

	return true;
}

/* WriteTile
	For now we're only doing external tiles.
	Note: Do more checking
	*/
bool trpgwArchive::WriteTile(unsigned int x,unsigned int y,unsigned int lod,
							 const trpgMemWriteBuffer *head,const trpgMemWriteBuffer *buf)
{
	FILE *tfp=NULL;

	if (!isValid())
		return false;

	// Get the basename and make a new file
	const char *base = tileTable.GetBaseName();
	if (!base)
		return false;

	// Make a new filename
	char filename[1024];
	// Note: Windows specific
	sprintf(filename,"%s/%s/tile_%d_%d_%d.tpt",dir,base,x,y,lod);
	if (!(tfp = fopen(filename,"wb")))
		return false;

	// Write the header first
	int len;
	const char *data;
	if (head) {
		data = head->getData();
		len = head->length();
		if (fwrite(data,sizeof(char),len,tfp) != (unsigned int)len) {
			fclose(tfp);
			return false;
		}
	}

	// Write the buffer out
	data = buf->getData();
	len = buf->length();
	if (fwrite(data,sizeof(char),len,tfp) != (unsigned int)len) {
		fclose(tfp);
		return false;
	}
	fclose(tfp);

	// Note: Should be updating the center in the tile table
	//       Also, last updated date
	return true;
}

/* DeleteTile
	Delete the given tile.
	Note: This is system specific.
	*/
bool trpgwArchive::DeleteTile(unsigned int x,unsigned int y,unsigned int lod)
{
	if (!isValid()) return false;

	const char *base = tileTable.GetBaseName();
	if (!base) return false;

	// Note: windows specific
	char filename[1024];
	sprintf(filename,"%s\\tile_%d_%d_%d.tpt",base,x,y,lod);
	TRPGDELETEFILE(filename);

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
}
// Reset back to a clean state (except for the buffer)
void trpgwGeomHelper::Reset()
{
	ResetTri();
	ResetPolygon();
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

			tex.push_back(polyTex[0]);
			tex.push_back(polyTex[id1]);
			tex.push_back(polyTex[id2]);
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
					tex.push_back(polyTex[i]);
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
	matPoly = -1;
	polyTex.resize(0);
	polyNorm.resize(0);
	polyVert.resize(0);
}

// Set the current color
// Note: Required
void trpgwGeomHelper::SetColor(trpgColor &col)
{
//	tmpColor = col;
}

// Set the current texture coord
// Note: Required
void trpgwGeomHelper::SetTexCoord(trpg2dPoint &pt)
{
	tmpTex = pt;
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
	matPoly = imat;
}

// Collect the current vertex data and add a new whole vertex
// Note: Deal with color
void trpgwGeomHelper::AddVertex(trpg3dPoint &pt)
{
	polyTex.push_back(tmpTex);
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
			int numVert = vert.size();
			// Make sure we've got quads
			if (numVert % 4 == 0) {
				int dtype = (dataType == UseDouble ? trpgGeometry::DoubleData : trpgGeometry::FloatData);
				// Just dump the quads into single geometry node
				trpgGeometry quads;
				quads.SetPrimType(trpgGeometry::Quads);
				quads.AddTexCoords(trpgGeometry::PerVertex);
				for (int i=0;i<numVert;i++) {
					quads.AddVertex((trpgGeometry::DataType)dtype,vert[i]);
					quads.AddNormal((trpgGeometry::DataType)dtype,norm[i]);
					quads.AddTexCoord((trpgGeometry::DataType)dtype,tex[i]);
				}
				quads.SetNumPrims(numVert/4);
				quads.AddMaterial(matTri);

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
	optVert(trpg3dPoint iv,trpg3dPoint in,trpg2dPoint itex) { v = iv; n = in; tex = itex; valid = true;}
	trpg3dPoint v;
	trpg3dPoint n;
	trpg2dPoint tex;
	bool valid;
	int operator == (const optVert &in) const { return (v.x == in.v.x && v.y == in.v.y && v.z == in.v.z &&
														n.x == in.n.x && n.y == in.n.y && n.z == in.n.z &&
														tex.x == in.tex.x && tex.y == in.tex.y); }
};
void trpgwGeomHelper::Optimize()
{
	/*bool isStrip = false;*/
	int dtype = (dataType == UseDouble ? trpgGeometry::DoubleData : trpgGeometry::FloatData);

	// Potentially writing to all of these
	strips.SetPrimType(trpgGeometry::TriStrips);
	strips.AddMaterial(matTri);
	strips.AddTexCoords(trpgGeometry::PerVertex);
	fans.SetPrimType(trpgGeometry::TriFans);
	fans.AddMaterial(matTri);
	fans.AddTexCoords(trpgGeometry::PerVertex);
	bags.SetPrimType(trpgGeometry::Triangles);
	bags.AddMaterial(matTri);
	bags.AddTexCoords(trpgGeometry::PerVertex);

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
		a[0] = optVert(vert[vid],norm[vid],tex[vid]);
		a[1] = optVert(vert[vid+1],norm[vid+1],tex[vid+1]);
		a[2] = optVert(vert[vid+2],norm[vid+2],tex[vid+2]);

		// If we've got two or more triangles to go, try to form something
		if (triId + 1 <numTri) {
			// Triangle B
			b[0] = optVert(vert[vid+3],norm[vid+3],tex[vid+3]);
			b[1] = optVert(vert[vid+4],norm[vid+4],tex[vid+4]);
			b[2] = optVert(vert[vid+5],norm[vid+5],tex[vid+5]);

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
				bool isStrip,flip=true;
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
						c[0] = optVert(vert[vid],norm[vid],tex[vid]);
						c[1] = optVert(vert[vid+1],norm[vid+1],tex[vid+1]);
						c[2] = optVert(vert[vid+2],norm[vid+2],tex[vid+2]);
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
				bool isFan;
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
						c[0] = optVert(vert[vid],norm[vid],tex[vid]);
						c[1] = optVert(vert[vid+1],norm[vid+1],tex[vid+1]);
						c[2] = optVert(vert[vid+2],norm[vid+2],tex[vid+2]);
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
