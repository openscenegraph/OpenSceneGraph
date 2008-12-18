#include <osg/BoundingSphere>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/Object>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/Node>
#include <osg/Notify>
#include <osg/StateSet>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osg/io_utils>

#include <iostream>
#include <string.h>

#include "VBSPReader.h"
#include "VBSPGeometry.h"


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

    // No map data yet
    entity_list = NULL;
    vertex_list = NULL;
    edge_list = NULL;
    surface_edges = NULL;
    face_list = NULL;
}


VBSPReader::~VBSPReader()
{
    // Clean up the data from the VBSP file
    delete [] entity_list;
    delete [] plane_list;
    delete [] vertex_list;
    delete [] edge_list;
    delete [] surface_edges;
    delete [] face_list;
    delete [] texinfo_list;
    delete [] texdata_list;
    delete [] texdata_string_table;
    delete [] texdata_string_data;
    delete [] dispinfo_list;
    delete [] displaced_vertex_list;
}


void VBSPReader::processEntities(std::istream & str, int offset,
                                 int length)
{
    char *   entities;
    char *   startPtr;
    char *   endPtr;
    int      i;
    int      entityLen;

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
    num_entities = 0;
    while ((startPtr != NULL) && (endPtr != NULL))
    {
        // Increment the count
        num_entities++;

        // Advance the pointers
        startPtr = strchr(endPtr, '{');
        if (startPtr != NULL)
            endPtr = strchr(startPtr, '}');
    }

    // Create the entity list
    entity_list = new char * [num_entities];
    memset(entity_list, 0, num_entities * sizeof(char *));

    // Parse the entities
    startPtr = entities;
    endPtr = strchr(entities, '}');
    for (i = 0; i < num_entities; i++)
    {
        // Get the length of this entity
        entityLen = endPtr - startPtr + 1;

        // Create the entity list entry and copy the entity information
        entity_list[i] = new char[entityLen + 1];
        strncpy(entity_list[i], startPtr, entityLen);

        // Advance the pointers
        startPtr = strchr(endPtr, '{');
        if (startPtr != NULL)
            endPtr = strchr(startPtr, '}');
    }

    // Free up the original entities string
    delete [] entities;
}


void VBSPReader::processPlanes(std::istream & str, int offset, int length)
{
    // Calculate the number of planes
    num_planes = length / sizeof(Plane);

    // Create the plane list
    plane_list = new Plane[num_planes];

    // Seek to the Planes lump
    str.seekg(offset);

    // Read in the planes
    str.read((char *) plane_list, sizeof(Plane) * num_planes);
}


void VBSPReader::processVertices(std::istream & str, int offset, int length)
{
    // Calculate the number of vertices
    num_vertices = length / 3 / sizeof(float);

    // Create the vertex list
    vertex_list = new Vec3f[num_vertices];

    // Seek to the Vertices lump
    str.seekg(offset);

    // Read in the vertices
    str.read((char *) vertex_list, sizeof(Vec3f) * num_vertices);
}


void VBSPReader::processEdges(std::istream & str, int offset, int length)
{
    // Calculate the number of edges
    num_edges = length / sizeof(Edge);

    // Create the edge list
    edge_list = new Edge[num_edges];

    // Seek to the Edges lump
    str.seekg(offset);

    // Read in the edge list
    str.read((char *) edge_list, sizeof(Edge) * num_edges);
}


void VBSPReader::processSurfEdges(std::istream & str, int offset, int length)
{
    // Calculate the number of edges
    num_surf_edges = length / sizeof(int);

    // Create the surface edges list
    surface_edges = new int[num_surf_edges];

    // Seek to the SurfEdges lump
    str.seekg(offset);

    // Read in the surface edge list
    str.read((char *) surface_edges, sizeof(int) * num_surf_edges);
}


void VBSPReader::processFaces(std::istream & str, int offset, int length)
{
    // Calculate the number of faces
    num_faces = length / sizeof(Face);

    // Create the face list
    face_list = new Face[num_faces];

    // Seek to the Faces lump
    str.seekg(offset);

    // Read in the faces
    str.read((char *) face_list, sizeof(Face) * num_faces);
}


