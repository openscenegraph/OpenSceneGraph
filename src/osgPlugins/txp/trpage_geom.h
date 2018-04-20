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

#ifndef _trpage_geom_h_
#define _trpage_geom_h_

/* trpage_geom.h
   Geometry and node definitions.
   These are the objects that get read from and written to archives.
*/

#include <trpage_sys.h>

#include <trpage_io.h>
#include <trpage_swap.h>



// Forward declarations

class trpgMaterial;
class trpgTextureEnv;
class trpgMatTable;

#define ISMASTER 1
#define ISLOCAL 2

/* This is the archive header structure.  There is one per TerraPage archive.
   You don't write it directly, but instead fill it out and pass it to
   a trpgwArchive (if you're writing), or get it back from a trpgr_Archive
   (if you're reading).
   If you're putting together a reader, just use the default methods for
   reading this class.  Since it's only read once, the overhead is low.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgHeader : public trpgReadWriteable
{
public:
    trpgHeader(void);
    ~trpgHeader(void);

    // Set the TerraPage version info.
    void    SetVersion(int major,int minor);
    // Set the database version info.
    void    SetDbVersion(int major,int minor);
    /* Set the tile size for the given LOD.  See GetTileSize for more info.
       Each LOD must have its size set, otherwise the trpgHeader won't be valid.
       You must set the number of LODs with SetNumLods first.
       You should use the AddLod method if you can, which handles all of this.
    */
    void    SetTileSize(int lod,const trpg2dPoint &size);
    // Origin defaults to 0,0,0
    void    SetOrigin(const trpg3dPoint &);
    // 2D archive extents.  Must be set.
    void    SetExtents(const trpg2dPoint &sw,const trpg2dPoint &ne);

    typedef enum {DatabaseLocal,Absolute,TileLocal} trpgTileType;
    // How the coordinates are treated with respect to real world values.
    void    SetTileOriginType(trpgTileType);

    /* Number of terrain LODs.  If you use this method when building a database
       you have to use the SetLodRange and SetLodSize methods on each LOD as well.
       It's better to use AddLod instead of calling these three methods.
    */
    void    SetNumLods(int);
    /* Number of tiles (x,y) for each LOD.
       The single argument version assumes lod = 0, num lods = 1.
    */
    void    SetLodSize(int lod,const trpg2iPoint &);
    void    SetLodSize(const trpg2iPoint *);
    /* Set the range for the given terrain LOD.
       The single argument version assumes lod = 0, num lods = 1.
    */
    void    SetLodRange(int,float64);
    void    SetLodRange(const float64 *);
    // Increase the number of terrain LODs, adding a new one with the given size and range
    void    AddLod(const trpg2iPoint &size,const trpg2dPoint &ext,float64 range);

    // Set details for an LOD, resizing if necessary
    void    SetLod(const trpg2iPoint &size,const trpg2dPoint &ext,float64 range,unsigned int lod);

    // Keep track of the maximum assigned group IDs (for tile paging)
    void    SetMaxGroupID(int);
    /* Instead of keeping a count of all the group IDs you added and then
       calling SetMaxGroupID, you can call this function and it will return
       the next valid groupID to you.  It will also keep track of the maximum.
    */
    int            AddGroupID(void);

    // Validity check
    bool        isValid(void) const;
    // Resets the contents back to empty
    void    Reset(void);

    // TerraVista version information is two integers.
    bool    GetVersion(int &,int &) const;
    // Database version information is user defined.  Put whatever you want here.
    bool    GetDbVersion(int &,int &) const;
    /* This is the extents, in X/Y of a
       single tile.  All tiles within an LOD should be the same size (although this is not
       enforced).  It's also assumed that a given tile lives entirely within
       its own extents (as calculated with this value), although that's not
       enforced either. */
    bool    GetTileSize(int,trpg2dPoint &) const;
    /* The meaning of the database origin varies depending on the value returned
       by GetTileOriginType.  If the database is Absolute, then this value
       will be the lower left corner.  If the database is DatabaseLocal or
       TileLocal you can use this value to determine the real world coordinates.
       Just add origin + coordinate.
    */
    bool    GetOrigin(trpg3dPoint &) const;
    /* These are the 2D extents that the database covers.  You can use this
       information to determine where the middle is, for example.
    */
    bool    GetExtents(trpg2dPoint &sw,trpg2dPoint &ne) const;
    /* The tile origin type tells you the coordinate system of each database
       tile.  There are three type:
       * Absolute - All coordinate values are absolute.  No translation is required.
       * DatabaseLocal - All coordinate values are local to the database.  That is
       if you want to determine the real world value do: coord + origin.
       * TileLocal - Each tile has its own coordinate system starting from the lower left
       corner.  We do this to get around floating point accuracy problems (although we
       can do Double coordinates if necessary, as well).  To determine the
       real world coordinate value do: tileID*tileSize + coord.
    */
    bool    GetTileOriginType(trpgTileType &) const;
    /* Group IDs are used by TerraPage to hook level of detail structures together.
       A TerraPage database can have an arbitrary number of terrain LODs, each stored
       separately.  To hook them together we use trpgAttach nodes and number each group &
       LOD node.  This returns the maximum group ID in the file, which is important
       to know if you're keeping an array of them. */
    bool    GetMaxGroupID(int &) const;

    /* A TerraPage archive can contain any number of terrain LODs (a typical number is 4).
       Each of these terrain LODs is accessed separately (as are the tiles within them).
       This returns the number of terrain LODs in the file.  It will be at least 1.
       See trpgAttach for instructions on how to hook the terrain LODs together.

       For version 2.1 and over, this number represent the depest lod that was found
       in the gaming area. With variable lod, tiles will not ncessarily fill out all of
       the gaming area for all of the lods. trpgAttach node are no longer used, instead
       see trpgChildRef
    */
    bool    GetNumLods(int32 &) const;
    /* A terrain LOD conceptually covers the entire database and is broken up
       into some X x Y set of tiles.  We make no assumptions about the number
       of tiles in each terrain LOD.  That's entirely up to the writer.  This
       returns the number of tiles in 2D for a given terrain LOD. */
    bool    GetLodSize(int32,trpg2iPoint &) const;
    /* It's up to the TerraPage archive writer to make their terrain LOD structure
       work by using trpgAttach nodes.  The scheme they're using may be quad-tree
       or replacement LOD or something where the highest LOD isn't even terrain.
       It really doesn't matter.  However, the reader does need a hint as to
       when tiles for a given LOD must be pulled in.  This returns that range
       in database coordinates (usually meters).
    */
    bool    GetLodRange(int32,float64 &) const;

    // Read/Write functions

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;
    // {secret}
    bool    ReadLodInfo(trpgReadBuffer &);

    void    SetMaster(bool isMaster)
    {
        if((verMajor >= TRPG_NOMERGE_VERSION_MAJOR) && (verMinor >=TRPG_NOMERGE_VERSION_MINOR))
        {
            if(isMaster)
                flags |= ISMASTER;//set the master flag
            else
                flags &= ~ISMASTER;//clear the master flag
        }
    }
    bool    GetIsMaster() const
    {
        return ((flags & ISMASTER)==ISMASTER);
    }
    void    SetLocal(bool isLocal)
    {
        if((verMajor >= TRPG_NOMERGE_VERSION_MAJOR) && (verMinor >=TRPG_NOMERGE_VERSION_MINOR))
        {
            if(isLocal)
                flags |= ISLOCAL;//set the local flag
            else
                flags &= ~ISLOCAL;//clear the local flag
        }
    }
    bool    GetIsLocal() const
    {
        return ((flags & ISLOCAL)==ISLOCAL);
    }
    void    SetBlocks(int r,int c)
    {
        rows = r;
        cols = c;
    }
    void    GetBlocks(int &r,int &c)
    {
        r = rows;
        c = cols;
    }
protected:
    int verMinor,verMajor;
    int dbVerMinor,dbVerMajor;
    int maxGroupID;
    trpg2dPoint sw,ne;
    trpg3dPoint origin;
    trpgTileType tileType;

    int numLods;
    std::vector<trpg2dPoint> tileSize;
    std::vector<trpg2iPoint> lodSizes;
    std::vector<float64>   lodRanges;
    int32   flags;
    int32    rows;
    int32    cols;
};

/* The Texture Environment is used by the trpgMaterial to define texture
   related parameters.  A trpgTextureEnv is associated with each texture
   used in a trpgMaterial.  So, for example, if there are 2 textures in
   a material, there will be two texture environments.
   Most of these parameters come straight from the OpenGL specification.  It's
   best to consult that for an exact meaning.

   If you doing a TerraPage reader, expect to get a trpgTextureEnv when
   dealing with trpgMaterial definitions.  If you're doing a writer, you'll
   need to build these in the course of building a trpgMaterial.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgTextureEnv : public trpgReadWriteable
{
    friend class trpgMatTable;
public:
    trpgTextureEnv(void);
    ~trpgTextureEnv(void);

    // Environment mode values
    enum {Alpha,Blend,Decal,Modulate,AddDetail,ModulateDetail};
    // Set the application mode for the texture.
    void    SetEnvMode(int);
    // Values used by SetMinFilter and SetMagFilter
    enum {Point, Linear, MipmapPoint, MipmapLinear,
      MipmapBilinear, MipmapTrilinear, Nearest};
    // Set the Minification filter for a texture
    void    SetMinFilter(int);
    // Set the Magnification filter for a texture
    void    SetMagFilter(int);

    // Values used by SetWrap
    enum {Clamp,Repeat};
    // Set the texture wrapping for S and T, respectively
    void    SetWrap(int,int);
    // Set the texture border color
    void    SetBorderColor(const trpgColor &);

    /* The environment mode controls how the texture is applied.
       It can take the following values:
       Alpha - Used to change the alpha values on a polygon.
       Blend - Blended with the polygon color
       Decal - Doesn't take polygon color into account.
       Modulate - See openGL spec for definition.
       AddDetail - See openGL spec for definition.
       ModulateDetail - See openGL spec for definition.
    */
    bool    GetEnvMode(int32 &) const;
    /* The Minification and Magnification filters control how texture
       mipmap levels are used.  We support the values: Point, Linear,
       MipmapPoint, MipmapLinear,
       MipmapBilinear, MipmapTrilinear, Nearest
    */
    bool    GetMinFilter(int32 &) const;
    // Get the magnification filter
    bool    GetMagFilter(int32 &) const;
    /* Wrapping controls how textures are used near the edges.
       There are two valid values: Clamp, Repeat.
    */
    bool    GetWrap(int &,int &) const;
    /* This maps straight into the OpenGL definition of border color. */
    bool    GetBorderColor(trpgColor &) const;

    // Validity check
    bool        isValid(void) const;
    // Resets the contents back to empty
    void    Reset(void);
    // Writes this class to a write buffer
    bool        Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

protected:
    int envMode;
    int minFilter;
    int magFilter;
    int wrapS,wrapT;
    trpgColor borderCol;
};

