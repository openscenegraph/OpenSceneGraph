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
#include "Volume.h"
#include "Group.h"

using namespace ive;

void Volume::write(DataOutputStream* out)
{
    // Write Volume's identification.
    out->writeInt(IVEVOLUME);
    // If the osg class is inherited by any other class we should also write this to file.
    osg::Group*  group = dynamic_cast<osg::Group*>(this);
    if(group)
        ((ive::Group*)(group))->write(out);
    else
        out_THROW_EXCEPTION("Volume::write(): Could not cast this osgVolume::Volume to an osg::Group.");
}

void Volume::read(DataInputStream* in)
{
    // Peek on Volume's identification.
    int id = in->peekInt();
    if (id != IVEVOLUME) in_THROW_EXCEPTION("Volume::read(): Expected Volume identification.");

    // Read Volume's identification.
    id = in->readInt();
    // If the osg class is inherited by any other class we should also read this from file.
    osg::Group*  group = dynamic_cast<osg::Group*>(this);
    if(group)
        ((ive::Group*)(group))->read(in);
    else
        in_THROW_EXCEPTION("Volume::read(): Could not cast this osgVolume::Volume to an osg::Group.");

}
