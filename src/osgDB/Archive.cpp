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

#include <streambuf>

using namespace osgDB;

float Archive::s_currentSupportedVersion = 0.0;

osgDB::Archive* osgDB::openArchive(const std::string& filename, Archive::ArchiveStatus status, unsigned int indexBlockSizeHint)
{
    return openArchive(filename, status, indexBlockSizeHint, Registry::instance()->getUseObjectCacheHint());
}

osgDB::Archive* osgDB::openArchive(const std::string& filename, Archive::ArchiveStatus status, unsigned int indexBlockSizeHint,Registry::CacheHintOptions useObjectCache)
{
    ReaderWriter::ReadResult result = osgDB::Registry::instance()->openArchive(filename, status, indexBlockSizeHint, useObjectCache);
    return result.takeArchive();
}

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
    _data = (blockSize!=0) ? new char[blockSize]  : 0;
    if (_data)
    {
        _blockSize = blockSize;
        
        // initialize the array
        char* end = _data + _blockSize;
        for(char* ptr=_data; ptr < end; ++ptr) *ptr = 0;
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
        osg::notify(osg::INFO)<<"Allocation Problem in Archive::IndexBlock::read(std::istream& in)"<<std::endl;
        return 0;
    }

    osg::notify(osg::INFO)<<"Read index block"<<std::endl;
    
    return indexBlock.release();
    
}

std::string Archive::IndexBlock::getFirstFileName() const
{
    char* ptr = _data;
    char* end_ptr = _data + _offsetOfNextAvailableSpace;
    if (ptr<end_ptr)
    {
        ptr += sizeof(pos_type);
        ptr += sizeof(size_type);

        unsigned int filename_size = *(reinterpret_cast<unsigned int*>(ptr));
        ptr += sizeof(unsigned int);

        return std::string(ptr, ptr+filename_size);
    }
    else
    {
        return std::string();
    }
}

bool Archive::IndexBlock::getFileReferences(FileNamePositionMap& indexMap) const
{
    if (!_data || _offsetOfNextAvailableSpace==0) return false;
    
    bool valuesAdded = false;
    
    char* ptr = _data;
    char* end_ptr = _data + _offsetOfNextAvailableSpace;
    while (ptr<end_ptr)
    {
        pos_type position = *(reinterpret_cast<pos_type*>(ptr)); 
        ptr += sizeof(pos_type);
        
        size_type size = *(reinterpret_cast<size_type*>(ptr)); 
        ptr += sizeof(size_type);

        unsigned int filename_size = *(reinterpret_cast<unsigned int*>(ptr));
        ptr += sizeof(unsigned int);
        
        std::string filename(ptr, ptr+filename_size);
        
        // record this entry into the FileNamePositionMap
        indexMap[filename] = PositionSizePair(position,size);
        
        ptr += filename_size;
        
        valuesAdded = true;
    }
    return valuesAdded;
}

void Archive::IndexBlock::write(std::ostream& out)
{
    if (_filePosition==pos_type(0))
    {
        osg::notify(osg::INFO)<<"Archive::IndexBlock::write() setting _filePosition"<<std::endl;
        _filePosition = out.tellp();
    }
    else
    {
         out.seekp(_filePosition);
    }
    osg::notify(osg::INFO)<<"Archive::IndexBlock::write() to _filePosition"<<out.tellp()<<std::endl;

    out.write(reinterpret_cast<char*>(&_blockSize), sizeof(_blockSize));
    out.write(reinterpret_cast<char*>(&_filePositionNextIndexBlock), sizeof(_filePositionNextIndexBlock));
    out.write(reinterpret_cast<char*>(&_offsetOfNextAvailableSpace), sizeof(_offsetOfNextAvailableSpace));

    out.write(reinterpret_cast<char*>(_data),_blockSize);
    
    osg::notify(osg::INFO)<<"Archive::IndexBlock::write()"<<std::endl;
}