/* The material definition for TerraPage encompasses those things that have to
   do with visual display that can be indexed and disassociated from the
   polygons themselves.  This covers things like color, texture, alpha
   and a few more obscure ones.
   Materials are indexed centrally in a trpgMatTable.

   This material definition borrows heavily from the OpenGL specification.
   Please refer to that for a good definition of all the fields.

   If you're doing a TerraPage reader you'll need to deal with these in two places.
   First, is when you read the archive header and get a trpgMatTable back.  You'll
   want to translate them into your own internal representation and keep track of
   the mapping.  Later, when parsing trpgGeometry nodes, you'll run into them
   again.  This time they will be material indices into a trpgMatTable.  At that
   point you'll want to map these indices into your own material definition table.

   If you're doing a TerraPage writer you'll need to create one of these for every
   unique material-like object you have.  Since trpgMaterial objects are indexed
   centrally in a TerraPage archive, you should take advantage of that and use
   as few as possible.  After defining one, you'll want to add it to a trpgMatTable
   and keep track of the material index that returns.  This will be the mapping from
   your own internal material table (or whatever you've got) into the archive's
   material table.  A trpgMaterial sets up defaults that work pretty well, so just
   fill in what you need to use.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgMaterial : public trpgReadWriteable
{
    friend class trpgMatTable;
public:
    trpgMaterial(void);
    ~trpgMaterial(void);
    // Set base material color
    void    SetColor(const trpgColor &);
    // Ambient color
    void    SetAmbient(const trpgColor &);
    // Diffuse color (the most commonly used)
    void    SetDiffuse(const trpgColor &);
    // Specular color used in lighting
    void    SetSpecular(const trpgColor &);
    // Emissive color used in lighting
    void    SetEmission(const trpgColor &);
    // Shininess used in lighting
    void    SetShininess(float64);

    enum {Smooth,Flat};
    // Shading model
    void    SetShadeModel(int);
    // Point size
    void    SetPointSize(float64);
    // Line width
    void    SetLineWidth(float64);
    enum {Front,Back,FrontAndBack};
    // Cull mode.  See GetCullMode
    void    SetCullMode(int);
#ifdef Always
    // None and Always appear to be defined on the SGI
    //  in such a way as to interfere with a local enum
    //  declaration within a class
    enum {trNone,trAlways,Equal,GreaterThanOrEqual,GreaterThan,
      LessThanOrEqual,LessThan,Never,NotEqual};
#else
    enum {None,Always,Equal,GreaterThanOrEqual,GreaterThan,
      LessThanOrEqual,LessThan,Never,NotEqual};
#endif
    // Alpha Function.  See GetAlphaFunc
    void    SetAlphaFunc(int);
    // Alpha Ref value.  See GetAlphaRef
    void    SetAlphaRef(float64);
    // Alpha value for any polygon using this material
    void    SetAlpha(float64);
    // Generate normals automatically from geometry
    void    SetAutoNormal(bool);

    /* Set the total number of textures used by this trpgMaterial.
       This works with SetTexture.  We recommend that you used
       AddTexture instead of these two methods. */
    void    SetNumTexture(int);
    /* Works with SetNumTexture.
       This method sets the texture ID and texture environment for the given
       texture instance in this material.  Use AddTexture instead, if you can.
    */
    void    SetTexture(int no,int id,const trpgTextureEnv &);
    /* This method takes a texture ID that refers to a trpgTexTable and a
       trpgTextureEnv which specifies the application information relating
       to this texture instance.  It returns the reference number (i.e. the
       3rd texture in this material, etc...)
    */
    int        AddTexture(int,const trpgTextureEnv &);

    // Number of tiles this material is used in
    void    SetNumTiles(int);
    // Adds a count to the number of tiles this material is used in and returns that number
    int        AddTile(void);

    // Sets bump map status (color etc... isn't important)
    void    SetIsBumpMap(bool);

    // Set an attribute (e.g. fid/smc)
    enum {TR_FID,TR_SMC,TR_STP,TR_SWC};
    void    SetAttr(int attrCode,int val);

    // Return the current color
    bool    GetColor(trpgColor &) const;
    // Returns the ambient color
    bool    GetAmbient(trpgColor &) const;
    // Returns the diffuse color (the most commonly used color)
    bool    GetDiffuse(trpgColor &) const;
    // Specular color used for lighting
    bool    GetSpecular(trpgColor &) const;
    // Emissive color used for lighting
    bool    GetEmission(trpgColor &) const;
    // Shininess used for lighting
    bool    GetShininess(float64 &) const;

    // The shading model can be either Smooth or Flat
    bool    GetShadeModel(int &) const;
    // Point size
    bool    GetPointSize(float64 &) const;
    // Line width
    bool    GetLineWidth(float64 &) const;
    /* Cull mode determines whether geometry will be rejected if it's Front facing, Back
       facing, or neither (FrontAndBack)
    */
    bool    GetCullMode(int &) const;
    /* This controls what alpha values in a texture mean.  It can take the values:
       None,Always,Equal,GreaterThanOrEqual,GreaterThan,
       LessThanOrEqual,LessThan,Never,NotEqual
    */
    bool    GetAlphaFunc(int &) const;
    /* The Alpha Ref is a value used in some of the Alpha Functions */
    bool    GetAlphaRef(float64 &) const;
    // Whether or not to generate normals from geometry
    bool    GetAutoNormal(bool &) const;
    // A single Alpha value that applies to any polygons using this material
    bool    GetAlpha(float64 &) const;
    /* One of the useful things about TerraPage is that it contains enough information
       to page textures & materials as well as terrain.  This is part of that.
       It returns the number of tiles this material is used in.  The trpgTexture has
       its own which is used for paging textures.  You only want to pay attention to
       this if you have some costly material definition in your hardware and so have
       to swap them in and out.
    */
    bool    GetNumTile(int &) const;

    /* There can be multiple textures per material.  This returns the number.
       The first is the base texture, so just use that if you can only do 1 texture per poly.
    */
    bool    GetNumTexture(int &) const;
    /* TerraPage supports multiple textures per polygon.  Some hardware can do this,
       some can't.  If you can support it, here's how this works.
       This method returns the texture definition for the Nth texture used in this material.
       That consists of a texture ID which points into a trpgTexTable and a trpgTextureEnv
       which contains the texture application information.
       Multiple materials can also appear per trpgGeometry, so be aware that there are
       two ways to have multiple textures per polygon.
    */
    bool    GetTexture(int no,int &id,trpgTextureEnv &) const;

    // Return whether or not this material is a bump map
    bool    GetIsBumpMap(bool &) const;

    // Get an attribute (e.g. fid/smc)
    bool    GetAttr(int attrCode,int &val) const;

    // Validity check
    bool    isValid(void) const;
    // Resets the contents back to empty
    void    Reset(void);
    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    // Note: Need to do equality operator

protected:
    bool isBump;
    trpgColor color;
    trpgColor ambient;
    trpgColor diffuse;
    trpgColor specular;
    trpgColor emission;
    float64 shininess;
    int shadeModel;
    float64 pointSize;
    float64 lineWidth;
    int cullMode;
    int alphaFunc;
    float64 alpha;
    float64 alphaRef;
    bool autoNormal;
    int numTex;
    int32 numTile;
    struct _attrSet
    {
        int fid;
        int smc;
        int stp;
        int swc;
    } attrSet;
    std::vector<int> texids;
    std::vector<trpgTextureEnv> texEnvs;
};

/* Light attributes are used by the trpgLight objects.
   They control all aspects of how lights are constructed and
   displayed.  Lights are very complicated and often system specific.
   It's best to consult the Get methods to figure out what options
   are available.
*/
TX_EXDECL class TX_CLDECL trpgLightAttr : public trpgReadWriteable
{
public:
    // Default constructor
    trpgLightAttr(void);
    // Copy constructor
    trpgLightAttr(const trpgLightAttr &);
    // Destructor
    ~trpgLightAttr(void);

    // Light Flags
    enum LightFlags
    {
        // Light flags
        trpg_Day =        0x0001,
        trpg_Dusk =        0x0002,
        trpg_Night =        0x0004,
        trpg_Directional =    0x0008,
        trpg_BackColor =    0x0010,
        trpg_Reflective =    0x0020,

        // Animation flags
        trpg_Flashing =        0x0100,
        trpg_Rotating =        0x0200,
        trpg_ClockWise =    0x0400,
        trpg_AnimationMask =    0x0f00,

        // Performer light flags
        trpg_Perspective =    0x1000,
        trpg_Fade =        0x2000,
        trpg_ZBuffer =        0x4000,
        trpg_FogPunch =        0x8000,
        trpg_PerformerMask =    0xf000
    };

    // Light Type
    enum LightType
    {
        trpg_Raster,
        trpg_Calligraphic,
        trpg_RASCAL
    };

    // Light Directionality
    enum LightDirectionality
    {
        trpg_Omnidirectional,
        trpg_Bidirectional,
        trpg_Unidirectional
    } ;

    // Light Quality
    enum LightQuality
    {
        trpg_Off,
        trpg_Low,
        trpg_Medium,
        trpg_High,
        trpg_Undefined
    };

    // Struct for Performer Lights
    struct PerformerAttr
    {
        PerformerAttr() : flags(0),minPixelSize(0),maxPixelSize(0),actualSize(0),
            transparentPixelSize(0),transparentFallofExp(0),transparentScale(0),
            transparentClamp(0),fogScale(0) {};
        int32                    flags;
        float64                    minPixelSize;
        float64                    maxPixelSize;
        float64                    actualSize;
        float64                    transparentPixelSize;
        float64                    transparentFallofExp;
        float64                    transparentScale;
        float64                    transparentClamp;
        float64                    fogScale;
    };

    // Struct for Animated Lights
    struct AnimationAttr
    {
        AnimationAttr() : period(0),phaseDelay(0),timeOn(0),vector(trpg3dPoint(0,0,0)),flags(0) {};
        float64                    period;
        float64                    phaseDelay;
        float64                    timeOn;
        trpg3dPoint                vector;
        int32                    flags;
    };

    // Struct for Calligraphic Lights
    struct CalligraphicAttr
    {
        CalligraphicAttr() : drawOrder(0),minDefocus(0),maxDefocus(0) {} ;
        int32                    drawOrder;
        float64                    minDefocus;
        float64                    maxDefocus;
    };

    // Setters
    // Set Type
    void SetType( trpgLightAttr::LightType );

    // Set Directionality
    void SetDirectionality( trpgLightAttr::LightDirectionality );

    // Set Front Color
    void SetFrontColor( trpgColor );

    // Set Front Intensity
    void SetFrontIntensity( float64 );

    // Set Back Color
    void SetBackColor( trpgColor );

    // Set Back Intensity
    void SetBackIntensity( float64 );

    // Set Normal
    void SetNormal( trpg3dPoint );

    // Set SMC
    void SetSMC( int32 );

    // Set FID
    void SetFID( int32 );

    // Set Flags
    void SetFlags( int32 );

    // Set Horizontal Lobe Angle
    void SetHLobeAngle( float64 );

    // Set Vertical Lobe Angle
    void SetVLobeAngle( float64 );

    // Set Lobe Roll Angle
    void SetLobeRollAngle( float64 );

    // Set Lobe Falloff
    void SetLobeFalloff( float64 );

    // Set Ambient Intensity
    void SetAmbient( float64 );

    // Set Quality
    void SetQuality( trpgLightAttr::LightQuality );

    // Set RASCAL Significance
    void SetRascalSignificance( float64 );

    // Set Random Intensity
    void SetRandomIntensity( trpgLightAttr::LightQuality );

    // Set the attributes for Calligraphics Lights
    void SetCalligraphicAttr( trpgLightAttr::CalligraphicAttr& );

    // Set Calligraphic Draw Order
    void SetCalligraphicDrawOrder( int32 );

    // Set Calligraphic Minimum Defocus
    void SetCalligraphicMinDefocus( float64 );

    // Set Calligraphic Maximum Defocus
    void SetCalligraphicMaxDefocus( float64 );

    // Set the attributes for Performer Lights
    void SetPerformerAttr( trpgLightAttr::PerformerAttr& );

    // Set the flags for Perfromer Lights
    void SetPerformerFlags( int32 );

    // Set the minimum pixel size for Performer Lights
    void SetPerformerMinPixelSize( float64 );

    // Set the maximum pixel size for Performer Lights
    void SetPerformerMaxPixelSize( float64 );

    // Set the actual size for Performer Lights
    void SetPerformerActualSize( float64 );

    // Set the transparent pixel size for Performer Lights
    void SetPerformerTpPixelSize( float64 );

    // Set the transparent Falloff exponent for Performer Lights
    void SetPerformerTpFalloffExp( float64 );

    // Set the transparent scale for Performer Lights
    void SetPerformerTpScale( float64 );

    // Set the transparent clamp for Performer Lights
    void SetPerformerTpClamp( float64 );

    // Set the fog scale for Performer Lights
    void SetPerformerFogScale( float64 );

    // Set the attributes for animated lights
    void SetAnimationAttr( trpgLightAttr::AnimationAttr& );

    // Set the animation period for animated lights
    void SetAnimationPeriod( float64 );

    // Set the animation phase delay for animated lights
    void SetAnimationPhaseDelay( float64 );

    // Set the time on for animated lights
    void SetAnimationTimeOn( float64 );

    // Set the animation vector for animated lights
    void SetAnimationVector( trpg3dPoint );

    // Set the flags for animated lights
    void SetAnimationFlags( int32 );

    // Set the comment for this light point type
    void SetComment(const char *);

    // Getters
    // See the comments for the Setters
    void GetType( trpgLightAttr::LightType& );
    void GetDirectionality( trpgLightAttr::LightDirectionality& );
    void GetFrontColor( trpgColor& );
    void GetFrontIntensity( float64& );
    void GetBackColor( trpgColor& );
    void GetBackIntensity( float64& );
    void GetNormal( trpg3dPoint& );
    void GetSMC( int32& );
    void GetFID( int32& );
    void GetFlags( int32& );
    void GetHLobeAngle( float64& );
    void GetVLobeAngle( float64& );
    void GetLobeRollAngle( float64& );
    void GetLobeFalloff( float64& );
    void GetAmbient( float64& );
    void GetQuality( trpgLightAttr::LightQuality& );
    void GetRascalSignificance( float64& );
    void GetRandomIntensity( trpgLightAttr::LightQuality& );
    void GetCalligraphicAttr( trpgLightAttr::CalligraphicAttr& );
    void GetCalligraphicDrawOrder( int32& );
    void GetCalligraphicMinDefocus( float64& );
    void GetCalligraphicMaxDefocus( float64& );
    void GetPerformerAttr( trpgLightAttr::PerformerAttr& );
    void GetPerformerFlags( int32& );
    void GetPerformerMinPixelSize( float64& );
    void GetPerformerMaxPixelSize( float64& );
    void GetPerformerActualSize( float64& );
    void GetPerformerTpPixelSize( float64& );
    void GetPerformerTpFalloffExp( float64& );
    void GetPerformerTpScale( float64& );
    void GetPerformerTpClamp( float64& );
    void GetPerformerFogScale( float64& );
    void GetAnimationAttr( trpgLightAttr::AnimationAttr& );
    void GetAnimationPeriod( float64& );
    void GetAnimationPhaseDelay( float64& );
    void GetAnimationTimeOn( float64& );
    void GetAnimationVector( trpg3dPoint& );
    void GetAnimationFlags( int32& );
    const char *GetComment();

    // operators
    trpgLightAttr& operator = (const trpgLightAttr&);
    bool operator == (const trpgLightAttr&);
    bool operator != (const trpgLightAttr&);

    // Writes this class to a write buffer
    bool Write(trpgWriteBuffer &);

    // Reads this class from a read buffer
    bool Read(trpgReadBuffer &);

    // Prints this class to a print buffer
    bool Print(trpgPrintBuffer &) const;

    // Validity check
    bool isValid(void) const;

    // Resets the contents back to empty
    void Reset(void);


protected:
    struct DataSet
    {
        LightType        type;
        LightDirectionality    directionality;
        trpgColor        frontColor;
        float64            frontIntensity;
        trpgColor        backColor;
        float64            backIntensity;
        trpg3dPoint        normal;
        int32            smc;
        int32            fid;
        int32            flags;
        float64            horizontalLobeAngle;
        float64            verticalLobeAngle;
        float64            lobeRollAngle;
        float64            lobeFalloff;
        float64            ambientIntensity;
        LightQuality        quality;
        LightQuality        randomIntensity;
        float64            rascalSignificance;
        CalligraphicAttr    calligraphicAttr;
        PerformerAttr        performerAttr;
        AnimationAttr        animationAttr;
        char            *commentStr;
    } data;
};

