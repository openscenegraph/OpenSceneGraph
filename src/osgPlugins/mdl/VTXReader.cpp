#include <osg/Geometry>
#include <osg/Group>
#include <osg/Node>
#include <osg/Notify>
#include <osg/StateSet>
#include <osg/Switch>
#include <iostream>

#include "VTXReader.h"


using namespace mdl;
using namespace osg;
using namespace osgDB;


VTXReader::VTXReader(VVDReader * vvd, MDLRoot * mdlRoot)
{
    // Save the VVD reader, as we'll need it to read vertex data
    vvd_reader = vvd;

    // Save the root of the MDL tree, as we'll need it to make sure we
    // index the vertex data properly
    mdl_root = mdlRoot;

    // Initialize the root group
    model_root = NULL;
}


VTXReader::~VTXReader()
{
}


ref_ptr<Group> VTXReader::processBodyPart(std::istream * str, int offset,
                                          BodyPart * currentPart)
{
    int               i;
    VTXBodyPart       part;
    Model *           currentModel;
    ref_ptr<Group>    partSwitch;
    ref_ptr<Group>    modelGroup;

    // Seek to the body part
    str->seekg(offset);

    // Read it
    str->read((char *) &part, sizeof(VTXBodyPart));

    // If there is more than one model, create a switch to select between them
    // (it seems that only one model is supposed to be seen at a given time,
    // but I don't know the mechanism in the engine that selects a desired
    // model)
    if (part.num_models > 1)
    {
        partSwitch = new Switch();
    }

    // Process the models
    for (i = 0; i < part.num_models; i++)
    {
        // Get the corresponding MDL model from the current body part
        currentModel = currentPart->getModel(i);

        // Process the model
        modelGroup = processModel(str, 
                                  offset + part.model_offset +
                                  (i * sizeof(VTXModel)),
                                  currentModel);

        // If there is more than one model, add this model to the part group
        if (part.num_models > 1)
        {
            // Add the model to the switch
            partSwitch->addChild(modelGroup.get());

            // If this is the first child, turn it on, otherwise turn it off
            if (i == 0)
                ((osg::Switch *)partSwitch.get())->setValue(i, true);
            else
                ((osg::Switch *)partSwitch.get())->setValue(i, false);
        }
    }

    // If there is only one model, just return it
    if (part.num_models == 1)
        return modelGroup;
    else
        return partSwitch;
}


ref_ptr<Group> VTXReader::processModel(std::istream * str, int offset,
                                       Model * currentModel)
{
    int              i;
    VTXModel         model;
    float            lastDistance;
    float            distance;
    LOD *            lodNode = 0;
    ref_ptr<Group>   group;
    ref_ptr<Group>   result;

    // Seek to the model
    str->seekg(offset);

    // Read it
    str->read((char *) &model, sizeof(VTXModel));

    // If we have multiple LODs, create an LOD node for them
    if (model.num_lods > 1)
        lodNode = new LOD();

    // Initialize the distances
    lastDistance = 0.0;
    distance = 0.0;

    // Process the LODs
    for (i = 0; i < model.num_lods; i++)
    {
        // Process the LOD group, passing the current MDL model through
        group = processLOD(i, &distance, str,
                           offset + model.lod_offset +
                           (i * sizeof(VTXModelLOD)),
                           currentModel);

        // If this isn't the only LOD, add it to the LOD node
        if (model.num_lods > 1)
        {
            lodNode->addChild(group.get());

            // Fix the LOD distances
            if (distance < 0)
            {
                // Fix negative distance (my best guess is that these mean
                // for the LOD to never switch out)
                distance = 100000.0;
            }

            // Set the ranges on the previous LOD (now that we know the
            // switch point for this one)
            if (i > 0)
                lodNode->setRange(i-1, lastDistance, distance);

            lastDistance = distance;
        }
    }

    if (i > 1)
       lodNode->setRange(i-1, lastDistance, 100000.0);

    // Return either the LOD node or the single LOD group
    if (model.num_lods > 1)
        result = lodNode;
    else
        result = group;

    return result;
}


