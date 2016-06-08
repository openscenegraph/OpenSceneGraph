
#include "VBSPData.h"
#include <string.h>


using namespace bsp;
using namespace osg;


VBSPData::VBSPData()
{
}


VBSPData::~VBSPData()
{
}


void VBSPData::addEntity(std::string & newEntity)
{
    entity_list.push_back(newEntity);
}


int VBSPData::getNumEntities() const
{
    return entity_list.size();
}


const std::string & VBSPData::getEntity(int index) const
{
    return entity_list[index];
}


void VBSPData::addModel(Model & newModel)
{
    model_list.push_back(newModel);
}


int VBSPData::getNumModels() const
{
    return model_list.size();
}


const Model & VBSPData::getModel(int index) const
{
    return model_list[index];
}


void VBSPData::addPlane(bsp::Plane & newPlane)
{
    plane_list.push_back(newPlane);
}


int VBSPData::getNumPlanes() const
{
    return plane_list.size();
}


const bsp::Plane & VBSPData::getPlane(int index) const
{
    return plane_list[index];
}


void VBSPData::addVertex(osg::Vec3f & newVertex)
{
    // Scale the vertex from inches up to meter scale
    vertex_list.push_back(newVertex * 0.0254f);
}


int VBSPData::getNumVertices() const
{
    return vertex_list.size();
}


const osg::Vec3f & VBSPData::getVertex(int index) const
{
    return vertex_list[index];
}


void VBSPData::addEdge(Edge & newEdge)
{
    edge_list.push_back(newEdge);
}


int VBSPData::getNumEdges() const
{
    return edge_list.size();
}


const Edge & VBSPData::getEdge(int index) const
{
    return edge_list[index];
}


void VBSPData::addSurfaceEdge(int & newSurfEdge)
{
    surface_edge_list.push_back(newSurfEdge);
}


int VBSPData::getNumSurfaceEdges() const
{
    return surface_edge_list.size();
}


int VBSPData::getSurfaceEdge(int index) const
{
    return surface_edge_list[index];
}


void VBSPData::addFace(Face & newFace)
{
    face_list.push_back(newFace);
}


int VBSPData::getNumFaces() const
{
    return face_list.size();
}


const Face & VBSPData::getFace(int index) const
{
    return face_list[index];
}


void VBSPData::addTexInfo(TexInfo & newTexInfo)
{
    texinfo_list.push_back(newTexInfo);
}


int VBSPData::getNumTexInfos() const
{
    return texinfo_list.size();
}


const TexInfo & VBSPData::getTexInfo(int index) const
{
    return texinfo_list[index];
}


void VBSPData::addTexData(TexData & newTexData)
{
    texdata_list.push_back(newTexData);
}


int VBSPData::getNumTexDatas() const
{
    return texdata_list.size();
}


const TexData & VBSPData::getTexData(int index) const
{
    return texdata_list[index];
}


void VBSPData::addTexDataString(std::string & newTexDataString)
{
    texdata_string_list.push_back(newTexDataString);
}


int VBSPData::getNumTexDataStrings() const
{
    return texdata_string_list.size();
}


const std::string & VBSPData::getTexDataString(int index) const
{
    return texdata_string_list[index];
}


void VBSPData::addDispInfo(DisplaceInfo & newDispInfo)
{
    dispinfo_list.push_back(newDispInfo);
}


int VBSPData::getNumDispInfos() const
{
    return dispinfo_list.size();
}


const DisplaceInfo & VBSPData::getDispInfo(int index) const
{
    return dispinfo_list[index];
}


void VBSPData::addDispVertex(DisplacedVertex & newDispVertex)
{
    displaced_vertex_list.push_back(newDispVertex);
}


int VBSPData::getNumDispVertices() const
{
    return displaced_vertex_list.size();
}


const DisplacedVertex & VBSPData::getDispVertex(int index) const
{
    return displaced_vertex_list[index];
}


void VBSPData::addStaticPropModel(std::string & newModel)
{
    static_prop_model_list.push_back(newModel);
}


int VBSPData::getNumStaticPropModels() const
{
    return static_prop_model_list.size();
}


const std::string & VBSPData::getStaticPropModel(int index) const
{
    return static_prop_model_list[index];
}


void VBSPData::addStaticProp(StaticPropV4 & newProp)
{
    StaticProp newPropV5;

    // Create a version 5 static prop and copy the data from the given
    // version 4 prop into it
    memcpy(&newPropV5, &newProp, sizeof(StaticPropV4));
    newPropV5.forced_fade_scale = 1.0;

    // Add the new prop to the list
    static_prop_list.push_back(newPropV5);
}


void VBSPData::addStaticProp(StaticProp & newProp)
{
    static_prop_list.push_back(newProp);
}


int VBSPData::getNumStaticProps() const
{
    return static_prop_list.size();
}


const StaticProp & VBSPData::getStaticProp(int index) const
{
    return static_prop_list[index];
}


void VBSPData::addStateSet(StateSet * stateSet)
{
    state_set_list.push_back(stateSet);
}


int VBSPData::getNumStateSets() const
{
    return state_set_list.size();
}


StateSet * VBSPData::getStateSet(int index) const
{
    return state_set_list[index].get();
}