/* Lights in TerraPage are called out as separate objects.  The trpgLight
   object will appear in the scenegraph as an individual light structure.
   These may consist of single lights or light strings.
   The trpgLight points into the trpgLightTable for its attributes.
*/
TX_EXDECL class TX_CLDECL trpgLight : public trpgReadWriteable
{
public:
    // Default constructor
    trpgLight(void);
    // Copy constructor
    trpgLight(const trpgLight &);
    // Destructor
    ~trpgLight(void);

    // Set the index pointing into the Light Table
    void SetAttrIndex(int);

    // Add a new location this light is located at
    void AddVertex(trpg3dPoint);

    // Returns the number of locations, this light is located at
    void GetNumVertices(uint32 &) const;

    // Returns the location at a given index
    bool GetVertex(uint32, trpg3dPoint &) const;

    // Returns the whole list of vertices
    bool GetVertices(trpg3dPoint *) const;
    bool GetVertices(float64 *) const;
    bool GetVertices(float32 *) const;


    // Returns the index of the Light Attributes in the Light Table
    void GetAttrIndex(int &) const;

    // Validity check
    bool isValid(void) const;

    // Resets the contents back to empty
    void Reset(void);

    // Writes this class to a write buffer
    bool Write(trpgWriteBuffer &);

    // Reads this class from a read buffer
    bool Read(trpgReadBuffer &);

    // Prints this class to a print buffer
    bool Print(trpgPrintBuffer &) const;

    // operator
    trpgLight& operator = (const trpgLight &);

protected:
    // Here we store the light locations
    std::vector<trpg3dPoint>    lightPoints;
    int                index;
};

/* The Light Table is stored in the TerraPage header and consolidates
   the attributes for all light points.  trpgLight structures will index
   into this table to define their appearance.
*/
TX_EXDECL class TX_CLDECL trpgLightTable : public trpgReadWriteable
{
public:
    // Default constructor
    trpgLightTable(void);

    // Copy constructor
    trpgLightTable(const trpgLightTable &);

    // Destructor
    ~trpgLightTable(void);

    // Adds the given light attrib to the table and increments the total light attrib count.
    int AddLightAttr(const trpgLightAttr&);

    /* This is the same as AddLightAttr except that it searches for a matching light attrib
       first.  This is convenient for writers who aren't keeping track of their
       own light attrib internally.
    */
    int    FindAddLightAttr(const trpgLightAttr&);

    // Returns the number of light attrib in this table
    bool GetNumLightAttrs(int &) const;

    // Returns a pointer to a light attrib from a given index
    const trpgLightAttr* GetLightAttrRef(int) const;

    // Validity check
    bool isValid(void) const;

    // Resets the contents back to empty
    void Reset(void);

    // Writes this class to a write buffer
    bool Write(trpgWriteBuffer &);

    // Reads this class from a read buffer
    bool Read(trpgReadBuffer &);

    // Prints this class to a print buffer
    bool Print(trpgPrintBuffer &) const;

    // operator
    trpgLightTable & operator = (const trpgLightTable &);

    typedef std::map<int,trpgLightAttr> LightMapType;
    LightMapType *getLightMap() { return &lightMap; }
protected:
    // Here we store the light attributes
    //std::vector<trpgLightAttr> lightList;
    LightMapType lightMap;

};

/* Range Info
   These classes live within the trpgRangeTable.  They encapsulate range
   information that ties trpgLod nodes together.  Their purpose is to provide
   data to the run-time system about the relative priorities of classes of
   features.  See the trpgRangeTable for a complete explanation.
*/
TX_EXDECL class TX_CLDECL trpgRange : public trpgReadWriteable
{
public:
    trpgRange(void);
    ~trpgRange(void);

    // Copy constructor
    trpgRange(const trpgRange &);

    // Set the name and subName (can be NULL)
    void    SetCategory(const char *cat,const char *subCat);

    /* Get the name and subName
       Name should be the major category (e.g. "trees")
       SubName should be the minor category (e.g. "big trees")
    */
    void    GetCategory(char *cat,int maxCatLen,char *subCat,int maxSubCatLen) const;

    // Set the lod info
    void    SetLodInfo(double in,double out);

    // Get the lod info (this will be duplicated in the trpgLod node)
    // However, you may want to change these values yourself during a run
    void    GetLodInfo(double &in,double &out) const;

    // Set the priority
    void    SetPriority(int);

    /* Get the priority.
       Priority is a hint to the run-time system as to the relative importance
       of range limited features.  Features with lower priorities should be
       sacrified before those with higher priorities.
    */
    void    GetPriority(int &) const;

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);

    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);

    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    // Reset back to a clean state
    void Reset(void);

    // Equality operator
    bool operator == (const trpgRange &) const;

    // Assignment operator
    trpgRange & operator = (const trpgRange &);

protected:
    double inLod,outLod;
    char *category;
    char *subCategory;
    int priority;
};

/* The Range Table is new to 2.0.  The goal is to provide information about
   certain classes of features within a TerraPage database.  It's intended
   to solve the following problem.

   Let's say that you have an Image Generator that must draw a given TerraPage
   database at 60Hz.  However, it's only hitting 30Hz reliably.  Often, some
   feature set (e.g. bushes) must be sacrified in order to reach the target frame rate.
   Determining which features can be throttled back can often be very difficult.
   Scaling all of the LODs will often work, but it's not very elegant.  It's
   not selective (you may lose important features as well as unimportant ones)
   and it doesn't easily let turn whole feature sets off.

   If you do want to selectively turn features on and off or throttle them
   back, you can do so based on node names within the LODs.  This can work, however
   you've got the additional overhead of traversing the tree looking for LODs
   and trying to figure out what the names mean.  The range table is intended
   to make this task easier.

   Each trpgLod can (but isn't required to) have an index into the Range Table.
   That index will point to a trpgRange that contains global information
   about that class of LODs.  This includes the in/out information already contained
   within the LOD as well as global info.  The global information includes a category
   and sub-category as well as an integer priority.  The category data is there to
   allow run-time systems to tell users what they're dropping (or allow them to pick).
   The priority is there to tell systems what to get rid of first (according to the
   user who build the TerraPage archive).

   Priorities are relative to each other within the Range Table.  There can be
   duplicates and there may be holes in the numbering.

   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgRangeTable : public trpgReadWriteable
{
public:
    trpgRangeTable(void);
    ~trpgRangeTable(void);

    // Get the given range info
    bool    GetRange(int id,trpgRange &) const;

    // Get the number of entries in the table
    int        GetNumRanges() {return int(rangeMap.size());}
    // Set the range info for the given ID
    bool    SetRange(int id,trpgRange &);

    // Add a new range and return the ID
    int        AddRange(trpgRange &);

    // Find a matching range and return it or add a new one if needed
    int        FindAddRange(trpgRange &);

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);

    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);

    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    // Resets the contents back to empty
    void    Reset(void);

    // Assignment operator
    trpgRangeTable & operator = (const trpgRangeTable &);

protected:
    typedef std::map<int,trpgRange> RangeMapType;
    RangeMapType rangeMap;
    //std::vector<trpgRange> rangeList;
};

/* All materials are centrally indexed in TerraPage.  There is one material
   table per archive.  All trpgGeometry nodes point to that material table (with indices)
   for their trpgMaterial definitions.

   The material table has one wrinkle.  It is divided up into sub-tables or channels.
   Each sub-table has the same number of materials, so there will be NxM trpgMaterial
   structures in a trpgMatTable.  The sub-tables are intended for use in simple sensor
   simulations.  For example, the base table (0) is the purely visual, out the window
   representation.  The next table (1) might the Infra-Red version.  It's up to the run-time
   system to switch between these two.  TerraPage simply provides the means for keeping
   track of it.

   If you're doing a TerraPage reader you'll get a trpgMatTable from the trpgr_Archive.
   This is your central index for materials.  If you can handle the multiple channels/sub-tables
   then you can access those as you need.  If you can't, just use 0 for the sub-table index where appropriate.

   If you're doing a TerraPage writer you'll need to build up a trpgMatTable to pass to
   trpgwArchive.  If you're only doing a single sub-table (i.e. visible materials only)
   just use AddMaterial and add them as you go.  The trpgMaterial object you build up
   for a given material are copied when you call the add function.  So you can have a single
   trpgMaterial, modify just a few fields and call AddMaterial repeatedly.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgMatTable : public trpgReadWriteable
{
public:
    trpgMatTable(void);
    ~trpgMatTable(void);
    /* If you intend to have more than one material sub-table you'll
       need to set this first before doing anything else.
    */
    void    SetNumTable(int);
    /* This sets the total number of materials.  Each sub-table will
       have this many of its own materials.  If you call this function
       you can't use AddMaterial.
    */
    void    SetNumMaterial(int);
    /* Sets a material definition for the given sub-table material ID
       combination.  If you only have one sub-table you can use
       AddMaterial instead.
       The two argument version assumes subTable = 0
    */
    //void    SetMaterial(int subTable,int mat,const trpgMaterial &);
    void    SetMaterial(int,const trpgMaterial &);

    /* This function should be used if you only have a single material sub-table.
       It searches for a matching material and then adds a new one if it doesn't
       find a match.  The new (or old) ID is returned.
    */
    int        AddMaterial(const trpgMaterial &,bool lookForExisting=true);


    // The following method is unused currently
    /* This function should be used when adding materials to multiple sub-tables.
       It searches for a matching material and then adds a new one if it doesn't
       find a match.  The new (or old) ID is returned.
    */
    //int    AddMaterialInSubtable(const trpgMaterial &,int table,bool lookForExisting=true);

    /* Return the number of sub-tables.  This will, most commonly, be 1.
       Any value more than 1 means the archive has alternate material definitions
       (think IR or Radar versions).
    */
    bool    GetNumTable(int &) const;
    /* The number of materials per sub-table.  Each sub-table has the same number
       of materials.  So there will be N x M number of materials total, but you'll
       only see M of them at any given time.
    */
    bool    GetNumMaterial(int &) const;

    /* Returns the material definition for the given subTable and the given material
       ID.  The most common subTable will be 0 (visual).  The material ID comes
       from the value(s) in trpgGeometry.
    */
    bool    GetMaterial(int subTable,int matID,trpgMaterial &) const;

    /* This is a convenience method for getting a reference to a trpgMaterial object.
       The reason you might want to do this is if you don't want to create a full
       trpgMaterial object to pass to GetMaterial.
       The returned value is only valid until the next GetMaterialRef call.
    */
    const trpgMaterial *GetMaterialRef(int,int) const;

    // Validity check
    bool    isValid(void) const;
    // Resets the contents back to empty
    void    Reset(void);

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;


protected:
    int numTable;
    int numMat;
    typedef std::map<int,trpgMaterial> MaterialMapType;
    MaterialMapType materialMap;
};

