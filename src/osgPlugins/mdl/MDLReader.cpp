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
#include <osg/StateSet>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osg/io_utils>
#include <iostream>

#include "MDLReader.h"
#include "VVDReader.h"
#include "VTXReader.h"


using namespace mdl;
using namespace osg;
using namespace osgDB;


namespace
{
    // Turn any '\' into '/', so that the path is usable on Windows and Unix.
    void sanitizePath(std::string& path)
    {
        size_t pos = 0;
        while ((pos = path.find_first_of("\\", pos)) != std::string::npos)
        {
            path[pos] = '/';
            ++pos;
        }
    }

    // Try to find 'searchPath'/'filename'.'extension' in 'materials' directory.
    std::string findFileInPath(const std::string& searchPath,
            const std::string& filename,
            const std::string& extension)
    {
        std::string filepath;

        if ((filename[0] == '\\') || (filename[0] == '/'))
        {
            filepath = searchPath + filename + extension;
        }
        else
        {
            filepath = searchPath + "/" + filename + extension;
        }

        // Look for the texture at this location
        //std::cerr << "findFileInPath trying '" << filepath << "'\n";
        filepath = findDataFile(filepath, CASE_INSENSITIVE);
        //std::cerr << "findFileInPath found '" << filepath << "'\n";

        return filepath;
    }

    // Try to find 'searchPath'/'path'/'filename'.'extension' in 'materials' directory.
    std::string findFileInPath(const std::string& searchPath,
            const std::string& path,
            const std::string& filename,
            const std::string& extension)
    {
        std::string filepath;

        if ((path[0] == '\\') || (path[0] == '/'))
        {
            filepath = searchPath + path + filename + extension;
        }
        else
        {
            filepath = searchPath + "/" + path + filename + extension;
        }

        // Look for the texture at this location
        //std::cerr << "findFileInPath trying '" << filepath << "'\n";
        filepath = findDataFile(filepath, CASE_INSENSITIVE);
        //std::cerr << "findFileInPath found '" << filepath << "'\n";

        return filepath;
    }
}

MDLReader::MDLReader()
{
    // Start with no root node
    root_node = NULL;
}


MDLReader::~MDLReader()
{
}


