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

#ifdef __APPLE__
#define USE_POSIX_SPAWN 1
#endif

#ifdef USE_POSIX_SPAWN

#include <spawn.h>
#include <sys/wait.h>

int osg_system(const char* command)
{
    pid_t pid;
    posix_spawn(&pid, command, NULL, NULL, NULL, NULL);
    return waitpid(pid, NULL, 0);
}

#else // use tranditional C sysmtem call for osg_system implementation

#include <stdlib.h>

int osg_system(const char* command)
{
    return system(command);
}

#endif

}