void VBSPReader::processTexInfo(std::istream & str, int offset, int length)
{
    // Calculate the number of texinfos
    num_texinfo_entries = length / sizeof(TexInfo);

    // Create the texinfo list
    texinfo_list = new TexInfo[num_texinfo_entries];

    // Seek to the TexInfo lump
    str.seekg(offset);

    // Read in the texinfo entries
    str.read((char *) texinfo_list, sizeof(TexInfo) * num_texinfo_entries);
}


void VBSPReader::processTexData(std::istream & str, int offset, int length)
{
    // Calculate the number of texdatas
    num_texdata_entries = length / sizeof(TexData);

    // Create the texdata list
    texdata_list = new TexData[num_texdata_entries];

    // Seek to the TexData lump
    str.seekg(offset);

    // Read in the texdata entries
    str.read((char *) texdata_list, sizeof(TexData) * num_texdata_entries);
}


void VBSPReader::processTexDataStringTable(std::istream & str, int offset,
                                           int length)
{
    // Calculate the number of table entries
    num_texdata_string_table_entries = length / sizeof(int);

    // Create the texdata string table
    texdata_string_table = new int[num_texdata_string_table_entries];

    // Seek to the TexDataStringTable lump
    str.seekg(offset);

    // Read in the texdata_string_table
    str.read((char *) texdata_string_table,
             sizeof(int) * num_texdata_string_table_entries);
}


void VBSPReader::processTexDataStringData(std::istream & str, int offset,
                                          int length)
{
    char *   texDataString;
    char *   startPtr;
    char *   endPtr;
    int      i;
    int      stringLen;

    // Create the buffer to load the texdata strings
    texDataString = new char[length];
    memset(texDataString, 0, length * sizeof(char));

    // Seek to the TexDataStringData lump
    str.seekg(offset);

    // Read the entire texdata string (this string is actually a
    // NULL-delimited list of strings)
    str.read((char *) texDataString, sizeof(char) * length);

    // Count the number of strings
    startPtr = texDataString;
    endPtr = startPtr + strlen(startPtr);
    num_texdata_strings = 0;
    while ((startPtr - texDataString) < length)
    {
        // Increment the count
        num_texdata_strings++;

        // Advance the pointers
        startPtr = endPtr+1;
        if ((startPtr - texDataString) < length)
            endPtr = startPtr + strlen(startPtr);
    }

    // Create the texdata string list
    texdata_string_data = new char * [num_texdata_strings];
    memset(texdata_string_data, 0, num_texdata_strings * sizeof(char *));

    // Parse the texdata strings
    startPtr = texDataString;
    endPtr = startPtr + strlen(startPtr);
    for (i = 0; i < num_texdata_strings; i++)
    {
        // Get the length of this string
        stringLen = endPtr - startPtr + 1;

        // Create the texdata string list entry and copy the string
        texdata_string_data[i] = new char[stringLen + 1];
        strncpy(texdata_string_data[i], startPtr, stringLen);

        // Advance the pointers
        startPtr = endPtr+1;
        if ((startPtr - texDataString) < length)
            endPtr = startPtr + strlen(startPtr);
    }

    // Free up the original texdata string
    free(texDataString);
}


void VBSPReader::processDispInfo(std::istream & str, int offset, int length)
{
    // Calculate the number of texinfos
    num_dispinfo_entries = length / sizeof(DisplaceInfo);

    // Create the texinfo list
    dispinfo_list = new DisplaceInfo[num_dispinfo_entries];

    // Seek to the DispInfo lump
    str.seekg(offset);

    // Read in the texinfo entries
    str.read((char *) dispinfo_list,
             sizeof(DisplaceInfo) * num_dispinfo_entries);
}


void VBSPReader::processDispVerts(std::istream & str, int offset, int length)
{
    // Calculate the number of displaced vertices
    num_displaced_vertices = length / sizeof(DisplacedVertex);

    // Create the texinfo list
    displaced_vertex_list = new DisplacedVertex[num_displaced_vertices];

    // Seek to the DispVerts lump
    str.seekg(offset);

    // Read in the displaced vertices
    str.read((char *) displaced_vertex_list,
             sizeof(DisplacedVertex) * num_displaced_vertices);
}


