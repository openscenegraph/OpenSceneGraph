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

#ifndef __FLTEXP_FLT_EXPORT_VISITOR_H__
#define __FLTEXP_FLT_EXPORT_VISITOR_H__ 1

#include <osg/NodeVisitor>
#include "ExportOptions.h"
#include "Types.h"
#include <osgDB/fstream>
#include <set>
#include <memory>

namespace osg {
    class DrawArrays;
    class DrawArrayLengths;
    class DrawElements;
    class Geometry;
    class StateSet;
    class Switch;
    class Material;
    class Texture2D;
}
namespace osgSim {
    class DOFTransform;
    class MultiSwitch;
    class LightPointNode;
    class ObjectRecordData;
}

namespace flt
{


class ExportOptions;
class DataOutputStream;
class MaterialPaletteManager;
class TexturePaletteManager;
class VertexPaletteManager;
class LightSourcePaletteManager;



/*!
   The main NodeVisitor for walking the scene graph during export.
   A collection of apply() methods is in FltExportVisitor.cpp.
   The apply() methods call several other methods for writing
   specific FLT record types, most of which are in exp*.cpp.
 */
class FltExportVisitor : public osg::NodeVisitor
{
public:
    FltExportVisitor( DataOutputStream* dos, ExportOptions* fltOpt );
    ~FltExportVisitor(  );

    bool complete( const osg::Node& node );

    virtual void apply( osg::Group& node );
    virtual void apply( osg::Sequence& node );
    virtual void apply( osg::Switch& node );
    virtual void apply( osg::LOD& node );
    virtual void apply( osg::MatrixTransform& node );
    virtual void apply( osg::PositionAttitudeTransform& node );
    virtual void apply( osg::Transform& node );
    virtual void apply( osg::LightSource& node );
    virtual void apply( osg::Geode& node );
    virtual void apply( osg::Node& node );
    virtual void apply( osg::ProxyNode& node );



    // Primary records
    void writeHeader( const std::string& headerName );
    void writeGroup( const osg::Group& node );
    void writeGroup( const osg::Group& group,
                     int32 flags,
                     int32 loopCount,
                     float32 loopDuration,
                     float32 lastFrameDuration);
    void writeSequence( const osg::Sequence& node );
    void writeObject( const osg::Group& node, osgSim::ObjectRecordData* ord );
    void writeDegreeOfFreedom( const osgSim::DOFTransform* dof );
    void writeExternalReference( const osg::ProxyNode& node );
    void writeLevelOfDetail( const osg::LOD& lod, const osg::Vec3d& center,
                             double switchInDist, double switchOutDist);
    void writeLightSource( const osg::LightSource& ls );
    void writeSwitch( const osgSim::MultiSwitch* ms );
    void writeSwitch( const osg::Switch* ms );
    void writeLightPoint( const osgSim::LightPointNode* lpn );

    // Ancillary records
    void writeComment( const osg::Node& node, DataOutputStream* dos=NULL );
    void writeLongID( const std::string& id, DataOutputStream* dos=NULL );
    void writeMatrix( const osg::Referenced* ref );
    void writeContinuationRecord( const unsigned short length );

    // Control records
    void writePush();
    void writePop();
    void writePushSubface();
    void writePopSubface();

    // Helper routine for traversing a pushed subtree
    void writePushTraverseWritePop(osg::Node& node)
    {
        writePush();
        traverse(node);
        writePop();
    }

    void writePushTraverseChildWritePop(osg::Node& node)
    {
        writePush();
        node.accept(*this);
        writePop();
    }

    // Geometry records
    void writeFace( const osg::Geode& geode, const osg::Geometry& geom, GLenum mode );
    void writeMesh( const osg::Geode& geode, const osg::Geometry& geom );
    int writeVertexList( int first, unsigned int count );
    int writeVertexList( const std::vector<unsigned int>& indices, unsigned int count );
    void writeMeshPrimitive( const std::vector<unsigned int>& indices, GLenum mode );
    void writeLocalVertexPool( const osg::Geometry& geom );
    void writeMultitexture( const osg::Geometry& geom );
    void writeUVList( int numVerts, const osg::Geometry& geom, const std::vector<unsigned int>& indices );
    void writeUVList( int numVerts, const osg::Geometry& geom, unsigned int first=0);

