
#include "VBSPEntity.h"
#include "VBSPGeometry.h"

#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>


using namespace bsp;
using namespace osg;
using namespace osgDB;


// strcasecmp for MSVC
#ifdef _MSC_VER
    #define strcasecmp  _stricmp
#endif


VBSPEntity::VBSPEntity(std::string & entityText, VBSPData * bspData)
{
    // Save a handle to the bsp data, as we'll need this to construct the
    // entity
    bsp_data = bspData;

    // Assume we're not visible at first
    entity_visible = false;

    // Assume no transform
    entity_transformed = false;

    // No model (external or internal) yet
    entity_model_index = -1;
    entity_model.clear();

    // Don't know the class yet
    entity_class = ENTITY_OTHER;

    // Parse the entity's text to gather parameters
    parseParameters(entityText);
}


VBSPEntity::~VBSPEntity()
{
}


void VBSPEntity::processWorldSpawn()
{
    // World spawn is definitely visible
    entity_visible = true;

    // World spawn is always centered at the origin, so there's no need for
    // a transform
    entity_transformed = false;

    // The world spawn's internal model index is always zero
    entity_model_index = 0;
}


void VBSPEntity::processEnv()
{
    // We don't support these entities yet, so leave them invisible
}


void VBSPEntity::processFuncBrush()
{
    // These entities are usually transformed
    entity_transformed = true;

    // Get the internal model index for this entity
    EntityParameters::iterator param = entity_parameters.find("model");
    if (param != entity_parameters.end())
    {
        // Get the model number
        std::string value = (*param).second;

        // Skip the leading asterisk (internal models are denoted with a
        // leading asterisk), and then parse the model number
        if (value[0] == '*')
        {
            value = value.substr(1, std::string::npos);
            entity_model_index = atoi(value.c_str());

            // Make the entity visible
            entity_visible = true;
        }
        else
        {
            // This shouldn't happen (brush entities don't reference
            // external models).  Leave the entity invisible in this case
            entity_visible = false;
        }
    }
    else
    {
        // We can't locate the model for this entity, so leave it invisible
        entity_visible = false;
    }

    // Get the origin and angles for this entity
    param = entity_parameters.find("origin");
    if (param != entity_parameters.end())
    {
        // Get the origin parameter's value
        std::string value = (*param).second;

        // Parse the value into a vector
        entity_origin = getVector(value);
    }
    param = entity_parameters.find("angles");
    if (param != entity_parameters.end())
    {
        // Get the origin parameter's value
        std::string value = (*param).second;

        // Parse the value into a vector
        entity_angles = getVector(value);
    }
}


void VBSPEntity::processProp()
{
    // These entities are visible
    entity_visible = true;

    // These entities are usually transformed
    entity_transformed = true;

    // Get the model we need to load for this entity
    EntityParameters::iterator param = entity_parameters.find("model");
    if (param != entity_parameters.end())
    {
        // Get the model parameter's value
        entity_model = (*param).second;
    }

    // Get the origin and angles for this entity
    param = entity_parameters.find("origin");
    if (param != entity_parameters.end())
    {
        // Get the origin parameter's value
        std::string value = (*param).second;

        // Parse the value into a vector
        entity_origin = getVector(value);
    }
    param = entity_parameters.find("angles");
    if (param != entity_parameters.end())
    {
        // Get the origin parameter's value
        std::string value = (*param).second;

        // Parse the value into a vector
        entity_angles = getVector(value);
    }
}


void VBSPEntity::processInfoDecal()
{
    // We don't support these entities yet, so leave them invisible
}


void VBSPEntity::processItem()
{
    // We don't support these entities yet, so leave them invisible
}