/* This class holds the texture definition TerraPage uses.  Textures are a little
   different than other TerraPage objects for the following reason: they aren't
   stored in the archive.  Instead they're stored individually on disk in your
   favorite image format.  We don't constrain what that format is, although SGI
   format (.rgb) is always the safest in this industry.

   Texture objects are really just references to these on-disk textures.  As such,
   they're pretty simple.  They just consist of a filename.  These trpgTexture
   objects will be indexed in a trpgTexTable.  The indices you get from trpgMaterial
   objects point to trpgTexture objects through that table.  trpgMaterial objects
   should be the only things that have texture indices.

   If you're doing a TerraPage reader textures are pretty simple to read in.  There
   are two ways to do it.  First, if you're not doing texture paging, simply read
   them all in, using the trpgTexTable to figure out where they all are.  If you
   are doing texture paging (highly recommended) then you'll need to call GetNumTile
   to figure out how many tiles a texture is used in.  If it's 1, then this is probably
   a geospecific textures and ought to be paged.  If it's more than 1, then it's a
   geotypical texture (i.e. a tree or road) and should be loaded in at the beginning.

   If you're doing a TerraPage writer you'll need to be creating trpgTexture objects
   as you go and adding them to your central trpgTexTable.  If you want to support
   texture paging set the numTile count to 1 for the geospecific textures and more
   than 1 for everything else.  There are utility functions for keeping track of all
   of this.  It's best to use those.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgTexture : public trpgReadWriteable
{
public:
    trpgTexture(void);
    trpgTexture(const trpgTexture &);
    ~trpgTexture(void);

    /* This enum is used to determine where the image is.
       External - Stored in an external file, just like TerraPage 1.x.
       Local - Stored in one of the archive files.  The type and size will be here.
       Use trpgrImageHelper to fetch the image, either initially or later.
       Global - This image is really large and parts of it will be referenced
       later within individual tiles.  Use the trpgrImageHelper class
       to reference the various parts of the image.  Don't load it yourself.
       Template - This isn't an image at all.  It's here to represent a class of
       images with a certain size and type.  You'll encounter inline materials
       (for pageable textures) in the tiles which make use of this template.
       If you use trpgrImageHelper to fetch those textures, you don't need to
       do anything with this texture.
    */
    typedef enum {External,Local,Global,Template} ImageMode;

    /* These are the types of images TerraPage can support for Local and Template
       image modes (see ImageMode).  If the image is External or Global, anything
       is fair game, but these are the only types TerraPage can store itself.
    */
    typedef enum
    {
        trpg_RGB8,
        trpg_RGBA8,
        trpg_INT8,
        trpg_INTA8,
        trpg_FXT1,
        trpg_Filler,  // This is not a texture format.  It's here to keep the numbering consistent
        trpg_RGBX, // MCM no longer
        trpg_Unknown,
        trpg_DDS,
        trpg_DXT1,
        trpg_DXT3,
        trpg_DXT5,
        // This is a bit ugly, but we can't change the size of the texture record without
        // breaking existing readers.  So there will be multiple MCM types to indicate the number
        // of bands and the nature of the extra channels, if any
        trpg_MCM5,
        trpg_MCM6R,
        trpg_MCM6A,
        trpg_MCM7RA,
        trpg_MCM7AR
    } ImageType;

    // Set the texture name.
    void    SetName(const char *);
    /* This is the texture name.  You pass in a string of a pre-defined length
       and it returns the texture name in that. */
    bool    GetName(char *retStr,int strLen) const;

    /* Sets the number of tiles this texture is used in.  This hint is used by
       readers to determine texture pageability. */
    void    SetNumTile(int);
    /* Instead of calling SetNumTile after you've built a database, you can call
       AddTile as you encounter each texture reference (per tile). */
    void    AddTile(void);

    /* This tells you the number of tiles this texture is used in.  You can
       use this to do texture paging (if you can support it).  It's a pretty
       general meachanism and will work for large scale geospecific terrain textures
       as well as things like specific building pictures.
       When GetImageMode returns Global, expect this value to always be 1.  For
       Template texture, it will be set to the total number of uses of the template
       (which should be fairly large).
    */
    bool    GetNumTile(int &) const;

    // Retrieve the image mode for this texture.  See ImageMode for details.
    bool    GetImageMode(ImageMode &) const;

    // Retrieve the image type for this texture.  See ImageType for details.
    // This method is only used if ImageMode is Local or Template
    bool    GetImageType(ImageType &) const;

    // Retrieve the size of this image. Valid only for Local and Template textures.
    bool    GetImageSize(trpg2iPoint &) const;

    // Get the location of a Local image
    bool    GetImageAddr(trpgwAppAddress &) const;

    // Figure out the image depth from the type
    bool    GetImageDepth(int32 &depth) const;

    // Determine whether this image (must be Local or Template) has all its mipmaps
    bool    GetIsMipmap(bool &) const;

    // Set the image mode of this texture.  Used by writers only.
    void    SetImageMode(ImageMode);

    // Set the image type of this texture.  See GetImageType for details.
    void    SetImageType(ImageType);

    // Set the image size of this texture.  See GetImageSize for details
    void    SetImageSize(const trpg2iPoint &);

    // Set the image location (For Local images only)
    void    SetImageAddr(const trpgwAppAddress &);

    // Set whether or not this is a full set of mipmaps
    void    SetIsMipmap(bool);

    // Set the storage sizes for all mipmap levels
    //    void    SetStorageSizes(vector<int> &);

    // Get the storage sizes for all mipmap levels.
    //    bool    GetStorageSizes(const vector<int> *) const;

    // Set the number of mipmap levels
    void    SetNumMipmap(int);

    // Set the number of layers used in an RGBX image
    void    SetNumLayer(int);

    // Get the number of layers used in an RGBX image.  RGBX images are typically for
    //  sensors and contain arbitrary data which is not visual.
    bool    GetNumLayer(int &) const;

    /* Utility to figure out the number of mipmap levels this image would have.
       Only really necessary for Local or Template images.
    */
    int32    CalcNumMipmaps() const;

    // Calculate the total size of this texture
    int32    CalcTotalSize() const;

    // Returns the size of a given mip level
    int32    MipLevelSize(int miplevel);

    // Returns the offset of the mip level in the whole texture data buffer
    int32    MipLevelOffset(int miplevel);

    // Validity check
    bool    isValid(void) const;
    // Resets the contents back to empty
    void    Reset(void);

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    trpgTexture & operator = (const trpgTexture &);
    int operator == (const trpgTexture &) const;
protected:
    // Mode for this texture.  See ImageMode for details
    ImageMode mode;

    // Type of texture (only valid if ImageMode is Local or Template)
    ImageType type;

    // The name really only has meaning for External or Global textures
    char *name;
    int useCount;

    // The size values are used only if this is a Local or Template image
    int sizeX,sizeY;

    // Whether or not there are mipmap levels
    bool isMipmap;

    // Number of mipmap levels, 0 if not present
    int numMipMap;

    // Number of layers (for RGBX)
    int numLayer;

    // Sizes of each level.  This is important for compressed textures in particular,
    //  which may have random sizes.  Used only for Local textures.
    std::vector<int> storageSize;

    // Offset of each level
    std::vector<int> levelOffset;

    // Address is used only for Local textures
    trpgwAppAddress addr;

    // calculate the mip level sizes
    void CalcMipLevelSizes();
};

/* The texture table keeps track of all the textures in a TerraPage archive.
   All textures are indexed centrally here.  The indices in trpgMaterial objects
   point into a trpgTexTable.  Although the trpgMatTable potentially has several
   sub-tables for different representations (visual, IR, etc..), the trpgTexTable
   is not affected by that.  All textures, no matter what their use, are indexed
   together here.

   If you're doing a TerraPage reader you'll get a trpgTexTable back from your
   trpgr_Archive.  You'll then want to iterate over the trpgTexture objects and
   load in the ones used in more than one tile.  If you can do texture paging
   you should leave the ones only used in 1 tile alone initially.  You may also
   want to set up a mapping from texture indices here into whatever your own texture
   repository is.  The texture indices in trpgMaterial objects refer to the listing
   here.

   If you're doing a TerraPage writer you'll want to create one of these and add
   textures as you go.  Textures are copied in when you call AddTexture or SetTexture
   so you can reused the trpgTexture object you put together to pass in.  The texture
   index returned by AddTexture should be used in the trpgMaterial you'll need to build.
   Textures don't live in isolation and must be applied to geometry through a trpgMaterial.
   After the trpgTexTable is built it will get passed to a trpgwArchive for writing.  That
   can be done right before you close the archive.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgTexTable : public trpgReadWriteable
{
public:
    trpgTexTable(void);
    trpgTexTable(const trpgTexTable &);
    ~trpgTexTable(void);

    /* Sets the total number of textures in this table.  This is used in
       combination with SetTexture.  If you can, you should use AddTexture
       and FindAddTexture instead.
    */
    void    SetNumTextures(int);
    /* Adds the given texture to the table and increments the total texture
       count.  If you use this, you should not use SetNumTextures and SetTexture.
    */
    int        AddTexture(const trpgTexture &);
    /* This is the same as AddTexture except that it searches for a matching texture
       first.  This is convenient for writers who aren't keeping track of their
       own textures internally.
    */
    int        FindAddTexture(const trpgTexture &);
    /* This sets the given texture ID to be the trpgTexture passed in.  It's used
       in combination with SetNumTextures.  Use AddTexture or FindAddTexture instead
       if you can.
    */
    void    SetTexture(int texID,const trpgTexture &);

    // Returns the number of textures in this table
    bool    GetNumTextures(int &) const;
    // This returns the trpgTexture corresponding to the given ID (from a trpgMaterial)
    bool    GetTexture(int texID,trpgTexture &) const;
    /* Does the same thing as GetTexture only it returns a pointer instead.
       You would use this if you don't want a new trpgTexture created for you.
       Assume the value it returns is only good until the next GetTextureRef call.
    */
    const trpgTexture *GetTextureRef(int) const;

    // Validity check
    bool    isValid(void) const;
    // Resets the contents back to empty
    void    Reset(void);

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;


    trpgTexTable & operator = (const trpgTexTable &);
    const trpgTexture *FindByName(const char *name, int &texid) const;
    void    SetCurrentBlock(int row, int col)
    {
        currentRow = row;
        currentCol = col;
    }

    //bool dumpGeoTypicalTextures(trpgwImageHelper *ihelper);
    typedef std::map<int,trpgTexture> TextureMapType;
    TextureMapType *getTextureMap()
    {
        return &textureMap;
    }
protected:

    TextureMapType textureMap;
    //These are used to initialize the row/col values for
    //multi-archive archives
    int currentRow;
    int currentCol;

};

/* Models are basically just references in TerraPage.  This class just points
   to a model from somewhere else.  There are two places it can point.  (1) It
   can point externally to a model in some arbitrary format (OpenFlight(tm) is
   a popular one).  (2) It can also point to a model within the TerraPage archive.
   The first case is much like trpgTexture objects are treated.  That is, the actual
   thing itself is on disk somewhere corresponding to a file name.  The second case is
   more like tile terrain geometry.  In that case there is scene node type data (LODs,
   groups, geometry, etc...) associated with it.

   trpgModel objects live within a trpgModelTable.  They are indexed there and referred
   to by trpgModelRef objects.  Those model references are the only things that explicitly
   use trpgModel objects.

   If you're doing a TerraPage reader you'll need to take into account whether the
   model is external or internal.  If it's external you'll need to read the given file
   and convert it to your own representation.  If it's internal you've probably already
   got the code for dealing with terrain tiles, which is essentially the same thing.
   Models can be paged, if you're so inclined.  They have tile reference counts just
   like trpgTexture objects.  If numTile == 1 then page it, if > 1 then don't.

   If you're doing a TerraPage writer you'll want to build up a trpgModelTable of these
   as you encounter them.  If your models are external in some other format then setting
   up a trpgModel is pretty easy.  If you want to do internal models, the support is not
   quite there yet.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgModel : public trpgReadWriteable
{
public:
    trpgModel(void);
    trpgModel(const trpgModel &);
    ~trpgModel(void);
    enum {Local,External};
    // Sets the name of the external model file and sets this model to External type.
    // 5.28.04 - It will not set the model to be as of external type. Use the method
    // from below to set the type of the model
    void    SetName(const char *);
    // Sets the on-disk reference to an internal model and sets this model to Internal type.
    // 5.28.04 - It will not set the model to be as of internal type. Use the method
    // from below to set the type of the model
    void    SetReference(trpgDiskRef);
    // Sets the model type
    void    SetType(int);
    /* Models are reference counted (per-tile).  It's up to the writer to set this
       value.  */
    void    SetNumTiles(int);
    /* TerraPage writers can use AddTile (starts at 0) every time they use this model
       in a tile.  Note that this is not for every instance within a tile.  So if
       you use a model 40 times within a tile, you call AddTile once.
       This is used instead of SetNumTiles. */
    void    AddTile(void);

    /* Returns the type (Local or External) of this model */
    bool    GetType(int &);
    /* If the model is external, this returns the file name of that model.
       You pass in a string and a length and it copies the filename into that. */
    bool    GetName(char *ret,int strLen) const;
    /* If the model is internal, this returns the disk reference to it.
       At some future data you'll be able to simply read these out of an archive. */
    bool    GetReference(trpgDiskRef &) const;
    /* Models are reference counted, like trpgTexture objects.  You can  use this
       value to determine whether or not you should page models.
    */
    bool    GetNumTiles(int &) const;

    // Validity check
    bool    isValid(void) const;
    // Resets the contents back to empty
    void    Reset(void);

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &, bool);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;


    trpgModel & operator =  (const trpgModel &);
    int operator == (const trpgModel &) const;


protected:
    int type;
    char *name;
    trpgDiskRef diskRef;
    int useCount;
};

