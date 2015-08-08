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

#include "ZipArchive.h"

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include <sys/types.h>
#include <sys/stat.h>

#include <sstream>
#include <cstdio>
#include "unzip.h"

#if !defined(S_ISDIR)
#  if defined( _S_IFDIR) && !defined( __S_IFDIR)
#    define __S_IFDIR _S_IFDIR
#  endif
#  define S_ISDIR(mode)    (mode&__S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
#endif


ZipArchive::ZipArchive()  :
_zipLoaded( false )
{
}

ZipArchive::~ZipArchive()
{
}

/** close the archive (on all threads) */
void ZipArchive::close()
{
    if ( _zipLoaded )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> exclusive(_zipMutex);
        if ( _zipLoaded )
        {
            // close the file (on one thread since it's a shared file)
            const PerThreadData& data = getDataNoLock();
            CloseZip( data._zipHandle );

            // clear out the file handles
            _perThreadData.clear();

            // clear out the index.
            _zipIndex.clear();

            _zipLoaded = false;
        }
    }
}

/** return true if file exists in archive.*/
bool ZipArchive::fileExists(const std::string& filename) const
{
    return GetZipEntry(filename) != NULL;
}

/** Get the file name which represents the master file recorded in the Archive.*/
std::string ZipArchive::getMasterFileName() const
{
    return std::string();
}

std::string ZipArchive::getArchiveFileName() const
{
    std::string result;
    if( _zipLoaded )
    {
        result = _mainRecord.name;
    }
    return result;
}

/** Get the full list of file names available in the archive.*/
bool ZipArchive::getFileNames(osgDB::Archive::FileNameList& fileNameList) const
{
    if(_zipLoaded)
    {
        ZipEntryMap::const_iterator iter = _zipIndex.begin();

        for(;iter != _zipIndex.end(); ++iter)
        {
            fileNameList.push_back((*iter).first);
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool ZipArchive::open(const std::string& file, ArchiveStatus status, const osgDB::ReaderWriter::Options* options)
{
    if ( !_zipLoaded )
    {
        // exclusive lock when we open for the first time:
        OpenThreads::ScopedLock<OpenThreads::Mutex> exclusiveLock( _zipMutex );

        if ( !_zipLoaded ) // double-check avoids race condition
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

            // save the filename + password so other threads can open the file
            _filename = osgDB::findDataFile( file, options );
            if (_filename.empty()) return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;

            _password = ReadPassword(options);

            // open the zip file in this thread:
            const PerThreadData& data = getDataNoLock();

            // establish a shared (read-only) index:
            if ( data._zipHandle != NULL )
            {
                IndexZipFiles( data._zipHandle );
                _zipLoaded = true;
            }
        }
    }

    return _zipLoaded;
}

bool ZipArchive::open(std::istream& fin, const osgDB::ReaderWriter::Options* options)
{
    if ( !_zipLoaded )
    {
        // exclusive lock when we open for the first time:
        OpenThreads::ScopedLock<OpenThreads::Mutex> exclusive(_zipMutex);

        if ( !_zipLoaded ) // double-check avoids race condition
        {
            osgDB::ReaderWriter::ReadResult result = osgDB::ReaderWriter::ReadResult(osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED);

            if (fin.fail()) return false;

            // read the stream into a memory buffer that we'll keep around for any other
            // threads that want to open the file
            std::stringstream buf;
            buf << fin.rdbuf();
            _membuffer = buf.str();

            _password = ReadPassword(options);

            // open on this thread:
            const PerThreadData& data = getDataNoLock();

            if ( data._zipHandle != NULL )
            {
                IndexZipFiles( data._zipHandle );
                _zipLoaded = true;
            }
        }
    }

    return _zipLoaded;
}

osgDB::ReaderWriter::ReadResult ZipArchive::readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    osgDB::ReaderWriter::ReadResult rresult = osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!_zipLoaded || !acceptsExtension(ext)) return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    const ZIPENTRY* ze = GetZipEntry(file);
    if(ze != NULL)
    {
        std::stringstream buffer;

        osgDB::ReaderWriter* rw = ReadFromZipEntry(ze, options, buffer);
        if (rw != NULL)
        {
            // Setup appropriate options
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = options ?
            static_cast<osgDB::ReaderWriter::Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) :
            new osgDB::ReaderWriter::Options;

            local_opt->setPluginStringData("STREAM_FILENAME", osgDB::getSimpleFileName(ze->name));

            osgDB::ReaderWriter::ReadResult readResult = rw->readObject(buffer,local_opt.get());
            if (readResult.success())
            {
                return readResult;
            }
        }
    }

    return rresult;
}

