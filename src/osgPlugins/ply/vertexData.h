/*
    vertexData.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.

    Header file of the VertexData class.
*/

/** note, derived from Equalizer LGPL source.*/


#ifndef MESH_VERTEXDATA_H
#define MESH_VERTEXDATA_H


#include <osg/Node>
#include <osg/PrimitiveSet>

#include <vector>

///////////////////////////////////////////////////////////////////////////////
//!
//! \class VertexData
//! \brief helps to read ply file and converts in to osg::Node format
//!
///////////////////////////////////////////////////////////////////////////////

// defined elsewhere
struct PlyFile;

namespace ply
{
    /*  Holds the flat data and offers routines to read, scale and sort it.  */
    class VertexData
    {
    public:
        // Default constructor
        VertexData();


        // Reads ply file and convert in to osg::Node and returns the same
        osg::Node* readPlyFile( const char* file, const bool ignoreColors = false );

        // to set the flag for using inverted face
        void useInvertedFaces() { _invertFaces = true; }

    private:

        enum VertexFields
        {
          NONE = 0,
          XYZ = 1,
          NORMALS = 2,
          RGB = 4,
          AMBIENT = 8,
          DIFFUSE = 16,
          SPECULAR = 32,
          RGBA = 64
        };

        // Function which reads all the vertices and colors if color info is
        // given and also if the user wants that information
        void readVertices( PlyFile* file, const int nVertices,
                           const int vertexFields );

        // Reads the triangle indices from the ply file
        void readTriangles( PlyFile* file, const int nFaces );

        bool        _invertFaces;

        // Vertex array in osg format
        osg::ref_ptr<osg::Vec3Array>   _vertices;
        // Color array in osg format
        osg::ref_ptr<osg::Vec4Array>   _colors;
        osg::ref_ptr<osg::Vec4Array>   _ambient;
        osg::ref_ptr<osg::Vec4Array>   _diffuse;
        osg::ref_ptr<osg::Vec4Array>   _specular;

        // Normals in osg format
        osg::ref_ptr<osg::Vec3Array> _normals;
        // The indices of the faces in premitive set
        osg::ref_ptr<osg::DrawElementsUInt> _triangles;
        osg::ref_ptr<osg::DrawElementsUInt> _quads;
    };
}


#endif // MESH_VERTEXDATA_H