/* Models (trpgModel) are indexed together in a model table.  There is one
   model table per TerraPage archive.  It holds the canonical list of models
   for the entire database.  It's pretty simple.  Just a list of models, really.
   the trpgModel object holds the real information.

   If you're doing a TerraPage reader you'll get one of these from a trpgr_Archive.
   You'll want to iterate over the models in it and figure out which ones to page,
   if you're doing model paging.  If not, then you can just read them all in
   at initialization time and index them as need per-tile.

   If you're doing a TerraPage writer you'll build one of these up for the entire
   database as you go.  Just call AddModel every time you finish a model definition.
   The finished table will be passed to trpgwArchive at the end.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgModelTable : public trpgReadWriteable
{
public:
    trpgModelTable(void);
    ~trpgModelTable(void);

    /* Set the total number of models in the table.  Use this in conjunction
       with SetModel.  If you can, use AddModel isntead of either of these.
    */
    void    SetNumModels(int);
    /* Add the given model to the table.  Makes a copy of the model you pass in
       and returns the new model ID which you'll need to reference in trpgModelRef.
    */
    int        AddModel(trpgModel &);
    /* Look for a given model.  If it's not there, add it. */
    int        FindAddModel(trpgModel &);
    /* Sets the model definition corresponding to the given ID.  Use this in conjunction
       with SetNumModels. */
    void    SetModel(int,const trpgModel &);

    // Returns the number of models in this table
    bool    GetNumModels(int &) const;
    /* Returns the Nth model.  trpgModelRef objects point into this table
       and that is where the model ID comes from. */
    bool    GetModel(int modID,trpgModel &) const;

    /* The same as GetModel only it returns a pointer to the trpgModel instead.
       Use this if you don't want to create a copy of the model.
       The result is only good until the next GetModelRef call.
    */
    trpgModel *GetModelRef(int);

    // Validity check
    bool    isValid(void) const;
    // Resets the contents back to empty
    void    Reset(void);

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    bool FindByName(const char *name, unsigned int &mId);
    typedef std::map<int,trpgModel> ModelMapType;
    ModelMapType *GetModelMap()
    {
        return &modelsMap;
    }
protected:

    ModelMapType modelsMap;
};

/* The tile table keeps track of tile locations within a TerraPage archive.
   Tiles can be stored either externally (as individual files) or locally
   (grouped together into bigger files).  The details are hidden from the
   reader completely and the writer in most cases.
   In version 2.1 the tile table only contains the location of lod 0 tiles.
   All other tiles location are stored in the parent tile as trpgChildRef
   nodes, so you need to parse the parent tile to get at them.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgTileTable : public trpgReadWriteable
{
public:
    // Tiles can be stored individually (External and ExternalSaved) or in grouped files (Local).
    // ExternalSaved still have an entry in the table. In this case the address data is not valid.

    enum TileMode {Local,External, ExternalSaved};

    trpgTileTable();
    ~trpgTileTable(void);

    // Set the tile storage mode: external or local
    void    SetMode(TileMode);
    // Set the total number of LODs
    // For version 2.1 only lod 0 is stored in the table
    void    SetNumLod(int numLod);
    // Set the number of tiles in each dimenion for each terrain LOD
    // This must agree with trpgHeader
    void    SetNumTiles(int numX,int numY,int lod);
    // Set the external address of a given tile as well as its Z value
    void    SetTile(int x,int y,int lod,trpgwAppAddress &,float32 min,float32 max);

    // Local or external tiles
    bool    GetMode(TileMode &) const;
    // Get the disk reference (local)
    bool    GetTile(int x,int y,int lod,trpgwAppAddress &,float32 &min,float32 &max) const;

    // Validity check
    bool    isValid(void) const;
    // Reads this class from a read buffer
    void    Reset(void);

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;
    /**
     * SetCurrentBlock is used in two cases:
     * 1. When building a block specific tile archive (MMB and non-MMB). (local is TRUE)
     * 2. When merging several blocks into memory for visualization. (local is FALSE)
     **/
    void    SetCurrentBlock(int row, int col, bool local)
    {
        currentRow = row;
        currentCol = col;
        localBlock = local;
    }
protected:
    TileMode mode;

    class LodInfo
    {
    public:
        int numX,numY;
        // Tile addresses into external Appendable files
        std::vector<trpgwAppAddress> addr;
        // Elevation of the midpoint of each tile
        // This is used for calculating bounding boxes
        std::vector<float> elev_min;
        std::vector<float> elev_max;
    };
    std::vector <LodInfo> lodInfo;
    //These are used to initialize the row/col values for
    //multi-archive archives
    int currentRow;
    int currentCol;
    bool localBlock; // if true, this tile table should only contain entries for one block.
};


/* Local materials are new to TerraPage 2.0.
   The idea is that for pageable one-time textures it makes more sense
   to define them in the tiles.  This keeps the size of Texture and
   Material tables down in the header.  It also lets us make use of
   Global textures by grabbing subimages.
   You'll encounter these in a tile right after the tile header.  They'll
   be referenced by ID in trpgGeometry nodes within that tile.  They
   can represent a sub-image of a Global texture or a whole Local texture.
   In either case, you can pass this class to trpgrTextureHelper and let
   it get the image data for you.
*/
TX_EXDECL class TX_CLDECL trpgLocalMaterial : public trpgReadWriteable
{
public:
    trpgLocalMaterial(void);
    ~trpgLocalMaterial(void);

    // Set the base material for this local material
    void    SetBaseMaterial(int32 matSubTable,int32 matID);

    /* Get the base material for this local material.  Base materials
       define the colors, specularity, texture environments, and in
       general everything not having to do with the texture image itself.
       In effect you're using the base material as your material definition and
       the local material is just telling you what part of the image to use.
       By convention, there should only be one global image used in any given
       trpgMaterial and it should be at texture index 0 (within that trpgMaterial).
       If you want to use multiple pageable textures per polygon, you can do
       that with multiple materials per trpgGeometry node.  Doing it any
       other way makes my head hurt (actually the thought of multiple, pageable
       textures per polygon makes my head hurt in general).
       // 2003-06-17.  Your prophecy (my headache) has come true.  MD
       */
    bool    GetBaseMaterial(int32 &matSubTable,int32 &matID) const;

    class SubImageInfo
    {
    public:
        int32 sx,sy;  // Source (sx,sy) in pixels
        int32 ex,ey;  // Source (ex,ey) in pixels
        int32 destWidth,destHeight;  // Size of destination image (in pixels)
    };

    // Set the sub image info (only for writers)
    void    SetSubImageInfo(const SubImageInfo &);

    /* Get the sub image info.  This is valid only for Global images
       (you can check what a given Local Material is by looking at the
       base material's texture).
    */
    bool    GetSubImageInfo(SubImageInfo &) const;

    // Set the address info (only for writers)
    void    SetAddr(const trpgwAppAddress &);

    // Set address info for one of multiple local textures
    void    SetNthAddr(unsigned int subtable, const trpgwAppAddress &);

    /* Get the address (location in an archive) for an image.
       This is valid for Local images only.  You can determine what
       a Local Material refers to by looking in its base material's texture.
    */
    bool    GetAddr(trpgwAppAddress &) const;

    // Get address info for one of multiple local textures
    bool    GetNthAddr(unsigned int subtable, trpgwAppAddress &) const;

    // Get the number of local images associated with this local material
    // Tile Local Images only
    bool    GetNumLocals(int &) const;

    // Set the storage sizes for all mipmap levels
    //    bool    SetStorageSizes(vector<int> &);

    // Get the storage sizes for all mipmap levels.
    //    bool    GetStorageSizes(const vector<int> *) const;

    // Validity check
    bool    isValid(void) const;
    // Reads this class from a read buffer
    void    Reset(void);

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

protected:
    int32 baseMatTable,baseMat;
    // These are valid for Global images
    int32 sx,sy,ex,ey,destWidth,destHeight;
    // Storage sizes for each mipmap level.  Important for compressed textures.
    //    vector<int> storageSize;
    // This is valid only for Tile Local images
    std::vector<trpgwAppAddress> addr;
};

/* The tile header is one of the more interesting parts of TerraPage.  Basically,
   it's a list of all the materials, local material  and models used in a tile.
   Tile headers are stuck at the beginning of terrain tiles to give you this
   information.  They can be read separately, in theory, although no one is doing
   that at present.

   If you're doing a TerraPage reader you will encounter one of these first thing
   when you parse a terrain tile.  These are here to aid texture and model paging.
   Texture paging is the most common and there are now two ways of doing it.  The
   simpler one (supported in version 1.0) is as follows.  Textures are indexed at
   the beginning of a TerraPage archive (in a trpgTexTable) and when listed in the
   trpgTileHeader for a tile must be loaded in for use.  You can tell the pageable
   textures from the non-pageable ones by looking at the tile reference count in
   the trpgTexture object.

   The second way of doing texture paging (in version 2.0) is more complex, but
   much more powerful.  One of the big problems we encountered with v1.0 was the
   enormous proliferation of texture files.  In addition, much information was
   duplicated between the different terrain resolutions.  Lastly, we (TERREX) have
   had a lot of success with wavelet compressed image pyramids for solving quite a
   few interesting problems.  This second approach to texture paging makes use of
   what we've learned from all of this.  The idea is that instead of having lots of
   little textures, we have a small number of very large images which can support
   extraction at any resolution.  In this way we re-use information between terrain
   levels of detail and we cut down on our disk usage, both in terms of number of
   files as well as data size.

   To implement this scheme we added the trpgLocalMaterial object.  trpgTexture
   objects tell you if they are global images (e.g. parts are pageable).  If they
   are, then they can be used in a trpgLocalMaterial.  These objects define the
   sub-image (extents and a resolution) for a texture/material that can be used
   within this tile (and only within this tile).  The trpgLocalMaterial objects
   reside within the trpgTileHeader and should be filled out (e.g. the sub-images
   paged) as soon as the trpgTileHeader is parsed because you'll be needing them
   in trpgGeometry nodes to follow.

   If you want to page models, you can do so by looking at the list of model IDs
   used in a tile.

   If you're doing a TerraPage writer you will need to construct one of these for
   each tile that you build (remember that tiles are per-terrain LOD).  You'll want
   to call AddMaterial for every material that you use in a tile and AddModel
   for every model.  You can call these methods multiple times and it will keep track
   of whether you've already added a model or material.  The tile header will then
   be passed to trpgwArchive along with the tile geometry and written to disk.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgTileHeader : public trpgReadWriteable
{
public:
    trpgTileHeader(void);
    ~trpgTileHeader(void);
    /* Add the given material reference to this tile if it's not already there.
       The ID passed in refers to a trpgMatTable. */
    void    AddMaterial(int);
    /* Add the given model reference to this tile if it's not already there.
       The ID passed in refers to a trpgModelTable. */
    void    AddModel(int);
    /* Add the given local material to the list of local materials in this tile.
       Local materials are used to page sub-pieces of very large images and
       are referenced by trpgGeometry nodes (in their material indices) within
       a given tile.  Local materials can't be referenced across tiles.
    */
    void    AddLocalMaterial(trpgLocalMaterial &);
    // {secret}
    void    SetDate(int32);
    /* Set the given material reference to the given value.
       It's easier to use AddMaterial(), if possible.
    */
    void    SetMaterial(int no,int id);
    /* Set the given model reference (in this tile) to the given value
       It's easier to use AddModel(), if possible.
    */
    void    SetModel(int no,int id);

    // Returns the number of materials used in this tile
    bool    GetNumMaterial(int32 &) const;
    /* Return the material ID of the Nth material reference.
       The ID returned points into a trpgMatTable. */
    bool    GetMaterial(int32 nth,int32 &matID) const;
    // This is the number of models used in this tile
    bool    GetNumModel(int32 &) const;
    /* Gets the model ID of the Nth model reference.
       The ID returned points into a trpgModelTable. */
    bool    GetModel(int32 nth,int32 &modID) const;
    // Returns the number of local materials in this tile
    bool    GetNumLocalMaterial(int32 &) const;
    /* Return the given local material.  Local materials are used
       to page sub-pieces of extremely large images.  The Local Material
       list within a tile is distinct from the list of materials used within
       a tile.  Local materials will be referenced by trpgGeometry nodes within
       a given tile.
    */
    bool    GetLocalMaterial(int32 nth,trpgLocalMaterial &locMat) const;
    /* Return a pointer to the list of locally defined materials.
       See GetLocalMaterial for an explanation of local materials.
    */
    const std::vector<trpgLocalMaterial> *GetLocalMaterialList(void) const;
    // {secret}
    bool    GetDate(int32 &) const;

    // Validity check
    bool    isValid(void) const;
    // Resets the contents back to empty
    void    Reset(void);

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;
    void    SetBlockNo(int r, int c)
    {
        row = r;
        col = c;
    }
    void    GetBlockNo(int &r, int &c)
    {
        r = row;
        c = col;
    }
protected:
    std::vector<int> matList;
    std::vector<int> modelList;
    std::vector<trpgLocalMaterial> locMats;
    // {secret}
    int date;
    //These are used to find them in the block archive.
    int row;
    int col;
};

/* The color info structure is used by the trpgGeometry class to store
   per vertex (or per primitive) color information.  It can be read directly
   by the user (all its data is public).  This structure is returned by
   trpgGeometry::GetColorSet().
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgColorInfo
{
public:
    trpgColorInfo(void);
    ~trpgColorInfo(void);

    /* This is a trpgGeometry::ColorType
     */
    int type;
    /* This refers to how the colors in the data array are used.
       It can take the values "Overall", "PerPrim" or "PerVertex".
    */
    int bind;
    /* The list of colors.  There will be one total (bind=Overall), one per
       primitive (bind=PerPrim), or one per vertex (bind=PerVertex).
    */
    std::vector<trpgColor> data;
    /* Resets the structure to a default state.
     */
    void Reset(void);

    // Print out debugging info
    bool Print(trpgPrintBuffer &) const;
};