osgDB::ReaderWriter::ReadResult ZipArchive::readImage(const std::string& file,const osgDB::ReaderWriter::Options* options) const
{
    osgDB::ReaderWriter::ReadResult rresult = osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!_zipLoaded || !acceptsExtension(ext)) return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    const ZIPENTRY* ze = GetZipEntry(file);
    if(ze != NULL)
    {
        std::stringstream buffer;

        osgDB::ReaderWriter* rw = ReadFromZipEntry(ze, options, buffer);
        if (rw != NULL)
        {
            // Setup appropriate options
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = options ?
            static_cast<osgDB::ReaderWriter::Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) :
            new osgDB::ReaderWriter::Options;

            local_opt->setPluginStringData("STREAM_FILENAME", osgDB::getSimpleFileName(ze->name));

            osgDB::ReaderWriter::ReadResult readResult = rw->readImage(buffer,local_opt.get());
            if (readResult.success())
            {
                return readResult;
            }
        }
    }

   return rresult;
}

osgDB::ReaderWriter::ReadResult ZipArchive::readHeightField(const std::string& file,const osgDB::ReaderWriter::Options* options) const
{
    osgDB::ReaderWriter::ReadResult rresult = osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!_zipLoaded || !acceptsExtension(ext)) return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    const ZIPENTRY* ze = GetZipEntry(file);
    if(ze != NULL)
    {
        std::stringstream buffer;

        osgDB::ReaderWriter* rw = ReadFromZipEntry(ze, options, buffer);
        if (rw != NULL)
        {
            // Setup appropriate options
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = options ?
            options->cloneOptions() :
            new osgDB::ReaderWriter::Options;

            local_opt->setPluginStringData("STREAM_FILENAME", osgDB::getSimpleFileName(ze->name));

            osgDB::ReaderWriter::ReadResult readResult = rw->readObject(buffer,local_opt.get());
            if (readResult.success())
            {
                return readResult;
            }
        }
    }

    return rresult;
}

osgDB::ReaderWriter::ReadResult ZipArchive::readNode(const std::string& file,const osgDB::ReaderWriter::Options* options) const
{
    osgDB::ReaderWriter::ReadResult rresult = osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!_zipLoaded || !acceptsExtension(ext)) return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    const ZIPENTRY* ze = GetZipEntry(file);
    if(ze != NULL)
    {
        std::stringstream buffer;

        osgDB::ReaderWriter* rw = ReadFromZipEntry(ze, options, buffer);
        if (rw != NULL)
        {
            // Setup appropriate options
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = options ?
                options->cloneOptions() :
                new osgDB::ReaderWriter::Options;

            local_opt->setPluginStringData("STREAM_FILENAME", osgDB::getSimpleFileName(ze->name));

            osgDB::ReaderWriter::ReadResult readResult = rw->readNode(buffer,local_opt.get());
            if (readResult.success())
            {
                return readResult;
            }
        }
    }

    return rresult;
}


osgDB::ReaderWriter::ReadResult ZipArchive::readShader(const std::string& file,const osgDB::ReaderWriter::Options* options) const
{
    osgDB::ReaderWriter::ReadResult rresult = osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!_zipLoaded || !acceptsExtension(ext)) return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    const ZIPENTRY* ze = GetZipEntry(file);
    if(ze != NULL)
    {
        std::stringstream buffer;

        osgDB::ReaderWriter* rw = ReadFromZipEntry(ze, options, buffer);
        if (rw != NULL)
        {
            // Setup appropriate options
            osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = options ?
                options->cloneOptions() :
                new osgDB::ReaderWriter::Options;

            local_opt->setPluginStringData("STREAM_FILENAME", osgDB::getSimpleFileName(ze->name));

            osgDB::ReaderWriter::ReadResult readResult = rw->readShader(buffer,local_opt.get());
            if (readResult.success())
            {
                return readResult;
            }
        }
    }

    return rresult;
}

osgDB::ReaderWriter::WriteResult ZipArchive::writeObject(const osg::Object& /*obj*/,const std::string& /*fileName*/,const osgDB::ReaderWriter::Options*) const
{
    return osgDB::ReaderWriter::WriteResult(osgDB::ReaderWriter::WriteResult::FILE_NOT_HANDLED);
}

