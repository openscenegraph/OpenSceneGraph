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
#include "DataOutputStream.h"
#include "Opcodes.h"
#include <osg/MatrixTransform>
#include <osg/Notify>


namespace flt
{


/** If the DataOutputStream parameter is NULL, write to the _records
    member variable. Otherwise, write to the specified DataOutputStream.
    */
void
FltExportVisitor::writeComment( const osg::Node& node, DataOutputStream* dos )
{
    if (dos==NULL)
        dos = _records;

    // Write all descriptions as Comment records.
    unsigned int nd = node.getNumDescriptions();
    unsigned int idx=0;
    while( idx < nd )
    {
        const std::string& com = node.getDescription( idx );
        unsigned int iLen = com.length() + 5;
        if (iLen > 0xffff)
        {
            // short overrun
            std::string warning( "fltexp: writeComment: Descriptions too long, resorts in short overrun. Skipping." );
            _fltOpt->getWriteResult().warn( warning );
            OSG_WARN << warning << std::endl;
            continue;
        }
        uint16 length( (uint16)iLen );

        dos->writeInt16( (int16) COMMENT_OP );
        dos->writeInt16( length );
        dos->writeString( com );

        idx++;
    }
}

/** If the DataOutputStream parameter is NULL, write to the _records
    member variable. Otherwise, write to the specified DataOutputStream.
    */
void
FltExportVisitor::writeLongID( const std::string& id, DataOutputStream* dos )
{
    if (dos==NULL)
        dos = _records;

    uint16 length( 2 + 2 + id.length() + 1 );  // +1 for terminating '\0'

    dos->writeInt16( (int16) LONG_ID_OP );
    dos->writeUInt16( length );
    dos->writeString( id );
}

void
FltExportVisitor::writeMatrix( const osg::Referenced* ref )
{
    const osg::RefMatrix* rm = dynamic_cast<const osg::RefMatrix*>( ref );
    if (!rm)
        return;

    uint16 length( 4 + (16 * sizeof(float32)) );

    _records->writeInt16( (int16) MATRIX_OP );
    _records->writeUInt16( length );

    int idx, jdx;
    for (idx=0; idx<4; idx++)
    {
        for (jdx=0; jdx<4; jdx++)
        {
            _records->writeFloat32( (*rm)( idx, jdx ) );
        }
    }
}

void
FltExportVisitor::writeContinuationRecord( const unsigned short length )
{
    OSG_DEBUG << "fltexp: Continuation record length: " << length+4 << std::endl;
    _records->writeInt16( (int16) CONTINUATION_OP );
    _records->writeUInt16( length+4 );
}


}
