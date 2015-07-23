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

#include "FltExportVisitor.h"
#include "ExportOptions.h"
#include "FltWriteResult.h"
#include "DataOutputStream.h"
#include "Opcodes.h"
#include "LightSourcePaletteManager.h"
#include "MaterialPaletteManager.h"
#include "TexturePaletteManager.h"
#include "VertexPaletteManager.h"
#include "AttrData.h"
#include "Utils.h"

#include <osgDB/FileUtils>
#include <osgDB/WriteFile>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LightSource>
#include <osg/LOD>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/PrimitiveSet>
#include <osg/ProxyNode>
#include <osg/Quat>
#include <osg/Sequence>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/Switch>
#include <osg/Material>
#include <osg/CullFace>
#include <osg/BlendFunc>
#include <osg/PolygonOffset>
#include <osgSim/DOFTransform>
#include <osgSim/MultiSwitch>
#include <osgSim/LightPointNode>
#include <osgSim/ObjectRecordData>

#ifdef _MSC_VER
// Disable this warning. It's OK for us to use 'this' in initializer list,
// as the texturePaletteManager merely stores a ref to it.
#pragma warning( disable : 4355 )
#endif

namespace flt
{


FltExportVisitor::FltExportVisitor( DataOutputStream* dos,
                                ExportOptions* fltOpt )

  : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
    _fltOpt( fltOpt ),
    _dos( *dos ),
    _materialPalette( new MaterialPaletteManager( *fltOpt ) ),
    _texturePalette( new TexturePaletteManager( *this, *fltOpt ) ),
    _lightSourcePalette( new LightSourcePaletteManager( ) ),
    _vertexPalette( new VertexPaletteManager( *fltOpt ) ),
    _firstNode( true )
{
    // Init the StateSet stack.
    osg::StateSet* ss = new osg::StateSet;

    int unit;
    for(unit=0; unit<8; unit++)
    {
        osg::TexEnv* texenv = new osg::TexEnv;
        ss->setTextureAttributeAndModes( unit, texenv, osg::StateAttribute::OFF );
        // TBD other texture state?
    }

    osg::Material* material = new osg::Material;
    ss->setAttribute( material, osg::StateAttribute::OFF );
    if (fltOpt->getLightingDefault())
        ss->setMode( GL_LIGHTING, osg::StateAttribute::ON );
    else
        ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    osg::CullFace* cf = new osg::CullFace;
    ss->setAttributeAndModes( cf, osg::StateAttribute::OFF );

    osg::BlendFunc* bf = new osg::BlendFunc;
    ss->setAttributeAndModes( bf, osg::StateAttribute::OFF );

    osg::PolygonOffset* po = new osg::PolygonOffset;
    ss->setAttributeAndModes( po, osg::StateAttribute::OFF );

    _stateSetStack.push_back( ss );


    // Temp file for storing records. Need a temp file because we don't
    // write header and palette until FltExportVisitor completes traversal.
    _recordsTempName = fltOpt->getTempDir() + "/ofw_temp_records";
    _recordsStr.open( _recordsTempName.c_str(), std::ios::out | std::ios::binary );
    _records = new DataOutputStream( _recordsStr.rdbuf(), fltOpt->getValidateOnly() );

    // Always write initial push level
    writePush();
}

FltExportVisitor::~FltExportVisitor()
{
    // Delete our temp file.
    if (_recordsStr.is_open())
    {
        OSG_WARN << "fltexp: FltExportVisitor destructor has an open temp file." << std::endl;
        // This should not happen. FltExportVisitor::complete should close
        // this file before we get to this destructor.
        return;
    }
    OSG_INFO << "fltexp: Deleting temp file " << _recordsTempName << std::endl;
    FLTEXP_DELETEFILE( _recordsTempName.c_str() );
}


void
FltExportVisitor::apply( osg::Group& node )
{
    ScopedStatePushPop guard( this, node.getStateSet() );

    if (_firstNode)
    {
        // On input, a FLT header creates a Group node.
        // On export, we always write a Header record, but then the first Node
        //   we export is the Group that was created from the original input Header.
        // On successive roundtrips, this results in increased redundant top-level Group nodes/records.
        // Avoid this by NOT outputting anything for a top-level Group node.
        _firstNode = false;
        traverse( node );
        return;
    }

    // A Group node could indicate one of many possible records.
    //   Header record -- Don't need to support this here. We always output a header.
    //   Group record -- HIGH
    //   Child of an LOD node -- HIGH Currently write out a Group record regardless.
    //   InstanceDefinition/InstanceReference -- MED --  multiparented Group is an instance
    //   Extension record -- MED
    //   Object record -- MED
    //   LightPointSystem record (if psgSim::MultiSwitch) -- LOW

    osgSim::MultiSwitch* multiSwitch = dynamic_cast<osgSim::MultiSwitch*>( &node );
    if (multiSwitch)
    {
        writeSwitch( multiSwitch );
    }

    else
    {
        osgSim::ObjectRecordData* ord =
            dynamic_cast< osgSim::ObjectRecordData* >( node.getUserData() );
        if (ord)
        {
            // This Group should write an Object Record.
            writeObject( node, ord );
        }
        else
        {
            // Handle other cases here.
            // For now, just output a Group record.
            writeGroup( node );
        }
    }

    writeMatrix( node.getUserData() );
    writeComment( node );
    writePushTraverseWritePop( node );
}

void
FltExportVisitor::apply( osg::Sequence& node )
{
    _firstNode = false;
    ScopedStatePushPop guard( this, node.getStateSet() );

    writeSequence( node );
    writeMatrix( node.getUserData() );
    writeComment( node );
    writePushTraverseWritePop( node );
}

void
FltExportVisitor::apply( osg::Switch& node )
{
    _firstNode = false;
    ScopedStatePushPop guard( this, node.getStateSet() );

    writeSwitch( &node );

    writeMatrix( node.getUserData() );
    writeComment( node );
    writePushTraverseWritePop( node );
}

void
FltExportVisitor::apply( osg::LOD& lodNode )
{
    _firstNode = false;
    ScopedStatePushPop guard( this, lodNode.getStateSet() );

    // LOD center - same for all children
    osg::Vec3d center = lodNode.getCenter();

    // Iterate children of the LOD and write a separate LOD record for each,
    // with that child's individual switchIn and switchOut properties
    for ( size_t i = 0; i < lodNode.getNumChildren(); ++i )
    {
        osg::Node* lodChild = lodNode.getChild(i);

        // Switch-in/switch-out distances may vary per child
        double switchInDist = lodNode.getMaxRange(i);
        double switchOutDist = lodNode.getMinRange(i);

        writeLevelOfDetail( lodNode, center, switchInDist, switchOutDist);
        writeMatrix( lodNode.getUserData() );
        writeComment( lodNode );

        // Traverse each child of the LOD
        writePushTraverseChildWritePop( *lodChild );
    }

}

void
FltExportVisitor::apply( osg::MatrixTransform& node )
{
    // Importer reads a Matrix record and inserts a MatrixTransform above
    //   the current node. We need to do the opposite: Write a Matrix record
    //   as an ancillary record for each child. We do that by storing the
    //   MatrixTransform in each child's UserData. Each child then checks
    //   UserData and writes a Matrix record if UserData is a MatrixTransform.

    _firstNode = false;
    ScopedStatePushPop guard( this, node.getStateSet() );

    osg::ref_ptr< osg::RefMatrix > m = new osg::RefMatrix;
    m->set( node.getMatrix() );
    if (node.getUserData())
    {
        const osg::RefMatrix* rm = dynamic_cast<const osg::RefMatrix*>( node.getUserData() );
        if (rm)
            (*m) *= *rm;
    }

    typedef std::vector< osg::ref_ptr< osg::Referenced > > UserDataList;

    UserDataList saveUserDataList( node.getNumChildren() );

    unsigned int idx;
    for( idx=0; idx<node.getNumChildren(); ++idx )
    {
        saveUserDataList[ idx ] = node.getChild( idx )->getUserData();
        node.getChild( idx )->setUserData( m.get() );
    }

    traverse( (osg::Node&)node );

    // Restore saved UserData.
    for( idx=0; idx< node.getNumChildren(); ++idx )
    {
      node.getChild( idx )->setUserData( saveUserDataList[ idx ].get() );
    }
}

void
FltExportVisitor::apply( osg::PositionAttitudeTransform& node )
{
    _firstNode = false;
    ScopedStatePushPop guard( this, node.getStateSet() );

    osg::ref_ptr<osg::RefMatrix> m = new osg::RefMatrix(
        osg::Matrix::translate( -node.getPivotPoint() ) *
        osg::Matrix::scale( node.getScale() ) *
        osg::Matrix::rotate( node.getAttitude() ) *
        osg::Matrix::translate( node.getPosition() ) );

    typedef std::vector< osg::ref_ptr< osg::Referenced > > UserDataList;
    UserDataList saveUserDataList( node.getNumChildren() );

    unsigned int idx;
    for( idx=0; idx<node.getNumChildren(); ++idx )
    {
        saveUserDataList[ idx ] = node.getChild( idx )->getUserData();
        node.getChild( idx )->setUserData( m.get() );
    }

    traverse( (osg::Node&)node );

    // Restore saved UserData.
    for( idx=0; idx<node.getNumChildren(); ++idx )
    {
        node.getChild( idx )->setUserData( saveUserDataList[ idx ].get() );
    }

}


void
FltExportVisitor::apply( osg::Transform& node )
{
    _firstNode = false;
    ScopedStatePushPop guard( this, node.getStateSet() );

    osgSim::DOFTransform* dof = dynamic_cast<osgSim::DOFTransform*>( &node );

    if (dof)
    {
        writeDegreeOfFreedom( dof);
    }

    writeMatrix( node.getUserData() );
    writeComment( node );
    writePushTraverseWritePop( node );
}

void
FltExportVisitor::apply( osg::LightSource& node )
{
    _firstNode = false;
    ScopedStatePushPop guard( this, node.getStateSet() );

    writeLightSource( node );
    writeMatrix( node.getUserData() );
    writeComment( node );
    writePushTraverseWritePop( node );
}

// Billboards also go through this code. The Geode is passed
// to writeFace and writeMesh. If those methods successfully cast
// the Geode to a Billboard, then they set the template mode
// bit accordingly.
void
FltExportVisitor::apply( osg::Geode& node )
{
    _firstNode = false;
    ScopedStatePushPop guard( this, node.getStateSet() );

    unsigned int idx;
    for (idx=0; idx<node.getNumDrawables(); idx++)
    {
        osg::Geometry* geom = node.getDrawable( idx )->asGeometry();
        if (!geom)
        {
            std::string warning( "fltexp: Non-Geometry Drawable encountered. Ignoring." );
            OSG_WARN << warning << std::endl;
            _fltOpt->getWriteResult().warn( warning );
            continue;
        }

        ScopedStatePushPop drawableGuard( this, geom->getStateSet() );

        // Push and pop subfaces if polygon offset is on.
        SubfaceHelper subface( *this, getCurrentStateSet() );

        if (atLeastOneFace( *geom ))
        {
            // If at least one record will be a Face record, then we
            //   need to write to the vertex palette.
            _vertexPalette->add( *geom );

            // Iterate over all PrimitiveSets and output Face records.
            unsigned int jdx;
            for (jdx=0; jdx < geom->getNumPrimitiveSets(); jdx++)
            {
                osg::PrimitiveSet* prim = geom->getPrimitiveSet( jdx );
                if ( isMesh( prim->getMode() ) )
                    continue;

                if (prim->getType() == osg::PrimitiveSet::DrawArraysPrimitiveType)
                    handleDrawArrays( dynamic_cast<osg::DrawArrays*>( prim ), *geom, node );
                else if (prim->getType() == osg::PrimitiveSet::DrawArrayLengthsPrimitiveType)
                    handleDrawArrayLengths( dynamic_cast<osg::DrawArrayLengths*>( prim ), *geom, node );
                else if ( (prim->getType() == osg::PrimitiveSet::DrawElementsUBytePrimitiveType) ||
                        (prim->getType() == osg::PrimitiveSet::DrawElementsUShortPrimitiveType) ||
                        (prim->getType() == osg::PrimitiveSet::DrawElementsUIntPrimitiveType) )
                    handleDrawElements( dynamic_cast<osg::DrawElements*>( prim ), *geom, node );
                else
                {
                    std::string warning( "fltexp: Unknown PrimitiveSet type." );
                    OSG_WARN << warning << std::endl;
                    _fltOpt->getWriteResult().warn( warning );
                    return;
                }
            }
        }

        if (atLeastOneMesh( *geom ))
        {
            // If at least one Mesh record, write out preamble mesh records
            //   followed by a Mesh Primitive record per PrimitiveSet.
            writeMesh( node, *geom );

            writeMatrix( node.getUserData() );
            writeComment( node );
            writeMultitexture( *geom );
            writeLocalVertexPool( *geom );

            writePush();

            unsigned int jdx;
            for (jdx=0; jdx < geom->getNumPrimitiveSets(); jdx++)
            {
                osg::PrimitiveSet* prim = geom->getPrimitiveSet( jdx );
                if ( !isMesh( prim->getMode() ) )
                    continue;

                if (prim->getType() == osg::PrimitiveSet::DrawArraysPrimitiveType)
                    handleDrawArrays( dynamic_cast<osg::DrawArrays*>( prim ), *geom, node );
                else if (prim->getType() == osg::PrimitiveSet::DrawArrayLengthsPrimitiveType)
                    handleDrawArrayLengths( dynamic_cast<osg::DrawArrayLengths*>( prim ), *geom, node );
                else if ( (prim->getType() == osg::PrimitiveSet::DrawElementsUBytePrimitiveType) ||
                        (prim->getType() == osg::PrimitiveSet::DrawElementsUShortPrimitiveType) ||
                        (prim->getType() == osg::PrimitiveSet::DrawElementsUIntPrimitiveType) )
                    handleDrawElements( dynamic_cast<osg::DrawElements*>( prim ), *geom, node );
                else
                {
                    std::string warning( "fltexp: Unknown PrimitiveSet type." );
                    OSG_WARN << warning << std::endl;
                    _fltOpt->getWriteResult().warn( warning );
                    return;
                }
            }

            writePop();
        }
    }

    // Would traverse here if this node could have children.
    //   traverse( (osg::Node&)node );
}

void
FltExportVisitor::apply( osg::Node& node )
{
    _firstNode = false;
    ScopedStatePushPop guard( this, node.getStateSet() );

    osgSim::LightPointNode* lpn = dynamic_cast< osgSim::LightPointNode* >( &node );
    if (lpn)
    {
        writeLightPoint( lpn );
    }
    else
    {
        // Unknown Node. Warn and return.
        // (Note, if the base class of this Node was a Group, then apply(Group&)
        //   would export a Group record then continue traversal. Because we are
        //   a Node, there's no way to continue traversal, so just return.)
        std::string warning( "fltexp: Unknown Node in OpenFlight export." );
        OSG_WARN << warning << std::endl;
        _fltOpt->getWriteResult().warn( warning );
        return;
    }
}

void
FltExportVisitor::apply( osg::ProxyNode& node )
{
    _firstNode = false;
    ScopedStatePushPop guard( this, node.getStateSet() );

    writeExternalReference( node );
    writeMatrix( node.getUserData() );
    writeComment( node );
}





bool
FltExportVisitor::complete( const osg::Node& node )
{
    // Always write final pop level
    writePop();
    // Done writing records, close the record data temp file.
    _recordsStr.close();

    // Write OpenFlight file front matter: header, vertex palette, etc.
    writeHeader( node.getName() );

    writeColorPalette();
    _materialPalette->write( _dos );
    _texturePalette->write( _dos );
    _lightSourcePalette->write( _dos );
    _vertexPalette->write( _dos );

    // Write Comment ancillary record and specify the _dos DataOutputStream.
    writeComment( node, &_dos );

    // Copy record data temp file into final OpenFlight file.
    // Yee-uck. TBD need better stream copy routine.
    char buf;
    osgDB::ifstream recIn;
    recIn.open( _recordsTempName.c_str(), std::ios::in | std::ios::binary );
    while (!recIn.eof() )
    {
        recIn.read( &buf, 1 );
        if (recIn.good())
            _dos << buf;
    }
    recIn.close();

    return true;
}



//
// StateSet stack support

void
FltExportVisitor::pushStateSet( const osg::StateSet* rhs )
{
    osg::StateSet* ss = new osg::StateSet( *( _stateSetStack.back().get() ) );

    if (rhs)
        ss->merge( *rhs );

    _stateSetStack.push_back( ss );
}

void
FltExportVisitor::popStateSet()
{
    _stateSetStack.pop_back();
}

const osg::StateSet*
FltExportVisitor::getCurrentStateSet() const
{
    return _stateSetStack.back().get();
}

void
FltExportVisitor::clearStateSetStack()
{
    _stateSetStack.clear();
}


void
FltExportVisitor::writeATTRFile( int unit, const osg::Texture2D* texture ) const
{
    std::string name;
    if (_fltOpt->getStripTextureFilePath())
        name = osgDB::getSimpleFileName( texture->getImage()->getFileName() );
    else
        name = texture->getImage()->getFileName();
    name += std::string( ".attr" );

    if ( osgDB::findDataFile( name ).empty() )
    {
        // No .attr file found. We should write one out.
        // Fill AttrData fields from current state.
        AttrData ad;

        ad.texels_u = texture->getImage()->s();
        ad.texels_v = texture->getImage()->t();

        switch( texture->getFilter( osg::Texture::MIN_FILTER ) )
        {
        case osg::Texture::LINEAR:
            ad.minFilterMode = AttrData::MIN_FILTER_BILINEAR;
            break;
        case osg::Texture::LINEAR_MIPMAP_LINEAR:
            ad.minFilterMode = AttrData::MIN_FILTER_MIPMAP_TRILINEAR;
            break;
        case osg::Texture::LINEAR_MIPMAP_NEAREST:
            ad.minFilterMode = AttrData::MIN_FILTER_MIPMAP_BILINEAR;
            break;
        case osg::Texture::NEAREST:
            ad.minFilterMode = AttrData::MIN_FILTER_POINT;
            break;
        case osg::Texture::NEAREST_MIPMAP_LINEAR:
            ad.minFilterMode = AttrData::MIN_FILTER_MIPMAP_LINEAR;
            break;
        case osg::Texture::NEAREST_MIPMAP_NEAREST:
            ad.minFilterMode = AttrData::MIN_FILTER_MIPMAP_POINT;
            break;
        default:
            ad.minFilterMode = AttrData::MIN_FILTER_MIPMAP_TRILINEAR;
            break;
        }
        switch( texture->getFilter( osg::Texture::MAG_FILTER ) )
        {
        case osg::Texture::NEAREST:
            ad.magFilterMode = AttrData::MAG_FILTER_POINT;
            break;
        default:
            ad.magFilterMode = AttrData::MAG_FILTER_BILINEAR;
            break;
        }

        // Repeat and Clamp
        switch( texture->getWrap( osg::Texture::WRAP_S ) )
        {
        case osg::Texture::CLAMP:
        case osg::Texture::CLAMP_TO_EDGE:
        case osg::Texture::CLAMP_TO_BORDER:
            ad.wrapMode_u = AttrData::WRAP_CLAMP;
            break;
        case osg::Texture::REPEAT:
        default:
            ad.wrapMode_u = AttrData::WRAP_REPEAT;
            break;
        case osg::Texture::MIRROR:
            if (_fltOpt->getFlightFileVersionNumber() >= 1610)
                ad.wrapMode_u = AttrData::WRAP_MIRRORED_REPEAT;
            else
                ad.wrapMode_u = AttrData::WRAP_REPEAT;
            break;
        }
        switch( texture->getWrap( osg::Texture::WRAP_T ) )
        {
        case osg::Texture::CLAMP:
        case osg::Texture::CLAMP_TO_EDGE:
        case osg::Texture::CLAMP_TO_BORDER:
            ad.wrapMode_v = AttrData::WRAP_CLAMP;
            break;
        case osg::Texture::REPEAT:
        default:
            ad.wrapMode_v = AttrData::WRAP_REPEAT;
            break;
        case osg::Texture::MIRROR:
            if (_fltOpt->getFlightFileVersionNumber() >= 1610)
                ad.wrapMode_v = AttrData::WRAP_MIRRORED_REPEAT;
            else
                ad.wrapMode_v = AttrData::WRAP_REPEAT;
            break;
        }

        const osg::StateSet* ss = getCurrentStateSet();
        const osg::TexEnv* texenv = dynamic_cast<const osg::TexEnv*>(
            ss->getTextureAttribute( unit, osg::StateAttribute::TEXENV ) );
        if (texenv)
        {
            switch( texenv->getMode())
            {
            case osg::TexEnv::DECAL:
                ad.texEnvMode = AttrData::TEXENV_DECAL;
                break;
            case osg::TexEnv::MODULATE:
            default:
                ad.texEnvMode = AttrData::TEXENV_MODULATE;
                break;
            case osg::TexEnv::BLEND:
                ad.texEnvMode = AttrData::TEXENV_BLEND;
                break;
            case osg::TexEnv::REPLACE:
                ad.texEnvMode = AttrData::TEXENV_COLOR;
                break;
            case osg::TexEnv::ADD:
                ad.texEnvMode = AttrData::TEXENV_ADD;
                break;
            }
        }

        osgDB::writeObjectFile( ad, name, _fltOpt.get() );
    }
}


}

