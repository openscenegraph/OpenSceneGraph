#include <osg/Notify>
#include <osg/Endian>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

#include "OSGA_Archive.h"

using namespace osgDB;

float OSGA_Archive::s_currentSupportedVersion = 0.0;
const unsigned int ENDIAN_TEST_NUMBER = 0x00000001;

OSGA_Archive::IndexBlock::IndexBlock(unsigned int blockSize):
    _requiresWrite(false),
    _filePosition(0),
    _blockSize(0),
    _filePositionNextIndexBlock(0),
    _offsetOfNextAvailableSpace(0),
    _data(0)
{
    allocateData(blockSize);
}

OSGA_Archive::IndexBlock::~IndexBlock()
{
    delete [] _data;
}

void OSGA_Archive::IndexBlock::allocateData(unsigned int blockSize)
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

OSGA_Archive::IndexBlock* OSGA_Archive::IndexBlock::read(std::istream& in, bool doEndianSwap)
{
    if (!in) return 0;

    osg::ref_ptr<IndexBlock> indexBlock = new IndexBlock;
    indexBlock->_filePosition = in.tellg();
    in.read(reinterpret_cast<char*>(&indexBlock->_blockSize), sizeof(indexBlock->_blockSize));
    in.read(reinterpret_cast<char*>(&indexBlock->_filePositionNextIndexBlock), sizeof(indexBlock->_filePositionNextIndexBlock));
    in.read(reinterpret_cast<char*>(&indexBlock->_offsetOfNextAvailableSpace), sizeof(indexBlock-> _offsetOfNextAvailableSpace));
    
    if (doEndianSwap)
    {
        osg::swapBytes(reinterpret_cast<char*>(&indexBlock->_blockSize), sizeof(indexBlock->_blockSize));
        osg::swapBytes(reinterpret_cast<char*>(&indexBlock->_filePositionNextIndexBlock), sizeof(indexBlock->_filePositionNextIndexBlock));
        osg::swapBytes(reinterpret_cast<char*>(&indexBlock->_offsetOfNextAvailableSpace), sizeof(indexBlock-> _offsetOfNextAvailableSpace));
    }

//    osg::notify(osg::INFO)<<"indexBlock->_blockSize="<<indexBlock->_blockSize<<std::endl;
//    osg::notify(osg::INFO)<<"indexBlock->_filePositionNextIndexBlock="<<indexBlock->_filePositionNextIndexBlock<<std::endl;
//    osg::notify(osg::INFO)<<"indexBlock->_offsetOfNextAvailableSpace="<<indexBlock->_offsetOfNextAvailableSpace<<std::endl;

    indexBlock->allocateData(indexBlock->_blockSize);
    if (indexBlock->_data)
    {
        in.read(reinterpret_cast<char*>(indexBlock->_data),indexBlock->_blockSize);

        if (doEndianSwap)
        {
            char* ptr = indexBlock->_data;
            char* end_ptr = indexBlock->_data + indexBlock->_offsetOfNextAvailableSpace;
            while (ptr<end_ptr)
            {
                osg::swapBytes(ptr,sizeof(pos_type)); 
                ptr += sizeof(pos_type);

                osg::swapBytes(ptr,sizeof(size_type)); 
                ptr += sizeof(size_type);

                osg::swapBytes(ptr,sizeof(unsigned int)); 
                unsigned int filename_size; // = *(reinterpret_cast<unsigned int*>(ptr));
                _read(ptr, filename_size);
                ptr += sizeof(unsigned int);
                ptr += filename_size;

                osg::notify(osg::INFO)<<"filename size="<<filename_size<<std::endl;

            }
        }
    }
    else
    {
        osg::notify(osg::INFO)<<"Allocation Problem in OSGA_Archive::IndexBlock::read(std::istream& in)"<<std::endl;
        return 0;
    }

    osg::notify(osg::INFO)<<"Read index block"<<std::endl;
    
    return indexBlock.release();
    
}

std::string OSGA_Archive::IndexBlock::getFirstFileName() const
{
    char* ptr = _data;
    char* end_ptr = _data + _offsetOfNextAvailableSpace;
    if (ptr<end_ptr)
    {
        ptr += sizeof(pos_type);
        ptr += sizeof(size_type);

        unsigned int filename_size; // = *(reinterpret_cast<unsigned int*>(ptr));
        _read(ptr, filename_size);
        ptr += sizeof(unsigned int);

        return std::string(ptr, ptr+filename_size);
    }
    else
    {
        return std::string();
    }
}

