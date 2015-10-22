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

////////////////////////////////////////////////////////////////////////////////////////////
//
// FileCache
//
FileCache::FileCache(const std::string& path):
    osg::Referenced(true),
    _fileCachePath(path)
{
    OSG_INFO<<"Constructed FileCache : "<<path<<std::endl;
}

FileCache::~FileCache()
{
    OSG_INFO<<"Destructed FileCache "<<std::endl;
}

bool FileCache::isFileAppropriateForFileCache(const std::string& originalFileName) const
{
    return osgDB::containsServerAddress(originalFileName);
}

std::string FileCache::createCacheFileName(const std::string& originalFileName) const
{
    std::string serverAddress = osgDB::getServerAddress(originalFileName);
    std::string cacheFileName = _fileCachePath + "/" +
                                serverAddress + (serverAddress.empty()?"":"/") +
                                osgDB::getServerFileName(originalFileName);

    OSG_DEBUG<<"FileCache::createCacheFileName("<<originalFileName<<") = "<<cacheFileName<<std::endl;

    return cacheFileName;
}

bool FileCache::existsInCache(const std::string& originalFileName) const
{
    if (osgDB::fileExists(createCacheFileName(originalFileName)))
    {
        return !isCachedFileBlackListed(originalFileName);
    }
    return false;
}

ReaderWriter::ReadResult FileCache::readObject(const std::string& originalFileName, const osgDB::Options* options) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty() && osgDB::fileExists(cacheFileName))
    {
        OSG_INFO<<"FileCache::readObjectFromCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        return osgDB::Registry::instance()->readObject(cacheFileName, options);
    }
    else
    {
        return 0;
    }
}

ReaderWriter::WriteResult FileCache::writeObject(const osg::Object& object, const std::string& originalFileName, const osgDB::Options* options) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty())
    {
        std::string path = osgDB::getFilePath(cacheFileName);

        if (!osgDB::fileExists(path) && !osgDB::makeDirectory(path))
        {
            OSG_NOTICE<<"Could not create cache directory: "<<path<<std::endl;
            return ReaderWriter::WriteResult::ERROR_IN_WRITING_FILE;
        }

        OSG_INFO<<"FileCache::writeObjectToCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        ReaderWriter::WriteResult result = osgDB::Registry::instance()->writeObject(object, cacheFileName, options);
        if (result.success())
        {
            removeFileFromBlackListed(originalFileName);
        }
        return result;
    }
    return ReaderWriter::WriteResult::FILE_NOT_HANDLED;
}

ReaderWriter::ReadResult FileCache::readImage(const std::string& originalFileName, const osgDB::Options* options) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty() && osgDB::fileExists(cacheFileName))
    {
        OSG_INFO<<"FileCache::readImageFromCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        return osgDB::Registry::instance()->readImage(cacheFileName, options);
    }
    else
    {
        return 0;
    }
}

ReaderWriter::WriteResult FileCache::writeImage(const osg::Image& image, const std::string& originalFileName, const osgDB::Options* options) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty())
    {
        std::string path = osgDB::getFilePath(cacheFileName);

        if (!osgDB::fileExists(path) && !osgDB::makeDirectory(path))
        {
            OSG_NOTICE<<"Could not create cache directory: "<<path<<std::endl;
            return ReaderWriter::WriteResult::ERROR_IN_WRITING_FILE;
        }

        OSG_INFO<<"FileCache::writeImageToCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        ReaderWriter::WriteResult result = osgDB::Registry::instance()->writeImage(image, cacheFileName, options);
        if (result.success())
        {
            removeFileFromBlackListed(originalFileName);
        }
        return result;
    }
    return ReaderWriter::WriteResult::FILE_NOT_HANDLED;
}

ReaderWriter::ReadResult FileCache::readHeightField(const std::string& originalFileName, const osgDB::Options* options) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty() && osgDB::fileExists(cacheFileName))
    {
        OSG_INFO<<"FileCache::readHeightFieldFromCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        return osgDB::Registry::instance()->readHeightField(cacheFileName, options);
    }
    else
    {
        return 0;
    }
}

