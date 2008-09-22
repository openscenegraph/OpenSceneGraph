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
#include <osgDB/FileNameUtils>
#include <osg/Version>

#include <osgDB/PluginQuery>

using namespace osgDB;

FileNameList osgDB::listAllAvailablePlugins()
{
    FileNameList pluginFiles;
    std::string validExtension = ADDQUOTES(OSG_PLUGIN_EXTENSION);

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
            if (pos==std::string::npos)
            {
                continue;
            }
            std::string ext = getFileExtensionIncludingDot(*itr);
            if (ext != validExtension)
            {
                continue;
            }
            pluginFiles.push_back(fullPath + std::string("/")+*itr);
        }
    }
    
    return pluginFiles;
}


bool osgDB::queryPlugin(const std::string& fileName, ReaderWriterInfoList& infoList)
{
    typedef std::set<const ReaderWriter*> ReaderWriterSet;
    ReaderWriterSet previouslyLoadedReaderWriters;

    const Registry::ReaderWriterList& rwList = osgDB::Registry::instance()->getReaderWriterList();
    for(Registry::ReaderWriterList::const_iterator itr = rwList.begin();
        itr != rwList.end();
        ++itr)
    {
        const ReaderWriter* rw = itr->get();
        previouslyLoadedReaderWriters.insert(rw);
    }

    if (osgDB::Registry::instance()->loadLibrary(fileName))
    {
        const Registry::ReaderWriterList& rwList = osgDB::Registry::instance()->getReaderWriterList();
        for(Registry::ReaderWriterList::const_iterator itr = rwList.begin();
            itr != rwList.end();
            ++itr)
        {
            const ReaderWriter* rw = itr->get();
            
            if (previouslyLoadedReaderWriters.count(rw)==0)
            {
                osg::ref_ptr<ReaderWriterInfo> rwi = new ReaderWriterInfo;
                rwi->plugin = fileName;
                rwi->description = rw->className();
                rwi->protocols = rw->supportedProtocols();
                rwi->extensions = rw->supportedExtensions();
                rwi->options = rw->supportedOptions();

                infoList.push_back(rwi.get());
            }
        }

        osgDB::Registry::instance()->closeLibrary(fileName);
        return true;
    }
    else
    {
        return false;
    }
}

static std::string padwithspaces(const std::string& str, unsigned int padLength)
{
    std::string newStr(str);
    while(newStr.length()<padLength) newStr.push_back(' ');
    return newStr;
}

bool osgDB::outputPluginDetails(std::ostream& out, const std::string& fileName)
{
    osgDB::ReaderWriterInfoList infoList;
    if (osgDB::queryPlugin(fileName, infoList))
    {
        out<<"Plugin "<<fileName<<std::endl;
        out<<"{"<<std::endl;

        for(osgDB::ReaderWriterInfoList::iterator rwi_itr = infoList.begin();
            rwi_itr != infoList.end();
            ++rwi_itr)
        {
            osgDB::ReaderWriterInfo& info = *(*rwi_itr);
            out<<"    ReaderWriter : "<<info.description<<std::endl;
            out<<"    {"<<std::endl;

            unsigned int longestOptionLength = 0;
            osgDB::ReaderWriter::FormatDescriptionMap::iterator fdm_itr;
            for(fdm_itr = info.protocols.begin();
                fdm_itr != info.protocols.end();
                ++fdm_itr)
            {
                if (fdm_itr->first.length()>longestOptionLength) longestOptionLength = fdm_itr->first.length();
            }

            for(fdm_itr = info.extensions.begin();
                fdm_itr != info.extensions.end();
                ++fdm_itr)
            {
                if (fdm_itr->first.length()>longestOptionLength) longestOptionLength = fdm_itr->first.length();
            }

            for(fdm_itr = info.options.begin();
                fdm_itr != info.options.end();
                ++fdm_itr)
            {
                if (fdm_itr->first.length()>longestOptionLength) longestOptionLength = fdm_itr->first.length();
            }

            unsigned int padLength = longestOptionLength+4;

            for(fdm_itr = info.protocols.begin();
                fdm_itr != info.protocols.end();
                ++fdm_itr)
            {
                out<<"        protocol   : "<<padwithspaces(fdm_itr->first, padLength)<<fdm_itr->second<<std::endl;
            }

            for(fdm_itr = info.extensions.begin();
                fdm_itr != info.extensions.end();
                ++fdm_itr)
            {
                out<<"        extensions : ."<<padwithspaces(fdm_itr->first, padLength-1)<<fdm_itr->second<<std::endl;
            }

            for(fdm_itr = info.options.begin();
                fdm_itr != info.options.end();
                ++fdm_itr)
            {
                out<<"        options    : "<<padwithspaces(fdm_itr->first, padLength)<<fdm_itr->second<<std::endl;
            }
            out<<"    }"<<std::endl;
        }
        out<<"}"<<std::endl<<std::endl;
        return true;
    }
    else
    {
        out<<"Plugin "<<fileName<<" not found."<<std::endl;
        return false;
    }
}