bool OSGA_Archive::IndexBlock::getFileReferences(FileNamePositionMap& indexMap) const
{
    if (!_data || _offsetOfNextAvailableSpace==0) return false;
    
    bool valuesAdded = false;
    
    char* ptr = _data;
    char* end_ptr = _data + _offsetOfNextAvailableSpace;
    while (ptr<end_ptr)
    {
        pos_type position; // = *(reinterpret_cast<pos_type*>(ptr)); 
        _read(ptr, position);
        ptr += sizeof(pos_type);
        
        size_type size; // = *(reinterpret_cast<size_type*>(ptr)); 
        _read(ptr, size);
        ptr += sizeof(size_type);

        unsigned int filename_size; // = *(reinterpret_cast<unsigned int*>(ptr));
        _read(ptr, filename_size);
        ptr += sizeof(unsigned int);
        
        std::string filename(ptr, ptr+filename_size);
        
        // record this entry into the FileNamePositionMap
        indexMap[filename] = PositionSizePair(position,size);
        
        ptr += filename_size;
        
        valuesAdded = true;
    }
    return valuesAdded;
}

void OSGA_Archive::IndexBlock::write(std::ostream& out)
{
    if (_filePosition==pos_type(0))
    {
        osg::notify(osg::INFO)<<"OSGA_Archive::IndexBlock::write() setting _filePosition"<<std::endl;
        _filePosition = out.tellp();
    }
    else
    {
         out.seekp(_filePosition);
    }
    osg::notify(osg::INFO)<<"OSGA_Archive::IndexBlock::write() to _filePosition"<<out.tellp()<<std::endl;

    out.write(reinterpret_cast<char*>(&_blockSize), sizeof(_blockSize));
    out.write(reinterpret_cast<char*>(&_filePositionNextIndexBlock), sizeof(_filePositionNextIndexBlock));
    out.write(reinterpret_cast<char*>(&_offsetOfNextAvailableSpace), sizeof(_offsetOfNextAvailableSpace));

    out.write(reinterpret_cast<char*>(_data),_blockSize);
    
    osg::notify(osg::INFO)<<"OSGA_Archive::IndexBlock::write() end"<<std::endl;
}


bool OSGA_Archive::IndexBlock::addFileReference(pos_type position, size_type size, const std::string& filename)
{
    if (spaceAvailable(position, size, filename))
    {
        char* ptr = _data+_offsetOfNextAvailableSpace;
        
        //*(reinterpret_cast<pos_type*>(ptr)) = position; 
        _write(ptr, position);
        ptr += sizeof(pos_type);
        
        //*(reinterpret_cast<size_type*>(ptr)) = size; 
        _write(ptr, size);
        ptr += sizeof(size_type);
        
        //*(reinterpret_cast<unsigned int*>(ptr)) = filename.size();
        _write(ptr, static_cast<unsigned int>(filename.size()));
        ptr += sizeof(unsigned int);
        
        for(unsigned int i=0;i<filename.size();++i, ++ptr)
        {
            *ptr = filename[i];
        }
        
        _offsetOfNextAvailableSpace = ptr-_data;
        
        _requiresWrite = true;

        osg::notify(osg::INFO)<<"OSGA_Archive::IndexBlock::addFileReference("<<(unsigned int)position<<", "<<filename<<")"<<std::endl;
        
        return true;
    }
    else
    {
        return false;
    }
}

void OSGA_Archive::IndexBlock::setPositionNextIndexBlock(pos_type position)
{
    _filePositionNextIndexBlock = position;
    _requiresWrite = true;
}

OSGA_Archive::OSGA_Archive()
{
}

OSGA_Archive::~OSGA_Archive()
{
    close();
}


