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
}

void Archive::open(const std::string& filename, Status status)
{
    if (status==READ)
    {
        _input.open(filename.c_str());
    }
    else // status==WRITE
    {
        _input.open(filename.c_str());
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

ReaderWriter::ReadResult Archive::readObject(const std::string& /*fileName*/,const Options*) { return ReadResult(ReadResult::FILE_NOT_HANDLED); }
ReaderWriter::ReadResult Archive::readImage(const std::string& /*fileName*/,const Options*) { return ReadResult(ReadResult::FILE_NOT_HANDLED); }
ReaderWriter::ReadResult Archive::readHeightField(const std::string& /*fileName*/,const Options*) { return ReadResult(ReadResult::FILE_NOT_HANDLED); }
ReaderWriter::ReadResult Archive::readNode(const std::string& /*fileName*/,const Options*) { return ReadResult(ReadResult::FILE_NOT_HANDLED); }

ReaderWriter::WriteResult Archive::writeObject(const osg::Object& /*obj*/,const std::string& /*fileName*/,const Options*) {return WriteResult(WriteResult::FILE_NOT_HANDLED); }
ReaderWriter::WriteResult Archive::writeImage(const osg::Image& /*image*/,const std::string& /*fileName*/,const Options*) {return WriteResult(WriteResult::FILE_NOT_HANDLED); }
ReaderWriter::WriteResult Archive::writeHeightField(const osg::HeightField& /*heightField*/,const std::string& /*fileName*/,const Options*) {return WriteResult(WriteResult::FILE_NOT_HANDLED); }
ReaderWriter::WriteResult Archive::writeNode(const osg::Node& /*node*/,const std::string& /*fileName*/,const Options*) { return WriteResult(WriteResult::FILE_NOT_HANDLED); }

ReaderWriter::ReadResult Archive::readObject(std::istream& /*fin*/,const Options*) { return ReadResult(ReadResult::FILE_NOT_HANDLED); }
ReaderWriter::ReadResult Archive::readImage(std::istream& /*fin*/,const Options*) { return ReadResult(ReadResult::FILE_NOT_HANDLED); }
ReaderWriter::ReadResult Archive::readHeightField(std::istream& /*fin*/,const Options*) { return ReadResult(ReadResult::FILE_NOT_HANDLED); }
ReaderWriter::ReadResult Archive::readNode(std::istream& /*fin*/,const Options*) { return ReadResult(ReadResult::FILE_NOT_HANDLED); }

ReaderWriter::WriteResult Archive::writeObject(const osg::Object& /*obj*/,std::ostream& /*fout*/,const Options*) {return WriteResult(WriteResult::FILE_NOT_HANDLED); }
ReaderWriter::WriteResult Archive::writeImage(const osg::Image& /*image*/,std::ostream& /*fout*/,const Options*) {return WriteResult(WriteResult::FILE_NOT_HANDLED); }
ReaderWriter::WriteResult Archive::writeHeightField(const osg::HeightField& /*heightField*/,std::ostream& /*fout*/,const Options*) {return WriteResult(WriteResult::FILE_NOT_HANDLED); }
ReaderWriter::WriteResult Archive::writeNode(const osg::Node& /*node*/,std::ostream& /*fout*/,const Options*) { return WriteResult(WriteResult::FILE_NOT_HANDLED); }
