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

float Archive::s_currentSupportedVersion = 0.0;

Archive::IndexBlock::IndexBlock(unsigned int blockSize):
    _requiresWrite(false),
    _filePosition(0),
    _blockSize(0),
    _offsetOfNextAvailableSpace(0),
    _data(0)
{
    allocateData(blockSize);
}

Archive::IndexBlock::~IndexBlock()
{
    delete [] _data;
}

void Archive::IndexBlock::allocateData(unsigned int blockSize)
{
    _data = (blockSize!=0) ? new unsigned char[blockSize]  : 0;
    if (_data)
    {
        _blockSize = blockSize;
        
        // initialize the array
        unsigned char* end = _data + _blockSize;
        for(unsigned char* ptr=_data; ptr < end; ++ptr) *ptr = 0;
    }
    else
    {
        _blockSize = 0;
    }
}

Archive::IndexBlock* Archive::IndexBlock::read(std::istream& in)
{
    if (!in) return 0;

    osg::ref_ptr<IndexBlock> indexBlock = new IndexBlock;
    indexBlock->_filePosition = in.tellg();
    in.read(reinterpret_cast<char*>(&indexBlock->_blockSize), sizeof(indexBlock->_blockSize));
    in.read(reinterpret_cast<char*>(&indexBlock->_filePositionNextIndexBlock), sizeof(indexBlock->_filePositionNextIndexBlock));
    in.read(reinterpret_cast<char*>(&indexBlock->_offsetOfNextAvailableSpace), sizeof(indexBlock-> _offsetOfNextAvailableSpace));

    indexBlock->allocateData(indexBlock->_blockSize);
    if (indexBlock->_data)
    {
        in.read(reinterpret_cast<char*>(indexBlock->_data),indexBlock->_blockSize);
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Allocation Problem in Archive::IndexBlock::read(std::istream& in)"<<std::endl;
        return 0;
    }

    osg::notify(osg::NOTICE)<<"Read index block"<<std::endl;
    
    return indexBlock.release();
    
}

bool Archive::IndexBlock::getFileReferences(FileNamePositionMap& indexMap)
{
    if (!_data || _offsetOfNextAvailableSpace==0) return false;
    
    bool valuesAdded = false;
    
    unsigned char* ptr = _data;
    unsigned char* end_ptr = _data + _offsetOfNextAvailableSpace;
    while (ptr<end_ptr)
    {
        pos_type position = *(reinterpret_cast<pos_type*>(ptr)); 
        ptr += sizeof(pos_type);
        
        unsigned int filename_size = *(reinterpret_cast<unsigned int*>(ptr));
        ptr += sizeof(unsigned int);
        
        std::string filename(ptr, ptr+filename_size);
        
        // record this entry into the FileNamePositionMap
        indexMap[filename] = position;
        
        ptr += filename_size;
        
        valuesAdded = true;
    }
    return valuesAdded;
}

void Archive::IndexBlock::write(std::ostream& out)
{
    if (_filePosition==pos_type(0))
    {
        _filePosition = out.tellp();
    }
    else
    {
         out.seekp(_filePosition);
    }

    out.write(reinterpret_cast<char*>(&_blockSize), sizeof(_blockSize));
    out.write(reinterpret_cast<char*>(&_filePositionNextIndexBlock), sizeof(_filePositionNextIndexBlock));
    out.write(reinterpret_cast<char*>(&_offsetOfNextAvailableSpace), sizeof(_offsetOfNextAvailableSpace));

    out.write(reinterpret_cast<char*>(_data),_blockSize);
    
    osg::notify(osg::NOTICE)<<"Archive::IndexBlock::write()"<<std::endl;
}


bool Archive::IndexBlock::addFileReference(pos_type position, const std::string& filename)
{
    if (spaceAvailable(position, filename))
    {
        unsigned char* ptr = _data+_offsetOfNextAvailableSpace;
        
        *(reinterpret_cast<pos_type*>(ptr)) = position; 
        ptr += sizeof(pos_type);
        
        *(reinterpret_cast<unsigned int*>(ptr)) = filename.size();
        ptr += sizeof(unsigned int);
        
        for(unsigned int i=0;i<filename.size();++i, ++ptr)
        {
            *ptr = filename[i];
        }
        
        _offsetOfNextAvailableSpace = ptr-_data;
        
        _requiresWrite = true;

        osg::notify(osg::NOTICE)<<"Archive::IndexBlock::addFileReference("<<(unsigned int)position<<", "<<filename<<")"<<std::endl;
        
        return true;
    }
    else
    {
        return false;
    }
}

void Archive::IndexBlock::setPositionNextIndexBlock(pos_type position)
{
    _filePositionNextIndexBlock = position;
    _requiresWrite = true;
}

Archive::Archive()
{
    osg::notify(osg::NOTICE)<<"Don't forget endian...."<<std::endl;
}

Archive::~Archive()
{
    close();
}

