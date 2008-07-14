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


bool osgDB::queryPlugin(const std::string& fileName, ReaderWriterInfoList& infoList)
{
    if (osgDB::Registry::instance()->loadLibrary(fileName))
    {
        const Registry::ReaderWriterList& rwList = osgDB::Registry::instance()->getReaderWriterList();
        for(Registry::ReaderWriterList::const_iterator itr = rwList.begin();
            itr != rwList.end();
            ++itr)
        {
            const ReaderWriter* rw = itr->get();
            osg::ref_ptr<ReaderWriterInfo> rwi = new ReaderWriterInfo;
            rwi->plugin = fileName;
            rwi->description = rw->className();
            rwi->protocols = rw->supportedProtocols();
            rwi->extensions = rw->supportedExtensions();
            rwi->options = rw->supportedOptions();
            
            infoList.push_back(rwi.get());
        }

        osgDB::Registry::instance()->closeLibrary(fileName);
        return true;
    }
    else
    {
        return false;
    }
}
