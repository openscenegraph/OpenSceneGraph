#include <osg/BlendFunc>
#include <osg/BoundingSphere>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/Object>
#include <osg/Material>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/Node>
#include <osg/Notify>
#include <osg/Quat>
#include <osg/StateSet>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TexEnv>
#include <osg/TexEnvCombine>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osg/io_utils>
#include <iostream>
#include <string.h>

#include "VBSPReader.h"
#include "VBSPEntity.h"


using namespace bsp;
using namespace osg;
using namespace osgDB;


// strcasecmp for MSVC
#ifdef _MSC_VER
   #define strcasecmp  _stricmp
#endif


VBSPReader::VBSPReader()
{
    // Start with no root node
    root_node = NULL;

    // Create the map data object
    bsp_data = new VBSPData();

    // No string table yet
    texdata_string = NULL;
    texdata_string_table = NULL;
    num_texdata_string_table_entries = 0;
}


VBSPReader::~VBSPReader()
{
    // Clean up the texdata strings and such
    delete [] texdata_string;
    delete [] texdata_string_table;
}


void VBSPReader::processEntities(std::istream & str, int offset,
                                 int length)
{
    char *          entities;
    char *          startPtr;
    char *          endPtr;
    int             numEntities;
    int             i;
    std::string     entityStr;
    size_t          entityLen;

    // Create the string
    entities = new char[length];
    memset(entities, 0, length * sizeof(char));

    // Seek to the Entities lump
    str.seekg(offset);

    // Read the entities string
    str.read((char *) entities, sizeof(char) * length);

    // Count the number of entities
    startPtr = entities;
    endPtr = strchr(entities, '}');
    numEntities = 0;
    while ((startPtr != NULL) && (endPtr != NULL))
    {
        // Increment the count
        numEntities++;

        // Advance the pointers
        startPtr = strchr(endPtr, '{');
        if (startPtr != NULL)
            endPtr = strchr(startPtr, '}');
    }

    // Parse the entities
    startPtr = entities;
    endPtr = strchr(entities, '}');
    for (i = 0;
         (i<numEntities) && (endPtr!=NULL && startPtr!=NULL);
         i++)
    {
        // Get the length of this entity
        entityLen = endPtr - startPtr + 1;

        // Create the entity list entry and copy the entity information
        entityStr = std::string(startPtr, entityLen);
        bsp_data->addEntity(entityStr);

        // Advance the pointers
        startPtr = strchr(endPtr, '{');
        if (startPtr != NULL)
            endPtr = strchr(startPtr, '}');
    }

    // Free up the original entities string
    delete [] entities;
}


void VBSPReader::processModels(std::istream & str, int offset, int length)
{
    int       numModels;
    int       i;
    Model *   models;

    // Calculate the number of models
    numModels = length / sizeof(Model);

    // Seek to the Models lump
    str.seekg(offset);

    // Read the models
    models = new Model[numModels];
    str.read((char *) models, sizeof(Model) * numModels);

    // Add the models to the model list
    for (i = 0; i < numModels; i++)
        bsp_data->addModel(models[i]);

    // Clean up
    delete [] models;
}


void VBSPReader::processPlanes(std::istream & str, int offset, int length)
{
    int       numPlanes;
    int       i;
    Plane *   planes;

    // Calculate the number of planes
    numPlanes = length / sizeof(Plane);

    // Seek to the Planes lump
    str.seekg(offset);

    // Read the planes
    planes = new Plane[numPlanes];
    str.read((char *) planes, sizeof(Plane) * numPlanes);

    // Add the planes to the plane list
    for (i = 0; i < numPlanes; i++)
        bsp_data->addPlane(planes[i]);

    // Clean up
    delete [] planes;
}


void VBSPReader::processVertices(std::istream & str, int offset, int length)
{
    int       numVertices;
    int       i;
    Vec3f *   vertices;

    // Calculate the number of vertices
    numVertices = length / 3 / sizeof(float);

    // Seek to the Vertices lump
    str.seekg(offset);

    // Read the vertex
    vertices = new Vec3f[numVertices];
    str.read((char *) vertices, sizeof(Vec3f) * numVertices);

    // Add it the vertices to the list
    for (i = 0; i < numVertices; i++)
        bsp_data->addVertex(vertices[i]);

    // Clean up
    delete [] vertices;
}


