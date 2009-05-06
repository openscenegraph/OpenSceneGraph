/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include "Exception.h"
#include "VolumeTransferFunctionProperty.h"

#include <osgDB/ReadFile>

using namespace ive;

void VolumeTransferFunctionProperty::write(DataOutputStream* out)
{
    // Write Layer's identification.
    out->writeInt(IVEVOLUMETRANSFERFUNCTIONPROPERTY);

    // If the osg class is inherited by any other class we should also write this to file.
    osg::Object* object = dynamic_cast<osg::Object*>(this);
    if (object)
        ((ive::Object*)(object))->write(out);
    else
        throw Exception("VolumeTransferFunctionProperty::write(): Could not cast this osgVolume::TransferFunctionProperty to an osg::Object.");

    // out->writeFloat(getValue());
}

void VolumeTransferFunctionProperty::read(DataInputStream* in)
{
    // Peek on Layer's identification.
    int id = in->peekInt();
    if (id != IVEVOLUMETRANSFERFUNCTIONPROPERTY)
        throw Exception("VolumeTransferFunctionProperty::read(): Expected CompositeProperty identification.");
    
    // Read Layer's identification.
    id = in->readInt();

    // If the osg class is inherited by any other class we should also read this from file.
    osg::Object* object = dynamic_cast<osg::Object*>(this);
    if (object)
        ((ive::Object*)(object))->read(in);
    else
        throw Exception("VolumeTransferFunctionProperty::write(): Could not cast this osgVolume::TransferFunctionProperty to an osg::Object.");

    // setValue(in->readFloat());
}