std::string MDLReader::getToken(std::string str, const char * delim,
                                size_t & index)
{
    size_t start;
    size_t end = std::string::npos;
    std::string   token;

    // Look for the first non-occurrence of the delimiters
    start = str.find_first_not_of(" \t\n\r\"", index);
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


ref_ptr<Texture> MDLReader::readTextureFile(std::string textureName)
{
    // Find the texture's image file
    std::string texExtension = osgDB::getFileExtensionIncludingDot(textureName);
    std::string texBaseName = osgDB::getNameLessExtension(textureName);

    if (texExtension.empty()) texExtension = ".vtf";

    std::string texFile = texBaseName + texExtension;
    std::string texPath = findDataFile(texFile, CASE_INSENSITIVE);

    // If we don't find it right away, check in a "materials" subdirectory
    if (texPath.empty())
    {
        // Check for a leading slash and concatenate appropriately
        texPath = findFileInPath("materials", texBaseName, texExtension);

        // Check up one directory if we don't find it here (the map file is
        // usually located in the "maps" directory, adjacent to the materials
        // directory)
        if (texPath.empty())
        {
            // Check for a leading slash and concatenate appropriately
            texPath = findFileInPath("../materials", texBaseName, texExtension);
        }
    }

    // If we found the file, read it, otherwise bail
    if (!texPath.empty())
    {
        osg::ref_ptr<Image> texImage = readRefImageFile(texPath);

        // If we got the image, create the texture attribute
        if (texImage.valid())
        {
            osg::ref_ptr<Texture> texture;

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

            return texture;
        }
        else
        {
            // We were unable to find the texture file
            OSG_WARN << "Couldn't find texture " << textureName << std::endl;

            // No texture
            return NULL;
        }
    }
    else
    {
        // We were unable to find the texture file
        OSG_WARN << "Couldn't find texture " << textureName << std::endl;

        // No texture
        return NULL;
    }

}


ref_ptr<StateSet> MDLReader::readMaterialFile(std::string materialName)
{
    std::string              mtlFileName;
    std::string              mtlPath;
    StringList::iterator     searchItr;
    std::ifstream *          mtlFile;
    std::string              line;
    size_t                   start;
    std::string              token;
    bool                     found;
    ref_ptr<StateSet>        stateSet;
    std::string              shaderName;
    std::string              texName;
    std::string              tex2Name;
    ref_ptr<Texture>         texture;
    ref_ptr<Texture>         texture2;
    ref_ptr<Material>        material;
    ref_ptr<BlendFunc>       blend;
    bool                     translucent;
    double                   alpha;

    // Find the material file
    mtlFileName = std::string(materialName) + ".vmt";
    mtlPath = findDataFile(mtlFileName, CASE_INSENSITIVE);

    // If we don't find it right away, search the texture file search paths
    if (mtlPath.empty())
    {
        searchItr = texture_paths.begin();
        while ((mtlPath.empty()) && (searchItr != texture_paths.end()))
        {
            std::string path = *searchItr;
            sanitizePath(path);

            // The search paths assume that materials are always located in
            // a "materials" subdirectory.  Also, check to see if there is
            // a leading slash and concatenate appropriately
            mtlPath = findFileInPath("materials", path, materialName, ".vmt");

            // Next path
            ++searchItr;
        }

        // If we still haven't found it, check up one directory level (the
        // model file is usually located in the "models" directory, adjacent
        // to the "materials" directory)
        if (mtlPath.empty())
        {
            searchItr = texture_paths.begin();
            while ((mtlPath.empty()) && (searchItr != texture_paths.end()))
            {
                std::string path = *searchItr;
                sanitizePath(path);

                // The search paths assume that materials are always located in
                // a "materials" subdirectory, but this time try going up one
                // level first
                mtlPath = findFileInPath("../materials", path, materialName, ".vmt");

                // Next path
                ++searchItr;
            }
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
        OSG_WARN << "Material " << materialName << " isn't valid." << std::endl;
        return NULL;
    }

    // No textures loaded yet
    texture = NULL;
    texture2 = NULL;

    // Assume not translucent unless the properties say otherwise
    translucent = false;

    // Assume full opacity
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
                if (!token.empty())
                {
                    if ((token == "1") || (token == "true"))
                        translucent = true;
                }
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
    if (equalCaseInsensitive(shaderName, "UnlitGeneric"))
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
            OSG_WARN << "No base texture for material " << materialName << std::endl;
            stateSet->setTextureMode(0, GL_TEXTURE_2D, StateAttribute::OFF);
        }

        // See if the material is translucent
        if (translucent)
        {
            // Add the blending attribute as well
            blend = new BlendFunc(BlendFunc::SRC_ALPHA,
                                  BlendFunc::ONE_MINUS_SRC_ALPHA);
            stateSet->setAttributeAndModes(blend.get(), StateAttribute::ON);
            stateSet->setMode(GL_BLEND, StateAttribute::ON);

            // Set the rendering hint for this stateset to transparent
            stateSet->setRenderingHint(StateSet::TRANSPARENT_BIN);
        }
    }
    else
    {
        // All other shaders fall back to fixed function

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

            // See if the material is translucent
            if ((translucent) || (alpha < 1.0))
            {
                // Add the blending attribute as well
                blend = new BlendFunc(BlendFunc::SRC_ALPHA,
                                      BlendFunc::ONE_MINUS_SRC_ALPHA);
                stateSet->setAttributeAndModes(blend.get(),
                                               StateAttribute::ON);
                stateSet->setMode(GL_BLEND, StateAttribute::ON);

                // Set the rendering hint for this stateset to transparent
                stateSet->setRenderingHint(StateSet::TRANSPARENT_BIN);
            }
        }
        else
        {
            OSG_WARN << "No base texture for material " << materialName << std::endl;
            stateSet->setTextureMode(0, GL_TEXTURE_2D, StateAttribute::OFF);
        }
    }

    // Close the file
    mtlFile->close();

    // Return the resulting StateSet
    return stateSet;
}


BodyPart * MDLReader::processBodyPart(std::istream * str, int offset)
{
    int              i;
    MDLBodyPart *    part;
    BodyPart *       partNode;
    Model *          modelNode;

    // Seek to the body part
    str->seekg(offset);

    // Read it
    part = new MDLBodyPart;
    str->read((char *) part, sizeof(MDLBodyPart));

    // Create the body part node
    partNode = new BodyPart(part);

    // Process the models
    for (i = 0; i < part->num_models; i++)
    {
        // Process the model
        modelNode = processModel(str, offset + part->model_offset +
                                      (i * sizeof(MDLModel)));

        // Add the model to the body part
        partNode->addModel(modelNode);
    }

    // Return the new node
    return partNode;
}


Model * MDLReader::processModel(std::istream * str, int offset)
{
    int            i;
    MDLModel *     model;
    Model *        modelNode;
    Mesh *         meshNode;

    // Seek to the model
    str->seekg(offset);

    // Read it
    model = new MDLModel;
    str->read((char *) model, sizeof(MDLModel));

    // Create the model node
    modelNode = new Model(model);

    // Process the meshes
    for (i = 0; i < model->num_meshes; i++)
    {
        // Process the mesh
        meshNode = processMesh(str, offset + model->mesh_offset +
                                    (i * sizeof(MDLMesh)));

        // Add the mesh to the model
        modelNode->addMesh(meshNode);
    }

    // Return the model node
    return modelNode;
}