void VBSPReader::processEdges(std::istream & str, int offset, int length)
{
    int      numEdges;
    int      i;
    Edge *   edges;

    // Calculate the number of edges
    numEdges = length / sizeof(Edge);

    // Seek to the Edges lump
    str.seekg(offset);

    // Read the edges
    edges = new Edge[numEdges];
    str.read((char *) edges, sizeof(Edge) * numEdges);

    // Add the edges to the edge list
    for (i = 0; i < numEdges; i++)
        bsp_data->addEdge(edges[i]);

    // Clean up
    delete [] edges;
}


void VBSPReader::processSurfEdges(std::istream & str, int offset, int length)
{
    int     numSurfEdges;
    int     i;
    int *   surfEdges;

    // Calculate the number of edges
    numSurfEdges = length / sizeof(int);

    // Seek to the SurfEdges lump
    str.seekg(offset);

    // Read the surface edges
    surfEdges = new int[numSurfEdges];
    str.read((char *) surfEdges, sizeof(int) * numSurfEdges);

    // Add the surface edges to the surface edge list
    for (i = 0; i < numSurfEdges; i++)
        bsp_data->addSurfaceEdge(surfEdges[i]);

    // Clean up
    delete [] surfEdges;
}


void VBSPReader::processFaces(std::istream & str, int offset, int length)
{
    int      numFaces;
    int      i;
    Face *   faces;

    // Calculate the number of faces
    numFaces = length / sizeof(Face);

    // Seek to the Faces lump
    str.seekg(offset);

    // Read the faces
    faces = new Face[numFaces];
    str.read((char *) faces, sizeof(Face) * numFaces);

    // Add the faces to the face list
    for (i = 0; i < numFaces; i++)
        bsp_data->addFace(faces[i]);

    // Clean up
    delete [] faces;
}


void VBSPReader::processTexInfo(std::istream & str, int offset, int length)
{
    int         numTexInfos;
    int         i;
    TexInfo *   texinfos;

    // Calculate the number of texinfos
    numTexInfos = length / sizeof(TexInfo);

    // Seek to the TexInfo lump
    str.seekg(offset);

    // Read in the texinfo entries
    texinfos = new TexInfo[numTexInfos];
    str.read((char *) texinfos, sizeof(TexInfo) * numTexInfos);

    // Add the texinfo entries to the texinfo list
    for (i = 0; i < numTexInfos; i++)
        bsp_data->addTexInfo(texinfos[i]);

    // Clean up
    delete [] texinfos;
}


void VBSPReader::processTexData(std::istream & str, int offset, int length)
{
    int         numTexDatas;
    int         i;
    TexData *   texdatas;

    // Calculate the number of texdatas
    numTexDatas = length / sizeof(TexData);

    // Seek to the TexData lump
    str.seekg(offset);

    // Read in the texdata entries
    texdatas = new TexData[numTexDatas];
    str.read((char *) texdatas, sizeof(TexData) * numTexDatas);

    // Add the texdata entries to the texdata list
    for (i = 0; i < numTexDatas; i++)
        bsp_data->addTexData(texdatas[i]);

    // Clean up
    delete [] texdatas;
}


void VBSPReader::processTexDataStringTable(std::istream & str, int offset,
                                           int length)
{
    int           i;
    int           index;
    std::string   texStr;

    // Calculate the number of table entries
    num_texdata_string_table_entries = length / sizeof(int);

    // Create the texdata string table
    texdata_string_table = new int[num_texdata_string_table_entries];

    // Seek to the TexDataStringTable lump
    str.seekg(offset);

    // Read in the texdata_string_table
    str.read((char *) texdata_string_table,
             sizeof(int) * num_texdata_string_table_entries);

    // If we have a texdata string loaded, parse the texdata strings now
    if (texdata_string != NULL)
    {
        for (i = 0; i < num_texdata_string_table_entries; i++)
        {
            // Add the strings from the string data, using the string table
            // to index it
            index = texdata_string_table[i];
            texStr = std::string(&texdata_string[index]);
            bsp_data->addTexDataString(texStr);
        }
    }
}


void VBSPReader::processTexDataStringData(std::istream & str, int offset,
                                          int length)
{
    int      i;
    int           index;
    std::string   texStr;

    // Create the buffer to hold the texdata string
    texdata_string = new char[length];
    memset(texdata_string, 0, length * sizeof(char));

    // Seek to the TexDataStringData lump
    str.seekg(offset);

    // Read the entire texdata string (this string is actually a
    // NULL-delimited list of strings)
    str.read((char *) texdata_string, sizeof(char) * length);

    // If we have a string table loaded, parse the texdata strings now
    // (if not, num_texdata_string_table_entries will be zero and we'll
    // skip this loop)
    for (i = 0; i < num_texdata_string_table_entries; i++)
    {
        // Add the strings from the string data, using the string table
        // to index it
        index = texdata_string_table[i];
        texStr = std::string(&texdata_string[index]);
        bsp_data->addTexDataString(texStr);
    }
}