ReaderWriter::WriteResult FileCache::writeHeightField(const osg::HeightField& hf, const std::string& originalFileName, const osgDB::Options* options) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty())
    {
        std::string path = osgDB::getFilePath(cacheFileName);

        if (!osgDB::fileExists(path) && !osgDB::makeDirectory(path))
        {
            OSG_NOTICE<<"Could not create cache directory: "<<path<<std::endl;
            return ReaderWriter::WriteResult::ERROR_IN_WRITING_FILE;
        }

        OSG_INFO<<"FileCache::writeHeightFieldToCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        ReaderWriter::WriteResult result = osgDB::Registry::instance()->writeHeightField(hf, cacheFileName, options);
        if (result.success())
        {
            removeFileFromBlackListed(originalFileName);
        }
        return result;
    }
    return ReaderWriter::WriteResult::FILE_NOT_HANDLED;
}

ReaderWriter::ReadResult FileCache::readNode(const std::string& originalFileName, const osgDB::Options* options, bool buildKdTreeIfRequired) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty() && osgDB::fileExists(cacheFileName))
    {
        OSG_INFO<<"FileCache::readNodeFromCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        return osgDB::Registry::instance()->readNode(cacheFileName, options, buildKdTreeIfRequired);
    }
    else
    {
        return 0;
    }
}

ReaderWriter::WriteResult FileCache::writeNode(const osg::Node& node, const std::string& originalFileName, const osgDB::Options* options) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty())
    {
        std::string path = osgDB::getFilePath(cacheFileName);

        if (!osgDB::fileExists(path) && !osgDB::makeDirectory(path))
        {
            OSG_NOTICE<<"Could not create cache directory: "<<path<<std::endl;
            return ReaderWriter::WriteResult::ERROR_IN_WRITING_FILE;
        }

        OSG_INFO<<"FileCache::writeNodeToCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        ReaderWriter::WriteResult result = osgDB::Registry::instance()->writeNode(node, cacheFileName, options);
        if (result.success())
        {
            removeFileFromBlackListed(originalFileName);
        }
        return result;
    }
    return ReaderWriter::WriteResult::FILE_NOT_HANDLED;
}


ReaderWriter::ReadResult FileCache::readShader(const std::string& originalFileName, const osgDB::Options* options) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty() && osgDB::fileExists(cacheFileName))
    {
        OSG_INFO<<"FileCache::readShaderFromCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        return osgDB::Registry::instance()->readShader(cacheFileName, options);
    }
    else
    {
        return 0;
    }
}

ReaderWriter::WriteResult FileCache::writeShader(const osg::Shader& shader, const std::string& originalFileName, const osgDB::Options* options) const
{
    std::string cacheFileName = createCacheFileName(originalFileName);
    if (!cacheFileName.empty())
    {
        std::string path = osgDB::getFilePath(cacheFileName);

        if (!osgDB::fileExists(path) && !osgDB::makeDirectory(path))
        {
            OSG_NOTICE<<"Could not create cache directory: "<<path<<std::endl;
            return ReaderWriter::WriteResult::ERROR_IN_WRITING_FILE;
        }

        OSG_INFO<<"FileCache::writeShaderToCache("<<originalFileName<<") as "<<cacheFileName<<std::endl;
        ReaderWriter::WriteResult result = osgDB::Registry::instance()->writeShader(shader, cacheFileName, options);
        if (result.success())
        {
            removeFileFromBlackListed(originalFileName);
        }
        return result;
    }
    return ReaderWriter::WriteResult::FILE_NOT_HANDLED;
}


bool FileCache::isCachedFileBlackListed(const std::string& originalFileName) const
{
    for(DatabaseRevisionsList::const_iterator itr = _databaseRevisionsList.begin();
        itr != _databaseRevisionsList.end();
        ++itr)
    {
        if ((*itr)->isFileBlackListed(originalFileName)) return true;
    }
    return false;
}