Mesh * MDLReader::processMesh(std::istream * str, int offset)
{
    MDLMesh *      mesh;
    Mesh *         meshNode;

    // Seek to the mesh
    str->seekg(offset);

    // Read it
    mesh = new MDLMesh;
    str->read((char *) mesh, sizeof(MDLMesh));

    // Create the mesh node
    meshNode = new Mesh(mesh);

    // Set the mesh's state set based on the material id
    meshNode->setStateSet((state_sets[mesh->material_index]).get());

    // Return the mesh node
    return meshNode;
}


bool MDLReader::readFile(const std::string & file)
{
    std::string       baseName;
    std::string       fileName;
    std::ifstream *   mdlFile;
    MDLHeader         header;
    int               i;
    unsigned int      j;
    int               offset;
    MDLRoot *         mdlRoot;
    BodyPart *        partNode;
    std::string       vvdFile;
    VVDReader *       vvdReader;
    std::string       vtxFile;
    VTXReader *       vtxReader;

    // Remember the model name
    mdl_name = getStrippedName(file);

    // Try to open the file
    fileName = findDataFile(file, CASE_INSENSITIVE);
    mdlFile = new osgDB::ifstream(fileName.c_str(), std::ios::binary);
    if (!mdlFile)
    {
        OSG_NOTICE << "MDL file not found" << std::endl;
        return false;
    }

    // Read the header
    mdlFile->read((char *) &header, sizeof(MDLHeader));

    // Make sure the file is a valid Valve MDL file
    if (header.magic_number != MDL_MAGIC_NUMBER)
    {
        OSG_NOTICE << "This is not a valid .mdl file" << std::endl;

        // Close the file before we quit
        mdlFile->close();
        delete mdlFile;

        return false;
    }

    // Make sure the version is one that we handle
    // TODO:  Not sure which versions are valid yet

    // Get the texture paths from the file (we'll have to search these paths
    // for each texture that we load)
    for (i = 0; i < header.num_texture_paths; i++)
    {
        int               texPathBase;
        int               texPathOffset;
        char              texPath[256];

        texPathBase = header.texture_path_offset + (i * sizeof(int));
        mdlFile->seekg(texPathBase);
        mdlFile->read((char *) &texPathOffset, sizeof(int));
        mdlFile->seekg(texPathOffset);

        // Read characters from the file until we reach the end of this path
        j = 0;
        do
        {
            mdlFile->get(texPath[j]);
            j++;
        }
        while ((j < sizeof(texPath)) && (texPath[j-1] != 0));

        // Store this path
        texture_paths.push_back(texPath);
    }

    // Read the texture info from the file, and create a StateSet for each
    // one
    for (i = 0; i < header.num_textures; i++)
    {
        int                  texBase;
        MDLTexture           tempTex;
        char                 texName[256];
        ref_ptr<StateSet>    stateSet;

        texBase = header.texture_offset + (i * sizeof(MDLTexture));
        mdlFile->seekg(texBase);
        mdlFile->read((char *) &tempTex, sizeof(MDLTexture));
        mdlFile->seekg(texBase + tempTex.tex_name_offset);
        j = 0;
        do
        {
            mdlFile->get(texName[j]);
            j++;
        }
        while ((j < sizeof(texName)) && (texName[j-1] != 0));

        // Load this texture
        stateSet = readMaterialFile(texName);

        // Add it to our list
        state_sets.push_back(stateSet);
    }

    // Create the root node of the MDL tree
    mdlRoot = new MDLRoot();

    // Process the main model's body parts
    for (i = 0; i < header.num_body_parts; i++)
    {
        // Calculate the offset to the next body part
        offset = header.body_part_offset + (i * sizeof(MDLBodyPart));

        // Process the body part and get the part's node
        partNode = processBodyPart(mdlFile, offset);

        // Add the body part to the MDL root node
        mdlRoot->addBodyPart(partNode);
    }

    // Open the VVD (vertex data) file that goes with this model
    vvdFile = findDataFile(getNameLessExtension(file) + ".vvd",
                           CASE_INSENSITIVE);
    vvdReader = new VVDReader();
    vvdReader->readFile(vvdFile);

    // Open the VTX file (index and primitive data) that goes with this model
    // (at this point, I don't see a reason not to always just use the DX9
    // version)
    vtxFile = findDataFile(getNameLessExtension(file) + ".dx90.vtx",
                           CASE_INSENSITIVE);
    vtxReader = new VTXReader(vvdReader, mdlRoot);
    vtxReader->readFile(vtxFile);

    // Get the root group from the VTX reader
    root_node = vtxReader->getModel();

    // Close the .mdl file
    mdlFile->close();
    delete mdlFile;

    // Close the two auxiliary readers
    delete vvdReader;
    delete vtxReader;

    // Clean up the MDL tree (we don't need its data anymore)
    delete mdlRoot;

    // Return true to indicate success
    return true;
}


ref_ptr<Node> MDLReader::getRootNode()
{
    return root_node;
}


