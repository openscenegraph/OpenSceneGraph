/*
    vertexData.cpp
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.

    Implementation of the VertexData class.
*/

/** note, derived from Equalizer LGPL source.*/

#include "typedefs.h"
#include "vertexData.h"
#include "ply.h"

#include <cstdlib>
#include <algorithm>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/io_utils>
#include <osgUtil/SmoothingVisitor>

using namespace std;
using namespace ply;


/*  Contructor.  */
VertexData::VertexData()
    : _invertFaces( false )
{
    // Initialize the members
    _vertices = NULL;
    _colors = NULL;
    _normals = NULL;
    _triangles = NULL;
    _diffuse = NULL;
    _ambient = NULL;
    _specular = NULL;
}


/*  Read the vertex and (if available/wanted) color data from the open file.  */
void VertexData::readVertices( PlyFile* file, const int nVertices,
                               const int fields )
{
    // temporary vertex structure for ply loading
    struct _Vertex
    {
        float           x;
        float           y;
        float           z;
        float           nx;
        float           ny;
        float           nz;
        unsigned char   red;
        unsigned char   green;
        unsigned char   blue;
        unsigned char   alpha;
        unsigned char   ambient_red;
        unsigned char   ambient_green;
        unsigned char   ambient_blue;
        unsigned char   diffuse_red;
        unsigned char   diffuse_green;
        unsigned char   diffuse_blue;
        unsigned char   specular_red;
        unsigned char   specular_green;
        unsigned char   specular_blue;
        float           specular_coeff;
        float           specular_power;
    } vertex;

    PlyProperty vertexProps[] =
    {
        { "x", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, x ), 0, 0, 0, 0 },
        { "y", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, y ), 0, 0, 0, 0 },
        { "z", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, z ), 0, 0, 0, 0 },
        { "nx", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, nx ), 0, 0, 0, 0 },
        { "ny", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, ny ), 0, 0, 0, 0 },
        { "nz", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, nz ), 0, 0, 0, 0 },
        { "red", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, red ), 0, 0, 0, 0 },
        { "green", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, green ), 0, 0, 0, 0 },
        { "blue", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, blue ), 0, 0, 0, 0 },
        { "alpha", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, alpha ), 0, 0, 0, 0 },
        { "ambient_red", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, ambient_red ), 0, 0, 0, 0 },
        { "ambient_green", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, ambient_green ), 0, 0, 0, 0 },
        { "ambient_blue", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, ambient_blue ), 0, 0, 0, 0 },
        { "diffuse_red", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, diffuse_red ), 0, 0, 0, 0 },
        { "diffuse_green", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, diffuse_green ), 0, 0, 0, 0 },
        { "diffuse_blue", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, diffuse_blue ), 0, 0, 0, 0 },
        { "specular_red", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, specular_red ), 0, 0, 0, 0 },
        { "specular_green", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, specular_green ), 0, 0, 0, 0 },
        { "specular_blue", PLY_UCHAR, PLY_UCHAR, offsetof( _Vertex, specular_blue ), 0, 0, 0, 0 },
        { "specular_coeff", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, specular_coeff ), 0, 0, 0, 0 },
        { "specular_power", PLY_FLOAT, PLY_FLOAT, offsetof( _Vertex, specular_power ), 0, 0, 0, 0 },
    };

    // use all 6 properties when reading colors, only the first 3 otherwise
    for( int i = 0; i < 3; ++i )
        ply_get_property( file, "vertex", &vertexProps[i] );

    if (fields & NORMALS)
      for( int i = 3; i < 6; ++i )
        ply_get_property( file, "vertex", &vertexProps[i] );

    if (fields & RGB)
      for( int i = 6; i < 9; ++i )
        ply_get_property( file, "vertex", &vertexProps[i] );

    if (fields & RGBA)
        ply_get_property( file, "vertex", &vertexProps[9] );

    if (fields & AMBIENT)
      for( int i = 10; i < 13; ++i )
        ply_get_property( file, "vertex", &vertexProps[i] );

    if (fields & DIFFUSE)
      for( int i = 13; i < 16; ++i )
        ply_get_property( file, "vertex", &vertexProps[i] );

    if (fields & SPECULAR)
      for( int i = 16; i < 21; ++i )
        ply_get_property( file, "vertex", &vertexProps[i] );

    // check whether array is valid otherwise allocate the space
    if(!_vertices.valid())
        _vertices = new osg::Vec3Array;

    if( fields & NORMALS )
    {
        if(!_normals.valid())
            _normals = new osg::Vec3Array;
    }

    // If read colors allocate space for color array
    if( fields & RGB || fields & RGBA)
    {
        if(!_colors.valid())
            _colors = new osg::Vec4Array;
    }

    if( fields & AMBIENT )
    {
        if(!_ambient.valid())
            _ambient = new osg::Vec4Array;
    }

    if( fields & DIFFUSE )
    {
        if(!_diffuse.valid())
            _diffuse = new osg::Vec4Array;
    }

    if( fields & SPECULAR )
    {
        if(!_specular.valid())
            _specular = new osg::Vec4Array;
    }

    // read in the vertices
    for( int i = 0; i < nVertices; ++i )
    {
        ply_get_element( file, static_cast< void* >( &vertex ) );
        _vertices->push_back( osg::Vec3( vertex.x, vertex.y, vertex.z ) );
        if (fields & NORMALS)
            _normals->push_back( osg::Vec3( vertex.nx, vertex.ny, vertex.nz ) );

        if( fields & RGBA )
            _colors->push_back( osg::Vec4( (unsigned int) vertex.red / 255.0,
                                           (unsigned int) vertex.green / 255.0 ,
                                           (unsigned int) vertex.blue / 255.0,
                                           (unsigned int) vertex.alpha / 255.0) );
        else if( fields & RGB )
            _colors->push_back( osg::Vec4( (unsigned int) vertex.red / 255.0,
                                           (unsigned int) vertex.green / 255.0 ,
                                           (unsigned int) vertex.blue / 255.0, 1.0 ) );
        if( fields & AMBIENT )
            _ambient->push_back( osg::Vec4( (unsigned int) vertex.ambient_red / 255.0,
                                            (unsigned int) vertex.ambient_green / 255.0 ,
                                            (unsigned int) vertex.ambient_blue / 255.0, 1.0 ) );

        if( fields & DIFFUSE )
            _diffuse->push_back( osg::Vec4( (unsigned int) vertex.diffuse_red / 255.0,
                                            (unsigned int) vertex.diffuse_green / 255.0 ,
                                            (unsigned int) vertex.diffuse_blue / 255.0, 1.0 ) );

        if( fields & SPECULAR )
            _specular->push_back( osg::Vec4( (unsigned int) vertex.specular_red / 255.0,
                                             (unsigned int) vertex.specular_green / 255.0 ,
                                             (unsigned int) vertex.specular_blue / 255.0, 1.0 ) );
    }
}