/*  This class represents a group of texture coordinates applied to a trpgGeometry
    class.  It is returned by trpgGeometry::GetTexCoordSet.  TerraPage supports
    multiple materials per polygon.  The way we implement this is as multiple
    materials on a trpgGeometry node.  The first material with be the "primary"
    and additional ones will be secondary and so on.
    To support this, we need multiple sets of texture coordinates.  That's what
    this structure is used for.
    {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgTexData
{
public:
    trpgTexData(void);
    ~trpgTexData(void);
    // This should always be set to PerVertex
    int bind;
    /* List of texture coordinates in 32 bit floating point.
       There should be one per vertex.  Either this or doubleData will be
       set, but never both.
    */
    std::vector<float32> floatData;
    /* List of texture coordinates in 64 bit floating point.
       There should be one per vertex.  Either this or floatData will be
       set, but never both.
    */
    std::vector<float64> doubleData;
    /* Initialize the texture coordinate data set with floating point or double values.
       num should correspond to the correct bind. */
    void set(int num,int bind,const float32 *);
    void set(int num,int bind,const float64 *);
    /* Resets the structure to a default state.
     */
    void Reset(void);

    // Print out debugging info
    bool Print(trpgPrintBuffer &) const;
};

/*  The trpgGeometry class represents a low level geometry node within the
    TerraPage "scene graph".  This is where the triangles (or quads, polygons, etc...)
    are actually kept.  If you're writing a TerraPage reader, you'll encounter a
    lot of these nodes.  If you're doing a writer, you'll need to construct them.
    You can use a trpgwGeomHelper to aid you in this process.
    We use data arrays to store lists of vertices, colors, texture coordinates, and
    normals.  These data arrays correspond pretty  closely to the respective OpenGL
    equivalents.

    In general, you'll want to do a GetPrimType() to figure out what primitive
    type (PrimType) a given node is holding.  It will typically be TriStrips,
    TriFans, or Triangles, but all the other types are valid as well.
    The next step is to get the vertices via a GetVertices() call and then get
    the normals and texture coordinates (via GetNormals() and GetTexCoordSet() calls).
    To get the material information call GetNumMaterial() (if you can support more
    than one texture per polygon) and then GetMaterial() for each material.  If you
    only support one material/texture per polygon then just do one GetMaterial() all.
    There's always guaranteed to be at least one material.

    It's a good idea to review the OpenGL specification for triangle arrays and
    such before diving into this structure.
    {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgGeometry : public trpgReadWriteable
{
public:
    trpgGeometry(void);
    ~trpgGeometry(void);
    typedef enum {Points,LineStrips,LineLoops,Lines,Polygons,TriStrips,
          TriFans,Triangles,QuadStrips,Quads} PrimType;
    // Set the primitive type for the geometry node
    void    SetPrimType(PrimType type);
    /* Some primitive types require lengths.  These include TriStrips and
       TriFans, but not Triangles, for example. */
    void    SetPrimLengths(int,const int *);
    /* Add a primitive length on to the end of the current primLength array.
       Use this if you're adding geometry one vertex at a time.
    */
    void    AddPrimLength(int);
    /* This just increments the number of primitives in the structure.  Use this
       if you're adding geometry one vertex at a time for a Triangle array, for example.
    */
    void    AddPrim(void);
    /* Set the total number of primitives.  You would use this only when the PrimType
       does not require primitive lengths (e.g. Triangles, but not TriStrips).  Use
       SetPrimLengths() or AddPrimLength() in that case.
    */
    void    SetNumPrims(int);  // Only when there are no primitive lengths
    /* This function sets the total number of materials applied to this group
       of geometry.  If you're only using one material, trpgGeometry defaults to
       1, so you can just do a SetMaterial() and ignore this.
    */
    void    SetNumMaterial(int);
    /* Set the given material instance (in this trpgGeometry node) to the given
       material ID.  The material ID is an index into a trpgMatTable.  You would
       need to do a SetNumMaterial() call first, before any number of SetMaterial()
       calls if there is more than one material.  If there is only one material,
       you can do a single SetMaterial(0,MatID) call.
       Negative materials IDs denote local material definitions (e.g. parts of
       a larger, paged image).  They will always be returned as positive by
       GetMaterial, however you must set them as negative so that GetMaterial
       can set its isLocal bit on return.
    */
    void    SetMaterial(int which,int matID,bool isLocal=false);
    /* This is the same as repeated SetMaterial() calls.
     */
    void    SetMaterials(int32 numMat,const int32 *matIDs);
    /* This adds the given material ID to the end of the material list.
       You can use this instead of SetNumMaterial() and SetMaterial().
    */
    int        AddMaterial(int matID);

    // These are primitive types used within the trpgGeometry structure.
    enum {VertexFloat,VertexDouble,NormalFloat,NormalDouble,
      Color,TextureFloat,TextureDouble,EdgeFlag};

    /* Used to tell some of the trpgGeometry methods what kind of
       data they're getting */
    typedef enum {FloatData,DoubleData} DataType;

    /* The SetVertices() methods will set either 32 or 64 bit floating
       point vertex arrays within the trpgGeometry structure.
       The num passed in is the number of vertices, not the number of individual
       floats or doubles (as with GetNumVertex).
    */
    void    SetVertices(int num,const float32 *);
    void    SetVertices(int num,const float64 *);
    /* This method will add a vertex to the end of the appropriate data array
       (either float or double, but never both).  You would use this method
       if you were building up a trpgGeometry structure vertex by vertex.
    */
    void    AddVertex(DataType type,trpg3dPoint &);
    /* Binding type used by colors, normals, and textures (just PerPrim).
     */
    typedef enum {Overall,PerPrim,PerVertex} BindType;
    /* The SetNormals() methods will set either the 32 or 64 bit floating
       point normal arrays within the trpgGeometry structure.
       The num of normals is determined by the bind type.  You should
       either set the 32 or 64 bit floating point arrays, but not both.
       num is the number of individual normals, not float values, unlike
       the GetNumNormal() call.
    */
    void    SetNormals(int num,BindType bind,const float32 *);
    void    SetNormals(int num,BindType bind,const float64 *);
    /* This method is used to add normals one by one of the given type.
       You would use this if you were adding geometry one vertex at a time
       in conjuntion with AddVertex().
    */
    void    AddNormal(DataType type,trpg3dPoint &);
    /* This constant is used to select the type of a color array
       passed to SetColors().
    */
    typedef enum {Ambient,Diffuse,Specular,Emission} ColorType;
    /* This method sets an array of color structures for a trpgGeometry node.
       The num should correspond to the bind type.  You can set as many of
       these color arrays as you like, they're simply stored in an array for
       later use.
    */
    void    SetColors(int num,ColorType type,BindType bind,const trpgColor *);
    /* The SetTexCoords() methods set a list of texture coordinates.  This
       essentially builds a trpgTexData class and pushes it onto the current
       list of texture coordinate sets.  Choose the appropriate method based
       on 32 or 64 bit floating point values.  num should be the number of
       texture coordinates, not the number of floats passed in.

       bind should be PerPrim in all cases.
    */
    void    SetTexCoords(int num,BindType bind,const float32 *);
    void    SetTexCoords(int num,BindType bind,const float64 *);
    /* This is the same as SetTexCoords(0,bind,NULL) */
    void    AddTexCoords(BindType bind);
    /* This method adds a texture coordinate to array n.  You would use
       this if you were adding vertices one at a time.
    */
    void    AddTexCoord(DataType type,trpg2dPoint &,int n=0);
    /* This method adds a texture coordinate to each trpgTexData.  You would use
       this if you were adding vertices one at a time with multiple materials.
    */
    void    AddTexCoord(DataType type,std::vector<trpg2dPoint> &pts);
    /* Edge flags are used by some primitives in OpenGL.  We don't typically
       use them, but they can be read and written with TerraPage.
    */
    void    SetEdgeFlags(int num,const char *);

    /* Returns the primitive type for this trpgGeometry structure.
     */
    bool    GetPrimType(PrimType &type) const;
    /* Number of primitives in this structure.  Primitives are things
       like a single triangle, a triangle strip or fan.  Some primitives
       require a primitive length array.
    */
    bool    GetNumPrims(int &num) const;
    /* This returns the primitive length array.  Some primitive types, like
       TriStrips or TriFans (but not Triangles) require a primitive length
       array to tell you how to break up the vertex/normal/texture/color
       arrays into individual primitives.  The array you pass in must be
       of a certain length (returned by GetNumPrims()) and will only be
       valid for some primitive types.
    */
    bool    GetPrimLengths(int *lenArray) const;
    /* TerraPage supports multiple materials per geometry set.  This method
       tells you how many are applied to this structure.  There will always
       be at least one.
    */
    bool    GetNumMaterial(int &num) const;
    /* Returns the material ID for a material instance.  num is the
       nth material instance.  matId is an index into a trpgMatTable.
       isLocal is true if the given material is local to this tile.
       Local materials are only used to page small pieces of a larger image.
    */
    bool    GetMaterial(int num,int32 &matID,bool &isLocal) const;
    /* This returns num_vertex.  The function returns the length
       of the vertex array dived by 3, which is the total number of
       vertices.
    */
    bool    GetNumVertex(int &num) const;
    /* The GetVertices() methods return a list of vertices in the given
       form (32 or 64 bit floating point).  These functions will convert to
       the appropriate format, just ask for the one you need.
       The length of the vertex array is determined by GetNumVertex(), which
       returns 3* the number of vertices.
    */
    bool    GetVertices(float32 *) const;
    bool    GetVertices(float64 *) const;
    /* This method lets you get an individual vertex.  The number of vertices
       can be determined by GetNumVertex()/3.
    */
    bool    GetVertex(int id,trpg3dPoint &) const;
    /* GetNumNormal() returns the number of normals.  See GetNumVertex()
       for an explanation of why.
    */
    bool    GetNumNormal(int &num) const;
    /* Much, like GetVertices(), these methods will copy the contents of
       the normal array into the array passed in.  They will convert the
       contents to the appropriate format (32 or 64 bit floating point).
       The length of the input array can be determined by GetNumNormal().
    */
    bool    GetNormals(float32 *) const;
    bool    GetNormals(float64 *) const;
    /* This returns the number of color sets in the trpgGeometry structure.
       There can be one color set per ColorType.  Color sets are either
       per vertex, per primitive, or per geometry unit (i.e. only one)
       arrays of color data.
    */
    bool    GetNumColorSets(int &num) const;
    /* This method fills out the trpgColorInfo class that you pass in.
       The number of color sets is determined by GetNumColorSets and
       the structure you passed can be reused (to save memory).
    */
    bool    GetColorSet(int id,trpgColorInfo *) const;
    /* Returns the number of texture coordinate sets.  There could (and
       should) be one texture coordinate set per material applied to this
       geometry.  Tex Coord sets can be retrieved with GetTexCoordSet.
    */
    bool    GetNumTexCoordSets(int &) const;
    /* This method returns the texture coordinate set specified by the given
       index.  GetNumTexCoordSets determines the total number of texture
       coordinate sets.
    */
    bool    GetTexCoordSet(int id,trpgTexData *) const;
    /* This method returns this trpgGeometry's texture coordinate set specified by the given
       index.  GetNumTexCoordSets determines the total number of texture
       coordinate sets.
    */
    const trpgTexData *GetTexCoordSet(int id) const;
    /* Returns the number of edge flags in this geometry node.
       Edge flags are used on certain primitive types, but are rather rare.
    */
    bool    GetNumEdgeFlag(int &num) const;
    /* This returns the edge flags for the current set of primitives.
     */
    bool    GetEdgeFlags(char *) const;

    // Returns true if the trpgGeometry structure is valid
    bool    isValid(void) const;
    // Resets the contents back to empty
    void    Reset(void);

    // Write self to a buffer
    bool    Write(trpgWriteBuffer &);
    // Read self from a buffer.  Check isValid() afterwards
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

protected:
    int primType;
    int numPrim;
    std::vector<int> primLength;
    std::vector<int> materials;
    std::vector<float> vertDataFloat;
    std::vector<double> vertDataDouble;
    int normBind;
    std::vector<float> normDataFloat;
    std::vector<double> normDataDouble;
    std::vector<trpgColorInfo> colors;
    std::vector<trpgTexData> texData;
    std::vector<char> edgeFlags;
};