void VBSPReader::processDispInfo(std::istream & str, int offset, int length)
{
    int              numDispInfos;
    int              i;
    DisplaceInfo *   dispinfos;

    // Calculate the number of dispinfos
    numDispInfos = length / sizeof(DisplaceInfo);

    // Seek to the DisplaceInfo lump
    str.seekg(offset);

    // Read in the dispinfo entries
    dispinfos = new DisplaceInfo[numDispInfos];
    str.read((char *) dispinfos, sizeof(DisplaceInfo) * numDispInfos);

    // Add the dispinfo entries to the displace info list
    for (i = 0; i < numDispInfos; i++)
        bsp_data->addDispInfo(dispinfos[i]);

    // Clean up
    delete [] dispinfos;
}


void VBSPReader::processDispVerts(std::istream & str, int offset, int length)
{
    int                 numDispVerts;
    int                 i;
    DisplacedVertex *   dispverts;

    // Calculate the number of displaced vertices
    numDispVerts = length / sizeof(DisplacedVertex);

    // Seek to the DispVert lump
    str.seekg(offset);

    // Read in the displaced vertices
    dispverts = new DisplacedVertex[numDispVerts];
    str.read((char *) dispverts, sizeof(DisplacedVertex) * numDispVerts);

    // Add the displaced vertices to the displaced vertex list
    for (i = 0; i < numDispVerts; i++)
        bsp_data->addDispVertex(dispverts[i]);

    // Clean up
    delete [] dispverts;
}


void VBSPReader::processGameData(std::istream & str, int offset, int /*length*/)
{
    GameHeader    gameHeader;
    GameLump *    gameLumps;
    int           i;

    // Read the header
    str.seekg(offset);
    str.read((char *) &gameHeader, sizeof(GameHeader));

    // Create and read in the game lump list
    gameLumps = new GameLump[gameHeader.num_lumps];
    str.read((char *) gameLumps, sizeof(GameLump) * gameHeader.num_lumps);

    // Iterate over the game lumps
    for (i = 0; i < gameHeader.num_lumps; i++)
    {
        // See if this is a lump we're interested in
        if (gameLumps[i].lump_id == STATIC_PROP_ID)
        {
             processStaticProps(str, gameLumps[i].lump_offset,
                                gameLumps[i].lump_length,
                                gameLumps[i].lump_version);
        }
    }

    // Clean up
    delete [] gameLumps;
}


void VBSPReader::processStaticProps(std::istream & str, int offset, int /*length*/,
                                    int lumpVersion)
{
    StaticPropModelNames    sprpModelNames;
    char                    modelName[130];
    std::string             modelStr;
    int                     i;
    StaticPropLeaves        sprpLeaves;
    StaticProps             sprpHeader;
    StaticPropV4            sprp4;
    StaticProp              sprp5;

    // First, read the static prop models dictionary
    str.seekg(offset);
    str.read((char *) &sprpModelNames, sizeof(StaticPropModelNames));
    for (i = 0; i < sprpModelNames.num_model_names; i++)
    {
        str.read(modelName, 128);
        modelName[128] = 0;
        modelStr = std::string(modelName);
        bsp_data->addStaticPropModel(modelStr);
    }

    // Next, skip over the static prop leaf array
    str.read((char *) &sprpLeaves, sizeof(StaticPropLeaves));
    str.seekg(sprpLeaves.num_leaf_entries * sizeof(unsigned short),
              std::istream::cur);

    // Finally, read in the static prop entries
    str.read((char *) &sprpHeader, sizeof(StaticProps));
    for (i = 0; i < sprpHeader.num_static_props; i++)
    {
        // The version number determines how much we read for each prop
        if (lumpVersion == 4)
        {
            // Read the static prop and add it to the bsp data
            str.read((char *) &sprp4, sizeof(StaticPropV4));
            bsp_data->addStaticProp(sprp4);
        }
        else if (lumpVersion == 5)
        {
            // Read the static prop and add it to the bsp data
            str.read((char *) &sprp5, sizeof(StaticProp));
            bsp_data->addStaticProp(sprp5);
        }
    }
}


