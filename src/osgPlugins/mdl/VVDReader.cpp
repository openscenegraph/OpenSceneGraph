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

#include "VVDReader.h"


using namespace mdl;
using namespace osg;
using namespace osgDB;


VVDReader::VVDReader()
{
    // Initialize the vertex buffer arrays
    memset(vertex_buffer, 0, sizeof(vertex_buffer));
    memset(vertex_buffer_size, 0, sizeof(vertex_buffer_size));
}


VVDReader::~VVDReader()
{
    int i;

    // Clean up the vertex buffer arrays
    for (i = 0; i < MAX_LODS; i++)
        delete [] vertex_buffer[i];
}


bool VVDReader::readFile(const std::string & file)
{
    osgDB::ifstream *   vvdFile;
    VVDHeader           header;
    int                 vertIndex;
    int                 i, j;

    // Remember the map name
    vvd_name = getStrippedName(file);

    vvdFile = new osgDB::ifstream(file.c_str(), std::ios::binary);
    if (!vvdFile)
    {
        notify(NOTICE) << "Vertex data file not found" << std::endl;
        return false;
    }

    // Read the header
    memset(&header, 0xcd, sizeof(VVDHeader));
    vvdFile->read((char *) &header, sizeof(VVDHeader));

    // Make sure the file is a valid Valve VVD file
    if (header.magic_number != VVD_MAGIC_NUMBER)
    {
        notify(NOTICE) << "Vertex data file not valid" << std::endl;
        return false;
    }

    // Make sure the version is one that we handle
    // TODO:  Not sure which versions are valid yet

    // Read the fixup table
    fixup_table = new VVDFixupEntry[header.num_fixups];
    vvdFile->seekg(header.fixup_table_offset);
    for (i = 0; i < header.num_fixups; i++)
        vvdFile->read((char *) &fixup_table[i], sizeof(VVDFixupEntry));

    // Create the vertex buffers
    for (i = 0; i < header.num_lods; i++)
    { 
        // Create the vertex buffer for this LOD
        vertex_buffer[i] = new VVDVertex[header.num_lod_verts[i]];
        vertex_buffer_size[i] = header.num_lod_verts[i];

        // See if this model needs fixups
        if (header.num_fixups > 0)
        {
            // Scan the fixup table and apply any fixups at this LOD
            vertIndex = 0;
            for (j = 0; j < header.num_fixups; j++)
            {
                // Skip this entry if the LOD number is lower (more detailed)
                // than the LOD we're working on
                if (fixup_table[j].lod_number >= i)
                {
                    // Seek to the vertex indicated by the fixup table entry
                    vvdFile->seekg(header.vertex_data_offset + 
                                   fixup_table[j].source_vertex_id *
                                   sizeof(VVDVertex));

                    // Read the number of vertices specified
                    vvdFile->read((char *) &vertex_buffer[i][vertIndex],
                                  fixup_table[j].num_vertices *
                                  sizeof(VVDVertex));

                    // Advance the target index
                    vertIndex += fixup_table[j].num_vertices;
                }
            }
        }
        else
        {
            // Seek to the vertex data
            vvdFile->seekg(header.vertex_data_offset);

            // Just read the vertices directly
            vvdFile->read((char *) &vertex_buffer[i][0],
                          header.num_lod_verts[i] * sizeof(VVDVertex));
        }

        // Scale the vertices from inches up to meters
        for (j = 0; j < vertex_buffer_size[i]; j++)
            vertex_buffer[i][j].vertex_position *= 0.0254;
    }

    // Close the file
    vvdFile->close();
    delete vvdFile;

    return true;
}


int VVDReader::getNumLODVertices(int lod)
{
    return vertex_buffer_size[lod];
}


Vec3 VVDReader::getVertex(int lod, int index)
{
    return vertex_buffer[lod][index].vertex_position;
}


Vec3 VVDReader::getNormal(int lod, int index)
{
    return vertex_buffer[lod][index].vertex_normal;
}


Vec2 VVDReader::getTexCoords(int lod, int index)
{
    return vertex_buffer[lod][index].vertex_texcoord;
}

