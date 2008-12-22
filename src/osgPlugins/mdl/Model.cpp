
#include "Model.h"
#include "VVDReader.h"


using namespace mdl;


Model::Model(MDLModel * myModel)
{
    // Save the model information
    my_model = myModel;
}


Model::~Model()
{
    // Clean up the associated data
    delete my_model;
}


MDLModel * Model::getModel()
{
    return my_model;
}


int Model::getVertexBase()
{
    // Return the base index for this model's vertices
    return my_model->vertex_index / sizeof(VVDVertex);
}


void Model::addMesh(Mesh * newMesh)
{
    // Add the new node to our list
    model_meshes.push_back(newMesh);
}


int Model::getNumMeshes()
{
    return model_meshes.size();
}


Mesh * Model::getMesh(int meshIndex)
{
    if ((meshIndex < 0) || (meshIndex >= static_cast<int>(model_meshes.size())))
        return NULL;
    else
        return model_meshes[meshIndex];
}

