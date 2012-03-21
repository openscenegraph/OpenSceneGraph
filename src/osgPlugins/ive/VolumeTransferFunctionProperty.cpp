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
#include "Object.h"

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
        out_THROW_EXCEPTION("VolumeTransferFunctionProperty::write(): Could not cast this osgVolume::TransferFunctionProperty to an osg::Object.");


    osg::TransferFunction1D* tf = dynamic_cast<osg::TransferFunction1D*>(getTransferFunction());
    if (tf)
    {

        out->writeUInt(1); // TransferFunction1D
        out->writeUInt(tf->getNumberImageCells());

        const osg::TransferFunction1D::ColorMap& colourMap = tf->getColorMap();

        // count the number of colour entries in the map so we can write it to the .ive file
        unsigned int numColours = 0;
        for(osg::TransferFunction1D::ColorMap::const_iterator itr = colourMap.begin();
            itr != colourMap.end();
            ++itr)
        {
            ++numColours;
        }

        // write out the num of colours
        out->writeUInt(numColours);

        // write out the colour map entires
        for(osg::TransferFunction1D::ColorMap::const_iterator itr = colourMap.begin();
            itr != colourMap.end();
            ++itr)
        {
            out->writeFloat(itr->first);
            out->writeVec4(itr->second);
        }
    }
    else
    {
        out->writeUInt(0);
    }
}

void VolumeTransferFunctionProperty::read(DataInputStream* in)
{
    // Peek on Layer's identification.
    int id = in->peekInt();
    if (id != IVEVOLUMETRANSFERFUNCTIONPROPERTY)
        in_THROW_EXCEPTION("VolumeTransferFunctionProperty::read(): Expected CompositeProperty identification.");

    // Read Layer's identification.
    id = in->readInt();

    // If the osg class is inherited by any other class we should also read this from file.
    osg::Object* object = dynamic_cast<osg::Object*>(this);
    if (object)
        ((ive::Object*)(object))->read(in);
    else
        in_THROW_EXCEPTION("VolumeTransferFunctionProperty::write(): Could not cast this osgVolume::TransferFunctionProperty to an osg::Object.");

    unsigned int numDimensions = in->readUInt();
    if (numDimensions==1)
    {
        osg::TransferFunction1D* tf = new osg::TransferFunction1D;
        setTransferFunction(tf);

        tf->allocate(in->readUInt());

        osg::TransferFunction1D::ColorMap& colourMap = tf->getColorMap();

        // count the number of colour entries in the map so we can write it to the .ive file
        unsigned int numColours = in->readUInt();
        for(unsigned int i=0; i<numColours; ++i)
        {
            float value = in->readFloat();
            osg::Vec4 colour = in->readVec4();
            colourMap[value] = colour;
        }

        tf->updateImage();
    }
    else
    {
    }

}
