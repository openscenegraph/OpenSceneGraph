#include <osg/Notify>
#include <osgDB/Archive>
#include <osgDB/FileNameUtils>

class OSGA_Archive : public osgDB::Archive
{
    public:
        OSGA_Archive();
        virtual ~OSGA_Archive(); 

        virtual const char* libraryName() const { return "osga"; }

        virtual const char* className() const { return "Archive"; }

        virtual bool acceptsExtension(const std::string& extension)
        {
            return osgDB::equalCaseInsensitive(extension,"osga");
        }
        
        /** open the archive.*/
        virtual bool open(const std::string& filename, ArchiveStatus status, unsigned int indexBlockSizeHint=4096);

        /** open the archive for reading.*/
        virtual bool open(std::istream& fin);

        /** close the archive.*/
        virtual void close();

        /** return true if file exists in archive.*/        
        virtual bool fileExists(const std::string& filename) const;
        
        /** Get the file name which represents the master file recorded in the Archive.*/
        virtual std::string getMasterFileName() const;
        
        typedef std::vector<std::string> FileNameList;
        
        /** Get the full list of file names available in the archive.*/
        virtual bool getFileNames(FileNameList& fileNameList) const;


        /** Read an osg::Object of specified file name from the Archive.*/
        virtual ReadResult readObject(const std::string& fileName,const Options* options=NULL);

        /** Read an osg::Image of specified file name from the Archive.*/
        virtual ReadResult readImage(const std::string& fileName,const Options* options=NULL);

        /** Read an osg::HeightField of specified file name from the Archive.*/
        virtual ReadResult readHeightField(const std::string& fileName,const Options* options=NULL);

        /** Read an osg::Node of specified file name from the Archive.*/
        virtual ReadResult readNode(const std::string& fileName,const Options* options=NULL);

        /** Write an osg::Object with specified file name to the Archive.*/
        virtual WriteResult writeObject(const osg::Object& obj,const std::string& fileName,const Options* options=NULL);

        /** Write an osg::Image with specified file name to the Archive.*/
        virtual WriteResult writeImage(const osg::Image& image,const std::string& fileName,const Options* options=NULL);

        /** Write an osg::HeightField with specified file name to the Archive.*/
        virtual WriteResult writeHeightField(const osg::HeightField& heightField,const std::string& fileName,const Options* options=NULL);

        /** Write an osg::Node with specified file name to the Archive.*/
        virtual WriteResult writeNode(const osg::Node& node,const std::string& fileName,const Options* options=NULL);
        
        
    protected:
  
        #if defined(_MSC_VER)
        typedef __int64 pos_type;
        typedef __int64 size_type;
        #else
        typedef unsigned long long pos_type;
        typedef unsigned long long size_type;
        #endif
        
        typedef std::pair<pos_type, size_type> PositionSizePair;
        typedef std::map<std::string, PositionSizePair> FileNamePositionMap;

        class IndexBlock : public osg::Referenced
        {
        public:
            IndexBlock(unsigned int blockSize=0);
            
            inline pos_type getPosition() const { return _filePosition; }

            inline unsigned int getBlockSize() const { return _blockSize; }


            void setPositionNextIndexBlock(pos_type position);
            
            inline pos_type getPositionNextIndexBlock() const { return _filePositionNextIndexBlock; }


            static IndexBlock* read(std::istream& in, bool doEndianSwap);
            
            std::string getFirstFileName() const;

            bool getFileReferences(FileNamePositionMap& indexMap) const;
            

            inline bool requiresWrite() const { return _requiresWrite; }
            
            void write(std::ostream& out);
            
            inline bool spaceAvailable(pos_type, size_type, const std::string& filename) const
            {
                unsigned requiredSize = sizeof(pos_type)+sizeof(size_type)+sizeof(unsigned int)+filename.size();
                return (_offsetOfNextAvailableSpace + requiredSize)<_blockSize;
            }
            
            bool addFileReference(pos_type position, size_type size, const std::string& filename);
            


        protected:
            
            void allocateData(unsigned int blockSize);
        
            virtual ~IndexBlock();
            bool            _requiresWrite;
            pos_type        _filePosition;

            unsigned int    _blockSize;
            pos_type        _filePositionNextIndexBlock;
            unsigned int    _offsetOfNextAvailableSpace;
            char*           _data;
        };
    
        /** Functor used in internal implementations.*/
        struct ReadFunctor
        {
            ReadFunctor(const std::string& filename, const ReaderWriter::Options* options):
                _filename(filename),
                _options(options) {}

            virtual ~ReadFunctor() {}
            virtual ReaderWriter::ReadResult doRead(ReaderWriter& rw, std::istream& input) const = 0;

            std::string _filename;
            const ReaderWriter::Options* _options;
        };
        
        struct ReadObjectFunctor;
        struct ReadImageFunctor;
        struct ReadHeightFieldFunctor;
        struct ReadNodeFunctor;

        /** Functor used in internal implementations.*/
        struct WriteFunctor
        {
            WriteFunctor(const std::string& filename, const ReaderWriter::Options* options):
                _filename(filename),
                _options(options) {}

            virtual ~WriteFunctor() {}
            virtual ReaderWriter::WriteResult doWrite(ReaderWriter& rw, std::ostream& output) const = 0;

            std::string _filename;
            const ReaderWriter::Options* _options;
        };

        struct WriteObjectFunctor;
        struct WriteImageFunctor;
        struct WriteHeightFieldFunctor;
        struct WriteNodeFunctor;


        ReaderWriter::ReadResult read(const ReadFunctor& readFunctor);
        ReaderWriter::WriteResult write(const WriteFunctor& writeFunctor);
    
        typedef std::list< osg::ref_ptr<IndexBlock> >   IndexBlockList;
        
        bool _open(std::istream& fin);

        void writeIndexBlocks();
        
        bool addFileReference(pos_type position, size_type size, const std::string& fileName);
        
        static float        s_currentSupportedVersion;
        float               _version;
        ArchiveStatus       _status;
        std::ifstream       _input;
        std::fstream        _output;
        
        std::string         _masterFileName;
        IndexBlockList      _indexBlockList;
        FileNamePositionMap _indexMap;


};