osgDB::ReaderWriter::WriteResult ZipArchive::writeImage(const osg::Image& /*image*/,const std::string& /*fileName*/,const osgDB::ReaderWriter::Options*) const
{
    return osgDB::ReaderWriter::WriteResult(osgDB::ReaderWriter::WriteResult::FILE_NOT_HANDLED);
}

osgDB::ReaderWriter::WriteResult ZipArchive::writeHeightField(const osg::HeightField& /*heightField*/,const std::string& /*fileName*/,const osgDB::ReaderWriter::Options*) const
{
    return osgDB::ReaderWriter::WriteResult(osgDB::ReaderWriter::WriteResult::FILE_NOT_HANDLED);
}

osgDB::ReaderWriter::WriteResult ZipArchive::writeNode(const osg::Node& /*node*/,const std::string& /*fileName*/,const osgDB::ReaderWriter::Options*) const
{
    return osgDB::ReaderWriter::WriteResult(osgDB::ReaderWriter::WriteResult::FILE_NOT_HANDLED);
}

osgDB::ReaderWriter::WriteResult ZipArchive::writeShader(const osg::Shader& /*shader*/,const std::string& /*fileName*/,const osgDB::ReaderWriter::Options*) const
{
    return osgDB::ReaderWriter::WriteResult(osgDB::ReaderWriter::WriteResult::FILE_NOT_HANDLED);
}


osgDB::ReaderWriter* ZipArchive::ReadFromZipEntry(const ZIPENTRY* ze, const osgDB::ReaderWriter::Options* options, std::stringstream& buffer) const
{
    if (ze != 0)
    {
        char* ibuf = new (std::nothrow) char[ze->unc_size];
        if (ibuf)
        {
            // fetch the handle for the current thread:
            const PerThreadData& data = getData();
            if ( data._zipHandle != NULL )
            {
                ZRESULT result = UnzipItem(data._zipHandle, ze->index, ibuf, ze->unc_size);
                bool unzipSuccesful = CheckZipErrorCode(result);
                if(unzipSuccesful)
                {
                    buffer.write(ibuf,ze->unc_size);
                }

                delete[] ibuf;

                std::string file_ext = osgDB::getFileExtension(ze->name);

                osgDB::ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension(file_ext);
                if (rw != NULL)
                {
                    return rw;
                }
            }
        }
        else
        {
            //std::cout << "Error- failed to allocate enough memory to unzip file '" << ze->name << ", with size '" << ze->unc_size << std::endl;
        }
    }

    return NULL;
}

void CleanupFileString(std::string& strFileOrDir)
{
    if (strFileOrDir.empty())
    {
        return;
    }

    // convert all separators to unix-style for conformity
    for (unsigned int i = 0; i < strFileOrDir.length(); ++i)
    {
        if (strFileOrDir[i] == '\\')
        {
            strFileOrDir[i] = '/';
        }
    }

    // get rid of trailing separators
    if (strFileOrDir[strFileOrDir.length()-1] == '/')
    {
        strFileOrDir = strFileOrDir.substr(0, strFileOrDir.length()-1);
    }

    //add a beginning separator
    if(strFileOrDir[0] != '/')
    {
        strFileOrDir.insert(0, "/");
    }
}

void ZipArchive::IndexZipFiles(HZIP hz)
{
    if(hz != NULL && !_zipLoaded)
    {
        //mZipRecord = hz;

        GetZipItem(hz, -1, &_mainRecord);
        int numitems = _mainRecord.index;

        // Now loop through each file in zip
        for (int i = 0; i < numitems; i++)
        {
            ZIPENTRY* ze = new ZIPENTRY();

            GetZipItem(hz, i, ze);
            std::string name = ze->name;

            CleanupFileString(name);

            if(!name.empty())
            {
                _zipIndex.insert(ZipEntryMapping(name, ze));
            }
        }
    }
}

ZIPENTRY* ZipArchive::GetZipEntry(const std::string& filename)
{
    ZIPENTRY* ze = NULL;
    std::string fileToLoad = filename;
    CleanupFileString(fileToLoad);

    ZipEntryMap::iterator iter = _zipIndex.find(fileToLoad);
    if(iter != _zipIndex.end())
    {
        ze = (*iter).second;
    }

    return ze;
}

const ZIPENTRY* ZipArchive::GetZipEntry(const std::string& filename) const
{
    ZIPENTRY* ze = NULL;
    std::string fileToLoad = filename;
    CleanupFileString(fileToLoad);

    ZipEntryMap::const_iterator iter = _zipIndex.find(fileToLoad);
    if(iter != _zipIndex.end())
    {
        ze = (*iter).second;
    }

    return ze;
}

