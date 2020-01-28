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
//! \details element properties grouped according @osgmapping (max 4)
//! \details default: geometry vertices in "vertex" element / primitivesets in "face" element
//! \details custom mapping possible with lua script file "ply2osgMapper.lua" and/or overriding VertexSemantics
//!
///////////////////////////////////////////////////////////////////////////////

namespace ply
{
    typedef struct VertexSemantic {    /* description of a mapping */
      const char *name;                 /* property name retrieve from the file */
      int internal_type;               /* target data type */
      int osgmapping;                  /* index of the array to mapping int out geometry... */
    } VertexSemantic;

    typedef std::vector<VertexSemantic> VertexSemantics;
    struct ArrayFactory : public osg::Referenced
    {
        virtual osg::Array * createArray() = 0;
        virtual void addElement(char * dataptr, osg::Array*) = 0;
    };
    struct DrawElementFactory : public osg::Referenced
    {
        virtual osg::DrawElements * createDrawElement() = 0;
        virtual void addElement(char * dataptr, osg::DrawElements*) = 0;
    };

    class VertexData
    {
        osg::ref_ptr<ArrayFactory> _arrayfactories[PLY_END_TYPE][4];
        osg::ref_ptr<DrawElementFactory> _prfactories[PLY_END_TYPE];

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

        ///  Read the vertex data from the open file.
        void readVertices( PlyFile* file, const int nVertices, char*  elemName,
                         PlyProperty** props, int numprops);
        bool _invertFaces;
    };
}


#endif // MESH_VERTEXDATA_H