bool Archive::IndexBlock::addFileReference(pos_type position, size_type size, const std::string& filename)
{
    if (spaceAvailable(position, size, filename))
    {
        char* ptr = _data+_offsetOfNextAvailableSpace;
        
        *(reinterpret_cast<pos_type*>(ptr)) = position; 
        ptr += sizeof(pos_type);
        
        *(reinterpret_cast<size_type*>(ptr)) = size; 
        ptr += sizeof(size_type);
        
        *(reinterpret_cast<unsigned int*>(ptr)) = filename.size();
        ptr += sizeof(unsigned int);
        
        for(unsigned int i=0;i<filename.size();++i, ++ptr)
        {
            *ptr = filename[i];
        }
        
        _offsetOfNextAvailableSpace = ptr-_data;
        
        _requiresWrite = true;

        osg::notify(osg::INFO)<<"Archive::IndexBlock::addFileReference("<<(unsigned int)position<<", "<<filename<<")"<<std::endl;
        
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

bool Archive::open(const std::string& filename, ArchiveStatus status, unsigned int indexBlockSize)
{
    if (status==READ)
    {
        _status = status;
        _input.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);

        if (_input)
        {
            osg::notify(osg::INFO)<<"trying Archive::open("<<filename<<")"<<std::endl;

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
                
                osg::notify(osg::INFO)<<"Archive::open("<<filename<<") succeeded"<<std::endl;
                
                // now need to build the filename map.
                _indexMap.clear();                

                if (!_indexBlockList.empty())
                {
                    _masterFileName = _indexBlockList.front()->getFirstFileName();
                }

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
                    osg::notify(osg::INFO)<<"    filename "<<(mitr->first)<<" pos="<<(int)((mitr->second).first)<<" size="<<(int)((mitr->second).second)<<std::endl;
                }


                return true;
            }
        }

        osg::notify(osg::INFO)<<"Archive::open("<<filename<<") failed"<<std::endl;

        _input.close();
        return false;
    }
    else if (status==WRITE)
    {
        if (status==WRITE && open(filename,READ))
        {
            _input.close();
            _status = WRITE;

            _output.open(filename.c_str(), std::ios_base::binary | std::ios_base::in | std::ios_base::out);
  
            
            
            osg::notify(osg::INFO)<<"File position after open = "<<(int)_output.tellp()<<" is_open "<<_output.is_open()<<std::endl;

            // place write position at end of file.
            _output.seekp(0, std::ios::end);
            

            osg::notify(osg::INFO)<<"File position after seekp = "<<(int)_output.tellp()<<std::endl;

            osg::notify(osg::INFO)<<"Archive::open("<<filename<<") open for writing"<<std::endl;

            return true;
        }
        else // no file opened or using create so resort to creating the archive.
        {
            osg::notify(osg::INFO)<<"Archive::open("<<filename<<"), archive being created."<<std::endl;

            _status = WRITE;
            _output.open(filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            _output<<"osga";
            _output.write(reinterpret_cast<char*>(&s_currentSupportedVersion),sizeof(float));

            IndexBlock *indexBlock = new IndexBlock(indexBlockSize);
            if (indexBlock)
            {
                indexBlock->write(_output);
                _indexBlockList.push_back(indexBlock);
            }

            osg::notify(osg::INFO)<<"File position after write = "<<(int)_output.tellp()<<std::endl;

            // place write position at end of file.
            _output.seekp(0,std::ios::end);

            osg::notify(osg::INFO)<<"File position after seekp = "<<(int)_output.tellp()<<std::endl;

            return true;
        }
        
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Archive::open("<<filename<<") is a strange place!!."<<std::endl;
        return false;
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

std::string Archive::getMasterFileName() const
{
    return _masterFileName;
}

bool Archive::getFileNames(FileNameList& fileNameList) const
{
    fileNameList.clear();
    fileNameList.reserve(_indexMap.size());
    for(FileNamePositionMap::const_iterator itr=_indexMap.begin();
        itr!=_indexMap.end();
        ++itr)
    {
        fileNameList.push_back(itr->first);
    }
    return !fileNameList.empty();
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

bool Archive::addFileReference(pos_type position, size_type size, const std::string& fileName)
{
    if (_status==READ)
    {
        osg::notify(osg::INFO)<<"Archive::getPositionForNewEntry("<<fileName<<") failed, archive opened as read only."<<std::endl;
        return false;
    }
    
    if (!_output)
    {
        osg::notify(osg::INFO)<<"Archive::getPositionForNewEntry("<<fileName<<") failed, _output set up."<<std::endl;
        return false;
    }
    
    
    // if the masterFileName isn't set yet use this fileName
    if (_masterFileName.empty()) _masterFileName = fileName;


    // get an IndexBlock with space available if possible
    unsigned int blockSize = 4096;
    osg::ref_ptr<IndexBlock> indexBlock = _indexBlockList.empty() ? 0 : _indexBlockList.back();
    osg::ref_ptr<IndexBlock> previousBlock = indexBlock;
    if (indexBlock.valid())
    {
        blockSize = indexBlock->getBlockSize();
        if (!(indexBlock->spaceAvailable(position, size, fileName)))
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
        return indexBlock->addFileReference(position, size, fileName);
    }
    return false;
}



class proxy_streambuf : public std::streambuf
{
   public:
   
      proxy_streambuf(std::streambuf* streambuf, unsigned int numChars):
        _streambuf(streambuf),
        _numChars(numChars),
        value_peeked(false),
        peek_value(0) {}
   
      /// Destructor deallocates no buffer space.
      virtual ~proxy_streambuf()  {}

      std::streambuf* _streambuf;
      unsigned int _numChars;
      
      bool value_peeked;
      int_type peek_value;

    protected:

      virtual int_type uflow ()
      {
         if (_numChars==0) return -1;
         
         --_numChars;

         int_type val = value_peeked ? peek_value : _streambuf->sbumpc();
         value_peeked = false;
                  
         return val;
      }

      virtual int_type 
      underflow()
      {
        if (value_peeked) return peek_value;

        value_peeked = true;
        peek_value = _streambuf->sbumpc();
        return peek_value;
      }
};

struct Archive::ReadObjectFunctor : public Archive::ReadFunctor
{
    ReadObjectFunctor(const std::string& filename, const ReaderWriter::Options* options):ReadFunctor(filename,options) {}
    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw, std::istream& input) const { return rw.readObject(input, _options); }    
};

struct Archive::ReadImageFunctor : public Archive::ReadFunctor
{
   ReadImageFunctor(const std::string& filename, const ReaderWriter::Options* options):ReadFunctor(filename,options) {}
    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw, std::istream& input)const  { return rw.readImage(input, _options); }    
};

struct Archive::ReadHeightFieldFunctor : public Archive::ReadFunctor
{
    ReadHeightFieldFunctor(const std::string& filename, const ReaderWriter::Options* options):ReadFunctor(filename,options) {}
    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw, std::istream& input) const { return rw.readHeightField(input, _options); }    
};