std::string VBSPReader::getToken(std::string str, const char * delim,
                                 size_t & index)
{
    std::string   token;
    size_t        end = std::string::npos;

    // Look for the first non-occurrence of the delimiters
    size_t start = str.find_first_not_of(delim, index);
    if (start != std::string::npos)
    {
        // From there, look for the first occurrence of a delimiter
        end = str.find_first_of(delim, start+1);
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
        token = "";
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


ref_ptr<Texture> VBSPReader::readTextureFile(std::string textureName)
{
    std::string   texFile;
    std::string   texPath;
    osg::ref_ptr<Image>       texImage;
    osg::ref_ptr<Texture>     texture;

    // Find the texture's image file
    texFile = std::string(textureName) + ".vtf";
    texPath = findDataFile(texFile, CASE_INSENSITIVE);

    // If we don't find it right away, check in a "materials" subdirectory
    if (texPath.empty())
    {
        texFile = "materials/" + std::string(textureName) + ".vtf";
        texPath = findDataFile(texFile, CASE_INSENSITIVE);

        // Check up one directory if we don't find it here (the map file is
        // usually located in the "maps" directory, adjacent to the materials
        // directory)
        if (texPath.empty())
        {
            texFile = "../materials/" + std::string(textureName) + ".vtf";
            texPath = findDataFile(texFile, CASE_INSENSITIVE);
        }
    }

    // If we found the file, read it, otherwise bail
    if (!texPath.empty())
    {
        texImage = readRefImageFile(texPath);

        // If we got the image, create the texture attribute
        if (texImage != NULL)
        {
            // Create the texture
            if (texImage->t() == 1)
            {
                texture = new Texture1D(texImage.get());
            }
            else if (texImage->r() == 1)
            {
                texture = new Texture2D(texImage.get());
            }
            else
            {
                texture = new Texture3D(texImage.get());
            }

            // Set texture attributes
            texture->setWrap(Texture::WRAP_S, Texture::REPEAT);
            texture->setWrap(Texture::WRAP_T, Texture::REPEAT);
            texture->setWrap(Texture::WRAP_R, Texture::REPEAT);
            texture->setFilter(Texture::MAG_FILTER, Texture::LINEAR);
            texture->setFilter(Texture::MIN_FILTER,
                               Texture::LINEAR_MIPMAP_LINEAR);
        }
        else
        {
            // We were unable to find the texture file
            OSG_WARN << "Couldn't find texture " << textureName;
            OSG_WARN << std::endl;

            // No texture
            texture = NULL;
        }
    }
    else
    {
        // We were unable to find the texture file
        OSG_WARN << "Couldn't find texture " << textureName;
        OSG_WARN << std::endl;

        // No texture
        texture = NULL;
    }

    return texture;
}


ref_ptr<StateSet> VBSPReader::readMaterialFile(std::string materialName)
{
    std::string             mtlFileName;
    std::string             mtlPath;
    osgDB::ifstream *       mtlFile;
    std::string             line;
    std::string::size_type  start = std::string::npos;
    std::string             token;
    bool                    found = false;
    ref_ptr<StateSet>       stateSet;
    std::string             shaderName;
    ref_ptr<Texture>        texture;
    ref_ptr<Texture>        texture2;
    ref_ptr<TexEnvCombine>  combiner0;
    ref_ptr<TexEnvCombine>  combiner1;
    ref_ptr<Material>       material;
    ref_ptr<BlendFunc>      blend;
    bool                    translucent;
    double                  alpha;

    // Find the material file
    mtlFileName = std::string(materialName) + ".vmt";
    mtlPath = findDataFile(mtlFileName, CASE_INSENSITIVE);

    // If we don't find it right away, check in a "materials" subdirectory
    if (mtlPath.empty())
    {
        mtlFileName = "materials/" + std::string(materialName) + ".vmt";
        mtlPath = findDataFile(mtlFileName, CASE_INSENSITIVE);

        // Check up one directory if we don't find it here (the map file is
        // usually located in the "maps" directory, adjacent to the materials
        // directory)
        if (mtlPath.empty())
        {
            mtlFileName = "../materials/" + std::string(materialName) + ".vmt";
            mtlPath = findDataFile(mtlFileName, CASE_INSENSITIVE);
        }
    }

    // See if we found the file
    if (!mtlPath.empty())
    {
        // Try to open the file, bail out if we fail
        mtlFile = new osgDB::ifstream(mtlPath.c_str(), std::ifstream::in);
        if (!mtlFile)
            return NULL;
    }
    else
    {
        // Didn't find the material file, so return NULL
        OSG_WARN << "Can't find material " << materialName << std::endl;
        return NULL;
    }

    // First, look for the shader name
    found = false;
    while ((!found) && (!mtlFile->eof()))
    {
        // Read a line from the file
        std::getline(*mtlFile, line);

        // Try to find the shader name
        start = 0;
        token = getToken(line, " \t\n\r\"", start);

        // If we got something, it must be the shader
        if ((!token.empty()) && (token.compare(0, 2, "//") != 0))
        {
            shaderName = token;
            found = true;
        }
    }

    // If we didn't find a shader, this isn't a valid material file
    if (!found)
    {
        mtlFile->close();
        OSG_WARN << "Material " << materialName << " isn't valid.";
        OSG_WARN << std::endl;
        return NULL;
    }

    // No textures loaded yet
    texture = NULL;
    texture2 = NULL;

    // Assume no transparency unless the properties say otherwise
    translucent = false;

    // Assume fully opaque
    alpha = 1.0;

    // Read the material properties next
    while (!mtlFile->eof())
    {
        // Get the next line
        std::getline(*mtlFile, line);

        // Look for tokens starting at the beginning
        start = 0;
        token = getToken(line, " \t\n\r\"", start);

        while ((!token.empty()) && (token.compare(0, 2, "//") != 0))
        {
            if (equalCaseInsensitive(token, "$basetexture"))
            {
                // Get the base texture name
                token = getToken(line, " \t\n\r\"", start);

                // Read the texture
                if (!token.empty())
                    texture = readTextureFile(token);
            }
            else if (equalCaseInsensitive(token, "$basetexture2"))
            {
                // Get the second base texture name
                token = getToken(line, " \t\n\r\"", start);

                // Read the texture
                if (!token.empty())
                    texture2 = readTextureFile(token);
            }
            else if ((equalCaseInsensitive(token, "$translucent")) ||
                     (equalCaseInsensitive(token, "$alphatest")))
            {
                // Get the translucency setting
                token = getToken(line, " \t\n\r\"", start);

                // Interpret the setting
                if ((token == "1") || (token == "true"))
                     translucent = true;
            }
            else if (equalCaseInsensitive(token, "$alpha"))
            {
                // Get the translucency setting
                token = getToken(line, " \t\n\r\"", start);

                // Interpret the setting
                if (!token.empty())
                {
                   alpha = osg::asciiToDouble(token.c_str());
                }
            }

            // Try the next token
            token = getToken(line, " \t\n\r\"", start);
        }
    }

    // Start with no StateSet (in case the stuff below fails)
    stateSet = NULL;

    // Check the shader's name
    if (equalCaseInsensitive(shaderName, "WorldVertexTransition"))
    {
        // Make sure we have both textures
        if (texture.valid() && texture2.valid())
        {
            // Create a StateSet for the following state
            stateSet = new osg::StateSet();

            // Attach the two textures
            stateSet->setTextureAttributeAndModes(0, texture.get(),
                                                  osg::StateAttribute::ON);
            stateSet->setTextureAttributeAndModes(1, texture2.get(),
                                                  osg::StateAttribute::ON);

            // On the first texture unit, set up a combiner operation to
            // interpolate between the textures on units 0 and 1, using
            // the fragment's primary alpha color as the interpolation
            // parameter (NOTE: we need ARB_texture_env_crossbar for this)
            combiner0 = new osg::TexEnvCombine();
            combiner0->setConstantColor(osg::Vec4f(1.0, 1.0, 1.0, 1.0));

            combiner0->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
            combiner0->setSource0_RGB(osg::TexEnvCombine::TEXTURE0);
            combiner0->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
            combiner0->setSource1_RGB(osg::TexEnvCombine::TEXTURE1);
            combiner0->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
            combiner0->setSource2_RGB(osg::TexEnvCombine::PRIMARY_COLOR);
            combiner0->setOperand2_RGB(osg::TexEnvCombine::SRC_ALPHA);

            combiner0->setCombine_Alpha(osg::TexEnvCombine::REPLACE);
            combiner0->setSource0_Alpha(osg::TexEnvCombine::CONSTANT);
            combiner0->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);

            combiner0->setScale_RGB(1.0);
            combiner0->setScale_Alpha(1.0);

            stateSet->setTextureAttributeAndModes(0, combiner0.get(),
                                                  osg::StateAttribute::ON);

            // On the second texture unit, do a typical modulate operation
            // between the interpolated texture color from the previous
            // unit and the fragment's primary (lit) RGB color.  Force the
            // alpha to be 1.0, since this HL2 shader is never transparent
            combiner1 = new osg::TexEnvCombine();
            combiner1->setConstantColor(osg::Vec4f(1.0, 1.0, 1.0, 1.0));

            combiner1->setCombine_RGB(osg::TexEnvCombine::MODULATE);
            combiner1->setSource0_RGB(osg::TexEnvCombine::PREVIOUS);
            combiner1->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
            combiner1->setSource1_RGB(osg::TexEnvCombine::PRIMARY_COLOR);
            combiner1->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);

            combiner1->setCombine_Alpha(osg::TexEnvCombine::REPLACE);
            combiner1->setSource0_Alpha(osg::TexEnvCombine::CONSTANT);
            combiner1->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);

            combiner1->setScale_RGB(1.0);
            combiner1->setScale_Alpha(1.0);

            stateSet->setTextureAttributeAndModes(1, combiner1.get(),
                                                  osg::StateAttribute::ON);

            // Add a material to the state set
            material = new Material();
            material->setAmbient(Material::FRONT_AND_BACK,
                                 Vec4(1.0, 1.0, 1.0, 1.0) );
            material->setDiffuse(Material::FRONT_AND_BACK,
                                 Vec4(1.0, 1.0, 1.0, 1.0) );
            material->setSpecular(Material::FRONT_AND_BACK,
                                  Vec4(0.0, 0.0, 0.0, 1.0) );
            material->setShininess(Material::FRONT_AND_BACK, 1.0);
            material->setEmission(Material::FRONT_AND_BACK,
                              Vec4(0.0, 0.0, 0.0, 1.0) );
            material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
            stateSet->setAttributeAndModes(material.get(), StateAttribute::ON);
        }
    }
    else if (equalCaseInsensitive(shaderName, "UnlitGeneric"))
    {
        // Create the StateSet
        stateSet = new StateSet();

        // Disable lighting on this StateSet
        stateSet->setMode(GL_LIGHTING, StateAttribute::OFF);

        // Add the texture attribute (or disable texturing if no base texture)
        if (texture != NULL)
        {
            stateSet->setTextureAttributeAndModes(0, texture.get(),
                                                  StateAttribute::ON);
            stateSet->setTextureAttributeAndModes(0,
                                                  new TexEnv(TexEnv::MODULATE),
                                                  StateAttribute::ON);

            // See if the material is translucent
            if (translucent)
            {
                // Create and apply a blend function attribute to the
                // state set
                blend = new BlendFunc(BlendFunc::SRC_ALPHA,
                                      BlendFunc::ONE_MINUS_SRC_ALPHA);
                stateSet->setAttributeAndModes(blend.get(), StateAttribute::ON);

                // Set the state set's rendering hint to transparent
                stateSet->setRenderingHint(StateSet::TRANSPARENT_BIN);
            }
        }
        else
        {
            OSG_WARN << "No base texture for material " << materialName;
            OSG_WARN << std::endl;
            stateSet->setTextureMode(0, GL_TEXTURE_2D, StateAttribute::OFF);
        }
    }
    else
    {
        // All other shaders fall back to fixed function
        // TODO:  LightMappedGeneric shader

        // Create the StateSet
        stateSet = new StateSet();

        // Add a material to the state set
        material = new Material();
        material->setAmbient(Material::FRONT_AND_BACK,
                             Vec4(1.0, 1.0, 1.0, 1.0) );
        material->setDiffuse(Material::FRONT_AND_BACK,
                             Vec4(1.0, 1.0, 1.0, 1.0) );
        material->setSpecular(Material::FRONT_AND_BACK,
                              Vec4(0.0, 0.0, 0.0, 1.0) );
        material->setShininess(Material::FRONT_AND_BACK, 1.0);
        material->setEmission(Material::FRONT_AND_BACK,
                              Vec4(0.0, 0.0, 0.0, 1.0) );
        material->setAlpha(Material::FRONT_AND_BACK, alpha);
        stateSet->setAttributeAndModes(material.get(), StateAttribute::ON);

        // Add the texture attribute (or disable texturing if no base texture)
        if (texture != NULL)
        {
            stateSet->setTextureAttributeAndModes(0, texture.get(),
                                                  StateAttribute::ON);
            stateSet->setTextureAttributeAndModes(0,
                                                  new TexEnv(TexEnv::MODULATE),
                                                  StateAttribute::ON);

            // See if the material is translucent
            if (translucent)
            {
                // Create and apply a blend function attribute to the
                // state set
                blend = new BlendFunc(BlendFunc::SRC_ALPHA,
                                      BlendFunc::ONE_MINUS_SRC_ALPHA);
                stateSet->setAttributeAndModes(blend.get(), StateAttribute::ON);

                // Set the state set's rendering hint to transparent
                stateSet->setRenderingHint(StateSet::TRANSPARENT_BIN);
            }
        }
        else
        {
            OSG_WARN << "No base texture for material " << materialName;
            OSG_WARN << std::endl;
            stateSet->setTextureMode(0, GL_TEXTURE_2D, StateAttribute::OFF);
        }
    }

    // Close the file
    mtlFile->close();

    // Return the resulting StateSet
    return stateSet;
}