/* This is a standard Group that you might see in any reasonable
   scene graph.  It holds a number of children.  TerraPage isn't
   actually a scene graph, it just represents one.  That means that there
   are no pointers to children here.  Instead you'll encounter this group
   while reading a terrain tile or model.  That tells you to create a group
   (or whatever you call it) in your own system and get read to dump child
   trees into it.  A push will follow this object, then the children (however
   deep they may be) then a pop.

   All groups have IDs.  These IDs are unique among groups and group-like things
   (i.e. LODs) and are used to hook trpgAttach geometry into a scene graph hierarchy
   as you page in higher terrain levels of detail.

   If you're doing a TerraPage reader, the group tells you to put together
   your generic container in a scene graph and get read for the push/children/pop.
   The NumChild field should tell you how many children will follow, but a writer
   can easily forget to put it, so be wary.  You'll also want to look at the group
   ID and build a mapping from that ID (look at the max group ID in trpgHeader) to
   your own group structure.  A trpgAttach is allowed to page itself into any group.

   If you're doing a TerraPage writer you'll create one of these, fill out the
   numChild hint, the group ID and then write it.  You'll then write a Push, then
   the children hierarchies (which can be anything) followed by a Pop.  You'll want
   to keep track of the group ID you assigned in case one of the children is a
   pageable chunk of terrain hierarchy.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgGroup : public trpgReadWriteable
{
public:
    trpgGroup(void);
    virtual        ~trpgGroup(void);
    // Resets the contents back to empty
    virtual void    Reset(void);

    /* Set the total number of children this group will have */
    virtual void    SetNumChild(int);
    // Starting from 0, increments the number of children
    virtual int        AddChild(void);
    /* The writer is supposed to set this value to the number of
       children. */
    virtual bool    GetNumChild(int &) const;

    /* Set the group ID */
    virtual void    SetID(int);
    // Returns the group ID
    virtual bool    GetID(int &) const;

    /* Set the group name */
    void    SetName(const char* );
    // Returns the group name
    const char*        GetName(void) const;

    // Validity check
    virtual bool    isValid(void) const;
    // Writes this class to a write buffer
    virtual bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool        Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool        Print(trpgPrintBuffer &) const;

protected:
    int id;
    int numChild;
    char* name;
};

/* Structurally, an attach is just like a trpgGroup.  It does everything a group
   does, plus a little bit more.  The attach node is how TerraPage does terrain
   database paging across LODs.

   In TerraPage we don't enfoced a terrain LOD structure.  Let's say you built
   your database as a quad-tree.  That is, there is one root tile per block,
   4 children, each of which has 4 of its own children etc...  That would imply
   a certain structure (4 children per tile until you reach the bottom).  That would
   also lock you into a database scheme (quad-tree).  However, let's assume that
   someone else wanted to do replacement LOD for their terrain.  That scheme works
   by having one child per tile.  If you want to support both of these then you're
   asking the reader to do a lot of thinking and you can pretty much assume that the
   one you don't test won't work.  We decided to avoid all that by coming up with a
   generic scene graph paging scheme.  It's a little more painfull, but once you deal
   with it you get whatever weird scheme the writer is using for free without having to
   think about it.

   Here's how it works.  Each trpgGroup and group-like structure (trpgLod for example)
   has a unique group ID.  You can have one trpgAttach at the start of a given terrain
   tile.  That trpgAttach can point to any group within the scene graph (with a group ID).
   Level of detail for the terrain is controlled by trpgLod nodes as if everything was
   loaded in all the time.  That is, you'll see the same thing no matter whether every node
   is loaded into memory or just the nearby ones.  The theoretical scene graph structure
   is the same no matter what.  It's the ranges in your trpgHeader that tell you when
   things ought to be loaded in, but the actual display logic is contained within the trpgLod
   objects.  It sounds complicated and it is... for the writer.  But for the reader it's
   really simple.

   If you're doing a TerraPage reader all you'll need to do is keep a mapping from group
   ID's to your own internal scene graph node representation.  Then when a trpgAttach shows
   up you can turn it into your own group-like thing and stuff it and its children into
   the scene graph.  When it wanders out of range (the trpgHeader tells you that for a given
   terrain LOD) you simply delete it.  If you out-run your paging you've got two options:
   (1) Display holes.  That's what will happen when the LOD above a given tile trpgAttach
   turns on without the children in memory; or (2) Don't switch LODs that don't have all
   their children loaded in yet.  Be aware that a trpgAttach inherits from trpgGroup and
   so can have trpgAttach children.  So don't absorb the trpgAttach even though it's extra
   hierarchy.  Also, don't make any assumptions that there is really terrain in a given
   tile.  The writer may have chosen to page buildings or trees.  You never know and there's
   no reason to assume.

   If you're doing a TerraPage writer this is slightly more complex than writing a normal
   format, depending on the structure of your internal scene graph.  If you don't want
   to write more than one pageable terrain LOD you can just ignore trpgAttach nodes.  This
   doesn't mean you can only have one terrain LOD, it only means they won't be pageable.
   If you do want to fully support it, what you'll need to
   do is give all your groups (or whatever will become groups) unique IDs, keeping in mind
   to update the trpgHeader max group ID as you go.  Start at the lowest terrain lod.  This
   one doesn't need to have a trpgAttach node since it's at the top.  Traverse toward
   the higher lods.  When you hit one, spit out a trpgAttach, giving it the group ID of
   the trpgGroup directly above it.  Then treat the node you just created as a trpgGroup
   (i.e. do its children as normal).  You will also need to keep the trpgTileHeader for
   each tile around.  It's best to index these by (x,y,lod) index.  You'll need to build
   that tile header up *just for this tile geometry*.  That means you have to stop adding
   material/model references when you start defining the next tile.  Depending on how you
   write out your scene graph it may make sense to keep a stack of trpgTileHeader and
   trpgMemWriteBuffer objects around indexed by tile (x,y,lod).

   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgAttach : public trpgGroup
{
public:
    trpgAttach(void);
    ~trpgAttach(void);
    // Resets the contents back to empty
    void    Reset(void);

    // Set the parent of the group/LOD/whatever to attach to when loaded in
    void    SetParentID(int);
    // Retrieve the parent ID we'll need to attach this thing into the scene graph
    bool    GetParentID(int &) const;

    /* The writer is supposed to set this value to a unique position with relation
       to its parent group. */
    void    SetChildPos(int);
    /* The child position is a hint as to which child this is in its parent group.
       That is, if there are 3 children, of which this is one, then it could be
       at child position 0, 1, or 3 in its parent.  You can safely ignore this if
       you want to just this node to its parent's list of children. */
    bool    GetChildPos(int &) const;

    // Validity check
    bool    isValid(void) const;
    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

protected:
    int parentID,childPos;
};

TX_EXDECL class TX_CLDECL trpgChildRef : public trpgReadWriteable
{
public:
    trpgChildRef();
    ~trpgChildRef();
    // Resets the contents back to empty
    void    Reset();

    // Set the tile grid location
    void SetTileLoc(int gx,int gy,int glod);

    // Get the tile grid location
    bool GetTileLoc(int &gx,int &gy,int &glod) const;


    void SetTileAddress(const trpgwAppAddress& gAddr);
    void SetTileAddress(int32 file, int32 offset);
    void SetTileZValue( float gZmin, float gZmax);
    bool GetTileAddress(int32& file, int32& offset) const;
    bool GetTileAddress(trpgwAppAddress& gAddr) const;
    bool GetTileZValue( float& gZmin, float& gZmax) const;


    // Validity check
    bool    isValid(void) const;
    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

protected:
    // Grid Location
    int x,y,lod;
    // File Location
    trpgwAppAddress addr;

    float zmin, zmax;

};

/* The billboard inherits from the standard trpgGroup.  It tells the reader that
   everything underneath this node is supposed to be treated like a stamp or billboard
   (depending on your terminology).  That means it's supposed to be rotated towards
   the eye point in some way.  There are no restrictions on the number, depth, or type
   of children.  In theory you could have another billboard as a child, although we have
   no idea what that should look like.

   If you're doing a TerraPage reader treat everything underneath this group as rotatable.
   Pay attention to the Type in particular.  There's a shorthand for rotating a bunch
   of objects that is a little confusing.

   If you're doing a TerraPage write this is pretty simple.  For the standard tree example
   use one of these with one or more trpgGeometry children.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgBillboard : public trpgGroup
{
public:
    trpgBillboard(void);
    ~trpgBillboard(void);
    enum {Individual,Group};
    // Set the type.  See GetType for details.
    void    SetType(int);
    // Set the center.
    void    SetCenter(const trpg3dPoint &);
    enum {Axial,World,Eye};
    // Set the rotation mode.
    void    SetMode(int);
    // Set the rotation axis if mode == Axial
    void    SetAxis(const trpg3dPoint &);

    /* The type controls how the billboard node relates to its children.  There
       are two modes: (1) Group - This is the obvious one.  Everything below
       this node rotates around the center in the way specified by GetMode.  (2) Individual - This
       is a little weirder.  Basically, it's here for trees.  It's assumed that
       there will be one or more trpgGeometry nodes below this node.  Each single
       primitive is supposed to rotate "separately".  That is, you must take into
       account the unique center of each one and rotate it around that.  If you have
       some optimization scheme where you can deal with groups of billboards (ala Performer)
       it is valid to do so in the Individual case. */
    bool    GetType(int &) const;
    /* Center of the thing to be rotated.  For Group this does the obvious thing.
       For Individual it should be the center of the group of things you want to rotate.
       This has no use if you're going to rotate each primitive separately, but if you've
       got some sort of optimized scheme for doing so (ala Performer) this information is useful.
    */
    bool    GetCenter(trpg3dPoint &) const;
    /* The mode will be one of: (1) Axial - rotate around the Axis.  This is the normal
       one for tree. (2) Eye - Always rotate toward the eye point.  (3) world.
    */
    bool    GetMode(int &) const;
    /* The axis used when GetMode returns Axial. */
    bool    GetAxis(trpg3dPoint &) const;

    // Resets the contents back to empty
    void    Reset(void);

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;
protected:
    int type;
    int mode;
    trpg3dPoint center;
    trpg3dPoint axis;
};

/* TerraPage level of detail nodes are pretty simple.  Even though they don't inherit from trpgGroup,
   they have many of the same calls and act, structurally at least, in much the same way.  These
   act as a switch.  When the user's eye point is within a distance then the children of this
   node should be turned on for display.  Otherwise, the children will be invisible.

   A simple on/off test for a TerraPage lod might look like this:
   If ( in < dist < out || out < dist < in) then
   Turn children on
   else
   Turn children off.

   There is also a transition width can be used to fade LODs in and out around
   the transition zones.  Keep in mind that these LODs are binary.  Children
   are either on or off (in the process of being turned off).  The position of
   a child doesn't have any special meaning with respect to range.

   If you're doing a TerraPage reader you'll need to turn this into your own LOD
   structure.  Keep in mind that trpgAttach nodes can and do attach to trpgLod
   nodes.  If you have a general purpose LOD in your scene graph this should be
   pretty easy.  However, you must have implemented the concept of LOD center and
   you definitely should *not* recalculate the LOD center yourself based on the
   center of child geometry.  They may not be the same.  In fact, many terrain
   LOD schemes depend on them not being the same.

   If you're doing a TerraPage writer you'll need to use these both for geometry
   that you want to disappear at certain distances (e.g. trees, houses, etc..), but
   also terrain.  Any terrain LOD scheme you implement must use these to drop out
   polygons in the distance.  You'll need to set the center and in/out/width info.
   Otherwise it's like a group.

   In TerraPage 2.0 we added the Range Index field.  This field is here to help
   run-time systems gracefully degrade visual fidelity.  The trpgLod node still acts
   like a normal LOD and you can safely ignore the range index field.  If, however,
   you want to use it, please read the documentation on the trpgRangeTable for further
   information.

   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgLod : public trpgReadWriteable
{
public:
    trpgLod(void);
    ~trpgLod(void);
    // Set the calculated center
    void    SetCenter(const trpg3dPoint &);
    // Set the number of children hint
    void    SetNumChild(int);
    // Set the LOD information
    void    SetLOD(double in,double out,double width);

    // Center of this LOD.  Distance from the viewer is calculated from this.
    bool    GetCenter(trpg3dPoint &) const;
    // Number of children hint.
    bool    GetNumChild(int &) const;
    // LOD specific information.  in and out can be switched.  width is
    // the transition range for doing fading.
    bool    GetLOD(double &in,double &out,double &width) const;

    // Set the group ID
    void    SetID(int);

    // Group IDs are used here the same way as in trpgGroup
    bool    GetID(int &) const;

    /* Set the lod name */
    void    SetName(const char* );
    // Returns the lod name
    const char* GetName(void) const;

    // Set the range Index
    void    SetRangeIndex(int ri);
    // Get the range index
    bool    GetRangeIndex(int &ri) const;

    // Resets the contents back to empty
    void    Reset(void);

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

protected:
    int numRange;
    double switchIn,switchOut,width;
    trpg3dPoint center;
    int id;
    char* name;
    int rangeIndex;
};