bool Archive::create(const std::string& filename, unsigned int indexBlockSize)
{
    _status = WRITE;
    _output.open(filename.c_str());
    _output<<"osga";
    _output.write(reinterpret_cast<char*>(&s_currentSupportedVersion),sizeof(float));
    
    IndexBlock *indexBlock = new IndexBlock(indexBlockSize);
    if (indexBlock)
    {
        indexBlock->write(_output);
        _indexBlockList.push_back(indexBlock);
    }
    return true;
}

bool Archive::open(const std::string& filename, Status status)
{
    if (status==READ)
    {
        _status = status;
        _input.open(filename.c_str());

        if (_input)
        {
            osg::notify(osg::NOTICE)<<"trying Archive::open("<<filename<<")"<<std::endl;

            char identifier[4];
            _input.read(identifier,4);
            bool validArchive = (identifier[0]=='o' && identifier[1]=='s' && identifier[2]=='g' && identifier[3]=='a');
            
            if (validArchive) 
            {
                _input.read(reinterpret_cast<char*>(&_version),sizeof(_version));
                
                IndexBlock *indexBlock = 0;
                
                while ( (indexBlock=Archive::IndexBlock::read(_input)) != 0)
                {
                    _indexBlockList.push_back(indexBlock);
                    if (indexBlock->getPositionNextIndexBlock()==pos_type(0)) break;
                    
                    _input.seekg(indexBlock->getPositionNextIndexBlock());
                }
                
                osg::notify(osg::NOTICE)<<"Archive::open("<<filename<<") succeeded"<<std::endl;
                
                // now need to build the filename map.
                _indexMap.clear();                
                for(IndexBlockList::iterator itr=_indexBlockList.begin();
                    itr!=_indexBlockList.end();
                    ++itr)
                {
                    (*itr)->getFileReferences(_indexMap);
                }
                
                for(FileNamePositionMap::iterator mitr=_indexMap.begin();
                    mitr!=_indexMap.end();
                    ++mitr)
                {
                    osg::notify(osg::NOTICE)<<"    filename "<<(mitr->first)<<" pos="<<(int)(mitr->second)<<std::endl;
                }


                return true;
            }
        }

        osg::notify(osg::NOTICE)<<"Archive::open("<<filename<<") failed"<<std::endl;

        _input.close();
        return false;
    }
    else // status==WRITE
    {
        if (open(filename,READ))
        {
            _input.close();
            
            _status = WRITE;
            _output.open(filename.c_str());
            
            return true;
        }
        else // no file opened so resort to creating the archive.
        {
            return create(filename, 4096);
        }
        
    }
}

void Archive::close()
{
    _input.close();
    
    if (_status==WRITE)
    {
        writeIndexBlocks();
        _output.close();
    }
}

void Archive::writeIndexBlocks()
{
    if (_status==WRITE)
    {
        for(IndexBlockList::iterator itr=_indexBlockList.begin();
            itr!=_indexBlockList.end();
            ++itr)
        {
            if ((*itr)->requiresWrite())
            {
                (*itr)->write(_output);
            }
        }
    }
}

bool Archive::fileExists(const std::string& filename) const
{
    return (_indexMap.count(filename)!=0);
}

bool Archive::addFileReference(pos_type position, const std::string& fileName)
{
    if (_status==READ)
    {
        osg::notify(osg::NOTICE)<<"Archive::getPositionForNewEntry("<<fileName<<") failed, archive opened as read only."<<std::endl;
        return false;
    }
    
    if (!_output)
    {
        osg::notify(osg::NOTICE)<<"Archive::getPositionForNewEntry("<<fileName<<") failed, _output set up."<<std::endl;
        return false;
    }
    
    // get an IndexBlock with space available if possible
    unsigned int blockSize = 4096;
    osg::ref_ptr<IndexBlock> indexBlock = _indexBlockList.empty() ? 0 : _indexBlockList.back();
    osg::ref_ptr<IndexBlock> previousBlock = indexBlock;
    if (indexBlock.valid())
    {
        blockSize = indexBlock->getBlockSize();
        if (!(indexBlock->spaceAvailable(position, fileName)))
        {
            previousBlock = indexBlock;
            indexBlock = 0;
        }
    }

    // if not one available create a new block.    
    if (!indexBlock)
    {
        if (previousBlock.valid()) previousBlock->setPositionNextIndexBlock(_output.tellp());
    
        indexBlock = new IndexBlock(blockSize);
        indexBlock->write(_output);
        _indexBlockList.push_back(indexBlock.get());
    }
    
    if (indexBlock.valid())
    {
        return indexBlock->addFileReference(position, fileName);
    }
    return false;
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
    
    _input.seekg(itr->second);

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
    
    pos_type position = _output.tellp();
    
    WriteResult result = rw->writeObject(obj, _output, options);
    
    if (result.success()) addFileReference(position, fileName);
    
    return result;
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
