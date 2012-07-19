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
#include "VolumeTile.h"
#include "Group.h"
#include "VolumeLayer.h"

#include <osgVolume/RayTracedTechnique>
#include <osgVolume/FixedFunctionTechnique>

using namespace ive;

void VolumeTile::write(DataOutputStream* out)
{
    // Write VolumeTile's identification.
    out->writeInt(IVEVOLUMETILE);
    // If the osg class is inherited by any other class we should also write this to file.
    osg::Group*  group = dynamic_cast<osg::Group*>(this);
    if(group)
        ((ive::Group*)(group))->write(out);
    else
        out_THROW_EXCEPTION("VolumeTile::write(): Could not cast this osgVolume::VolumeTile to an osg::Group.");


    out->writeVolumeLocator(getLocator());
    out->writeVolumeLayer(getLayer());

    writeVolumeTechnique(out, getVolumeTechnique());

}

void VolumeTile::read(DataInputStream* in)
{
    // Peek on VolumeTile's identification.
    int id = in->peekInt();
    if (id != IVEVOLUMETILE) in_THROW_EXCEPTION("VolumeTile::read(): Expected Volume identification.");

    // Read VolumeTile's identification.
    id = in->readInt();
    // If the osg class is inherited by any other class we should also read this from file.
    osg::Group*  group = dynamic_cast<osg::Group*>(this);
    if(group)
        ((ive::Group*)(group))->read(in);
    else
        in_THROW_EXCEPTION("VolumeTile::read(): Could not cast this osgVolume::VolumeTile to an osg::Group.");

    setLocator(in->readVolumeLocator());
    setLayer(in->readVolumeLayer());

    setVolumeTechnique(readVolumeTechnique(in));
}

void VolumeTile::writeVolumeTechnique(DataOutputStream* out, osgVolume::VolumeTechnique* technique)
{
    if (dynamic_cast<osgVolume::RayTracedTechnique*>(technique))
    {
        out->writeBool(true);
        out->writeInt(IVEVOLUMERAYTRACEDTECHNIQUE);
    }
    if (dynamic_cast<osgVolume::FixedFunctionTechnique*>(technique))
    {
        out->writeBool(true);
        out->writeInt(IVEVOLUMEFIXEDPIPELINETECHNIQUE);
    }
    else
    {
        out->writeBool(false);
    }
}

osgVolume::VolumeTechnique* VolumeTile::readVolumeTechnique(DataInputStream* in)
{
    bool hasTechnique = in->readBool();
    if (!hasTechnique) return 0;

    int id = in->readInt();
    if (id==IVEVOLUMERAYTRACEDTECHNIQUE)
    {
        return new osgVolume::RayTracedTechnique;
    }
    else if (id==IVEVOLUMEFIXEDPIPELINETECHNIQUE)
    {
        return new osgVolume::FixedFunctionTechnique;
    }
    else
    {
        return 0;
    }
}