ref_ptr<Group> VTXReader::processLOD(int lodNum, float * distance,
                                     std::istream * str, int offset,
                                     Model * currentModel)
{
    int              i;
    VTXModelLOD      lod;
    Mesh *           currentMesh;
    int              vertexOffset;
    ref_ptr<Group>   lodGroup;
    ref_ptr<Geode>   geode;

    // Seek to the LOD
    str->seekg(offset);

    // Read it
    str->read((char *) &lod, sizeof(VTXModelLOD));

    // Create a group to hold this LOD
    lodGroup = new Group();

    // Process the meshes
    vertexOffset = currentModel->getVertexBase();
    for (i = 0; i < lod.num_meshes; i++)
    {
        // Get the corresponding MDL mesh from the current model
        currentMesh = currentModel->getMesh(i);

        // Process the mesh to get a geode
        geode = processMesh(lodNum, str,
                            offset + lod.mesh_offset + (i * VTX_MESH_SIZE),
                            vertexOffset);

        // Set the geode's state set based on the current mesh node's state
        // set
        geode->setStateSet(currentMesh->getStateSet());

        // Add the geode to the group
        lodGroup->addChild(geode.get());

        // Add the number of vertices for this mesh at this LOD to the offset,
        // so we can start the next mesh at the proper vertex ID
        vertexOffset += currentMesh->getNumLODVertices(lodNum);
    }

    // Set the distance for this LOD
    *distance = lod.switch_point;

    // Return the LOD group that we created
    return lodGroup;
}


ref_ptr<Geode> VTXReader::processMesh(int lodNum, std::istream * str,
                                      int offset, int vertexOffset)
{
    int                 i;
    VTXMesh             mesh;
    ref_ptr<Geode>      geode;
    ref_ptr<Geometry>   geom;

    // Seek to the mesh
    str->seekg(offset);

    // Read it
    str->read((char *) &mesh, VTX_MESH_SIZE);

    // Create a geode to hold the geometry
    geode = new Geode();

    // Process the strip groups
    for (i = 0; i < mesh.num_strip_groups; i++)
    {
        // Process the strip group to get a Geometry
        geom = processStripGroup(lodNum, str,
            offset + mesh.strip_group_offset + (i * VTX_STRIP_GROUP_SIZE),
            vertexOffset);

        // Add the geometry to the geode
        geode->addDrawable(geom.get());
    }

    // Return the geode
    return geode;
}


ref_ptr<Geometry> VTXReader::processStripGroup(int lodNum, std::istream * str,
                                               int offset, int vertexOffset)
{
    int                       i;
    VTXStripGroup             stripGroup;
    VTXVertex                 vtxVertex;
    int                       vertexID;
    ref_ptr<Vec3Array>        vertexArray;
    ref_ptr<Vec3Array>        normalArray;
    ref_ptr<Vec2Array>        texcoordArray;
    unsigned short            index;
    unsigned short *          indexArray;
    ref_ptr<Geometry>         geom;
    ref_ptr<PrimitiveSet>     primSet;

    // Seek to the strip group
    str->seekg(offset);

    // Read it
    str->read((char *) &stripGroup, VTX_STRIP_GROUP_SIZE);

    // Create and fill the vertex arrays
    vertexArray = new Vec3Array();
    normalArray = new Vec3Array();
    texcoordArray = new Vec2Array();
    str->seekg(offset + stripGroup.vertex_offset);
    for (i = 0; i < stripGroup.num_vertices; i++)
    {
        // Get the vertex ID from the strip group
        str->read((char *) &vtxVertex, VTX_VERTEX_SIZE);
        vertexID = vtxVertex.orig_mesh_vertex_id + vertexOffset;

        // Get the corresponding vertex, normal, texture coordinates from the
        // VVD file
        vertexArray->push_back(vvd_reader->getVertex(lodNum, vertexID));
        normalArray->push_back(vvd_reader->getNormal(lodNum, vertexID));
        texcoordArray->push_back(vvd_reader->getTexCoords(lodNum, vertexID));
    }

    // Create the geometry and add the vertex data to it
    geom = new Geometry();
    geom->setVertexArray(vertexArray.get());
    geom->setNormalArray(normalArray.get());
    geom->setNormalBinding(Geometry::BIND_PER_VERTEX);
    geom->setTexCoordArray(0, texcoordArray.get());

    // Create and fill the index array
    indexArray = new unsigned short[stripGroup.num_indices];
    str->seekg(offset + stripGroup.index_offset);
    for (i = 0; i < stripGroup.num_indices; i++)
    {
        // Get the index from the file
        str->read((char *) &index, sizeof(unsigned short));

        // Add to the array
        indexArray[i] = index;
    }

    // Process the strips
    for (i = 0; i < stripGroup.num_strips; i++)
    {
        // Process the strip to create a triangle list or strip
        primSet = processStrip(indexArray, str,
            offset + stripGroup.strip_offset + (i * VTX_STRIP_SIZE));

        // Add the primitive set to the geometry
        geom->addPrimitiveSet(primSet.get());
    }

    // Clean up
    delete [] indexArray;

    // Return the geometry
    return geom;
}