/*  Read the index data from the open file.  */
void VertexData::readTriangles( PlyFile* file, const int nFaces )
{
    // temporary face structure for ply loading
    struct _Face
    {
        unsigned char   nVertices;
        int*            vertices;
    } face;

    PlyProperty faceProps[] =
    {
        { "vertex_indices|vertex_index", PLY_INT, PLY_INT, offsetof( _Face, vertices ),
          1, PLY_UCHAR, PLY_UCHAR, offsetof( _Face, nVertices ) }
    };

    ply_get_property( file, "face", &faceProps[0] );

    //triangles.clear();
    //triangles.reserve( nFaces );
    if(!_triangles.valid())
        _triangles = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    if(!_quads.valid())
        _quads = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);


    // read the faces, reversing the reading direction if _invertFaces is true
    for( int i = 0 ; i < nFaces; i++ )
    {
        ply_get_element( file, static_cast< void* >( &face ) );
        MESHASSERT( face.vertices != 0 );
        if( (unsigned int)(face.nVertices) > 4 )
        {
            free( face.vertices );
            throw MeshException( "Error reading PLY file. Encountered a "
                                 "face which does not have three or four vertices." );
        }

        unsigned short index;
        for(int j = 0 ; j < face.nVertices ; j++)
        {
            index = ( _invertFaces ? face.nVertices - 1 - j : j );
            if(face.nVertices == 4)
                _quads->push_back(face.vertices[index]);
            else
                _triangles->push_back(face.vertices[index]);
        }

        // free the memory that was allocated by ply_get_element
        free( face.vertices );
    }
}


