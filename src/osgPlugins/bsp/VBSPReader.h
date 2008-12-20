#ifndef __VBSP_READER_H_
#define __VBSP_READER_H_


#include <osg/Geometry>
#include <osg/Node>
#include <osg/Object>
#include <osg/StateSet>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osg/Referenced>

#include "VBSPData.h"

namespace bsp
{


// The magic number for a Valve BSP file is 'VBSP' in little-endian
// order
const int MAGIC_NUMBER = (('P'<<24)+('S'<<16)+('B'<<8)+'V');


enum LumpType
{
    ENTITIES_LUMP,
    PLANES_LUMP,
    TEXDATA_LUMP,
    VERTICES_LUMP,
    VISIBILITY_LUMP,
    NODES_LUMP,
    TEXINFO_LUMP,
    FACES_LUMP,
    LIGHTING_LUMP,
    OCCLUSION_LUMP,
    LEAFS_LUMP,
    UNUSED_LUMP_11,
    EDGES_LUMP,
    SURFEDGES_LUMP,
    MODELS_LUMP,
    WORLD_LIGHTS_LUMP,
    LEAF_FACES_LUMP,
    LEAF_BRUSHES_LUMP,
    BRUSHES_LUMP,
    BRUSH_SIDES_LUMP,
    AREAS_LUMP,
    AREA_PORTALS_LUMP,
    PORTALS_LUMP,
    CLUSTERS_LUMP,
    PORTAL_VERTS_LUMP,
    CLUSTER_PORTALS_LUMP,
    DISPINFO_LUMP,
    ORIGINAL_FACES_LUMP,
    UNUSED_LUMP_28,
    PHYS_COLLIDE_LUMP,
    VERT_NORMALS_LUMP,
    VERT_NORMAL_INDICES_LUMP,
    DISP_LIGHTMAP_ALPHAS_LUMP,
    DISP_VERTS_LUMP,
    DISP_LIGHTMAP_SAMPLE_POS_LUMP,
    GAME_LUMP,
    LEAF_WATER_DATA_LUMP,
    PRIMITIVES_LUMP,
    PRIM_VERTS_LUMP,
    PRIM_INDICES_LUMP,
    PAK_FILE_LUMP,
    CLIP_PORTAL_VERTS_LUMP,
    CUBEMAPS_LUMP,
    TEXDATA_STRING_DATA_LUMP,
    TEXDATA_STRING_TABLE_LUMP,
    OVERLAYS_LUMP,
    LEAF_MIN_DIST_TO_WATER_LUMP,
    FACE_MACRO_TEXTURE_INFO_LUMP,
    DISP_TRIS_LUMP,
    PHYS_COLLIDE_SURFACE_LUMP,
    UNUSED_LUMP_50,
    UNUSED_LUMP_51,
    UNUSED_LUMP_52,
    LIGHTING_HDR_LUMP,
    WORLD_LIGHTS_HDR_LUMP,
    LEAF_LIGHT_HDR_1_LUMP,
    LEAF_LIGHT_HDR_2_LUMP,
    UNUSED_LUMP_57,
    UNUSED_LUMP_58,
    UNUSED_LUMP_59,
    UNUSED_LUMP_60,
    UNUSED_LUMP_61,
    UNUSED_LUMP_62,
    UNUSED_LUMP_63,
    MAX_LUMPS
};


const char LUMP_DESCRIPTION[][64] =
{
    "ENTITIES_LUMP",
    "PLANES_LUMP",
    "TEXDATA_LUMP",
    "VERTICES_LUMP",
    "VISIBILITY_LUMP",
    "NODES_LUMP",
    "TEXINFO_LUMP",
    "FACES_LUMP",
    "LIGHTING_LUMP",
    "OCCLUSION_LUMP",
    "LEAFS_LUMP",
    "UNUSED_LUMP_11",
    "EDGES_LUMP",
    "SURFEDGES_LUMP",
    "MODELS_LUMP",
    "WORLD_LIGHTS_LUMP",
    "LEAF_FACES_LUMP",
    "LEAF_BRUSHES_LUMP",
    "BRUSHES_LUMP",
    "BRUSH_SIDES_LUMP",
    "AREAS_LUMP",
    "AREA_PORTALS_LUMP",
    "PORTALS_LUMP",
    "CLUSTERS_LUMP",
    "PORTAL_VERTS_LUMP",
    "CLUSTER_PORTALS_LUMP",
    "DISPINFO_LUMP",
    "ORIGINAL_FACES_LUMP",
    "UNUSED_LUMP_28",
    "PHYS_COLLIDE_LUMP",
    "VERT_NORMALS_LUMP",
    "VERT_NORMAL_INDICES_LUMP",
    "DISP_LIGHTMAP_ALPHAS_LUMP",
    "DISP_VERTS_LUMP",
    "DISP_LIGHTMAP_SAMPLE_POS_LUMP",
    "GAME_LUMP",
    "LEAF_WATER_DATA_LUMP",
    "PRIMITIVES_LUMP",
    "PRIM_VERTS_LUMP",
    "PRIM_INDICES_LUMP",
    "PAK_FILE_LUMP",
    "CLIP_PORTAL_VERTS_LUMP",
    "CUBEMAPS_LUMP",
    "TEXDATA_STRING_DATA_LUMP",
    "TEXDATA_STRING_TABLE_LUMP",
    "OVERLAYS_LUMP",
    "LEAF_MIN_DIST_TO_WATER_LUMP",
    "FACE_MACRO_TEXTURE_INFO_LUMP",
    "DISP_TRIS_LUMP",
    "PHYS_COLLIDE_SURFACE_LUMP",
    "UNUSED_LUMP_50",
    "UNUSED_LUMP_51",
    "UNUSED_LUMP_52",
    "LIGHTING_HDR_LUMP",
    "WORLD_LIGHTS_HDR_LUMP",
    "LEAF_LIGHT_HDR_1_LUMP",
    "LEAF_LIGHT_HDR_2_LUMP",
    "UNUSED_LUMP_57",
    "UNUSED_LUMP_58",
    "UNUSED_LUMP_59",
    "UNUSED_LUMP_60",
    "UNUSED_LUMP_61",
    "UNUSED_LUMP_62",
    "UNUSED_LUMP_63"
};


struct LumpEntry
{
    int   file_offset;
    int   lump_length;
    int   lump_version;
    char  ident_code[4];
};


struct Header
{
    int        magic_number;
    int        bsp_version;
    LumpEntry  lump_table[MAX_LUMPS];
    int        map_revision;
};


struct GameHeader
{
    int              num_lumps;