Vec3f VBSPEntity::getVector(std::string str)
{
    float x, y, z;

    // Look for the first non-whitespace
    std::string::size_type start = str.find_first_not_of(" \t\r\n", 0);

    // Look for the first whitespace after this
    std::string::size_type end = str.find_first_of(" \t\r\n", start);

    if ((end > start) && (start != std::string::npos))
        x = osg::asciiToFloat(str.substr(start, end-start).c_str());
    else
        return Vec3f();

    // Look for the next non-whitespace
    start = str.find_first_not_of(" \t\r\n", end+1);

    // Look for the first whitespace after this
    end = str.find_first_of(" \t\r\n", start);

    if ((end > start) && (start != std::string::npos))
        y = osg::asciiToFloat(str.substr(start, end-start).c_str());
    else
        return Vec3f();

    // Look for the next non-whitespace
    start = str.find_first_not_of(" \t\r\n", end+1);

    // Look for the first whitespace after this
    end = str.find_first_of(" \t\r\n", start);
    if (end == std::string::npos)
        end = str.length();

    if ((end > start) && (start != std::string::npos))
        z = osg::asciiToFloat(str.substr(start, end-start).c_str());
    else
        return Vec3f();

    // If we get this far, return the vector that we parsed
    return Vec3f(x, y, z);
}


std::string VBSPEntity::getToken(std::string str, size_t & index)
{
    std::string::size_type end = std::string::npos;
    std::string   token;

    // Look for the first quotation mark
    std::string::size_type start = str.find_first_of("\"", index);
    if (start != std::string::npos)
    {
        // From there, look for the next occurrence of a delimiter
        start++;
        end = str.find_first_of("\"", start);
        if (end != std::string::npos)
        {
            // Found a delimiter, so grab the string in between
            token = str.substr(start, end-start);
        }
        else
        {
            // Ran off the end of the string, so just grab everything from
            // the first good character
            token = str.substr(start);
        }
    }
    else
    {
        // No token to be found
        token.clear();
    }

    // Update the index (in case we want to keep looking for tokens in this
    // string)
    if (end != std::string::npos)
        index = end+1;
    else
        index = std::string::npos;

    // Return the token
    return token;
}


void VBSPEntity::parseParameters(std::string & entityText)
{
    // Create a string stream on the entity text
    std::istringstream str(entityText, std::istringstream::in);

    // Iterate over the parameters
    while (!str.eof())
    {
        // Get the next line of text
        std::string line;
        std::getline(str, line);

        // Look for the first quotation mark on the line
        size_t start = 0;
        std::string token = getToken(line, start);

        // If we have a valid token it will be the parameter name (the key),
        // look for a second token, which will be the parameter's value
        while (!token.empty())
        {
            // Save the token as the key
            std::string key = token;

            // Get the next token
            start++;
            token = getToken(line, start);

            // See if the token is valid
            if (!token.empty())
            {
                // This token is the value, create an entity parameter from
                // these two strings and add it to our parameters map
                EntityParameter param(key, token);
                entity_parameters.insert(param);
            }
        }
    }

    // Now that we have all of the parameters, figure out what kind of entity
    // this is
    EntityParameters::iterator param = entity_parameters.find("classname");

    // See if we found the class
    if (param == entity_parameters.end())
    {
        // We need the class to be able to do anything with this entity
        return;
    }

    // Get the class name and process the entity appropriately
    class_name = (*param).second;
    if (class_name.compare("worldspawn") == 0)
    {
        // This is the entity that represents the main geometry of the map
        // (the terrain and much of the static geometry)
        entity_class = ENTITY_WORLDSPAWN;
        processWorldSpawn();
    }
    else if (class_name.compare(0, 3, "env") == 0)
    {
        // This is an environmental effect (such as a fire or dust cloud)
        entity_class = ENTITY_ENV;
        processEnv();
    }
    else if ((class_name.compare("func_brush") == 0) ||
             (class_name.compare("func_illusionary") == 0) ||
             (class_name.compare("func_wall_toggle") == 0) ||
             (class_name.compare("func_breakable") == 0))
    {
        // This is secondary map geometry, created along with the main
        // map geometry (not an external model)
        entity_class = ENTITY_FUNC_BRUSH;
        processFuncBrush();
    }
    else if (class_name.compare(0, 4, "prop") == 0)
    {
        // This is a "prop", an external model placed somewhere in the
        // scene
        entity_class = ENTITY_PROP;
        processProp();
    }
    else if (class_name.compare("infodecal") == 0)
    {
        // This is a decal, which applies a texture to some surface in the
        // scene
        entity_class = ENTITY_INFO_DECAL;
        processInfoDecal();
    }
    else if (class_name.compare(0, 4, "item") == 0)
    {
        // This is an "item".  Like a prop, these are external models
        // placed in the scene, but the specific model is determined
        // directly by the entity's class.  In HL2, these entities are
        // useable by the player (ammunition and health packs are examples)
        entity_class = ENTITY_ITEM;
        processItem();
    }
}


