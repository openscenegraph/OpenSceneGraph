/* 
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or (at
 * your option) any later version. The full license is in the LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * OpenSceneGraph Public License for more details.
*/

//
// Copyright(c) 2008 Skew Matrix Software LLC.
//

#ifndef __FLTEXP_VERTEX_PALETTE_MANAGER_H__
#define __FLTEXP_VERTEX_PALETTE_MANAGER_H__ 1


#include "DataOutputStream.h"
#include "ExportOptions.h"
#include <osg/Array>
#include <osgDB/fstream>
#include <map>

namespace osg {
    class Geometry;
}


namespace flt
{


/*!
   Manages writing the Vertex Palette record during export.
   Maintains a map to ensure that instanced VertexArray data is only
   written once to the palette. Writes the palette record to a temp
   file and copies it to FltExportVisitor::_dos after the scene graph
   has been completely walked.
 */
class VertexPaletteManager
{
public:
    VertexPaletteManager( const ExportOptions& fltOpt );
    ~VertexPaletteManager();

    void add( const osg::Geometry& geom );
    void add( const osg::Array* key,
        const osg::Vec3dArray* v, const osg::Vec4Array* c,
        const osg::Vec3Array* n, const osg::Vec2Array* t,
        bool colorPerVertex, bool normalPerVertex, bool allowSharing=true );

    unsigned int byteOffset( unsigned int idx ) const;

    void write( DataOutputStream& dos ) const;

    /*!
       Static utility routines for handling the morass of array
       types that could be found in a Geometry object's vertex/
       normal/texcoord/color data. */
    static osg::ref_ptr< const osg::Vec2Array > asVec2Array( const osg::Array* in, const unsigned int n );
    static osg::ref_ptr< const osg::Vec3Array > asVec3Array( const osg::Array* in, const unsigned int n );
    static osg::ref_ptr< const osg::Vec3dArray > asVec3dArray( const osg::Array* in, const unsigned int n );
    static osg::ref_ptr< const osg::Vec4Array > asVec4Array( const osg::Array* in, const unsigned int n );

protected:
    typedef enum {
        VERTEX_C,
        VERTEX_CN,
        VERTEX_CNT,
        VERTEX_CT
    } PaletteRecordType;

    static PaletteRecordType recordType( const osg::Array* v, const osg::Array* c,
        const osg::Array* n, const osg::Array* t );
    unsigned int recordSize( PaletteRecordType recType );

    void writeRecords( const osg::Vec3dArray* v, const osg::Vec4Array* c,
        const osg::Vec3Array* n, const osg::Vec2Array* t,
        bool colorPerVertex, bool normalPerVertex );

    unsigned int _currentSizeBytes;

    struct ArrayInfo {
        ArrayInfo();

        unsigned int _byteStart;
        unsigned int _idxSizeBytes;
        unsigned int _idxCount;
    };
    ArrayInfo* _current;
    ArrayInfo _nonShared;

    typedef std::map< const osg::Array*, ArrayInfo > ArrayMap;
    ArrayMap _arrayMap;

    mutable osgDB::ofstream _verticesStr;
    DataOutputStream* _vertices;
    std::string _verticesTempName;

    const ExportOptions& _fltOpt;
};


}

#endif