    // This is followed by this many GameLump entries (see below)
};

   
struct GameLump
{
    int               lump_id;
    unsigned short    lump_flags;
    unsigned short    lump_version;
    int               lump_offset;
    int               lump_length;
}; 
   
   
// This is the ID for the static prop game lump
const int STATIC_PROP_ID = (('s'<<24)+('p'<<16)+('r'<<8)+'p');


struct StaticPropModelNames
{  
    int    num_model_names;

    // This is followed by this many names, each 128 characters long
};


struct StaticPropLeaves
{
    int   num_leaf_entries;

    // This is followed by this many unsigned shorts, indicating which BSP
    // leaves this prop occupies
};


struct StaticProps
{
    int   num_static_props;

    // This is followed by this many StaticProp entries (see VBSPData.h), note
    // that there are two possible StaticProp versions, depending on the
    // version of the GameLump
};


class VBSPReader
{
protected:

    std::string                map_name;

    VBSPData *                 bsp_data;

    osg::ref_ptr<osg::Node>    root_node;

    char *                     texdata_string;
    int *                      texdata_string_table;
    int                        num_texdata_string_table_entries;


    void   processEntities(std::istream & str, int offset, int length);
    void   processModels(std::istream & str, int offset, int length);
    void   processPlanes(std::istream & str, int offset, int length);
    void   processVertices(std::istream & str, int offset, int length);
    void   processEdges(std::istream & str, int offset, int length);
    void   processSurfEdges(std::istream & str, int offset, int length);
    void   processFaces(std::istream & str, int offset, int length);
    void   processTexInfo(std::istream & str, int offset, int length);
    void   processTexData(std::istream & str, int offset, int length);
    void   processTexDataStringTable(std::istream & str, int offset,
                                     int length);
    void   processTexDataStringData(std::istream & str, int offset, int length);
    void   processDispInfo(std::istream & str, int offset, int length);
    void   processDispVerts(std::istream & str, int offset, int length);
    void   processGameData(std::istream & str, int offset, int length);
    void   processStaticProps(std::istream & str, int offset, int length,
                              int lumpVersion);

    std::string       getToken(std::string str, const char * delim,
                               size_t & index);

    osg::ref_ptr<osg::StateSet>   createBlendShader(osg::Texture * tex1,
                                                    osg::Texture * tex2);

    osg::ref_ptr<osg::Texture>    readTextureFile(std::string file);
    osg::ref_ptr<osg::StateSet>   readMaterialFile(std::string file);

    void   createScene();

public:

    VBSPReader();
    virtual ~VBSPReader();

    bool   readFile(const std::string & file);

    osg::ref_ptr<osg::Node>   getRootNode();
};


}

#endif