ref_ptr<Group> VBSPEntity::createBrushGeometry()
{
    int                 i;
    int                 numGeoms;
    VBSPGeometry **     vbspGeomList;
    Model               currentModel;
    Face                currentFace;
    TexInfo             currentTexInfo;
    TexData             currentTexData;
    const char *        texName;
    char                currentTexName[256];
    int                 currentGeomIndex;
    VBSPGeometry *      currentGeom;
    ref_ptr<Group>      entityGroup;
    ref_ptr<Group>      geomGroup;
    std::stringstream   groupName;

    // Create a list of VBSPGeometry objects for each texdata entry in the
    // scene.  These objects will hold the necessary geometry data until we
    // convert them back into OSG geometry objects.  We potentially will need
    // one for each state set in the map
    numGeoms = bsp_data->getNumStateSets();
    vbspGeomList = new VBSPGeometry *[numGeoms];

    // Initialize the list to all NULL for now.  We'll create the geometry
    // objects as we need them
    memset(vbspGeomList, 0, sizeof(VBSPGeometry *) * numGeoms);

    // Get this entity's internal model from the bsp data
    currentModel = bsp_data->getModel(entity_model_index);

    // Iterate over the face list and assign faces to the appropriate geometry
    // objects
    for (i = 0; i < currentModel.num_faces; i++)
    {
        // Get the current face
        currentFace = bsp_data->getFace(currentModel.first_face + i);

        // Get the texdata used by this face
        currentTexInfo = bsp_data->getTexInfo(currentFace.texinfo_index);
        currentTexData = bsp_data->getTexData(currentTexInfo.texdata_index);

        // Get the texture name
        texName = bsp_data->
            getTexDataString(currentTexData.name_string_table_id).c_str();
        strcpy(currentTexName, texName);

        // See if this is a non-drawable surface
        if ((strcasecmp(currentTexName, "tools/toolsareaportal") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsblocklos") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsblockbullets") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsblocklight") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsclip") != 0) &&
            (strcasecmp(currentTexName, "tools/toolscontrolclip") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsdotted") != 0) &&
            (strcasecmp(currentTexName, "tools/toolshint") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsinvisible") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsinvisibleladder") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsnodraw") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsnpcclip") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsoccluder") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsorigin") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsskip") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsskybox") != 0) &&
            (strcasecmp(currentTexName, "tools/toolsskyfog") != 0) &&
            (strcasecmp(currentTexName, "tools/toolstrigger") != 0))
        {
            // Get or create the corresponding VBSPGeometry object from the
            // list
            currentGeomIndex = currentTexInfo.texdata_index;
            currentGeom = vbspGeomList[currentGeomIndex];
            if (currentGeom == NULL)
            {
                // Create the geometry object
                vbspGeomList[currentGeomIndex] = new VBSPGeometry(bsp_data);
                currentGeom = vbspGeomList[currentGeomIndex];
            }

            // Add the face to the appropriate VBSPGeometry object
            currentGeom->addFace(currentModel.first_face + i);
        }
    }

    // Create a top-level group to hold the geometry objects
    if (entity_transformed)
    {
        // Create a matrix transform
        MatrixTransform * entityXform = new MatrixTransform();

        // Set it up with the entity's transform information (scale the
        // position from inches to meters)
        Matrixf transMat, rotMat;
        Quat roll, yaw, pitch;
        transMat.makeTranslate(entity_origin * 0.0254);
        pitch.makeRotate(osg::DegreesToRadians(entity_angles.x()),
                         Vec3f(0.0, 1.0, 0.0));
        yaw.makeRotate(osg::DegreesToRadians(entity_angles.y()),
                       Vec3f(0.0, 0.0, 1.0));
        roll.makeRotate(osg::DegreesToRadians(entity_angles.z()),
                        Vec3f(1.0, 0.0, 0.0));
        rotMat.makeRotate(roll * pitch * yaw);

        // Set the transform matrix
        entityXform->setMatrix(rotMat * transMat);

        // Use the transform node as the main entity group
        entityGroup = entityXform;
    }
    else
    {
        // Create a group to represent the entire entity
        entityGroup = new Group();
    }

    // Iterate over the geometry array and convert each geometry object
    // into OSG geometry
    for (i = 0; i < numGeoms; i++)
    {
        // Get the next geometry object (if any)
        currentGeom = vbspGeomList[i];
        if (currentGeom != NULL)
        {
            // Convert the BSP geometry to OSG geometry
            geomGroup = currentGeom->createGeometry();

            // Make sure the geometry converted properly
            if (geomGroup.valid())
            {
                // Set this group's state set
                geomGroup->setStateSet(bsp_data->getStateSet(i));

                // Add the geometry group to the entity group
                entityGroup->addChild(geomGroup.get());
            }
        }
    }

    // Name the entity group
    groupName << class_name << ":" << entity_model_index;
    entityGroup->setName(groupName.str());

    // Return the group we created
    return entityGroup;
}