osgDB::FileType ZipArchive::getFileType(const std::string& filename) const
{
    const ZIPENTRY* ze = GetZipEntry(filename);
    if(ze != NULL)
    {
    #ifdef ZIP_STD
        if (ze->attr & S_IFDIR)
    #else
        if (ze->attr & FILE_ATTRIBUTE_DIRECTORY)
    #endif
        {
            return osgDB::DIRECTORY;
        }
        else
        {
            return osgDB::REGULAR_FILE;
        }
    }

    return osgDB::FILE_NOT_FOUND;
}

osgDB::DirectoryContents ZipArchive::getDirectoryContents(const std::string& dirName) const
{
    osgDB::DirectoryContents dirContents;

    ZipEntryMap::const_iterator iter = _zipIndex.begin();
    ZipEntryMap::const_iterator iterEnd = _zipIndex.end();

    for(; iter != iterEnd; ++iter)
    {
        std::string searchPath = dirName;
        CleanupFileString(searchPath);

        if(iter->first.size() > searchPath.size())
        {
            size_t endSubElement = iter->first.find(searchPath);

            //we match the whole string in the beginning of the path
            if(endSubElement == 0)
            {
                std::string remainingFile = iter->first.substr(searchPath.size() + 1, std::string::npos);
                size_t endFileToken = remainingFile.find_first_of('/');

                if(endFileToken == std::string::npos)
                {
                    dirContents.push_back(remainingFile);
                }
            }
        }
    }

    return dirContents;
}

std::string ZipArchive::ReadPassword(const osgDB::ReaderWriter::Options* options) const
{
    //try pulling it off the options first
    std::string password = "";
    if(options != NULL)
    {
        const osgDB::AuthenticationMap* credentials = options->getAuthenticationMap();
        if(credentials != NULL)
        {
            const osgDB::AuthenticationDetails* details = credentials->getAuthenticationDetails("ZipPlugin");
            if(details != NULL)
            {
                password = details->password;
            }
        }
    }

    //if no password, try the registry
    if(password.empty())
    {
        osgDB::Registry* reg = osgDB::Registry::instance();
        if(reg != NULL)
        {
            const osgDB::AuthenticationMap* credentials = reg->getAuthenticationMap();
            if(credentials != NULL)
            {
                const osgDB::AuthenticationDetails* details = credentials->getAuthenticationDetails("ZipPlugin");
                if(details != NULL)
                {
                    password = details->password;
                }
            }
        }
    }

    return password;
}

bool ZipArchive::CheckZipErrorCode(ZRESULT result) const
{
    if(result == ZR_OK)
    {
        return true;
    }
    else
    {
        unsigned buf_size  = 1025;
        char* buf = new (std::nothrow) char[buf_size];
        buf[buf_size - 1] = 0;

        if (buf)
        {
            FormatZipMessage(result, buf, buf_size - 1);

            //print error message
            //sprintf(buf, "%s");
            OSG_WARN << "Error loading zip file: " << getArchiveFileName() << ", Zip loader returned error: " << buf << "\n";
            delete [] buf;
        }

        return false;
    }
}

const ZipArchive::PerThreadData&
ZipArchive::getData() const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> exclusive( const_cast<ZipArchive*>(this)->_zipMutex );
    return getDataNoLock();
}


const ZipArchive::PerThreadData&
ZipArchive::getDataNoLock() const
{
    // get/create data for the currently running thread:
    OpenThreads::Thread* current = OpenThreads::Thread::CurrentThread();

    PerThreadDataMap::const_iterator i = _perThreadData.find( current );

    if ( i == _perThreadData.end() || i->second._zipHandle == NULL )
    {
        // cache pattern: cast to const for caching purposes
        ZipArchive* ncThis = const_cast<ZipArchive*>(this);

        // data does not already exist, so open the ZIP with a handle exclusively for this thread:
        PerThreadData& data = ncThis->_perThreadData[current];
        if ( !_filename.empty() )
        {
            data._zipHandle = OpenZip( _filename.c_str(), _password.c_str() );
        }
        else if ( !_membuffer.empty() )
        {
            data._zipHandle = OpenZip( (void*)_membuffer.c_str(), _membuffer.length(), _password.c_str() );
        }
        else
        {
            data._zipHandle = NULL;
        }
        return data;
    }
    else
    {
        return i->second;
    }
}
