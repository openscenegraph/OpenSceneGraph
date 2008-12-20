
#ifndef __MESH_H_
#define __MESH_H_


#include <osg/Vec3f>
#include <osg/StateSet>

#include "MDLLimits.h"


namespace mdl
{


struct MDLMeshVertexData
{
    // Used by the Source engine for cache purposes.  This value is allocated
    // in the file, but no meaningful data is stored there
    int    model_vertex_data_ptr;

    // Indicates the number of vertices used by each LOD of this mesh
    int    num_lod_vertices[MAX_LODS];
};


struct MDLMesh
{
    int                  material_index;
    int                  model_index;

    int                  num_vertices;
    int                  vertex_offset;

    int                  num_flexes;
    int                  flex_offset;

    int                  material_type;
    int                  material_param;

    int                  mesh_id;

    osg::Vec3f           mesh_center;

    MDLMeshVertexData    vertex_data;

    int                  unused_array[8];
};


class Mesh
{
protected:

    MDLMesh *    my_mesh;

    osg::ref_ptr<osg::StateSet>   state_set;

public:

    Mesh(MDLMesh * myMesh);
    virtual ~Mesh();

    void               setStateSet(osg::StateSet * stateSet);
    osg::StateSet *    getStateSet();

    MDLMesh *          getMesh();

    int                getNumLODVertices(int lodNum);
};


}

#endif

