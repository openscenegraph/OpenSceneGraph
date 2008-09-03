/**********************************************************************
 *
 *    FILE:           ShapeAttributeList.cpp
 *
 *    DESCRIPTION:    Read/Write osgSim::ShapeAttributeList in binary 
 *                    format to disk.
 *
 *    CREATED BY:     John Vidar Larring
 *
 *    HISTORY:        Created 25.8.2008
 *
 **********************************************************************/

#include "Exception.h"
#include "ShapeAttributeList.h"

using namespace ive;

void ShapeAttributeList::write(DataOutputStream* out)
{

    // Write ShapeAttributeList's identification.
    out->writeInt(IVESHAPEATTRIBUTELIST);

    // Write ShapeAttributeList's properties.

    // Write size of list
    out->writeUInt(size());

    // Write elements of the list
    osgSim::ShapeAttributeList::const_iterator it = begin();
    for (const_iterator it = begin(); it != end(); it++)
    {
        write(out, *it);
    }
}

void ShapeAttributeList::read(DataInputStream* in)
{
    // Peek on ShapeAttributeList's identification.
    int id = in->peekInt();
    if(id == IVESHAPEATTRIBUTELIST){
        // Read ShapeAttributeList's identification.
        id = in->readInt();

        // Read ShapeAttributeList's properties

        // Read size of the list
        unsigned int count = in->readUInt();
        
        resize(count);

        // Read elements of the list
        for (unsigned int i=0; i < count; i++)
        {
            read(in, (*this)[i]);
        }
    }
    else{
        throw Exception("ShapeAttributeList::read(): Expected ShapeAttributeList identification.");
    }
}

void ShapeAttributeList::write(DataOutputStream* out, const osgSim::ShapeAttribute& sa)
{
    // Write name
    out->writeString(sa.getName());
    
    // Write datatype
    osgSim::ShapeAttribute::Type type = sa.getType();
    out->writeInt((int)type);

    // Write data
    switch (type)
    {
    case osgSim::ShapeAttribute::INTEGER:
        out->writeInt(sa.getInt());
        break;
    case osgSim::ShapeAttribute::DOUBLE:
        out->writeDouble(sa.getDouble());
        break;
    case osgSim::ShapeAttribute::STRING:
        out->writeBool(sa.getString() != 0);
        if (sa.getString()) out->writeString(std::string(sa.getString()));
        break;
    default:
        // Ignore Unknown data type
        break;
    }
}

void ShapeAttributeList::read(DataInputStream* in, osgSim::ShapeAttribute& sa)
{
    // Read name
    sa.setName(in->readString());

    // Read type
    int type = in->readInt();

    // Read data
    switch (type)
    {
        case osgSim::ShapeAttribute::INTEGER:
            sa.setValue(in->readInt()); 
            break;
        case osgSim::ShapeAttribute::DOUBLE:
            sa.setValue(in->readDouble()); 
            break;
        case osgSim::ShapeAttribute::STRING:
            if (in->readBool())
                sa.setValue(in->readString().c_str());
            else
                sa.setValue((char*)0);
            break;
        default:
            // Ignore Unknown data type
            break;
    }
}