bool OSGA_Archive::open(const std::string& filename, ArchiveStatus status, unsigned int indexBlockSize)
{
    SERIALIZER();

    if (status==READ)
    {
        _status = status;
        _input.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);

        return _open(_input);
    }
    else
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

            osg::notify(osg::INFO)<<"OSGA_Archive::open("<<filename<<") open for writing"<<std::endl;

            return true;
        }
        else // no file opened or using create so resort to creating the archive.
        {
            osg::notify(osg::INFO)<<"OSGA_Archive::open("<<filename<<"), archive being created."<<std::endl;

            _status = WRITE;
            _output.open(filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            _output<<"osga";
            _output.write(reinterpret_cast<const char*>(&ENDIAN_TEST_NUMBER),4);
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
}

bool OSGA_Archive::open(std::istream& fin)
{
    SERIALIZER();

    osg::notify(osg::NOTICE)<<"OSGA_Archive::open"<<std::endl;
    static_cast<std::istream&>(_input).rdbuf(fin.rdbuf());
    return _open(_input);
}

bool OSGA_Archive::_open(std::istream& input)
{
    if (input)
    {
        char identifier[4];
        input.read(identifier,4);

        bool validArchive = (identifier[0]=='o' && identifier[1]=='s' && identifier[2]=='g' && identifier[3]=='a');
        if (validArchive) 
        {

            unsigned int endianTestWord=0;
            input.read(reinterpret_cast<char*>(&endianTestWord),4);
            bool doEndianSwap = (endianTestWord!=ENDIAN_TEST_NUMBER);

            input.read(reinterpret_cast<char*>(&_version),sizeof(_version));
            if (doEndianSwap)
            {
                osg::swapBytes(reinterpret_cast<char*>(&_version),sizeof(_version));
            }

            osg::notify(osg::INFO)<<"OSGA_Archive::open() doEndianSwap="<<doEndianSwap<<std::endl;
            osg::notify(osg::INFO)<<"OSGA_Archive::open() Version="<<_version<<std::endl;

            IndexBlock *indexBlock = 0;

            while ( (indexBlock=OSGA_Archive::IndexBlock::read(input, doEndianSwap)) != 0)
            {
                _indexBlockList.push_back(indexBlock);
                if (indexBlock->getPositionNextIndexBlock()==pos_type(0)) break;

                input.seekg(indexBlock->getPositionNextIndexBlock());
            }

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
    return false;
}

void OSGA_Archive::close()
{
    SERIALIZER();

    _input.close();
    
    if (_status==WRITE)
    {
        writeIndexBlocks();
        _output.close();
    }
}

std::string OSGA_Archive::getMasterFileName() const
{
    return _masterFileName;
}

bool OSGA_Archive::getFileNames(FileNameList& fileNameList) const
{
    SERIALIZER();

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


void OSGA_Archive::writeIndexBlocks()
{
    SERIALIZER();

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

bool OSGA_Archive::fileExists(const std::string& filename) const
{
    return (_indexMap.count(filename)!=0);
}

bool OSGA_Archive::addFileReference(pos_type position, size_type size, const std::string& fileName)
{
    SERIALIZER();

    if (_status==READ)
    {
        osg::notify(osg::INFO)<<"OSGA_Archive::getPositionForNewEntry("<<fileName<<") failed, archive opened as read only."<<std::endl;
        return false;
    }
    
    if (!_output)
    {
        osg::notify(osg::INFO)<<"OSGA_Archive::getPositionForNewEntry("<<fileName<<") failed, _output set up."<<std::endl;
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
        _numChars(numChars) {
            setg(&oneChar, (&oneChar)+1, (&oneChar)+1);
    }
   
      /// Destructor deallocates no buffer space.
      virtual ~proxy_streambuf()  {}

      std::streambuf* _streambuf;

    protected:

      unsigned int _numChars;      
      char_type oneChar;
      
      virtual int_type underflow()
      {
     if ( gptr() == &oneChar ) return traits_type::to_int_type(oneChar);
        
         if ( _numChars==0 ) return traits_type::eof();         
         --_numChars;

           int_type next_value = _streambuf->sbumpc();
     
         if ( !traits_type::eq_int_type(next_value,traits_type::eof()) )
     {
       setg(&oneChar, &oneChar, (&oneChar)+1);
            oneChar = traits_type::to_char_type(next_value);
     }
    
         return next_value;
      }
};

struct OSGA_Archive::ReadObjectFunctor : public OSGA_Archive::ReadFunctor
{
    ReadObjectFunctor(const std::string& filename, const ReaderWriter::Options* options):ReadFunctor(filename,options) {}
    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw, std::istream& input) const { return rw.readObject(input, _options); }    
};

struct OSGA_Archive::ReadImageFunctor : public OSGA_Archive::ReadFunctor
{
   ReadImageFunctor(const std::string& filename, const ReaderWriter::Options* options):ReadFunctor(filename,options) {}
    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw, std::istream& input)const  { return rw.readImage(input, _options); }    
};

struct OSGA_Archive::ReadHeightFieldFunctor : public OSGA_Archive::ReadFunctor
{
    ReadHeightFieldFunctor(const std::string& filename, const ReaderWriter::Options* options):ReadFunctor(filename,options) {}
    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw, std::istream& input) const { return rw.readHeightField(input, _options); }    
};

struct OSGA_Archive::ReadNodeFunctor : public OSGA_Archive::ReadFunctor
{
    ReadNodeFunctor(const std::string& filename, const ReaderWriter::Options* options):ReadFunctor(filename,options) {}
    virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw, std::istream& input) const { return rw.readNode(input, _options); }    
};

ReaderWriter::ReadResult OSGA_Archive::read(const ReadFunctor& readFunctor)
{
    SERIALIZER();

    if (_status!=READ) 
    {
        osg::notify(osg::INFO)<<"OSGA_Archive::readObject(obj, "<<readFunctor._filename<<") failed, archive opened as read only."<<std::endl;
        return ReadResult(ReadResult::FILE_NOT_HANDLED);
    }
    
    FileNamePositionMap::const_iterator itr = _indexMap.find(readFunctor._filename);
    if (itr==_indexMap.end())
    {
        osg::notify(osg::INFO)<<"OSGA_Archive::readObject(obj, "<<readFunctor._filename<<") failed, file not found in archive"<<std::endl;
        return ReadResult(ReadResult::FILE_NOT_FOUND);
    }
    
    ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension(getLowerCaseFileExtension(readFunctor._filename));
    if (!rw)
    {
        osg::notify(osg::INFO)<<"OSGA_Archive::readObject(obj, "<<readFunctor._filename<<") failed to find appropriate plugin to write file."<<std::endl;
        return ReadResult(ReadResult::FILE_NOT_HANDLED);
    }
    
    osg::notify(osg::INFO)<<"OSGA_Archive::readObject(obj, "<<readFunctor._filename<<")"<<std::endl;
    
    _input.seekg(itr->second.first);

    // set up proxy stream buffer to proide the faked ending.
    std::istream& ins = _input;
    proxy_streambuf mystreambuf(ins.rdbuf(),itr->second.second);
    ins.rdbuf(&mystreambuf);

    ReaderWriter::ReadResult result = readFunctor.doRead(*rw, _input);

    ins.rdbuf(mystreambuf._streambuf);
    
    return result;
}

ReaderWriter::ReadResult OSGA_Archive::readObject(const std::string& fileName,const Options* options) const
{
    return const_cast<OSGA_Archive*>(this)->read(ReadObjectFunctor(fileName, options));
}

ReaderWriter::ReadResult OSGA_Archive::readImage(const std::string& fileName,const Options* options) const
{
    return const_cast<OSGA_Archive*>(this)->read(ReadImageFunctor(fileName, options));
}

ReaderWriter::ReadResult OSGA_Archive::readHeightField(const std::string& fileName,const Options* options) const
{
    return const_cast<OSGA_Archive*>(this)->read(ReadHeightFieldFunctor(fileName, options));
}

ReaderWriter::ReadResult OSGA_Archive::readNode(const std::string& fileName,const Options* options) const
{
    return const_cast<OSGA_Archive*>(this)->read(ReadNodeFunctor(fileName, options));
}


struct OSGA_Archive::WriteObjectFunctor : public OSGA_Archive::WriteFunctor
{
    WriteObjectFunctor(const osg::Object& object, const std::string& filename, const ReaderWriter::Options* options):
        WriteFunctor(filename,options),
        _object(object) {}
    const osg::Object& _object;
    
    virtual ReaderWriter::WriteResult doWrite(ReaderWriter& rw, std::ostream& output) const { return rw.writeObject(_object, output, _options); } 
};

struct OSGA_Archive::WriteImageFunctor : public OSGA_Archive::WriteFunctor
{
    WriteImageFunctor(const osg::Image& object, const std::string& filename, const ReaderWriter::Options* options):
        WriteFunctor(filename,options),
        _object(object) {}
    const osg::Image& _object;

    virtual ReaderWriter::WriteResult doWrite(ReaderWriter& rw, std::ostream& output) const { return rw.writeImage(_object, output, _options); }    
};

struct OSGA_Archive::WriteHeightFieldFunctor : public OSGA_Archive::WriteFunctor
{
    WriteHeightFieldFunctor(const osg::HeightField& object, const std::string& filename, const ReaderWriter::Options* options):
        WriteFunctor(filename,options),
        _object(object) {}
    const osg::HeightField& _object;

    virtual ReaderWriter::WriteResult doWrite(ReaderWriter& rw, std::ostream& output) const { return rw.writeHeightField(_object, output, _options); }    
};

struct OSGA_Archive::WriteNodeFunctor : public OSGA_Archive::WriteFunctor
{
    WriteNodeFunctor(const osg::Node& object, const std::string& filename, const ReaderWriter::Options* options):
        WriteFunctor(filename,options),
        _object(object) {}
    const osg::Node& _object;

    virtual ReaderWriter::WriteResult doWrite(ReaderWriter& rw, std::ostream& output) const { return rw.writeNode(_object, output, _options); }    
};

ReaderWriter::WriteResult OSGA_Archive::write(const WriteFunctor& writeFunctor)
{
    SERIALIZER();

    if (_status!=WRITE) 
    {
        osg::notify(osg::INFO)<<"OSGA_Archive::write(obj, "<<writeFunctor._filename<<") failed, archive opened as read only."<<std::endl;
        return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }

    ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension(getLowerCaseFileExtension(writeFunctor._filename));
    if (!rw)
    {
        osg::notify(osg::INFO)<<"OSGA_Archive::write(obj, "<<writeFunctor._filename<<") failed to find appropriate plugin to write file."<<std::endl;
        return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }
    
    osg::notify(osg::INFO)<<"OSGA_Archive::write(obj, "<<writeFunctor._filename<<")"<<std::endl;
    
    // place write position at end of file.
    _output.seekp(0,std::ios::end);
    
    pos_type position = _output.tellp();
    
    WriteResult result = writeFunctor.doWrite(*rw,_output);
    
    pos_type final_position = _output.tellp();
    size_type size = size_type(final_position-position);

    if (result.success()) addFileReference(position, size, writeFunctor._filename);
    
    return result;
}


ReaderWriter::WriteResult OSGA_Archive::writeObject(const osg::Object& obj,const std::string& fileName,const Options* options) const
{
    osg::notify(osg::INFO)<<"OSGA_Archive::writeObject(obj, "<<fileName<<")"<<std::endl;
    return const_cast<OSGA_Archive*>(this)->write(WriteObjectFunctor(obj, fileName, options));
}

ReaderWriter::WriteResult OSGA_Archive::writeImage(const osg::Image& image,const std::string& fileName,const Options* options) const
{
    osg::notify(osg::INFO)<<"OSGA_Archive::writeImage(obj, "<<fileName<<")"<<std::endl;
    return const_cast<OSGA_Archive*>(this)->write(WriteImageFunctor(image, fileName, options));
}

ReaderWriter::WriteResult OSGA_Archive::writeHeightField(const osg::HeightField& heightField,const std::string& fileName,const Options* options) const
{
    osg::notify(osg::INFO)<<"OSGA_Archive::writeHeightField(obj, "<<fileName<<")"<<std::endl;
    return const_cast<OSGA_Archive*>(this)->write(WriteHeightFieldFunctor(heightField, fileName, options));
}

ReaderWriter::WriteResult OSGA_Archive::writeNode(const osg::Node& node,const std::string& fileName,const Options* options) const
{
    osg::notify(osg::INFO)<<"OSGA_Archive::writeNode(obj, "<<fileName<<")"<<std::endl;
    return const_cast<OSGA_Archive*>(this)->write(WriteNodeFunctor(node, fileName, options));
}

