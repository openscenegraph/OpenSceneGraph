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

#ifndef _txpage_write_h_
// {secret}
#define _txpage_write_h_

/* trpage_write.h
	Classes that are used to write paging archives.
	*/

#include "trpage_sys.h"
#include "trpage_io.h"
#include "trpage_swap.h"

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
	int numFan;			// Number of distinct fans

	int stripStat[15];  // Strip length stats
	int fanStat[15];    // Fan length stats

	int stripGeom;		// Number of seperate trpgGeometry nodes for strips
	int fanGeom;		// Same for fans
	int bagGeom;		// Same for bags

	int stateChanges;	// Number of distinct material switches

	// Helper functions
	inline void AddStripStat(int val) { stripStat[MIN(14,val)]++; totalStripTri += val; totalTri += val; numStrip++;}
	inline void AddFanStat(int val) { fanStat[MIN(14,val)]++; totalFanTri += val; totalTri += val; numFan++;}
	inline void AddBagStat(int val) { totalBagTri += val; totalTri += val;}
	inline void AddQuadStat(int val) { totalQuad++; }
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
	virtual void Reset();
	// Start/End polygon definition
	virtual void StartPolygon();
	virtual void EndPolygon();
	virtual void ResetPolygon();  // If you change your mind about the current poly
	// Set the current state
	// Note: Currently you *must* set all of these
	virtual void SetColor(trpgColor &);
	virtual void SetTexCoord(trpg2dPoint &);
	virtual void SetNormal(trpg3dPoint &);
	virtual void SetMaterial(int32);
	// Pull the state info together and add a vertex
	virtual void AddVertex(trpg3dPoint &);

	// Dump whatever we're doing and move on
	virtual void FlushGeom();

	// Get statistics for whatever we built
	trpgwGeomStats *GetStats() { return &stats; }
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
	virtual void Optimize();

	// Reset Triangle arrays
	virtual void ResetTri();

	// Collections of geometry
	trpgGeometry strips,fans,bags;

	// Temporary data arrays for triangles/quads
	int32 matTri;
	vector<trpg2dPoint> tex;
	vector<trpg3dPoint> norm,vert;
	// Data arrays for a polygon
	int32 matPoly;
	vector<trpg2dPoint> polyTex;
	vector<trpg3dPoint> polyNorm,polyVert;
	// Single points
	trpg2dPoint tmpTex;
	trpg3dPoint tmpNorm;
	trpgColor tmpCol;

	// Geometry status built up as we go
	trpgwGeomStats stats;
};

/* Paging Archive
	This is a writeable paging archive.
	It organizes where things get written and how.
	{group:Archive Writing}
	*/
TX_EXDECL class TX_CLDECL trpgwArchive : public trpgCheckable {
public:
	trpgwArchive(trpgEndian ness=LittleEndian);
	void init(trpgEndian);
	virtual ~trpgwArchive(void);

	// Set functions.  Have to fill all these out before writing
	virtual bool SetHeader(const trpgHeader &);
	virtual bool SetMaterialTable(const trpgMatTable &);
	virtual bool SetTextureTable(const trpgTexTable &);
	virtual bool SetModelTable(const trpgModelTable &);

	// Note: For now, everything is external
//	enum {Local,External};
//	virtual bool SetModelMode(int);
//	virtual bool SetTileMode(int);

	// Write functions.
	// For now, the header is written last.
	virtual bool OpenFile(const char *,const char *);
	virtual void CloseFile(void);
//	virtual bool OpenFile(const char *);
	virtual bool WriteHeader(void);
	virtual bool WriteTile(unsigned int,unsigned int,unsigned int,const trpgMemWriteBuffer *,const trpgMemWriteBuffer *);
	virtual bool DeleteTile(unsigned int,unsigned int,unsigned int);
//	virtual bool WriteModel(unsigned int,trpgMemWriteBuffer &);

	bool isValid(void) const;
	char* getDir(){return dir;};
protected:
	trpgEndian ness,cpuNess;
	// Fed in from the outside
	char dir[1024];       // Directory where we're doing all this
	trpgHeader header;
	trpgMatTable matTable;
	trpgTexTable texTable;
	trpgModelTable modelTable;

	// Used by this class only
	trpgTileTable tileTable;
//	int modelMode,tileMode;
	FILE *fp;
};

#endif