ref_ptr<PrimitiveSet> VTXReader::processStrip(unsigned short * indexArray,
                                              std::istream * str,
                                              int offset)
{
    VTXStrip                strip;
    DrawElementsUShort *    drawElements;
    ref_ptr<PrimitiveSet>   primSet;
    unsigned short *        start;
    unsigned short *        end;

    // Seek to the strip
    str->seekg(offset);

    // Read it.  We have to do this in a kind of screwy way because of the
    // weird byte packing.  Valve uses pragma pack, but we can't do that
    // because it's non-portable.  Of course, I'm assuming a 4-byte alignment
    // here, which might also be non-portable...
    str->read((char *) &strip, VTX_STRIP_SIZE - 8);
    str->read((char *) &strip.num_bone_state_changes, 8);

    // Get the range of indices in question from the strip
    start = &indexArray[strip.index_offset];
    end = &indexArray[strip.index_offset + strip.num_indices];

    // Create the primitive set (based on the flag)
    if (strip.strip_flags & STRIP_IS_TRI_LIST)
        drawElements =
            new DrawElementsUShort(PrimitiveSet::TRIANGLES, start, end);
    else
        drawElements =
            new DrawElementsUShort(PrimitiveSet::TRIANGLE_STRIP, start, end);

    // Flip the indices to get the front faces correct
    std::reverse(drawElements->begin(), drawElements->end());

    // Return the primitive set
    primSet = drawElements;
    return primSet;
}


bool VTXReader::readFile(const std::string & file)
{
    osgDB::ifstream *   vtxFile;
    VTXHeader           header;
    int                 i;
    BodyPart *          currentPart;
    ref_ptr<Group>      partGroup;
    Group *             rootGroup;

    // Remember the map name
    vtx_name = getStrippedName(file);

    vtxFile = new osgDB::ifstream(file.c_str(), std::ios::binary);
    if (!vtxFile || vtxFile->fail())
    {
        notify(NOTICE) << "Vertex index file not found" << std::endl;
        return false;
    }

    // Read the header
    vtxFile->read((char *) &header, sizeof(VTXHeader));

    // Make sure the version is one that we handle
    // TODO:  Not sure which versions are valid yet

    // Create the root group
    rootGroup = new Group();

    // Process the body parts
    for (i = 0; i < header.num_body_parts; i++)
    {
        // Get the corresponding body part from the MDL tree
        currentPart = mdl_root->getBodyPart(i);

        // Process the body part
        partGroup = processBodyPart(vtxFile, 
                                    header.body_part_offset +
                                        (i * sizeof(VTXBodyPart)),
                                    currentPart);

        // Add the result to the root group
        rootGroup->addChild(partGroup.get());
    }

    // Set the model's root node
    model_root = rootGroup;

    // Close the file
    vtxFile->close();
    delete vtxFile;

    return true;
}


ref_ptr<Node> VTXReader::getModel()
{
    return model_root;
}