bool FileCache::removeFileFromBlackListed(const std::string& originalFileName) const
{
    for(DatabaseRevisionsList::const_iterator dr_itr = _databaseRevisionsList.begin();
        dr_itr != _databaseRevisionsList.end();
        ++dr_itr)
    {
        DatabaseRevisions* dr = dr_itr->get();

        if (dr->getDatabasePath().length()>=originalFileName.length()) continue;
        if (originalFileName.compare(0,dr->getDatabasePath().length(), dr->getDatabasePath())!=0) continue;

        std::string localPath(originalFileName,
                            dr->getDatabasePath().empty() ? 0 : dr->getDatabasePath().length()+1,
                            std::string::npos);

        for(DatabaseRevisions::DatabaseRevisionList::const_iterator itr = dr->getDatabaseRevisionList().begin();
            itr != dr->getDatabaseRevisionList().end();
            ++itr)
        {
            DatabaseRevision* revision = const_cast<DatabaseRevision*>(itr->get());

            if (revision->getFilesAdded() && revision->getFilesAdded()->removeFile(localPath))
            {
                std::string cacheFileName = revision->getFilesAdded()->getName();
                if (containsServerAddress(cacheFileName)) cacheFileName = createCacheFileName(cacheFileName);
                if (!cacheFileName.empty()) writeObjectFile(*(revision->getFilesAdded()), cacheFileName);
            }

            if (revision->getFilesRemoved() && revision->getFilesRemoved()->removeFile(localPath))
            {
                std::string cacheFileName = revision->getFilesRemoved()->getName();
                if (containsServerAddress(cacheFileName)) cacheFileName = createCacheFileName(cacheFileName);
                if (!cacheFileName.empty()) writeObjectFile(*(revision->getFilesRemoved()), cacheFileName);
            }

            if (revision->getFilesModified() && revision->getFilesModified()->removeFile(localPath))
            {
                std::string cacheFileName = revision->getFilesModified()->getName();
                if (containsServerAddress(cacheFileName)) cacheFileName = createCacheFileName(cacheFileName);
                if (!cacheFileName.empty()) writeObjectFile(*(revision->getFilesModified()), cacheFileName);
            }
        }
    }
    return false;
}

