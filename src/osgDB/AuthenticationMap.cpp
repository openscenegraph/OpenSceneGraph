/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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

#include <osgDB/AuthenticationMap>
#include <osgDB/FileNameUtils>

using namespace osgDB;

void AuthenticationMap::addAuthenticationDetails(const std::string& path, AuthenticationDetails* details)
{
   _authenticationMap[path] = details;
}

const AuthenticationDetails* AuthenticationMap::getAuthenticationDetails(const std::string& path) const
{
    // see if the full filename has its own authentication details
    AuthenticationDetailsMap::const_iterator itr = _authenticationMap.find(path);
    if (itr != _authenticationMap.end()) return itr->second.get();

    // now look to see if the paths to the file have their own authentication details
    std::string basePath = osgDB::getFilePath(path);
    while(!basePath.empty())
    {
        itr = _authenticationMap.find(basePath);
        if (itr != _authenticationMap.end()) return itr->second.get();

        basePath = osgDB::getFilePath(basePath);
    }
    return 0;
}

