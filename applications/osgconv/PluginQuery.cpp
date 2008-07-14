/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/FileUtils>
#include <osg/Version>

#include "PluginQuery.h"

using namespace osgDB;

FileNameList osgDB::listAllAvailablePlugins()
{
    FileNameList pluginFiles;

    std::string pluginDirectoryName = std::string("osgPlugins-")+std::string(osgGetVersion());
    std::string fullPath = osgDB::findLibraryFile(pluginDirectoryName);
    if (!fullPath.empty())
    {
        osgDB::DirectoryContents contents = getDirectoryContents(fullPath);
        for(DirectoryContents::iterator itr = contents.begin();
            itr != contents.end();
            ++itr)
        {
            std::string::size_type pos = itr->find("osgdb_");
            if (pos!=std::string::npos)
            {
                pluginFiles.push_back(fullPath + std::string("/")+*itr);
            }
        }
    }
    
    return pluginFiles;
}