/* Layers are used to draw subface geometry.  That is, geometry that is
   coplanar.  This object should be treated just like a group otherwise.
   Its existence implies the layering effect.  There is no other associated
   information.

   If you're doing a TerraPage reader you should assume that each child,
   starting at 0 should be draw one after the other using whatever subfacing
   scheme you support.  There are no restrictions on what the children may
   be, but we strongly recommend that writers keep this simple.  Keep in
   mind that trpgAttach nodes can legally appear as children.  If you can pull
   it off this has a rather nice effect (think strips on runways).  If not,
   don't sweat it.

   If you're doing a TerraPage writer, this is fairly simple.  Obey the ordering
   constraints and try to keep this simple.  Ideally that would mean just a few
   trpgGeometry nodes below this node.  Also keep in mind that layering works
   very poorly on most OpenGL systems.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgLayer : public trpgGroup
{
public:
    trpgLayer(void);
    ~trpgLayer(void);
    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    // Resets the contents back to empty
    void    Reset(void);
protected:
};

/* This is pretty much a standard 4x4 static transform.  It has a matrix
   which controls where its children wind up in 3D.  Otherwise it acts just
   like a trpgGroup.
   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgTransform : public trpgGroup
{
public:
    trpgTransform(void);
    ~trpgTransform(void);

    // Set the 4x4 matrix
    void    SetMatrix(const float64 *);

    // Get the 4x4 matrix
    bool    GetMatrix(float64 *) const;

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    // Resets the contents back to empty
    void    Reset(void);
protected:
    float64 m[4][4];
};

/* TerraPage treats model references pretty much like instances.  Models
   are organized centrally in a trpgModelTable.  This class simply points
   into there with a model ID.  There is also a 4x4 matrix (ala trpgTransform)
   which moves the model to its final location.

   If you're doing a TerraPage reader you should already have dealt with the
   trpgModelTable by this point.  Presumably you've got a mapping from model IDs
   to small scene graphs in your own representation.  This can be treated just like
   an instance into one of those.

   If you're doing a TerraPage writer this is pretty simple.  When you encounter
   a model (external reference) add it to your trpgModelTable and stuff the resulting
   model ID into one of these.  Stick that trpgModelRef into your tile data stream.
   You'll need to fill out the matrix to scale/translate/rotate it as well.
   The model is assumed to be spatially within the tile it's written into.  That isn't
   enforced, though.

   {group:Read/Write Classes}
*/
TX_EXDECL class TX_CLDECL trpgModelRef : public trpgReadWriteable
{
public:
    trpgModelRef(void);
    ~trpgModelRef(void);
    // Set the model ID.  Must come from a trpgModelTable
    void    SetModel(int);
    // Set the 4x4 rotate/translate/scale matrix
    void    SetMatrix(const float64 *);

    // Model ID pointing into a trpgModelTable
    bool    GetModel(int32 &) const;
    // Positional matrix.  Works just like a trpgTransform.
    bool    GetMatrix(float64 *) const;

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    // Resets the contents back to empty
    void    Reset(void);
protected:
    int modelRef;
    float64 m[4][4];
};

/* The Text Styles are used to consolidate attributes related to labels.
   It would be inefficient to store the font, for instance, for every
   single label, so we do it in the Text Style table.
   For the most part, text styles are very simple, consisting of
   very basic font and material information.
*/
TX_EXDECL class TX_CLDECL trpgTextStyle : public trpgReadWriteable
{
public:
    trpgTextStyle(void);
    ~trpgTextStyle(void);

    bool operator == (const trpgTextStyle&) const;

    // Set the material ID.  Should point into trpgMatTable
    void    SetMaterial(int);
    // Get the material ID.  Points into trpgMatTable
    int        GetMaterial(void) const;

    // Set the font description
    void    SetFont(std::string &);
    // Get the font description
    const std::string *GetFont(void) const;

    // Set bold to on/off
    void    SetBold(bool);
    // Return value of bold status
    bool    GetBold(void) const;

    // Set italic on/off
    void    SetItalic(bool);
    // Return value of italic status
    bool    GetItalic(void) const;

    // Set underline on/off
    void    SetUnderline(bool);
    // Return value of underline status
    bool    GetUnderline(void) const;

    // Set character size: MUST BE IN METER
    void    SetCharacterSize(float32);
    // Get character (e.g. font) size
    float32    GetCharacterSize(void) const;

    // Write this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    // Reset the contents back to empty
    void    Reset(void);

    // Return object validity
    bool isValid(void) const;
protected:
    std::string font;
    bool bold,italic,underline;
    float32 characterSize;
    int matId;
};

/* The text style table is just a collection of trpgTextStyle obejcts for the
   whole archive.  Individual text styles will be indexed by trpgLabelProperty
   objects.  It is from these that you decide how to display a given label.
*/
TX_EXDECL class TX_CLDECL trpgTextStyleTable : public trpgReadWriteable
{
public:
    trpgTextStyleTable(void);
    ~trpgTextStyleTable(void);

    // Add a single text style
    int        AddStyle(const trpgTextStyle &);

    int        FindAddStyle(const trpgTextStyle &);

    // Figure out how many text styles there are
    int        GetNumStyle(void) const;

    // Return a reference to the given style
    const trpgTextStyle *GetStyleRef(int) const;

    // Validity check
    bool    isValid(void) const;
    // Reset the contents back to empty
    void    Reset(void);

    // Write this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Read this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Print this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;
    typedef std::map<int,trpgTextStyle> StyleMapType;
    const StyleMapType *getStyleMap()const  { return &styleMap; }
protected:
    //std::vector<trpgTextStyle> styles;

    StyleMapType styleMap;
};



/* The Support Styles are used to consolidate attributes related to labels.
   For the most part, support styles are very simple, consisting of
   very basic description of object to use for drawing support.
*/
TX_EXDECL class TX_CLDECL trpgSupportStyle : public trpgReadWriteable
{
public:
    trpgSupportStyle(void);
    ~trpgSupportStyle(void);

    bool operator == (const trpgSupportStyle&) const;

    typedef enum {Line,Cylinder,MaxSupportType} SupportType;

    // Set the support type.  Get GetSupportType() for more information
    void    SetType(SupportType);
    /* Return the support types.  Supports are geometry that run from the
       bottom middle of the label object to some point in space.
       Most likely that point is somewhere on the ground, but not
       necessarily.  How the support is drawn is controlled both by
       the support type and the material ID for the support.
    */
    SupportType    GetType(void) const;

    // Set the material ID.  Should point into trpgMatTable
    void    SetMaterial(int);
    // Get the material ID.  Points into trpgMatTable
    int        GetMaterial(void) const;


    // Write this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    // Reset the contents back to empty
    void    Reset(void);

    // Return object validity
    bool        isValid(void) const;
protected:
    SupportType type;
    int matId;
};

/* The support style table is just a collection of trpgSupportStyle obejcts for the
   whole archive.  Individual support styles will be indexed by trpgLabelProperty
   objects.  It is from these that you decide how to display a given support.
*/
TX_EXDECL class TX_CLDECL trpgSupportStyleTable : public trpgReadWriteable
{
public:
    trpgSupportStyleTable(void);
    ~trpgSupportStyleTable(void);

    // Add a single text style
    int        AddStyle(const trpgSupportStyle &);

    int        FindAddStyle(const trpgSupportStyle &);

    // Figure out how many text styles there are
    int        GetNumStyle(void) const;

    // Return a reference to the given style
    const trpgSupportStyle *GetStyleRef(int) const;

    // Validity check
    bool    isValid(void) const;
    // Reset the contents back to empty
    void    Reset(void);

    // Write this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Read this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Print this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

protected:
    //std::vector<trpgSupportStyle> styles;
    typedef std::map<int,trpgSupportStyle> SupportStyleMapType;
    SupportStyleMapType supportStyleMap;
};



/* The Label property are used to consolidate attributes related to labels.
   Label properties are very simple, consisting of
   basic font style and support properties.
*/
TX_EXDECL class TX_CLDECL trpgLabelProperty : public trpgReadWriteable
{
public:
    trpgLabelProperty(void);
    ~trpgLabelProperty(void);

    typedef enum {VertBillboard,Billboard,Panel,Cube,MaxLabelType} LabelType;

    bool operator == (const trpgLabelProperty&) const;

    // Set the label type.  See GetType() for more information.
    void    SetType(LabelType);
    /* Return the label type.  This controls the geometry for the label.
       Panel labels are simple polygons.  Cube labels should display
       the text on every side with single sided polygons.
       Billboard labels rotate toward the user with no particular axes.
       That is, they will always be pointed directly toward the user.
       Vertical billboards rotate toward the user, but have an axis along +Z.
    */
    LabelType    GetType(void) const;

    // Set the font style ID.  Should point into trpgTextStyleTable
    void    SetFontStyle(int);
    // Get the font style ID.  Points into trpgTextStyleTable
    int        GetFontStyle(void) const;

    // Set the support style ID.  Should point into trpgSupportTable
    void    SetSupport(int);
    // Get the font style ID.  Points into trpgSupportTable
    int        GetSupport(void) const;

    // Write this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    // Reset the contents back to empty
    void    Reset(void);

    // Return object validity
    bool isValid(void) const;
protected:
    int fontId;
    int supportId;
    LabelType type;
};

/* The label property table is just a collection of trpgLabelProperty obejcts for the
   whole archive.  Individual label property will be indexed by trpgLable
   objects.
*/
TX_EXDECL class TX_CLDECL trpgLabelPropertyTable : public trpgReadWriteable
{
public:
    trpgLabelPropertyTable(void);
    ~trpgLabelPropertyTable(void);

    // Add a single label property
    int        AddProperty(const trpgLabelProperty &);

    // Find or Add a single label property
    int FindAddProperty(const trpgLabelProperty& property);

    // Figure out how many properties there are
    int        GetNumProperty(void) const;

    // Return a reference to the given property
    const trpgLabelProperty *GetPropertyRef(int) const;

    // Validity check
    bool    isValid(void) const;
    // Reset the contents back to empty
    void    Reset(void);

    // Write this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Read this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Print this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

protected:
    //std::vector<trpgLabelProperty> properties;
    typedef std::map<int,trpgLabelProperty> LabelPropertyMapType;
    LabelPropertyMapType labelPropertyMap;
};

/* Labels are objects that float above the terrain (usually) and display
   some text message to the user.  They're primarily used in 3D map sort of
   application.  In other words, they're not intended to be real world objects.
   You would use one if you want to float a message such as "Natural History Museum"
   over a specific building in a visual database.
*/
TX_EXDECL class TX_CLDECL trpgLabel : public trpgReadWriteable {
public:
    trpgLabel(void);
    ~trpgLabel(void);

    bool isValid(void) const;


    typedef enum {Left,Center,Right,MaxAlignmentType} AlignmentType;

    // Set the label property ID. This is an index into a tprgLabelPropertyTable
    void    SetProperty(int);
    int        GetProperty() const;



    // Set the text for this label.  See GetText() for more information
    void    SetText(const std::string &);
    /* Return the text for this label.  The text may contain basic formatting
       such as newlines and tabs.  Expect to see those as \n and \t respectively.
       Eventually, this text might contain HTML formatting, but we do not use
       that at present.
       Should not be empty.
    */
    const std::string *GetText(void) const;

    // Set the text alignment.  See GetAlignmentType() for more information.
    void    SetAlignment(AlignmentType);
    /* Return the alignment type.  This controls the alignment of the text
       with respect to the label geometry.
    */
    AlignmentType    GetAlignment(void) const;

    // Set the number of spaces between tabs
    void    SetTab(int);
    // Get the number of spaces between tabs
    int        GetTab(void) const;

    // Set the text scaling value: font size * scale = real world text size in meter
    void        SetScale(float32);
    float32     GetScale() const;

    // Set the text thickness: use to draw 3D text. if thickness = 0, we have
    // 2D text only.
    void        SetThickness(float32);
    float32     GetThickness() const;

    // Set the description for this label.  Set GetDesc() for more information
    void    SetDesc(const std::string &);
    /* Return the description for this label.  Descriptions should contain no
       formatting.  They are a description of what the label represents and may
       appear if a user interacts with the label, but should not be drawn in 3D.
       May be empty.
    */
    const std::string *GetDesc(void) const;

    // Set the URL for this label.  See GetURL() for more information.
    void    SetURL(const std::string &);
    /* Return the URL for this label.  The URL would invoke some sort of
       web browser if the user of the 3D application clicks on the label.
       May be empty.
    */
    const std::string *GetURL(void) const;

    // Set the number of spaces between tabs
    void    SetTabSize(int);
    // Get the number of spaces between tabs
    int        GetTabSize(void) const;

    // Set the location of the label.
    void SetLocation(const trpg3dPoint &);
    const trpg3dPoint& GetLocation() const;

    // Add a support.  See GetSupports() for more information
    void    AddSupport(const trpg3dPoint &);
    /* Return the support array.  Supports are linear features that run from
       the middle bottom of the label to some set of points in the database.
       These will often be on the ground, but need not be.  Support display
       is controlled by the SupportType and by the Support Material ID.
    */
    const std::vector<trpg3dPoint> *GetSupports(void) const;

    // Writes this class to a write buffer
    bool    Write(trpgWriteBuffer &);
    // Reads this class from a read buffer
    bool    Read(trpgReadBuffer &);
    // Prints this class to a print buffer
    bool    Print(trpgPrintBuffer &) const;

    // Resets the contents back to empty
    void    Reset(void);

protected:
    int propertyId;
    std::string text;  // Actual label text.  May contain formatting
    AlignmentType alignment;
    int tabSize;
    float32 scale;
    float32 thickness;
    std::string desc;
    std::string url;
    trpg3dPoint location;
    std::vector<trpg3dPoint> supports;
};

#endif
