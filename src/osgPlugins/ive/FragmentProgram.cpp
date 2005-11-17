/**********************************************************************
 *
 *    FILE:            FragmentProgram.cpp
 *
 *    DESCRIPTION:    Read/Write osg::FragmentProgram in binary format to disk.
 *
 *    CREATED BY:     rpk@blue-newt.com
 *
 *    HISTORY:        Created 04/20/2004
 *
 *    Copyright 2004 Blue Newt Software
 **********************************************************************/

#include "Exception.h"
#include "FragmentProgram.h"
#include "Object.h"

using namespace ive;

void FragmentProgram::write( DataOutputStream* out )
{
    // Write FragmentProgram identification.
    out->writeInt( IVEFRAGMENTPROGRAM );
    // If the osg class is inherited by any other class we should
    // also write this to file.
    osg::Object* obj = dynamic_cast<osg::Object*>(this);
    if( obj )
    {
        ( ( ive::Object* )( obj ) )->write( out );
    }
    else
    {
        throw Exception("Material::write(): Could not cast this osg::FragmentProgram to an osg::Object.");
    }

    // Write FragmentProgram properties.
    FragmentProgram::LocalParamList lpl = getLocalParameters();
    out->writeInt(lpl.size());
    for(FragmentProgram::LocalParamList::iterator i=lpl.begin(); i!=lpl.end(); ++i)
    {
        out->writeInt(i->first);
        out->writeVec4(i->second);
    }

    // Write program.
    out->writeString( this->getFragmentProgram() );
}

void FragmentProgram::read(DataInputStream* in){
    // Read FragmentProgram identification.
    int id = in->peekInt();
    if( id == IVEFRAGMENTPROGRAM )
    {
        // Code to read FragmentProgram properties.
        id = in->readInt();

        // handle Object data
        osg::Object*  obj = dynamic_cast<osg::Object*>(this);
        if( obj )
        {
            ( ( ive::Object* )( obj ) )->read( in );
        }
        else
        {
            throw Exception( "Material::read(): Could not cast this osg::FragmentProgram to an osg::Object." );
        }

        // Read data
        int i, size;
        size = in->readInt();
        for(i=0; i<size; i++)
        {
            int index = in->readInt();
            osg::Vec4 v = in->readVec4();
            this->setProgramLocalParameter( index, v );
        }

        std::string fp = in->readString();
        this->setFragmentProgram( fp );
    }
    else
    {
        throw Exception("FragmentProgram::read(): Expected FragmentProgram identification.");
    }
}
