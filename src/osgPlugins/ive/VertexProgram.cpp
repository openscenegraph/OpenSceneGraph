/**********************************************************************
 *
 *    FILE:            VertexProgram.cpp
 *
 *    DESCRIPTION:    Read/Write osg::VertexProgram in binary format to disk.
 *
 *    CREATED BY:     rpk@blue-newt.com
 *
 *    HISTORY:        Created 04/20/2004
 *
 *    Copyright 2004 Blue Newt Software
 **********************************************************************/

#include "Exception.h"
#include "VertexProgram.h"
#include "Object.h"

using namespace ive;

void VertexProgram::write( DataOutputStream* out )
{
    // Write VertexProgram identification.
    out->writeInt( IVEVERTEXPROGRAM );
    // If the osg class is inherited by any other class we should
    // also write this to file.
    osg::Object* obj = dynamic_cast<osg::Object*>(this);
    if( obj )
    {
        ( ( ive::Object* )( obj ) )->write( out );
    }
    else
    {
        out_THROW_EXCEPTION("Material::write(): Could not cast this osg::VertexProgram to an osg::Object.");
    }

    // Write VertexProgram properties.
    // Write program.
    out->writeString( this->getVertexProgram() );
}

void VertexProgram::read(DataInputStream* in){
    // Read VertexProgram identification.
    int id = in->peekInt();
    if( id == IVEVERTEXPROGRAM )
    {
        // Code to read VertexProgram properties.
        id = in->readInt();

        // handle Object data
        osg::Object*  obj = dynamic_cast<osg::Object*>(this);
        if( obj )
        {
            ( ( ive::Object* )( obj ) )->read( in );
        }
        else
        {
            in_THROW_EXCEPTION( "Material::read(): Could not cast this osg::VertexProgram to an osg::Object." );
        }

        // Read data
        std::string fp = in->readString();
        this->setVertexProgram( fp );
    }
    else
    {
        in_THROW_EXCEPTION("VertexProgram::read(): Expected VertexProgram identification.");
    }
}
