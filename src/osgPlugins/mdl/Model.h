
#ifndef __MODEL_H_
#define __MODEL_H_

#include <vector>

#include "Mesh.h"


namespace mdl
{


struct MDLModelVertexData
{
    // No useful values are stored in the file for this structure, but we
    // need the size to be right so we can properly read subsequent models
    // from the file
    int    vertex_data_ptr;
    int    tangent_data_ptr;
};


struct MDLModel
{
    char                  model_name[64];
    int                   model_type;
    float                 bounding_radius;
    int                   num_meshes;
    int                   mesh_offset;

    int                   num_vertices;
    int                   vertex_index;
    int                   tangents_index;

    int                   num_attachments;
    int                   attachment_offset;
    int                   num_eyeballs;
    int                   eyeball_offset;

    MDLModelVertexData    vertex_data;

    int                   unused_array[8];
};



class Model
{
protected:

    typedef std::vector<Mesh *>    MeshList;

    MDLModel *    my_model;

    MeshList      model_meshes;

public:

    Model(MDLModel * myModel);
    virtual ~Model();

    MDLModel *    getModel();

    int           getVertexBase();

    void          addMesh(Mesh * newMesh);
    int           getNumMeshes();
    Mesh *        getMesh(int meshIndex);
};


}

#endif

