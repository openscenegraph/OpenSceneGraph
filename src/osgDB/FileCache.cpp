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

#include <osgDB/FileCache>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

using namespace osgDB;

FileCache::FileCache(const std::string& path):
    _fileCachePath(path)
{
    osg::notify(osg::INFO)<<"Constructed FileCache : "<<path<<std::endl;
}

FileCache::~FileCache()
{
    osg::notify(osg::INFO)<<"Destructed FileCache "<<std::endl;
}

std::string FileCache::createCacheFileName(const std::string& originalFileName) const
{
    std::string cacheFileName = _fileCachePath + "/" + 
                                osgDB::getServerAddress(originalFileName) + "/" + 
                                osgDB::getServerFileName(originalFileName);
                                
    osg::notify(osg::INFO)<<"FileCache::createCacheFileName("<<originalFileName<<") = "<<cacheFileName<<std::endl;
    
    return cacheFileName;
}

bool FileCache::existsInCache(const std::string& originalFileName) const
{
    return osgDB::fileExists(createCacheFileName(originalFileName));
}

ReaderWriter::ReadResult FileCache::readNode(const std::string& originalFileName, const osgDB::ReaderWriter::Options* options, bool buildKdTreeIfRequired) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty() && osgDB::fileExists(cacheFileName))
    {
        osg::notify(osg::INFO)<<"FileCache::readNodeFromCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        return osgDB::Registry::instance()->readNode(cacheFileName, options, buildKdTreeIfRequired);
    }
    else
    {
        return 0;
    }
}

ReaderWriter::WriteResult FileCache::writeNode(const osg::Node& node, const std::string& originalFileName, const osgDB::ReaderWriter::Options* options) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty())
    {
        std::string path = osgDB::getFilePath(cacheFileName);

        if (!osgDB::fileExists(path) && !osgDB::makeDirectory(path))
        {
            osg::notify(osg::NOTICE)<<"Could not create cache directory: "<<path<<std::endl;
            return ReaderWriter::WriteResult::ERROR_IN_WRITING_FILE;
        }

        osg::notify(osg::INFO)<<"FileCache::writeNodeToCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        return osgDB::Registry::instance()->writeNode(node, cacheFileName, options);
    }
    return ReaderWriter::WriteResult::FILE_NOT_HANDLED;
}
