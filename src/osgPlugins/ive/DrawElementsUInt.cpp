/**********************************************************************
 *
 *    FILE:            DrawElementsUInt.cpp
 *
 *    DESCRIPTION:    Read/Write osg::DrawElementsUInt in binary format to disk.
 *
 *    CREATED BY:        Copied from DrawElementsUShort.cpp by Marco Jez
 *
 *
 *    HISTORY:        Created 20.3.2003
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "Exception.h"
#include "DrawElementsUInt.h"
#include "PrimitiveSet.h"
#include <osg/Endian>

using namespace ive;

void DrawElementsUInt::write(DataOutputStream* out){
    // Write DrawElementsUInt's identification.
    out->writeInt(IVEDRAWELEMENTSUINT);

    // If the osg class is inherited by any other class we should also write this to file.
    osg::PrimitiveSet*  prim = dynamic_cast<osg::PrimitiveSet*>(this);
    if(prim){
        ((ive::PrimitiveSet*)(prim))->write(out);
    }
    else
        out_THROW_EXCEPTION("DrawElementsUInt::write(): Could not cast this osg::DrawElementsUInt to an osg::PrimitiveSet.");
    // Write DrawElementsUInt's properties.

    // Write array length and its elements.
    out->writeInt(size());
    if (size()!=0) out->writeCharArray((const char*)&front(), size() * INTSIZE);
}

void DrawElementsUInt::read(DataInputStream* in)
{
    // Read DrawElementsUInt's identification.
    int id = in->peekInt();
    if(id == IVEDRAWELEMENTSUINT){
        // Code to read DrawElementsUInt's properties.
        id = in->readInt();
        // If the osg class is inherited by any other class we should also read this from file.
        osg::PrimitiveSet*  prim = dynamic_cast<osg::PrimitiveSet*>(this);
        if(prim){
            ((ive::PrimitiveSet*)(prim))->read(in);
        }
        else
            in_THROW_EXCEPTION("DrawElementsUInt::read(): Could not cast this osg::DrawElementsUInt to an osg::PrimitiveSet.");

        // Read array length and its elements.
        int size = in->readInt();
        resize(size);
        if (size!=0) in->readCharArray((char*)&front(), size * INTSIZE);

        if (in->_byteswap)
        {
           for (int i = 0 ; i < size ; i++ )
           {
                osg::swapBytes4((char*)&((*this)[i])) ;
           }
        }
    }
    else{
        in_THROW_EXCEPTION("DrawElementsUInt::read(): Expected DrawElementsUInt identification.");
    }
}