std::string VBSPReader::getToken(std::string str, const char * delim,
                                 std::string::size_type & index)
{
    std::string   token;

    // Look for the first non-occurrence of the delimiters
    std::string::size_type start = str.find_first_not_of(" \t\n\r\"", index);
    std::string::size_type end = std::string::npos;
    if (start != std::string::npos)
    {
        // From there, look for the first occurrence of a delimiter
        end = str.find_first_of(" \t\n\r\"", start+1);
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


std::string VBSPReader::findFileIgnoreCase(std::string filePath)
{
    std::string         path;
    std::string         file;
    DirectoryContents   dirList;
    std::string         result;

    // Split the filename from the directory
    path = getFilePath(filePath);
    file = getSimpleFileName(filePath);

    // Get the list of files in this directory
    dirList = getDirectoryContents(path);

    // Recursively find the path (in case some path elements are in the wrong
    // case)
    if (path.empty())
    {
        path = ".";
        dirList = getDirectoryContents(".");
    }
    else
    {
        path = findFileIgnoreCase(path);
        dirList = getDirectoryContents(path);
    }

    // Look for matching file in the directory, without regard to case
    DirectoryContents::iterator itr;
    for (itr = dirList.begin(); itr != dirList.end(); itr++)
    {
        if (equalCaseInsensitive(file, *itr))
        {
            // This file matches, so return it
            result = concatPaths(path, *itr);
            return result;
        }
    }

    // Return an empty string
    return std::string();
}


ref_ptr<Texture> VBSPReader::readTextureFile(std::string textureName)
{
    std::string   texFile;
    std::string   texPath;
    Image *       texImage;
    Texture *     texture;

    // Find the texture's image file
    texFile = std::string(textureName) + ".vtf";
    texPath = findFileIgnoreCase(texFile);

    // If we don't find it right away, check in a "materials" subdirectory
    if (texPath.empty())
    {
        texFile = "materials/" + std::string(textureName) + ".vtf";
        texPath = findFileIgnoreCase(texFile);

        // Check up one directory if we don't find it here (the map file is 
        // usually located in the "maps" directory, adjacent to the materials
        // directory)
        if (texPath.empty())
        {
            texFile = "../materials/" + std::string(textureName) + ".vtf";
            texPath = findFileIgnoreCase(texFile);
        }
    }

    // If we found the file, read it, otherwise bail
    if (!texPath.empty())
    {
        texImage = readImageFile(texPath);

        // If we got the image, create the texture attribute
        if (texImage != NULL)
        {
            // Create the texture
            if (texImage->t() == 1)
            {
                texture = new Texture1D();
                ((Texture1D *)texture)->setImage(texImage);
            }
            else if (texImage->r() == 1)
            {
                texture = new Texture2D();
                ((Texture2D *)texture)->setImage(texImage);
            }
            else
            {
                texture = new Texture3D();
                ((Texture3D *)texture)->setImage(texImage);
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
            notify(WARN) << "Couldn't find texture " << textureName;
            notify(WARN) << std::endl;

            // No texture
            texture = NULL;
        }
    }
    else
    {
        // We were unable to find the texture file
        notify(WARN) << "Couldn't find texture " << textureName;
        notify(WARN) << std::endl;

        // No texture
        texture = NULL;
    }

    return texture;
}


ref_ptr<StateSet> VBSPReader::createBlendShader(Texture * tex1, Texture * tex2)
{
    const char *  blendVtxShaderCode = 
    {
        "attribute float vBlendParam;\n"
        "\n"
        "varying float fBlendParam;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "   vec3 normal, lightDir;\n"
        "   vec4 ambient, diffuse;\n"
        "   float nDotL;\n"
        "\n"
        "   // Simple directional lighting (for now).  We're assuming a\n"
        "   // single light source\n"
        "   // TODO:  This is only used for terrain geometry, so it should be\n"
        "   //        lightmapped\n"
        "   normal = normalize(gl_NormalMatrix * gl_Normal);\n"
        "   lightDir = normalize(vec3(gl_LightSource[0].position));\n"
        "   nDotL = max(dot(normal, lightDir), 0.0);\n"
        "   ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;\n"
        "   diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;\n"
        "\n"
        "   // Calculate the vertex color\n"
        "   gl_FrontColor =  0.1 + ambient + nDotL * diffuse;\n"
        "\n"
        "   // Pass the texture blend parameter through to the fragment\n"
        "   // shader\n"
        "   fBlendParam = vBlendParam;\n"
        "\n"
        "   // The basic transforms\n"
        "   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "   gl_TexCoord[0] = vec4(gl_MultiTexCoord0.st, 0.0, 0.0);\n"
        "}\n"
    };

    const char *  blendFrgShaderCode = 
    {
        "uniform sampler2D tex1;\n"
        "uniform sampler2D tex2;\n"
        "\n"
        "varying float fBlendParam;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "   vec4 tex1Color;\n"
        "   vec4 tex2Color;\n"
        "\n"
        "   tex1Color = texture2D(tex1, gl_TexCoord[0].st) * fBlendParam;\n"
        "   tex2Color = texture2D(tex2, gl_TexCoord[0].st) *\n"
        "      (1.0 - fBlendParam);\n"
        "\n"
        "   gl_FragColor = gl_Color * (tex1Color + tex2Color);\n"
        "}\n"
    };

    // Create the stateset
    StateSet * stateSet = new StateSet();
 
    // Add the two textures
    stateSet->setTextureAttributeAndModes(0, tex1, StateAttribute::ON);
    stateSet->setTextureAttributeAndModes(1, tex2, StateAttribute::ON);

    // Create the vertex and fragment shaders
    Shader * blendVtxShader = new Shader(Shader::VERTEX);
    blendVtxShader->setShaderSource(blendVtxShaderCode);
    Shader * blendFrgShader = new Shader(Shader::FRAGMENT);
    blendFrgShader->setShaderSource(blendFrgShaderCode);

    // Create the two texture uniforms
    Uniform * tex1Sampler = new Uniform(Uniform::SAMPLER_2D, "tex1");
    tex1Sampler->set(0);
    Uniform * tex2Sampler = new Uniform(Uniform::SAMPLER_2D, "tex2");
    tex1Sampler->set(1);

    // Create the program
    Program * blendProgram = new Program();
    blendProgram->addShader(blendVtxShader);
    blendProgram->addShader(blendFrgShader);

    // The texture blending parameter will be on vertex attribute 1
    blendProgram->addBindAttribLocation("vBlendParam", (GLuint) 1);

    // Add everything to the StateSet
    stateSet->addUniform(tex1Sampler);
    stateSet->addUniform(tex2Sampler);
    stateSet->setAttributeAndModes(blendProgram, StateAttribute::ON);

    // Return the StateSet
    return stateSet;
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
    std::string             texName;
    std::string             tex2Name;
    ref_ptr<Texture>        texture;
    ref_ptr<Texture>        texture2;

    // Find the material file
    mtlFileName = std::string(materialName) + ".vmt";
    mtlPath = findFileIgnoreCase(mtlFileName);

    // If we don't find it right away, check in a "materials" subdirectory
    if (mtlPath.empty())
    {
        mtlFileName = "materials/" + std::string(materialName) + ".vmt";
        mtlPath = findFileIgnoreCase(mtlFileName);

        // Check up one directory if we don't find it here (the map file is
        // usually located in the "maps" directory, adjacent to the materials
        // directory)
        if (mtlPath.empty())
        {
            mtlFileName = "../materials/" + std::string(materialName) + ".vmt";
            mtlPath = findFileIgnoreCase(mtlFileName);
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
        notify(WARN) << "Can't find material " << materialName << std::endl;
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
        notify(WARN) << "Material " << materialName << " isn't valid.";
        notify(WARN) << std::endl;
        return NULL;
    }

    // No textures loaded yet
    texture = NULL;
    texture2 = NULL;

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
 
            // Try the next token
            token = getToken(line, " \t\n\r\"", start);
        }
    }

    // Start with no StateSet (in case the stuff below fails)
    stateSet = NULL;

    // Check the shader's name
    if (equalCaseInsensitive(shaderName, "WorldVertexTransition"))
    {
        // This shader blends between two textures based on a per-vertex
        // attribute.  This is used for displaced terrain surfaces in HL2 maps.
        stateSet = createBlendShader(texture.get(), texture2.get());
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
        }
        else
        {
            notify(WARN) << "No base texture for material " << materialName;
            notify(WARN) << std::endl;
            stateSet->setTextureMode(0, GL_TEXTURE_2D, StateAttribute::OFF);
        }
    }
    else
    {
        // All other shaders fall back to fixed function
        // TODO:  LightMappedGeneric shader

        // Create the StateSet
        stateSet = new StateSet();

        // Add the texture attribute (or disable texturing if no base texture)
        if (texture != NULL)
        {
            stateSet->setTextureAttributeAndModes(0, texture.get(),
                                                  StateAttribute::ON);
        }
        else
        {
            notify(WARN) << "No base texture for material " << materialName;
            notify(WARN) << std::endl;
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
    ref_ptr<Group>       group;
    VBSPGeometry **      vbspGeomList  = 0;
    ref_ptr<Group>       subGroup;
    Face                 currentFace;
    TexInfo              currentTexInfo;
    TexData              currentTexData;
    char *               currentTexName = 0;
    char                 prefix[64];
    char *               mtlPtr = 0;
    char *               tmpPtr = 0;
    char                 tempTex[256];
    int                  i;
    ref_ptr<StateSet>    stateSet;

    // Create the root group for the scene
    group = new Group();

    // Create VBSPGeometry object for each texdata entry in the scene
    // These objects will hold the necessary geometry data until we convert
    // them back into OSG geometry objects.  We need one for each texdata
    // entry because we'll need separate state sets for each one when they're
    // converted
    vbspGeomList = new VBSPGeometry *[num_texdata_entries];
    for (i = 0; i < num_texdata_entries; i++)
        vbspGeomList[i] = new VBSPGeometry(this);

    // Iterate over the face list and assign faces to the appropriate geometry
    // objects
    for (i = 0; i < num_faces; i++)
    {
        // Get the current face
        currentFace = face_list[i];

        // Get the texdata used by this face
        currentTexInfo = texinfo_list[currentFace.texinfo_index];
        currentTexData = texdata_list[currentTexInfo.texdata_index];

        // Get the texture name
        currentTexName = 
            texdata_string_data[currentTexData.name_string_table_id];

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
            // Add the face to the appropriate VBSPGeometry object
            vbspGeomList[currentTexInfo.texdata_index]->addFace(i);
        }
    }

    // Assemble the geometry
    for (i = 0; i < num_texdata_entries; i++)
    {
        // Create the OSG geometry from the VBSP geometry
        subGroup = vbspGeomList[i]->createGeometry();

        // Get the texdata entry and texture name
        currentTexData = texdata_list[i];
        currentTexName =
            texdata_string_data[currentTexData.name_string_table_id];

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
            strcpy(tempTex, mtlPtr);

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
            currentTexName = tempTex;
        }

        // Read the material for this geometry
        stateSet = readMaterialFile(currentTexName);

        // If we successfully created a StateSet, add it now
        if (stateSet != NULL)
            subGroup->setStateSet(stateSet.get());

        // Add the geometry to the main group
        group->addChild(subGroup.get());
    }

    // Clean up
    delete [] vbspGeomList;

    // Set the root node to the result
    root_node = group.get();
}


const char * VBSPReader::getEntity(int index) const
{
    return entity_list[index];
}


const bsp::Plane & VBSPReader::getPlane(int index) const
{
    return plane_list[index];
}


const osg::Vec3f & VBSPReader::getVertex(int index) const
{
    return vertex_list[index];
}


const Edge & VBSPReader::getEdge(int index) const
{
    return edge_list[index];
}


const int VBSPReader::getSurfaceEdge(int index) const
{
    return surface_edges[index];
}


const Face & VBSPReader::getFace(int index) const
{
    return face_list[index];
}


const TexInfo & VBSPReader::getTexInfo(int index) const
{
    return texinfo_list[index];
}


const TexData & VBSPReader::getTexData(int index) const
{
    return texdata_list[index];
}


const char * VBSPReader::getTexDataString(int index) const
{
    return texdata_string_data[index];
}


const DisplaceInfo & VBSPReader::getDispInfo(int index) const
{
    return dispinfo_list[index];
}


const DisplacedVertex & VBSPReader::getDispVertex(int index) const
{
    return displaced_vertex_list[index];
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


