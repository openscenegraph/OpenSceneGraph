/*
    vertexData.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    Copyright (c) 2020, Julien Valentin <mp3butcher@hotmail.com>
    All rights reserved.

    Header file of the VertexData class.
*/



#ifndef MESH_VERTEXDATA_H
#define MESH_VERTEXDATA_H


#include <osg/Node>
#include <osg/PrimitiveSet>

#include <vector>

#include "ply.h"
///////////////////////////////////////////////////////////////////////////////
//!
//! \class VertexData
//! \brief helps to read ply file and converts in to osg::Node format
//!
///////////////////////////////////////////////////////////////////////////////

namespace ply
{

    typedef std::pair<PlyProperty, int> VertexSemantic;
    typedef std::vector<VertexSemantic> VertexSemantics;
    struct ArrayFactory : public osg::Referenced
    {
        virtual osg::Array * getArray() { return 0; }
        virtual void addElement(char * dataptr, osg::Array*) = 0;
    };
    struct DrawElementFactory : public osg::Referenced
    {
        virtual osg::DrawElements * getDrawElement() = 0;
        virtual void addElement(char * dataptr, osg::DrawElements*) = 0;
    };

    /*  Holds the flat data and offers routines to read, scale and sort it.  */
    class VertexData
    {
        osg::ref_ptr<ArrayFactory> _arrayfactories[PLY_END_TYPE-1][4];
        osg::ref_ptr<DrawElementFactory> _prfactories[PLY_END_TYPE-1];
        /*struct name
        {
            name() {}
            ArrayFactory* factory;
            osg::ref_ptr<osg::Array> arr;
            int osgmapping;

        };*/

        typedef std::pair< ArrayFactory*, osg::ref_ptr<osg::Array> > AFactAndArray;
        typedef std::vector<AFactAndArray> AFactAndArrays;

        typedef std::pair< DrawElementFactory*, osg::ref_ptr<osg::DrawElements> > PFactAndDrawElement;
        typedef std::vector<PFactAndDrawElement> PFactAndDrawElements;

        typedef std::pair<AFactAndArrays , PFactAndDrawElements > APFactAndArrays;

        std::vector< std::pair< std::string, APFactAndArrays > > _factoryarrayspair; //per element name
    public:

        // Default constructor
        VertexData(const VertexSemantics& s);

        // Reads ply file and convert in to osg::Node and returns the same
        osg::Node* readPlyFile( const char* file, const bool ignoreColors = false );

        // to set the flag for using inverted face
        void useInvertedFaces() { _invertFaces = true; }

    private:
        VertexSemantics _semantics;

        // Function which reads all the vertices and colors if color info is
        // given and also if the user wants that information
        void readVertices( PlyFile* file, const int nVertices, char*  elemName,
                         PlyProperty** props, int numprops);

        // Reads the triangle indices from the ply file
        void readListProperty( PlyFile* file, const int nFaces, char*  elemName, PlyProperty* listprop);
        bool _invertFaces;

        // The indices of the faces in premitive set
        osg::ref_ptr<osg::DrawElementsUInt> _triangles;
        osg::ref_ptr<osg::DrawElementsUInt> _quads;
    };
}


#endif // MESH_VERTEXDATA_H
