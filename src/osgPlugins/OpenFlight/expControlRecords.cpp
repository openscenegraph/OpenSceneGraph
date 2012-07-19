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


namespace flt
{


void
FltExportVisitor::writePush()
{
    _records->writeInt16( (int16) PUSH_LEVEL_OP );
    _records->writeInt16( 4 );
}

void
FltExportVisitor::writePop()
{
    _records->writeInt16( (int16) POP_LEVEL_OP );
    _records->writeInt16( 4 );
}

void
FltExportVisitor::writePushSubface()
{
    _records->writeInt16( (int16) PUSH_SUBFACE_OP );
    _records->writeInt16( 4 );
}

void
FltExportVisitor::writePopSubface()
{
    _records->writeInt16( (int16) POP_SUBFACE_OP );
    _records->writeInt16( 4 );
}


}