ref_ptr<Group> VBSPEntity::createModelGeometry()
{
    std::string      modelFile;
    ref_ptr<Node>    modelNode;
    ref_ptr<Group>   entityGroup;

    // Try to load the model
    modelNode = osgDB::readNodeFile(entity_model);
    if (modelNode.valid())
    {
        // Create a group and add the model to it
        if (entity_transformed)
        {
            // Create a matrix transform
            MatrixTransform * entityXform = new MatrixTransform();

            // Set it up with the entity's transform information (scale
            // the position from inches to meters)
            Matrixf transMat, rotMat;
            Quat roll, yaw, pitch;
            transMat.makeTranslate(entity_origin * 0.0254);
            pitch.makeRotate(osg::DegreesToRadians(entity_angles.x()),
                             Vec3f(0.0, 1.0, 0.0));
            yaw.makeRotate(osg::DegreesToRadians(entity_angles.y()),
                           Vec3f(0.0, 0.0, 1.0));
            roll.makeRotate(osg::DegreesToRadians(entity_angles.z()),
                            Vec3f(1.0, 0.0, 0.0));
            rotMat.makeRotate(roll * pitch * yaw);

            // Set the transform matrix
            entityXform->setMatrix(rotMat * transMat);

            // Use the transform node as the main entity group
            entityGroup = entityXform;
        }
        else
        {
            // Create a group to represent the entire entity
            entityGroup = new Group();
        }

        // Add the model node to the group
        entityGroup->addChild(modelNode.get());

        // Set the group's name
        entityGroup->setName(class_name + std::string(":") + entity_model);
    }
    else
    {
        OSG_WARN << "Couldn't find prop \"" << entity_model << "\".";
        OSG_WARN << std::endl;

        // Leave the group empty (no model to show)
        entityGroup = NULL;
    }

    return entityGroup;
}


EntityClass VBSPEntity::getClass()
{
    return entity_class;
}


bool VBSPEntity::isVisible()
{
    return entity_visible;
}


ref_ptr<Group> VBSPEntity::createGeometry()
{
    // If we're not a visible entity, we have no geometry
    if (!entity_visible)
        return NULL;

    // Create the geometry for the entity based on the class
    if ((entity_class == ENTITY_WORLDSPAWN) ||
        (entity_class == ENTITY_FUNC_BRUSH))
    {
        return createBrushGeometry();
    }
    else if (entity_class == ENTITY_PROP)
    {
        return createModelGeometry();
    }

    // If we get here, we don't handle this kind of entity (yet)
    return NULL;
}