struct Archive::ReadNodeFunctor : public Archive::ReadFunctor
{
    ReadNodeFunctor(const std::string& filename, const ReaderWriter::Options* options):ReadFunctor(filename,options) {}
    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw, std::istream& input) const { return rw.readNode(input, _options); }    
};

ReaderWriter::ReadResult Archive::read(const ReadFunctor& readFunctor)
{
    if (_status!=READ) 
    {
        osg::notify(osg::INFO)<<"Archive::readObject(obj, "<<readFunctor._filename<<") failed, archive opened as read only."<<std::endl;
        return ReadResult(ReadResult::FILE_NOT_HANDLED);
    }
    
    FileNamePositionMap::const_iterator itr = _indexMap.find(readFunctor._filename);
    if (itr==_indexMap.end())
    {
        osg::notify(osg::INFO)<<"Archive::readObject(obj, "<<readFunctor._filename<<") failed, file not found in archive"<<std::endl;
        return ReadResult(ReadResult::FILE_NOT_FOUND);
    }
    
    ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension(getLowerCaseFileExtension(readFunctor._filename));
    if (!rw)
    {
        osg::notify(osg::INFO)<<"Archive::readObject(obj, "<<readFunctor._filename<<") failed to find appropriate plugin to write file."<<std::endl;
        return ReadResult(ReadResult::FILE_NOT_HANDLED);
    }
    
    osg::notify(osg::INFO)<<"Archive::readObject(obj, "<<readFunctor._filename<<")"<<std::endl;
    
    _input.seekg(itr->second.first);

    // set up proxy stream buffer to proide the faked ending.
    std::istream& ins = _input;
    proxy_streambuf mystreambuf(ins.rdbuf(),itr->second.second);
    ins.rdbuf(&mystreambuf);

    ReaderWriter::ReadResult result = readFunctor.doRead(*rw, _input);

    ins.rdbuf(mystreambuf._streambuf);
    
    return result;
}

ReaderWriter::ReadResult Archive::readObject(const std::string& fileName,const Options* options)
{
    return read(ReadObjectFunctor(fileName, options));
}

ReaderWriter::ReadResult Archive::readImage(const std::string& fileName,const Options* options)
{
    return read(ReadImageFunctor(fileName, options));
}