void VBSPReader::createScene()
{
    ref_ptr<Group>              group;
    ref_ptr<Group>              subGroup;
    TexData                     currentTexData;
    const char *                texName;
    char                        currentTexName[256];
    char                        prefix[64];
    char *                      mtlPtr;
    char *                      tmpPtr;
    char                        tempTex[256];
    std::string                 entityText;
    VBSPEntity *                currentEntity;
    int                         i;
    ref_ptr<StateSet>           stateSet;
    StaticProp                  staticProp;
    Matrixf                     transMat, rotMat;
    Quat                        yaw, pitch, roll;
    ref_ptr<MatrixTransform>    propXform;
    std::string                 propModel;
    ref_ptr<Node>               propNode;

    // Load the materials and create a StateSet for each one
    for (i = 0; i < bsp_data->getNumTexDatas(); i++)
    {
        // Get the texdata entry and texture name
        currentTexData = bsp_data->getTexData(i);
        texName = bsp_data->getTexDataString(currentTexData.name_string_table_id).c_str();

        osgDB::stringcopyfixedsize(currentTexName, texName);


        // See if this is referring to an environment mapped material (we don't
        // handle this yet)
        sprintf(prefix, "maps/%s/", map_name.c_str());
        if (strncmp(currentTexName, prefix, strlen(prefix)) == 0)
        {
            // This texture is referring to this map's PAK file, so it could
            // be an environment mapped texture (an existing material that is
            // modified by a cube map of the scene).  If so, we just need to
            // get the base material name
            mtlPtr = currentTexName;
            mtlPtr += strlen(prefix);

            // Now, we're pointing at the path to the material itself, so copy
            // what we've got so far
            osgDB::stringcopyfixedsize(tempTex, mtlPtr);

            // Now, we just need to trim the two or three cube map coordinates
            // from the end.
            // This isn't a perfect solution, but it should catch most cases.
            // The right way to do this would be to read the .vmt file from the
            // map's PAKFILE lump, and make use of the basetexture parameter in
            // it
            tmpPtr = strrchr(tempTex, '/');
            mtlPtr = strrchr(tempTex, '_');
            if ((mtlPtr != NULL) && (mtlPtr > tmpPtr))
                *mtlPtr = 0;
            mtlPtr = strrchr(tempTex, '_');
            if ((mtlPtr != NULL) && (mtlPtr > tmpPtr))
                *mtlPtr = 0;
            mtlPtr = strrchr(tempTex, '_');
            if ((mtlPtr != NULL) && (mtlPtr > tmpPtr))
                *mtlPtr = 0;

            // That should be it, so make it the texture name
            strcpy(currentTexName, tempTex);
        }

        // Read the material for this geometry
        stateSet = readMaterialFile(currentTexName);

        // Whether we successfully created a StateSet or not, add it to the
        // bsp data list now
        bsp_data->addStateSet(stateSet.get());
    }

    // Create the root group for the scene
    group = new Group();

    // Iterate through the list of entities, and try to convert all the
    // visible entities to geometry
    for (i = 0; i < bsp_data->getNumEntities(); i++)
    {
        // Get the entity
        entityText = bsp_data->getEntity(i);
        currentEntity = new VBSPEntity(entityText, bsp_data.get());

        // See if the entity is visible
        if (currentEntity->isVisible())
        {
            // Create geometry for the entity
            subGroup = currentEntity->createGeometry();

            // If the entity's geometry is valid, add it to the main group
            if (subGroup.valid())
                group->addChild(subGroup.get());
        }

        // Done with this entity
        delete currentEntity;
    }

    // Iterate through the list of static props, and add them to the scene
    // as well
    for (i = 0; i < bsp_data->getNumStaticProps(); i++)
    {
        // Get the static prop
        staticProp = bsp_data->getStaticProp(i);

        // Create a MatrixTransform for this prop (scale the position from
        // inches to meters)
        transMat.makeTranslate(staticProp.prop_origin * 0.0254);
        pitch.makeRotate(osg::DegreesToRadians(staticProp.prop_angles.x()),
                         Vec3f(0.0, 1.0, 0.0));
        yaw.makeRotate(osg::DegreesToRadians(staticProp.prop_angles.y()),
                       Vec3f(0.0, 0.0, 1.0));
        roll.makeRotate(osg::DegreesToRadians(staticProp.prop_angles.z()),
                        Vec3f(1.0, 0.0, 0.0));
        rotMat.makeRotate(roll * pitch * yaw);
        propXform = new MatrixTransform();
        propXform->setMatrix(rotMat * transMat);

        // Load the prop's model
        propModel = bsp_data->getStaticPropModel(staticProp.prop_type);
        propNode = osgDB::readRefNodeFile(propModel);

        // If we loaded the prop correctly, add it to the scene
        if (propNode.valid())
        {
            // Add the model to the transform node, and attach the transform
            // to the scene
            propXform->addChild(propNode.get());
            group->addChild(propXform.get());

            // Name the prop
            propXform->setName(std::string("prop_static:" + propModel));
        }
        else
        {
            OSG_WARN << "Couldn't find static prop \"" << propModel;
            OSG_WARN << "\"." << std::endl;

            // Couldn't find the prop, so get rid of the transform node
            propXform = NULL;
        }
    }

    // Set the root node to the result
    root_node = group.get();
}