/*  Open a PLY file and read vertex, color and index data. and returns the node  */
osg::Node* VertexData::readPlyFile( const char* filename, const bool ignoreColors )
{
    int     nPlyElems;
    char**  elemNames;
    int     fileType;
    float   version;
    bool    result = false;
    int     nComments;
    char**  comments;

    PlyFile* file = NULL;

    // Try to open ply file as for reading
    try{
            file  = ply_open_for_reading( const_cast< char* >( filename ),
                                          &nPlyElems, &elemNames,
                                          &fileType, &version );
    }
    // Catch the if any exception thrown
    catch( exception& e )
    {
        MESHERROR << "Unable to read PLY file, an exception occurred:  "
                    << e.what() << endl;
    }

    if( !file )
    {
        MESHERROR << "Unable to open PLY file " << filename
                  << " for reading." << endl;
        return NULL;
    }

    MESHASSERT( elemNames != 0 );


    nComments = file->num_comments;
    comments = file->comments;


    #ifndef NDEBUG
    MESHINFO << filename << ": " << nPlyElems << " elements, file type = "
             << fileType << ", version = " << version << endl;
    #endif

    for( int i = 0; i < nComments; i++ )
    {
        if( equal_strings( comments[i], "modified by flipply" ) )
        {
            _invertFaces = true;
        }

    }
    for( int i = 0; i < nPlyElems; ++i )
    {
        int nElems;
        int nProps;

        PlyProperty** props = NULL;
        try{
                props = ply_get_element_description( file, elemNames[i],
                                                     &nElems, &nProps );
        }
        catch( exception& e )
        {
            MESHERROR << "Unable to get PLY file description, an exception occurred:  "
                        << e.what() << endl;
        }
        MESHASSERT( props != 0 );

        #ifndef NDEBUG
        MESHINFO << "element " << i << ": name = " << elemNames[i] << ", "
                 << nProps << " properties, " << nElems << " elements" << endl;
        for( int j = 0; j < nProps; ++j )
        {
            MESHINFO << "element " << i << ", property " << j << ": "
                     << "name = " << props[j]->name << endl;
        }
        #endif

        // if the string is vertex means vertex data is started
        if( equal_strings( elemNames[i], "vertex" ) )
        {
 	    int fields = NONE;
            // determine if the file stores vertex colors
            for( int j = 0; j < nProps; ++j )
	      {
                // if the string have the red means color info is there
                if( equal_strings( props[j]->name, "x" ) )
                    fields |= XYZ;
                if( equal_strings( props[j]->name, "nx" ) )
                    fields |= NORMALS;
                if( equal_strings( props[j]->name, "alpha" ) )
                    fields |= RGBA;
                if ( equal_strings( props[j]->name, "red" ) )
                    fields |= RGB;
                if( equal_strings( props[j]->name, "ambient" ) )
                    fields |= AMBIENT;
                if( equal_strings( props[j]->name, "diffuse_red" ) )
                    fields |= DIFFUSE;
                if( equal_strings( props[j]->name, "specular_red" ) )
                    fields |= SPECULAR;
          }

            if( ignoreColors )
	      {
		fields &= ~(XYZ | NORMALS);
                MESHINFO << "Colors in PLY file ignored per request." << endl;
	      }

            try {
                // Read vertices and store in a std::vector array
                readVertices( file, nElems, fields );
                // Check whether all vertices are loaded or not
                MESHASSERT( _vertices->size() == static_cast< size_t >( nElems ) );

		// Check if all the optional elements were read or not
                if( fields & NORMALS )
                {
                    MESHASSERT( _normals->size() == static_cast< size_t >( nElems ) );
                }
                if( fields & RGB || fields & RGBA)
                {
                    MESHASSERT( _colors->size() == static_cast< size_t >( nElems ) );
                }
                if( fields & AMBIENT )
                {
                    MESHASSERT( _ambient->size() == static_cast< size_t >( nElems ) );
                }
                if( fields & DIFFUSE )
                {
                    MESHASSERT( _diffuse->size() == static_cast< size_t >( nElems ) );
                }
                if( fields & SPECULAR )
                {
                    MESHASSERT( _specular->size() == static_cast< size_t >( nElems ) );
                }

                result = true;
            }
            catch( exception& e )
            {
                MESHERROR << "Unable to read vertex in PLY file, an exception occurred:  "
                            << e.what() << endl;
                // stop for loop by setting the loop variable to break condition
                // this way resources still get released even on error cases
                i = nPlyElems;

            }
        }
        // If the string is face means triangle info started
        else if( equal_strings( elemNames[i], "face" ) )
        try
        {
            // Read Triangles
            readTriangles( file, nElems );
            // Check whether all face elements read or not
#if DEBUG
            unsigned int nbTriangles = (_triangles.valid() ? _triangles->size() / 3 : 0) ;
            unsigned int nbQuads = (_quads.valid() ? _quads->size() / 4 : 0 );

            MESHASSERT( (nbTriangles + nbQuads) == static_cast< size_t >( nElems ) );
#endif
            result = true;
        }
        catch( exception& e )
        {
            MESHERROR << "Unable to read PLY file, an exception occurred:  "
                      << e.what() << endl;
            // stop for loop by setting the loop variable to break condition
            // this way resources still get released even on error cases
            i = nPlyElems;
        }

        // free the memory that was allocated by ply_get_element_description
        for( int j = 0; j < nProps; ++j )
            free( props[j] );
        free( props );
    }

    ply_close( file );

    // free the memory that was allocated by ply_open_for_reading
    for( int i = 0; i < nPlyElems; ++i )
        free( elemNames[i] );
    free( elemNames );

   // If the result is true means the ply file is successfully read
   if(result)
   {
        // Create geometry node
        osg::Geometry* geom  =  new osg::Geometry;

        // set the vertex array
        geom->setVertexArray(_vertices.get());

        // Add the primitive set
        bool hasTriOrQuads = false;
        if (_triangles.valid() && _triangles->size() > 0 )
        {
            geom->addPrimitiveSet(_triangles.get());
            hasTriOrQuads = true;
        }

        if (_quads.valid() && _quads->size() > 0 )
        {
            geom->addPrimitiveSet(_quads.get());
            hasTriOrQuads = true;
        }

        // Print points if the file contains unsupported primitives
        if(!hasTriOrQuads)
            geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, _vertices->size()));


	// Apply the colours to the model; at the moment this is a
	// kludge because we only use one kind and apply them all the
	// same way. Also, the priority order is completely arbitrary

        if(_colors.valid())
        {
            geom->setColorArray(_colors.get(), osg::Array::BIND_PER_VERTEX );
        }
        else if(_ambient.valid())
        {
            geom->setColorArray(_ambient.get(), osg::Array::BIND_PER_VERTEX );
        }
        else if(_diffuse.valid())
        {
            geom->setColorArray(_diffuse.get(), osg::Array::BIND_PER_VERTEX );
        }
	else if(_specular.valid())
        {
            geom->setColorArray(_specular.get(), osg::Array::BIND_PER_VERTEX );
        }

        // If the model has normals, add them to the geometry
        if(_normals.valid())
        {
            geom->setNormalArray(_normals.get(), osg::Array::BIND_PER_VERTEX);
        }
        else
        {   // If not, use the smoothing visitor to generate them
            // (quads will be triangulated by the smoothing visitor)
            osgUtil::SmoothingVisitor::smooth((*geom), osg::PI/2);
        }

        // set flage true to activate the vertex buffer object of drawable
        geom->setUseVertexBufferObjects(true);


        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geom);
        return geode;
   }

    return NULL;
}


