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

#ifndef _trpage_managers_h_
#define _trpage_managers_h_

#include <deque>

/* This file contains class definitions for managers
	that help you keep track of data related to
	paging.  For instance, which tiles to load
	in at any given time and what textures you need
	to read for a given tile.
 */

class trpgPageManager;

/* Managed Tiles are used by the trpgPageManager to keep
	track of which tiles are loaded and what textures (and
	models) they need loaded into memory with them.
 */
TX_EXDECL class TX_CLDECL trpgManagedTile {
friend class trpgPageManager;
public:
	trpgManagedTile(void);

	// Called to clear any info out of this tile
	void Reset(void);

	/* Call this when you hit a tile header in your own
	   Scene parser callback.  The managed tile
	   can then keep track of which textures and models 
	   go with this tile.
	   */
	bool ParseTileHeader(trpgReadBuffer &);

	// Check if the tile is loaded (e.g. the header read in)
	bool IsLoaded(void);

	/* Set the tile location.  This resets any internal
		state we may be keeping.
	 */
	bool SetTileLoc(int x,int y,int lod);

	// Get the tile location
	bool GetTileLoc(int &x,int &y,int &lod);

	// Return a pointer to the tile header
	const trpgTileHeader *GetTileHead(void);

	/* Return a pointer to the list of locally defined
		materials.  As soon as the tile header is read by
		ParseTileHeader (which you call) you'll want to get
		this list and load the pageable textures.  You can
		use SetMatData to keep track of our internal texture
		structures.
		*/
	const std::vector<trpgLocalMaterial> *GetLocMatList(void) const;

	/* Returns a pointer to a single local material, if within
	    the valid range of local materials for this tile.
	 */
	const trpgLocalMaterial *GetLocMaterial(int id) const;

	/* Set Local Data for managed tile.  The local data would
	    probably be a pointer to the top of the scene graph you're
	    using to represent just this tile.
	 */
	void SetLocalData(void *);

	/* Returns the local data you set with SetLocalData.
	 */
	void *GetLocalData(void) const;

	/* Associates a void * with one of the materials referenced
		within this tile.  The idea here is that you'll want
		to load the texture for a given local material and then
		pass your own internal texture structure into here as
		a void *.  That way, the trpgPageManager will keep track
		of which textures you should unload when this tile goes
		out of range.
	 */
	bool SetMatData(int id,void *);

	/* Gets the void * data you associated with a given local
		material index.  See SetMatData for more information.
	 */
	void *GetMatData(int id) const;

	/* Add Group ID to this tile.  This is called by the page
	    manager to keep track of which group IDs belong to this tile.
	    We use this information to NULL out the appropriate positions
	    in the group map help by the page manager.
	 */
	void AddGroupID(int id);

	/* Retrieve the list of group IDs for this tile.
	 */
	const std::vector<int> *GetGroupIDs(void) const;

	/* Print the current status and information about this managed
	    Tile.
	 */
	void Print(trpgPrintBuffer &);
protected:
	// Set if a tile is currently loaded
	bool isLoaded;
	// Tile location info
	int x,y,lod;
	// Tile Header associated with this tile
	trpgTileHeader tileHead;
	// Data to keep associated with each individual local material index
	std::vector<void *> localMatData;
	// Used to keep track of group IDs in this tile
	std::vector<int> groupIDs;
	// Local data (probably the top of the local scene graph)
	void *localData;

	// Note: Should do models too if anyone wanted them.
};

/* The Page Manager is a helper class that can be used
	to keep track of: (1) which tiles need to be loaded
	into memory for a given viewer position, (2) which tiles
	are currently loaded and (3) which tiles need to be unloaded
	when the viewer position moves.  The tile list this
	class generates is guaranteed to be in the right order
	for loading.  You would use this class if you're implementing
	a TerraPage reader for your visual run-time system.
 */