ReaderWriter::ReadResult Archive::readHeightField(const std::string& fileName,const Options* options)
{
    return read(ReadHeightFieldFunctor(fileName, options));
}

ReaderWriter::ReadResult Archive::readNode(const std::string& fileName,const Options* options)
{
    return read(ReadNodeFunctor(fileName, options));
}


struct Archive::WriteObjectFunctor : public Archive::WriteFunctor
{
    WriteObjectFunctor(const osg::Object& object, const std::string& filename, const ReaderWriter::Options* options):
        WriteFunctor(filename,options),
        _object(object) {}
    const osg::Object& _object;
    
    virtual ReaderWriter::WriteResult doWrite(ReaderWriter& rw, std::ostream& output) const { return rw.writeObject(_object, output, _options); } 
};

struct Archive::WriteImageFunctor : public Archive::WriteFunctor
{
    WriteImageFunctor(const osg::Image& object, const std::string& filename, const ReaderWriter::Options* options):
        WriteFunctor(filename,options),
        _object(object) {}
    const osg::Image& _object;

    virtual ReaderWriter::WriteResult doWrite(ReaderWriter& rw, std::ostream& output)const  { return rw.writeImage(_object, output, _options); }    
};

struct Archive::WriteHeightFieldFunctor : public Archive::WriteFunctor
{
    WriteHeightFieldFunctor(const osg::HeightField& object, const std::string& filename, const ReaderWriter::Options* options):
        WriteFunctor(filename,options),
        _object(object) {}
    const osg::HeightField& _object;

    virtual ReaderWriter::WriteResult doWrite(ReaderWriter& rw, std::ostream& output) const { return rw.writeHeightField(_object, output, _options); }    
};

struct Archive::WriteNodeFunctor : public Archive::WriteFunctor
{
    WriteNodeFunctor(const osg::Node& object, const std::string& filename, const ReaderWriter::Options* options):
        WriteFunctor(filename,options),
        _object(object) {}
    const osg::Node& _object;

    virtual ReaderWriter::WriteResult doWrite(ReaderWriter& rw, std::ostream& output) const { return rw.writeNode(_object, output, _options); }    
};

ReaderWriter::WriteResult Archive::write(const WriteFunctor& writeFunctor)
{
    if (_status!=WRITE) 
    {
        osg::notify(osg::NOTICE)<<"Archive::writeObject(obj, "<<writeFunctor._filename<<") failed, archive opened as read only."<<std::endl;
        return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }

    ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension(getLowerCaseFileExtension(writeFunctor._filename));
    if (!rw)
    {
        osg::notify(osg::NOTICE)<<"Archive::writeObject(obj, "<<writeFunctor._filename<<") failed to find appropriate plugin to write file."<<std::endl;
        return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }
    
    osg::notify(osg::NOTICE)<<"Archive::writeObject(obj, "<<writeFunctor._filename<<")"<<std::endl;
    
    // place write position at end of file.
    _output.seekp(0,std::ios::end);
    
    pos_type position = _output.tellp();
    
    WriteResult result = writeFunctor.doWrite(*rw,_output);
    
    pos_type final_position = _output.tellp();
    size_type size = size_type(final_position-position);

    if (result.success()) addFileReference(position, size, writeFunctor._filename);
    
    return result;
}


ReaderWriter::WriteResult Archive::writeObject(const osg::Object& obj,const std::string& fileName,const Options* options)
{
    osg::notify(osg::NOTICE)<<"Archive::writeObject(obj, "<<fileName<<")"<<std::endl;
    return write(WriteObjectFunctor(obj, fileName, options));
}

ReaderWriter::WriteResult Archive::writeImage(const osg::Image& image,const std::string& fileName,const Options* options)
{
    return write(WriteImageFunctor(image, fileName, options));
}

ReaderWriter::WriteResult Archive::writeHeightField(const osg::HeightField& heightField,const std::string& fileName,const Options* options)
{
    return write(WriteHeightFieldFunctor(heightField, fileName, options));
}

ReaderWriter::WriteResult Archive::writeNode(const osg::Node& node,const std::string& fileName,const Options* options)
{
    osg::notify(osg::NOTICE)<<"Archive::writeNode(obj, "<<fileName<<")"<<std::endl;
    return write(WriteNodeFunctor(node, fileName, options));
}