    // Light Point records
    void writeLightPoint();

    // Exporter doesn't currently support color palette; write a dummy in its place to
    // support loaders that require it.
    void writeColorPalette();

    // StateSet stack support
    void pushStateSet( const osg::StateSet* rhs );
    void popStateSet();
    const osg::StateSet* getCurrentStateSet() const;
    void clearStateSetStack();

    // Write a .attr file if none exists in the data file path.
    void writeATTRFile( int unit, const osg::Texture2D* texture ) const;


private:
    // Methods for handling different primitive set types.
    //   These are defined in expGeometryRecords.cpp.
    void handleDrawArrays( const osg::DrawArrays* da, const osg::Geometry& geom, const osg::Geode& geode );
    void handleDrawArrayLengths( const osg::DrawArrayLengths* dal, const osg::Geometry& geom, const osg::Geode& geode );
    void handleDrawElements( const osg::DrawElements* de, const osg::Geometry& geom, const osg::Geode& geode );

    bool isLit( const osg::Geometry& geom ) const;
    bool isTextured( int unit, const osg::Geometry& geom ) const;
    bool isMesh( const GLenum mode ) const;
    bool atLeastOneFace( const osg::Geometry& geom ) const;
    bool atLeastOneMesh( const osg::Geometry& geom ) const;

    osg::ref_ptr< ExportOptions > _fltOpt;

    // _dos is the primary output stream, produces the actual .flt file.
    DataOutputStream& _dos;

    // _records is a temp file for most records. After the Header and palette
    // records are written to _dos, _records is copied onto _dos.
    osgDB::ofstream _recordsStr;
    DataOutputStream* _records;
    std::string _recordsTempName;

    // Track state changes during a scene graph walk.
    typedef std::vector< osg::ref_ptr<osg::StateSet> > StateSetStack;
    StateSetStack _stateSetStack;

    osg::ref_ptr<MaterialPaletteManager>     _materialPalette;
    osg::ref_ptr<TexturePaletteManager>      _texturePalette;
    osg::ref_ptr<LightSourcePaletteManager>  _lightSourcePalette;
    osg::ref_ptr<VertexPaletteManager>       _vertexPalette;

    // Used to avoid duplicate Header/Group records at top of output FLT file.
    bool _firstNode;
};

/*!
   Helper class to ensure symmetrical state push/pop behavior.
 */
class ScopedStatePushPop
{
    public:
        ScopedStatePushPop ( FltExportVisitor * fnv, const osg::StateSet *ss ) :
            fnv_( fnv )
        {
            fnv_->pushStateSet( ss );
        }
        virtual ~ScopedStatePushPop ()
        {
            fnv_->popStateSet();
        }

    private:
        FltExportVisitor * fnv_;
};


/*!
   Automatically handles writing the LongID ancillary record.
 */
struct IdHelper
{
    IdHelper(flt::FltExportVisitor& v, const std::string& id)
      : v_(v), id_(id), dos_(NULL)
    { }

    // Write an ancillary ID record upon destruction if name is too long
    ~IdHelper()
    {
        if (id_.length() > 8)
            v_.writeLongID(id_,dos_);
    }

    // Allow implicit conversion to the abbreviated name string
    operator const std::string() const
    {
        return( (id_.length() > 8) ? id_.substr(0, 8) : id_ );
    }

    flt::FltExportVisitor&  v_;
    const std::string        id_;
    DataOutputStream* dos_;

protected:

    IdHelper& operator = (const IdHelper&) { return *this; }

};


/*!
   Supports wrapping subfaces with push/pop records.
 */
struct SubfaceHelper
{
    SubfaceHelper(flt::FltExportVisitor& v, const osg::StateSet* ss )
      : v_(v)
    {
        _polygonOffsetOn = ( ss->getMode( GL_POLYGON_OFFSET_FILL ) == osg::StateAttribute::ON );
        if (_polygonOffsetOn)
            v_.writePushSubface();
    }

    ~SubfaceHelper()
    {
        if (_polygonOffsetOn)
            v_.writePopSubface();
    }

    flt::FltExportVisitor&  v_;
    bool _polygonOffsetOn;

protected:

    SubfaceHelper& operator = (const SubfaceHelper&) { return *this; }
};

}

#endif
