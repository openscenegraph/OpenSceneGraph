#ifndef __VBSP_DATA_H_
#define __VBSP_DATA_H_


#include <osg/Vec3f>
#include <osg/StateSet>
#include <osg/Referenced>
#include <string>


namespace bsp
{


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


struct Model
{
    osg::Vec3f    bound_min;
    osg::Vec3f    bound_max;
    osg::Vec3f    model_origin;
    int           head_node;
    int           first_face;
    int           num_faces;
};


struct StaticPropV4
{
    osg::Vec3f       prop_origin;
    osg::Vec3f       prop_angles;
    unsigned short   prop_type;
    unsigned short   first_leaf;
    unsigned short   leaf_count;
    unsigned char    prop_solid;
    unsigned char    prop_flags;
    unsigned int     prop_skin;
    float            min_fade_dist;
    float            max_fade_dist;

    osg::Vec3f       lighting_origin;
};


struct StaticProp
{
    osg::Vec3f       prop_origin;
    osg::Vec3f       prop_angles;
    unsigned short   prop_type;
    unsigned short   first_leaf;
    unsigned short   leaf_count;
    unsigned char    prop_solid;
    unsigned char    prop_flags;
    unsigned int     prop_skin;
    float            min_fade_dist;
    float            max_fade_dist;

    osg::Vec3f       lighting_origin;
    float            forced_fade_scale;
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
    unsigned int             allowed_verts[10];
};


struct DisplacedVertex
{
    osg::Vec3f   displace_vec;
    float        displace_dist;
    float        alpha_blend;
};


class VBSPData
{
protected:

    typedef std::vector<std::string>   EntityList;
    EntityList                         entity_list;

    typedef std::vector<Model>   ModelList;
    ModelList                    model_list;

    typedef std::vector<Plane>   PlaneList;
    PlaneList                    plane_list;

    typedef std::vector<osg::Vec3f>   VertexList;
    VertexList                        vertex_list;

    typedef std::vector<Edge>   EdgeList;
    EdgeList                    edge_list;

    typedef std::vector<int>   SurfEdgeList;
    SurfEdgeList               surface_edge_list;
 
    typedef std::vector<Face>   FaceList;
    FaceList                    face_list;

    typedef std::vector<TexInfo>   TexInfoList;
    TexInfoList                    texinfo_list;

    typedef std::vector<TexData>   TexDataList;
    TexDataList                    texdata_list;

    typedef std::vector<std::string>   TexDataStringList;
    TexDataStringList                  texdata_string_list;

    typedef std::vector<DisplaceInfo>   DisplaceInfoList;
    DisplaceInfoList                    dispinfo_list;

    typedef std::vector<DisplacedVertex>   DisplacedVertexList;
    DisplacedVertexList                    displaced_vertex_list;

    typedef std::vector<std::string>   StaticPropModelList;
    StaticPropModelList                static_prop_model_list;
   
    typedef std::vector<StaticProp>   StaticPropList;
    StaticPropList                    static_prop_list;
   
    typedef std::vector< osg::ref_ptr<osg::StateSet> >  StateSetList;
    StateSetList                                        state_set_list;
   
public:

    VBSPData();
    virtual ~VBSPData();

    void                      addEntity(std::string & newEntity);
    const int                 getNumEntities() const;
    const std::string &       getEntity(int index) const;

    void                      addModel(Model & newModel);
    const int                 getNumModels() const;
    const Model &             getModel(int index) const;

    void                      addPlane(Plane & newPlane);
    const int                 getNumPlanes() const;
    const Plane &             getPlane(int index) const;

    void                      addVertex(osg::Vec3f & newVertex);
    const int                 getNumVertices() const;
    const osg::Vec3f &        getVertex(int index) const;

    void                      addEdge(Edge & newEdge);
    const int                 getNumEdges() const;
    const Edge &              getEdge(int index) const;

    void                      addSurfaceEdge(int & newSurfEdge);
    const int                 getNumSurfaceEdges() const;
    const int                 getSurfaceEdge(int index) const;

    void                      addFace(Face & newFace);
    const int                 getNumFaces() const;
    const Face &              getFace(int index) const;

    void                      addTexInfo(TexInfo & newTexInfo);
    const int                 getNumTexInfos() const;
    const TexInfo &           getTexInfo(int index) const;

    void                      addTexData(TexData & newTexData);
    const int                 getNumTexDatas() const;
    const TexData &           getTexData(int index) const;

    void                      addTexDataString(std::string & newTexDataString);
    const int                 getNumTexDataStrings() const;
    const std::string &       getTexDataString(int index) const;

    void                      addDispInfo(DisplaceInfo & newDispInfo);
    const int                 getNumDispInfos() const;
    const DisplaceInfo &      getDispInfo(int index) const;

    void                      addDispVertex(DisplacedVertex & newDispVert);
    const int                 getNumDispVertices() const;
    const DisplacedVertex &   getDispVertex(int index) const;

    void                      addStaticPropModel(std::string & newModel);
    const int                 getNumStaticPropModels() const;
    const std::string &       getStaticPropModel(int index) const;

    void                      addStaticProp(StaticPropV4 & newProp);
    void                      addStaticProp(StaticProp & newProp);
    const int                 getNumStaticProps() const;
    const StaticProp &        getStaticProp(int index) const;

    void                      addStateSet(osg::StateSet * stateSet);
    const int                 getNumStateSets() const;
    osg::StateSet *           getStateSet(int index) const;
};


}

#endif
