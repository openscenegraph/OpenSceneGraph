#ifndef __VVD_READER_H_
#define __VVD_READER_H_


#include <osg/Geometry>
#include <osg/Matrixd>
#include <osg/Node>
#include <osg/Object>
#include <osg/StateSet>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osg/Referenced>

#include "MDLLimits.h"


namespace mdl
{


// The magic number for a Valve VVD file is 'IDSV' in little-endian
// order
const int VVD_MAGIC_NUMBER = (('V'<<24)+('S'<<16)+('D'<<8)+'I');


struct VVDHeader
{
    int    magic_number;
    int    vvd_version;
    int    check_sum;

    int    num_lods;
    int    num_lod_verts[MAX_LODS];

    int    num_fixups;
    int    fixup_table_offset;

    int    vertex_data_offset;

    int    tangent_data_offset;
};


struct VVDFixupEntry
{
    int   lod_number;

    int   source_vertex_id;
    int   num_vertices;
};


struct VVDBoneWeight
{
    float           weight[MAX_BONES_PER_VERTEX];
    char            bone[MAX_BONES_PER_VERTEX];
    unsigned char   num_bones;
};


struct VVDVertex
{
    VVDBoneWeight   bone_weights;
    osg::Vec3       vertex_position;
    osg::Vec3       vertex_normal;
    osg::Vec2       vertex_texcoord;
};


class VVDReader
{
protected:

    std::string       vvd_name;

    VVDVertex *       vertex_buffer[MAX_LODS];
    int               vertex_buffer_size[MAX_LODS];

    VVDFixupEntry *   fixup_table;

public:

    VVDReader();
    virtual ~VVDReader();

    bool   readFile(const std::string & file);

    int         getNumLODVertices(int lod);

    osg::Vec3   getVertex(int lod, int index);
    osg::Vec3   getNormal(int lod, int index);
    osg::Vec2   getTexCoords(int lod, int index);
};


}

#endif