bool FileCache::loadDatabaseRevisionsForFile(const std::string& originalFileName)
{
    OSG_INFO<<"FileCache::loadDatabaseRevisionsForFile("<<originalFileName<<")"<<std::endl;

    std::string revisionsFileName = originalFileName;
    if (getLowerCaseFileExtension(revisionsFileName)!="revisions") revisionsFileName += ".revisions";

    OSG_INFO<<"   revisionsFileName("<<revisionsFileName<<")"<<std::endl;

    osg::ref_ptr<DatabaseRevisions> dr_local;

    std::string cacheFileName = createCacheFileName(revisionsFileName);

    // check to see if revion file is already loaded.
    DatabaseRevisionsList::iterator ritr = _databaseRevisionsList.begin();
    for(;
        ritr != _databaseRevisionsList.end() && !dr_local;
        ++ritr)
    {
        OSG_INFO<<"   comparing "<<(*ritr)->getName()<<" to "<<revisionsFileName<<std::endl;

        if ((*ritr)->getName()==revisionsFileName)
        {
            OSG_INFO<<"Already loaded"<<std::endl;
            dr_local = *ritr;
        }
    }

    if (!dr_local)
    {
        if (!cacheFileName.empty() && osgDB::fileExists(cacheFileName))
        {
            OSG_INFO<<"   found revisions file in local cache, now loading it"<<std::endl;
            osg::ref_ptr<osg::Object> object = osgDB::readRefObjectFile(cacheFileName);
            dr_local = dynamic_cast<DatabaseRevisions*>(object.get());
            if (dr_local)
            {
                OSG_INFO<<"   loaded local revisions File("<<cacheFileName<<")"<<std::endl;
            }
        }
        else
        {
            OSG_INFO<<"   could not load found revisions file from local cache."<<std::endl;
        }
    }

    // now load revision file from remote server
    osg::ref_ptr<osg::Object> object = osgDB::readRefObjectFile(revisionsFileName+".curl");
    osg::ref_ptr<DatabaseRevisions> dr_remote = dynamic_cast<DatabaseRevisions*>(object.get());

    if (dr_remote.valid())
    {
        bool needToWriteRevisionsFileToDisk = true;
        if (dr_local.valid())
        {
            if (dr_local->getDatabaseRevisionList().size()==dr_remote->getDatabaseRevisionList().size())
            {
                unsigned int i;
                for(i=0; i<dr_local->getDatabaseRevisionList().size(); ++i)
                {
                    DatabaseRevision* revision_local = dr_local->getDatabaseRevision(i);
                    DatabaseRevision* revision_remote = dr_remote->getDatabaseRevision(i);
                    OSG_INFO<<"   Comparing local "<<revision_local->getName()<<" to remote "<<revision_remote->getName()<<std::endl;
                    if (revision_local->getName()!=revision_remote->getName()) break;
                }
                needToWriteRevisionsFileToDisk = (i!=dr_local->getDatabaseRevisionList().size());
                OSG_INFO<<"Local and remote revisions are different "<<needToWriteRevisionsFileToDisk<<std::endl;
            }
        }

        if (needToWriteRevisionsFileToDisk)
        {
            OSG_INFO<<"Need to write DatabaseRevions "<<revisionsFileName<<" to local FileCache"<<std::endl;
            if (!cacheFileName.empty()) writeObjectFile(*dr_remote, cacheFileName);
        }
        else
        {
            OSG_INFO<<"No need to write DatabaseRevions "<<revisionsFileName<<" to local FileCache"<<std::endl;
        }

    }


    osg::ref_ptr<DatabaseRevisions> dr = dr_remote.valid() ? dr_remote : dr_local;

    if (dr.valid())
    {
        OSG_INFO<<"   loaded remote revisions File("<<revisionsFileName<<")"<<std::endl;

        if (ritr != _databaseRevisionsList.end())
        {
            // replace already loaded DatabaseRevisions object
            OSG_INFO<<"Replacing already loaded DatabaseRevisions object"<<std::endl;
            *ritr = dr;
        }
        else
        {
            OSG_INFO<<"Added newly loaded DatabaseRevisions object "<<dr->getName()<<std::endl;
            _databaseRevisionsList.push_back(dr);
        }

        // now need to load the individual FileLists
        for(DatabaseRevisions::DatabaseRevisionList::iterator itr = dr->getDatabaseRevisionList().begin();
            itr != dr->getDatabaseRevisionList().end();
            ++itr)
        {
            DatabaseRevision* revision = itr->get();

            OSG_INFO<<"     now loaded DatabaseRevisions "<<revision->getName()<<" FileList contents"<<std::endl;
            if (revision->getFilesAdded())
            {
                FileList* fileList = readFileList(osgDB::concatPaths(revision->getDatabasePath(), revision->getFilesAdded()->getName()));
                if (fileList)
                {
                    revision->setFilesAdded(fileList);
                }
            }

            if (revision->getFilesRemoved())
            {
                FileList* fileList = readFileList(osgDB::concatPaths(revision->getDatabasePath(), revision->getFilesRemoved()->getName()));
                if (fileList)
                {
                    revision->setFilesRemoved(fileList);
                }
            }

            if (revision->getFilesModified())
            {
                FileList* fileList = readFileList(osgDB::concatPaths(revision->getDatabasePath(), revision->getFilesModified()->getName()));
                if (fileList)
                {
                    revision->setFilesModified(fileList);
                }
            }
        }

        return true;
    }
    else
    {
        OSG_NOTICE<<"   failed to read revisions File, object.get()="<<object.get()<<std::endl;
        return false;
    }
}

FileList* FileCache::readFileList(const std::string& originalFileName) const
{
    osg::ref_ptr<FileList> fileList;

    std::string cacheFileListName = createCacheFileName(originalFileName);
    if (!cacheFileListName.empty() && osgDB::fileExists(cacheFileListName))
    {
        osg::ref_ptr<osg::Object> object = osgDB::readRefObjectFile(cacheFileListName);
        fileList = dynamic_cast<osgDB::FileList*>(object.get());
        if (fileList) OSG_INFO<<"     loadeded FileList from local cache "<<fileList->getName()<<std::endl;
    }

    if (!fileList)
    {
        OSG_INFO<<"       complete_path="<<originalFileName<<std::endl;
        osg::ref_ptr<osg::Object> object = osgDB::readRefObjectFile(originalFileName+".curl");
        fileList = dynamic_cast<osgDB::FileList*>(object.get());
        if (fileList)
        {
            OSG_INFO<<"     loadeded FileList from remote system "<<fileList->getName()<<std::endl;
            OSG_INFO<<"     Need to write to local file cache "<<fileList->getName()<<std::endl;
            if (!cacheFileListName.empty()) writeObjectFile(*fileList, cacheFileListName);
        }
    }
    return fileList.release();
}
