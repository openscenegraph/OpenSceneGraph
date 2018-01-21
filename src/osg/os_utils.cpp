/* -*-c++-*- OpenSceneGraph - Copyright (C) 2018 Robert Osfield
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

#include <osg/os_utils>

extern "C" {

int osg_system(const char* command)
{
#ifdef OSG_SYSTEM_SUPPORTED
    return system(command);
#else
    printf("osg_system(%s) not supported.\n", command);
#endif
}

}