bool VBSPReader::readFile(const std::string & file)
{
    osgDB::ifstream *   mapFile = 0;
    Header              header;
    int                 i = 0;

    // Remember the map name
    map_name = getStrippedName(file);

    mapFile = new osgDB::ifstream(file.c_str(), std::ios::binary);
    if (!mapFile)
        return false;

    // Read the header
    mapFile->read((char *) &header, sizeof(Header));

    // Load the bsp file lumps that we care about
    for (i = 0; i < MAX_LUMPS; i++)
    {
        if ((header.lump_table[i].file_offset != 0) &&
            (header.lump_table[i].lump_length != 0))
        {
            // Process the lump
            switch (i)
            {
                case ENTITIES_LUMP:
                    processEntities(*mapFile, header.lump_table[i].file_offset,
                                    header.lump_table[i].lump_length);
                    break;
                case PLANES_LUMP:
                    processPlanes(*mapFile, header.lump_table[i].file_offset,
                                  header.lump_table[i].lump_length);
                    break;
                case VERTICES_LUMP:
                    processVertices(*mapFile, header.lump_table[i].file_offset,
                                    header.lump_table[i].lump_length);
                    break;
                case EDGES_LUMP:
                    processEdges(*mapFile, header.lump_table[i].file_offset,
                                 header.lump_table[i].lump_length);
                    break;
                case SURFEDGES_LUMP:
                    processSurfEdges(*mapFile, header.lump_table[i].file_offset,
                                     header.lump_table[i].lump_length);
                    break;
                case MODELS_LUMP:
                    processModels(*mapFile, header.lump_table[i].file_offset,
                                  header.lump_table[i].lump_length);
                    break;
                case FACES_LUMP:
                    processFaces(*mapFile, header.lump_table[i].file_offset,
                                 header.lump_table[i].lump_length);
                    break;
                case TEXINFO_LUMP:
                    processTexInfo(*mapFile, header.lump_table[i].file_offset,
                                   header.lump_table[i].lump_length);
                    break;
                case TEXDATA_LUMP:
                    processTexData(*mapFile, header.lump_table[i].file_offset,
                                   header.lump_table[i].lump_length);
                    break;
                case TEXDATA_STRING_TABLE_LUMP:
                    processTexDataStringTable(*mapFile,
                                              header.lump_table[i].file_offset,
                                              header.lump_table[i].lump_length);
                    break;
                case TEXDATA_STRING_DATA_LUMP:
                    processTexDataStringData(*mapFile,
                                             header.lump_table[i].file_offset,
                                             header.lump_table[i].lump_length);
                    break;
                case DISPINFO_LUMP:
                    processDispInfo(*mapFile, header.lump_table[i].file_offset,
                                    header.lump_table[i].lump_length);
                    break;
                case DISP_VERTS_LUMP:
                    processDispVerts(*mapFile, header.lump_table[i].file_offset,
                                     header.lump_table[i].lump_length);
                    break;
                case GAME_LUMP:
                    processGameData(*mapFile, header.lump_table[i].file_offset,
                                    header.lump_table[i].lump_length);
                    break;
            }
        }
    }

    // Create the OSG scene from the BSP data
    createScene();
    return true;
}


ref_ptr<Node> VBSPReader::getRootNode()
{
    return root_node;
}