TX_EXDECL class TX_CLDECL trpgPageManager {
public:
	trpgPageManager(void);
	virtual ~trpgPageManager(void);

	// Initialize with an archive
	virtual void Init(trpgr_Archive *);

	/* Set Paging Distance
	    This is the extra distance outside the visible range
	    we want to page.  The defaults will be valid.  You would
	    set this if you want to pull tiles in earlier.  Be sure
	    to call it before you call Init(), however.
	 */
	virtual bool SetPageDistFactor(double);

	/* Updates the current location for paging purposes.
		Returns true if any load or unloads need to happen.
	 */
	virtual bool SetLocation(trpg2dPoint &);

	/* Get next tile to load.
	    The paging manager is keeping track of which tiles
	    need to be loaded and in what order.  This method
	    returns a pointer to the next one.  The user is
	    expected to call AckLoad() after the tile is loaded.
	 */
	virtual trpgManagedTile *GetNextLoad(void);
	/* Acknowledge Tile Load.
	    This method should be called when a tile has been
	    loaded by the caller.  This method is used in conjunction
	    with GetNextLoad().
	 */
	virtual void AckLoad(void);

	/* Add Group ID to map.
	    This should be called when the user encounters a group-like
	    object while processing the scene graph data from a tile.
	    The groupId is given by TerraPage and the data should be
	    the corresponding group object that the user creates in
	    their own scenegraph toolkit.  This information can then
	    be retrieved later by GetGroupData().
	 */
	virtual void AddGroupID(trpgManagedTile *,int groupID,void *data);

	/* Get Group Data fetches the data cached by the user and
	    associated with the given groupID.  This would be used in
	    conjunction with trpgAttach nodes to implement geometry paging.
	 */
	virtual void *GetGroupData(int groupID);

	/* Get next tile to unload.
	    The paging manager keeps track of which tiles need
		to be unloaded based on a change of location.  It's
		best if you unload tiles before loading them, but
		that's really up to you.
	 */
	virtual trpgManagedTile *GetNextUnload(void);
	/* Acknowledge a tile unload.
		You should call this after you've "unloaded" a tile
		and all its associated textures.
	 */
	virtual void AckUnload(void);

	/* Stop paging entirely.  Call this right before you want to
	    shut down paging.  Everything active will wind up on the
	    unload lists.  Then you can unload those tiles and move on.
	 */
	virtual bool Stop(void);

	// Print current status and content information
	virtual void Print(trpgPrintBuffer &);

protected:
	trpgr_Archive *archive;

	// Center of paging
	trpg2dPoint pagePt;

	/* Information associated with each terrain level of
		detail as related to paging.  
	 */
	class LodPageInfo {
	    friend class trpgPageManager;
	public:
		LodPageInfo(void);
		virtual ~LodPageInfo(void);

		/* Initializes the class with its current LOD.
			It figures out all the rest.
		 */
		virtual bool Init(trpgr_Archive *, int myLod, double scale);

		/* Reset the location.  This forces a recalculation
			of what to load and unload if the cell has changed
			or if this is the first SetLocation.
		   The location passed in must be relative to the southwest
		   corner of the TerraPage archive.
		 */
		virtual bool SetLocation(trpg2dPoint &);

		// Return the next tile to load for this terrain lod
		virtual trpgManagedTile *GetNextLoad(void);
		// Acknowledge the load.  Move the active tile to the
		//  loaded list.
		virtual void AckLoad(void);
		// Return the next tile to unload for this terrain lod
		virtual trpgManagedTile *GetNextUnload(void);
		// Acknowledge the unload.  Move the active tile to the
		//  free list.
		virtual void AckUnload(void);
		// Called to stop paging.  Everything active is dumped on
		//  the unload list.
		virtual bool Stop(void);
		// Print current status and content information
		virtual void Print(trpgPrintBuffer &);
	protected:
		virtual void Clean(void);
		virtual void Update(void);

		// Check if the given tile is within the area we care about
		bool isWithin(trpgManagedTile *,trpg2iPoint &sw,trpg2iPoint &ne);

		bool valid;

		// Terrain LOD we're responsible for
		int lod;

		/* Adjusted (e.g. paranoid) distance outward from
		    which to page this terrain LOD.  This takes into
			account the distance in the header as well as 
			any factor the user may have added.
		 */
		double pageDist;

		/* Max tiles we could have loaded in at any given time.
			This is just a guess because it's up to the user
			to load (and, more importantly) unload.
		 */
		int maxNumTiles;

		// Size of a single cell.  Copied from the archive.
		trpg2dPoint cellSize;

		// Number of tiles (cells) in each direction
		trpg2iPoint lodSize;

		/* Area of interest size in cells
		   This is a linear distance "ahead" of the center cell.
		 */
		trpg2iPoint aoiSize;

		/* Our effective paging location sits at the middle
			of this cell.  We don't recalculate unless the
			cell changes. */
		trpg2iPoint cell;

		// List of tiles to load
		std::deque<trpgManagedTile *> load;
		// List of tiles to unload
		std::deque<trpgManagedTile *> unload;
		// List of currently loaded tiles
		std::deque<trpgManagedTile *> current;

		// Used by Update.  Here because we want to avoid memory allocs, if possible.
		std::vector<bool> tmpCurrent;

		// Set if a load is in progress
		// Load w/o ACK
		bool activeLoad;
		// Set if an unload is in progress
		// Unload w/o ACK
		bool activeUnload;

		// List of tile pointers we can reuse
		std::deque<trpgManagedTile *> freeList;
	};

	// Per terrain lod paging information
	std::vector<LodPageInfo> pageInfo;

	// Enumerated type for lastLoad
	typedef enum {Load,Unload,None} LoadType;
	/* Information about what the pending load/unload operation
		is.  It's up to the user to complete and acknowledge it.
	 */
	LoadType lastLoad;
	// LOD for the pending load/unload requested operation
	int lastLod;
	// Tile to be loaded/unloaded
	trpgManagedTile *lastTile;

	// Optional scaling factor
	double scale;

	// Mapping from group IDs to user defined data
	typedef std::map<int,void *> ManageGroupMap;
	ManageGroupMap groupMap;

	bool valid;
};

/* Page Manager Tester.  This class tests a given paging manager
    by applying likely 
 */
TX_EXDECL class TX_CLDECL trpgPageManageTester {
public:
	trpgPageManageTester();
	virtual ~trpgPageManageTester();

	/* Initialize the tester with a paging manager
	    and an archive.
	 */
	void Init(trpgPrintBuffer *,trpgPageManager *,trpgr_Archive *);

	/* Feeds the paging manager coordinates starting from
	    the lower left to upper right of the database in the
	    given increment.  
	 */
	void Fly_LL_to_UR(double dist=100.0);

	/* Jumps around randomly within the archive loading and
	    unloading as needed.
	 */
	void RandomTest(int no=100,int seed=-1);

protected:
	// Does the work of "load" and "unloading"
	void ProcessChanges();

    trpgPageManager *manager;
    trpgr_Archive *archive;
    trpgPrintBuffer *printBuf;
};

#endif
