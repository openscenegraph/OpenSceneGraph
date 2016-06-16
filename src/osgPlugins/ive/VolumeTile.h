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

#ifndef IVE_VOLUMETILE
#define IVE_VOLUMETILE 1

#include <osgVolume/VolumeTile>

#include "ReadWrite.h"

namespace ive
{

class VolumeTile : public osgVolume::VolumeTile
{
public:
	void write(DataOutputStream* out);
	void read(DataInputStream* in);

        void writeVolumeTechnique(DataOutputStream* out, osgVolume::VolumeTechnique* technique);
        osgVolume::VolumeTechnique* readVolumeTechnique(DataInputStream* out);

};

}

#endif
