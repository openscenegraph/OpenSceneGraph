#ifndef __VTX_READER_H_
#define __VTX_READER_H_


#include <osg/Array>
#include <osg/Geometry>
#include <osg/Matrixd>
#include <osg/Node>
#include <osg/Object>
#include <osg/StateSet>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osg/Referenced>

#include "MDLLimits.h"
#include "MDLRoot.h"
#include "VVDReader.h"


namespace mdl
{

struct VTXHeader
{
    int              vtx_version;
    int              vertex_cache_size;
    unsigned short   max_bones_per_strip;
    unsigned short   max_bones_per_tri;
    int              max_bones_per_vertex;

    int              check_sum;
    int              num_lods;

    int              mtl_replace_list_offset;

    int              num_body_parts;
    int              body_part_offset;
};


struct VTXMaterialReplacementList
{
    int   num_replacements;
    int   replacement_offset;
};


struct VTXMaterialReplacment
{
    short   material_id;
    int     replacement_material_name_offset;
};


struct VTXBodyPart
{
    int   num_models;
    int   model_offset;
};


struct VTXModel
{
    int   num_lods;
    int   lod_offset;
};


struct VTXModelLOD
{
    int     num_meshes;
    int     mesh_offset;
    float   switch_point;
};


enum VTXMeshFlags
{
   MESH_IS_TEETH  = 0x01,
   MESH_IS_EYES   = 0x02
};


struct VTXMesh
{
    int             num_strip_groups;
    int             strip_group_offset;

    unsigned char   mesh_flags;
};

// Can't rely on sizeof() because Valve explicitly packs these structures to
// 1-byte alignment in the file, which isn't portable
const int VTX_MESH_SIZE = 9;


enum VTXStripGroupFlags
{
    STRIP_GROUP_IS_FLEXED        = 0x01,
    STRIP_GROUP_IS_HW_SKINNED    = 0x02,
    STRIP_GROUP_IS_DELTA_FLEXED  = 0x04
};


struct VTXStripGroup
{
    int             num_vertices;
    int             vertex_offset;

    int             num_indices;
    int             index_offset;

    int             num_strips;
    int             strip_offset;

    unsigned char   strip_group_flags;
};

// Can't rely on sizeof() because Valve explicitly packs these structures to
// 1-byte alignment in the file, which isn't portable
const int VTX_STRIP_GROUP_SIZE = 25;


enum VTXStripFlags
{
    STRIP_IS_TRI_LIST   = 0x01,
    STRIP_IS_TRI_STRIP  = 0x02
};


struct VTXStrip
{
    int             num_indices;
    int             index_offset;

    int             num_vertices;
    int             vertex_offset;

    short           num_bones;

    unsigned char   strip_flags;

    int             num_bone_state_changes;
    int             bone_state_change_offset;
};


// Can't rely on sizeof() because Valve explicitly packs these structures to
// 1-byte alignment in the .vtx file, which isn't portable
const int VTX_STRIP_SIZE = 27;


struct VTXVertex
{
    unsigned char   bone_weight_index[MAX_BONES_PER_VERTEX];
    unsigned char   num_bones;

    short           orig_mesh_vertex_id;

    char            bone_id[MAX_BONES_PER_VERTEX];
};


// Can't rely on sizeof() because Valve explicitly packs these structures to
// 1-byte alignment in the .vtx file, which isn't portable
const int VTX_VERTEX_SIZE = 9;


struct VTXBoneStateChange
{
    int   hardware_id;
    int   new_bone_id;
};


class VTXReader
{
protected:

    std::string                vtx_name;

    VVDReader *                vvd_reader;

    MDLRoot *                  mdl_root;

    osg::ref_ptr<osg::Node>    model_root;

    osg::ref_ptr<osg::Group>          processBodyPart(std::istream * str,
                                                      int offset,
                                                      BodyPart * currentPart);
    osg::ref_ptr<osg::Group>          processModel(std::istream * str,
                                                   int offset,
                                                   Model * currentModel);
    osg::ref_ptr<osg::Group>          processLOD(int lodNum, float * distance,
                                                 std::istream * str,
                                                 int offset,
                                                 Model * currentModel);
    osg::ref_ptr<osg::Geode>          processMesh(int lodNum,
                                                  std::istream * str,
                                                  int offset, int vertexOffset);
    osg::ref_ptr<osg::Geometry>       processStripGroup(int lodNum,
                                                        std::istream * str,
                                                        int offset,
                                                        int vertexOffset);
    osg::ref_ptr<osg::PrimitiveSet>   processStrip(unsigned short * indexArray,
                                                   std::istream * str,
                                                   int offset);

public:

    VTXReader(VVDReader * vvd, MDLRoot * mdlRoot);
    virtual ~VTXReader();

    bool   readFile(const std::string & file);

    osg::ref_ptr<osg::Node>   getModel();
};


}

#endif
