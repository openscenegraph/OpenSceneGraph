
#include "Mesh.h"


using namespace mdl;


Mesh::Mesh(MDLMesh * myMesh)
{
    // Save the mesh information
    my_mesh = myMesh;

    // Initialize the state set to NULL
    state_set = NULL;
}


Mesh::~Mesh()
{
    // Clean up the associated data
    delete my_mesh;
}


void Mesh::setStateSet(osg::StateSet * stateSet)
{
    state_set = stateSet;
}


osg::StateSet * Mesh::getStateSet()
{
    return state_set.get();
}


MDLMesh * Mesh::getMesh()
{
    return my_mesh;
}


int Mesh::getNumLODVertices(int lodNum)
{
    return my_mesh->vertex_data.num_lod_vertices[lodNum];
}
