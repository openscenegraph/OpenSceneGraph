/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/Archive>

using namespace osgDB;

Archive::Archive()
{
}

Archive::~Archive()
{
    close();
}

void Archive::create(const std::string& filename, unsigned int indexBlockSize)
{
    _status = WRITE;
    _output.open(filename.c_str());
}

void Archive::open(const std::string& filename, Status status)
{
    if (status==READ)
    {
        _status = status;
        _input.open(filename.c_str());
    }
    else // status==WRITE
    {
        _status = status;
        _output.open(filename.c_str());
    }
}

void Archive::close()
{
    _input.close();
    _output.close();
}


bool Archive::fileExists(const std::string& filename) const
{
    return (_indexMap.count(filename)!=0);
}

ReaderWriter::ReadResult Archive::readObject(const std::string& fileName,const Options* options)
{
    if (_status!=READ) 
    {
        osg::notify(osg::NOTICE)<<"Archive::readObject(obj, "<<fileName<<") failed, archive opened as read only."<<std::endl;
        return ReadResult(ReadResult::FILE_NOT_HANDLED);
    }
    
    FileNamePositionMap::const_iterator itr = _indexMap.find(fileName);
    if (itr==_indexMap.end())
    {
        osg::notify(osg::NOTICE)<<"Archive::readObject(obj, "<<fileName<<") failed, file not found in archive"<<std::endl;
        return ReadResult(ReadResult::FILE_NOT_FOUND);
    }

    ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension(getLowerCaseFileExtension(fileName));
    if (!rw)
    {
        osg::notify(osg::NOTICE)<<"Archive::readObject(obj, "<<fileName<<") failed to find appropriate plugin to write file."<<std::endl;
        return ReadResult(ReadResult::FILE_NOT_HANDLED);
    }
    
    osg::notify(osg::NOTICE)<<"Archive::readObject(obj, "<<fileName<<")"<<std::endl;
    return rw->readObject(_input, options);
}

ReaderWriter::ReadResult Archive::readImage(const std::string& /*fileName*/,const Options*) { return ReadResult(ReadResult::FILE_NOT_HANDLED); }
ReaderWriter::ReadResult Archive::readHeightField(const std::string& /*fileName*/,const Options*) { return ReadResult(ReadResult::FILE_NOT_HANDLED); }
ReaderWriter::ReadResult Archive::readNode(const std::string& /*fileName*/,const Options*) { return ReadResult(ReadResult::FILE_NOT_HANDLED); }




ReaderWriter::WriteResult Archive::writeObject(const osg::Object& obj,const std::string& fileName,const Options* options)
{
    if (_status!=WRITE) 
    {
        osg::notify(osg::NOTICE)<<"Archive::writeObject(obj, "<<fileName<<") failed, archive opened as read only."<<std::endl;
        return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }

    ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension(getLowerCaseFileExtension(fileName));
    if (!rw)
    {
        osg::notify(osg::NOTICE)<<"Archive::writeObject(obj, "<<fileName<<") failed to find appropriate plugin to write file."<<std::endl;
        return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }
    
    osg::notify(osg::NOTICE)<<"Archive::writeObject(obj, "<<fileName<<")"<<std::endl;
    return rw->writeObject(obj, _output, options);
}

ReaderWriter::WriteResult Archive::writeImage(const osg::Image& /*image*/,const std::string& fileName,const Options* options)
{
    if (_status==READ) 
    {
        osg::notify(osg::NOTICE)<<"Archive::writeImage(obj, "<<fileName<<") failed, archive opened as read only."<<std::endl;
        return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }
    osg::notify(osg::NOTICE)<<"Archive::writeImage(obj, "<<fileName<<")"<<std::endl;
    return WriteResult(WriteResult::FILE_NOT_HANDLED);
}

ReaderWriter::WriteResult Archive::writeHeightField(const osg::HeightField& /*heightField*/,const std::string& fileName,const Options* options)
{
    if (_status!=WRITE) 
    {
        osg::notify(osg::NOTICE)<<"Archive::writeHeightField(obj, "<<fileName<<") failed, archive opened as read only."<<std::endl;
        return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }
    osg::notify(osg::NOTICE)<<"Archive::writeHeightField(obj, "<<fileName<<")"<<std::endl;
    return WriteResult(WriteResult::FILE_NOT_HANDLED);
}

ReaderWriter::WriteResult Archive::writeNode(const osg::Node& /*node*/,const std::string& fileName,const Options* options)
{
    if (_status!=WRITE) 
    {
        osg::notify(osg::NOTICE)<<"Archive::writeNode(obj, "<<fileName<<") failed, archive opened as read only."<<std::endl;
        return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }
    osg::notify(osg::NOTICE)<<"Archive::writeNode(obj, "<<fileName<<")"<<std::endl;
    return WriteResult(WriteResult::FILE_NOT_HANDLED);
}
