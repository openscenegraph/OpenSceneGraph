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
#include "VolumeImageLayer.h"
#include "VolumeLayer.h"

#include <osgDB/ReadFile>

using namespace ive;

void VolumeImageLayer::write(DataOutputStream* out)
{
    // Write Layer's identification.
    out->writeInt(IVEVOLUMEIMAGELAYER);

    // If the osg class is inherited by any other class we should also write this to file.
    osgVolume::Layer*  layer = dynamic_cast<osgVolume::Layer*>(this);
    if  (layer)
        ((ive::VolumeLayer*)(layer))->write(out);
    else
        out_THROW_EXCEPTION("VolumeImageLayer::write(): Could not cast this osgVolume::ImageLayer to an osgVolume::Layer.");


    IncludeImageMode imMode = out->getIncludeImageMode(getImage());

    if (getFileName().empty() && imMode==IMAGE_REFERENCE_FILE) imMode = IMAGE_INCLUDE_DATA;

    out->writeChar(imMode);
    out->writeImage(imMode,getImage());

}

void VolumeImageLayer::read(DataInputStream* in)
{
    // Peek on Layer's identification.
    int id = in->peekInt();
    if (id != IVEVOLUMEIMAGELAYER)
        in_THROW_EXCEPTION("VolumeImageLayer::read(): Expected ImageLayer identification.");

    // Read Layer's identification.
    id = in->readInt();

    // If the osg class is inherited by any other class we should also read this from file.
    osgVolume::Layer*  layer = dynamic_cast<osgVolume::Layer*>(this);
    if (layer)
        ((ive::VolumeLayer*)(layer))->read(in);
    else
        in_THROW_EXCEPTION("ImageLayer::read(): Could not cast this osgVolume::Layer to an osg::Group.");

    // Should we read image data from stream
    IncludeImageMode includeImg = (IncludeImageMode)in->readChar();

    if (includeImg==IMAGE_REFERENCE_FILE)
    {
        setFileName(in->readString());
    }
    else
    {
        setImage(in->readImage(includeImg));
    }
}
