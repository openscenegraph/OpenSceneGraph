#ifndef __VBSP_READER_H_
#define __VBSP_READER_H_


#include <osg/Geometry>
#include <osg/Matrixd>
#include <osg/Node>
#include <osg/Object>
#include <osg/StateSet>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osg/Referenced>


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


struct Plane
{
    osg::Vec3f   plane_normal;
    float        origin_dist;
    int          type;
};


struct Edge
{
    unsigned short   vertex[2];
};


struct Face
{
    unsigned short   plane_index;
    unsigned char    plane_side;
    unsigned char    on_node;
    int              first_edge;
    short            num_edges;
    short            texinfo_index;
    short            dispinfo_index;
    short            surface_fog_volume_id;
    unsigned char    styles[4];
    int              light_offset;
    float            face_area;
    int              lightmap_texture_mins_in_luxels[2];
    int              lightmap_texture_size_in_luxels[2];
    int              original_face;
    unsigned short   num_primitives;
    unsigned short   first_primitive_id;
    unsigned int     smoothing_groups;
};


struct TexInfo
{
    float   texture_vecs[2][4];
    float   lightmap_vecs[2][4];
    int     texture_flags;
    int     texdata_index;
};


struct TexData
{
    osg::Vec3f   texture_reflectivity;
    int          name_string_table_id;
    int          texture_width;
    int          texture_height;
    int          view_width;
    int          view_height;
};


struct DisplaceSubNeighbor
{
    unsigned short   neighbor_index;
    unsigned char    neighbor_orient;
    unsigned char    local_span;
    unsigned char    neighbor_span;
};


struct DisplaceNeighbor
{
    DisplaceSubNeighbor   sub_neighbors[2];
};


struct DisplaceCornerNeighbor
{
    unsigned short   neighbor_indices[4];
    unsigned char    neighbor_count;
};


struct DisplaceInfo
{
    osg::Vec3f               start_position;
    int                      disp_vert_start;
    int                      disp_tri_start;
    int                      power;
    int                      min_tesselation;
    float                    smooth_angle;
    int                      surface_contents;
    unsigned short           map_face;
    int                      lightmap_alpha_start;
    int                      lightmap_sample_pos_start;
    DisplaceNeighbor         edge_neighbors[4];
    DisplaceCornerNeighbor   corner_neighbors[4];
    unsigned long            allowed_verts[10];
};


struct DisplacedVertex
{
    osg::Vec3f   displace_vec;
    float        displace_dist;
    float        alpha_blend;
};


class VBSPReader
{
protected:

    std::string                map_name;

    osg::ref_ptr<osg::Node>    root_node;

    char **                    entity_list;
    int                        num_entities;

    Plane *                    plane_list;
    int                        num_planes;

    osg::Vec3f *               vertex_list;
    int                        num_vertices;

    Edge *                     edge_list;
    int                        num_edges;

    int *                      surface_edges;
    int                        num_surf_edges;

    Face *                     face_list;
    int                        num_faces;

    TexInfo *                  texinfo_list;
    int                        num_texinfo_entries;

    TexData *                  texdata_list;
    int                        num_texdata_entries;

    int *                      texdata_string_table;
    int                        num_texdata_string_table_entries;

    char **                    texdata_string_data;
    int                        num_texdata_strings;

    DisplaceInfo *             dispinfo_list;
    int                        num_dispinfo_entries;

    DisplacedVertex *          displaced_vertex_list;
    int                        num_displaced_vertices;
   

    void   writeBlanks(int count) const;

    void   processEntities(std::istream & str, int offset, int length);
    void   processPlanes(std::istream & str, int offset, int length);
    void   processVertices(std::istream & str, int offset, int length);
    void   processEdges(std::istream & str, int offset, int length);
    void   processSurfEdges(std::istream & str, int offset, int length);
    void   processFaces(std::istream & str, int offset, int length);
    void   processTexInfo(std::istream & str, int offset, int length);
    void   processTexData(std::istream & str, int offset, int length);
    void   processTexDataStringTable(std::istream & str, int offset,
                                     int length);
    void   processTexDataStringData(std::istream & str, int offset,
                                    int length);
    void   processDispInfo(std::istream & str, int offset, int length);
    void   processDispVerts(std::istream & str, int offset, int length);

    std::string       getToken(std::string str, const char * delim,
                               std::string::size_type & index);

    std::string       findFileIgnoreCase(std::string filePath);

    osg::ref_ptr<osg::StateSet>   createBlendShader(osg::Texture * tex1,
                                                    osg::Texture * tex2);

    osg::ref_ptr<osg::Texture>    readTextureFile(std::string file);
    osg::ref_ptr<osg::StateSet>   readMaterialFile(std::string file);

    void   createScene();

public:

    VBSPReader();
    virtual ~VBSPReader();
 
    const char *              getEntity(int index) const;
    const Plane &             getPlane(int index) const;
    const osg::Vec3f &        getVertex(int index) const;
    const Edge &              getEdge(int index) const;
    const int                 getSurfaceEdge(int index) const;
    const Face &              getFace(int index) const;
    const TexInfo &           getTexInfo(int index) const;
    const TexData &           getTexData(int index) const;
    const char *              getTexDataString(int index) const;
    const DisplaceInfo &      getDispInfo(int index) const;
    const DisplacedVertex &   getDispVertex(int index) const;

    bool   readFile(const std::string & file);

    osg::ref_ptr<osg::Node>   getRootNode();
};


}

#endif
