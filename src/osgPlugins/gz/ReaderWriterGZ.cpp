/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield 
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

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>

#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>

#include <zlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ReaderWriterGZ
//
class ReaderWriterGZ : public osgDB::ReaderWriter
{
    public:
    
        enum ObjectType
        {
            OBJECT,
            ARCHIVE,
            IMAGE,
            HEIGHTFIELD,
            NODE
        };

        ReaderWriterGZ();
      
        ~ReaderWriterGZ();

        virtual const char* className() const { return "HTTP Protocol Model Reader"; }
                                                                                            
        virtual ReadResult openArchive(const std::string& fileName,ArchiveStatus status, unsigned int , const Options* options) const
        {
            if (status!=READ) return ReadResult(ReadResult::FILE_NOT_HANDLED);
            else return readFile(ARCHIVE,fileName,options);
        }

        virtual ReadResult readObject(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            return readFile(OBJECT,fileName,options);
        }
                                                                                            
        virtual ReadResult readImage(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            return readFile(IMAGE,fileName,options);
        }

        virtual ReadResult readHeightField(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            return readFile(HEIGHTFIELD,fileName,options);
        }

        virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            return readFile(NODE,fileName,options);
        }

        ReadResult readFile(ObjectType objectType, osgDB::ReaderWriter* rw, std::istream& fin, const osgDB::ReaderWriter::Options* options) const;
        
        ReadResult readFile(ObjectType objectType, const std::string& fullFileName, const osgDB::ReaderWriter::Options* options) const;



        virtual WriteResult writeObject(const osg::Object& obj, const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            return writeFile(OBJECT, &obj, fileName, options);
        }
                                                                                            
        virtual WriteResult writeImage(const osg::Image& image, const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            return writeFile(IMAGE, &image, fileName, options);
        }

        virtual WriteResult writeHeightField(const osg::HeightField& hf, const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            return writeFile(HEIGHTFIELD, &hf, fileName, options);
        }

        virtual WriteResult writeNode(const osg::Node& node, const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            return writeFile(NODE, &node, fileName,options);
        }

        WriteResult writeFile(ObjectType objectType, const osg::Object* object, osgDB::ReaderWriter* rw, std::ostream& fin, const osgDB::ReaderWriter::Options* options) const;
        
        WriteResult writeFile(ObjectType objectType, const osg::Object* object, const std::string& fullFileName, const osgDB::ReaderWriter::Options* options) const;

};

ReaderWriterGZ::ReaderWriterGZ()
{
    // osg::notify(osg::NOTICE)<<"ReaderWriterGZ::ReaderWriterGZ()"<<std::endl;

    supportsExtension("osgz","Compressed .osg file extension.");
    supportsExtension("ivez","Compressed .ive file extension.");
    supportsExtension("gz","Compressed file extension.");
}

ReaderWriterGZ::~ReaderWriterGZ()
{
    // osg::notify(osg::NOTICE)<<"ReaderWriterGZ::~ReaderWriterGZ()"<<std::endl;
}

osgDB::ReaderWriter::ReadResult ReaderWriterGZ::readFile(ObjectType objectType, osgDB::ReaderWriter* rw, std::istream& fin, const osgDB::ReaderWriter::Options *options) const
{
    switch(objectType)
    {
        case(OBJECT): return rw->readObject(fin,options);
        case(ARCHIVE): return rw->openArchive(fin,options);
        case(IMAGE): return rw->readImage(fin,options);
        case(HEIGHTFIELD): return rw->readHeightField(fin,options);
        case(NODE): return rw->readNode(fin,options);
        default: break;
    }
    return ReadResult::FILE_NOT_HANDLED;
}

osgDB::ReaderWriter::ReadResult ReaderWriterGZ::readFile(ObjectType objectType, const std::string& fullFileName, const osgDB::ReaderWriter::Options *options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(fullFileName);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    osgDB::ReaderWriter* rw = 0;

    if (osgDB::equalCaseInsensitive(ext,"osgz"))
    {  
        rw = osgDB::Registry::instance()->getReaderWriterForExtension("osg");
        osg::notify(osg::NOTICE)<<"osgz ReaderWriter "<<rw<<std::endl;
    }
    else if (osgDB::equalCaseInsensitive(ext,"ivez"))
    {
        rw = osgDB::Registry::instance()->getReaderWriterForExtension("ive");
        osg::notify(osg::NOTICE)<<"ivez ReaderWriter "<<rw<<std::endl;
    }
    else
    {
        std::string baseFileName = osgDB::getNameLessExtension(fullFileName);        
        std::string baseExt = osgDB::getLowerCaseFileExtension(baseFileName);
        rw = osgDB::Registry::instance()->getReaderWriterForExtension(baseExt);
        osg::notify(osg::NOTICE)<<baseExt<<" ReaderWriter "<<rw<<std::endl;
    }


    std::string fileName = osgDB::findDataFile( fullFileName, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    // code for setting up the database path so that internally referenced file are searched for on relative paths. 
    osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
    local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

    std::ifstream fin(fileName.c_str());
    if (!fin) return ReadResult::ERROR_IN_READING_FILE;
    
    std::streampos start = fin.tellg();
    fin.seekg(0, std::ios::end);
    std::streampos end = fin.tellg();
    fin.seekg(start, std::ios::beg);
    std::streampos sourceLen = end - start;

    /* empty stream into memory, open that, and keep the pointer in a FreeTypeFont for cleanup */
    char* buffer = new char[sourceLen];
    fin.read(reinterpret_cast<char*>(buffer), sourceLen);
    if (!fin || (static_cast<std::streampos>(fin.gcount()) != sourceLen))
    {
        osg::notify(osg::WARN)<<" .... the compressed file could not be read from its stream"<<std::endl;
        return 0;
    }
    
    uLongf destLen = 1000000;
    
    std::string dest;
    dest.resize(destLen);
        
    int result = uncompress ((Bytef *)(&(*dest.begin())), &destLen,
                             (const Bytef *)buffer, sourceLen);
                                   
    delete [] buffer;

    std::stringstream strstream(dest);

    return readFile(objectType, rw, strstream, local_opt.get());
}


osgDB::ReaderWriter::WriteResult ReaderWriterGZ::writeFile(ObjectType objectType, const osg::Object* object, osgDB::ReaderWriter* rw, std::ostream& fout, const osgDB::ReaderWriter::Options *options) const
{
    switch(objectType)
    {
        case(OBJECT): return rw->writeObject(*object, fout, options);
        case(IMAGE): return rw->writeImage(*(dynamic_cast<const osg::Image*>(object)), fout, options);
        case(HEIGHTFIELD): return rw->writeHeightField(*(dynamic_cast<const osg::HeightField*>(object)), fout, options);
        case(NODE): return rw->writeNode(*(dynamic_cast<const osg::Node*>(object)), fout,options);
        default: break;
    }
    return WriteResult::FILE_NOT_HANDLED;
}

osgDB::ReaderWriter::WriteResult ReaderWriterGZ::writeFile(ObjectType objectType, const osg::Object* object, const std::string& fullFileName, const osgDB::ReaderWriter::Options *options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(fullFileName);
    if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

    osgDB::ReaderWriter* rw = 0;

    if (osgDB::equalCaseInsensitive(ext,"osgz"))
    {  
        rw = osgDB::Registry::instance()->getReaderWriterForExtension("osg");
        osg::notify(osg::NOTICE)<<"osgz ReaderWriter "<<rw<<std::endl;
    }
    else if (osgDB::equalCaseInsensitive(ext,"ivez"))
    {
        rw = osgDB::Registry::instance()->getReaderWriterForExtension("ive");
        osg::notify(osg::NOTICE)<<"ivez ReaderWriter "<<rw<<std::endl;
    }
    else
    {
        std::string baseFileName = osgDB::getNameLessExtension(fullFileName);        
        std::string baseExt = osgDB::getLowerCaseFileExtension(baseFileName);
        rw = osgDB::Registry::instance()->getReaderWriterForExtension(baseExt);
        osg::notify(osg::NOTICE)<<baseExt<<" ReaderWriter "<<rw<<std::endl;
    }
    
    std::stringstream strstream;
    osgDB::ReaderWriter::WriteResult writeResult = writeFile(objectType, object, rw, strstream, options);
    
    
    std::string str(strstream.str());
    
    uLong destLen = compressBound(str.size());
    
    osg::notify(osg::NOTICE)<<"compressBound("<<str.size()<<") = "<<destLen<<std::endl;
    
    std::string output;
    output.resize(destLen);
    
    int level = 6;
    
    char* source = new char[str.size()];
    char* dest = new char[destLen];

    int result = compress2( (Bytef *)(&(*output.begin())), &destLen,
                            (const Bytef *)(&(*str.begin())), str.size(),
                             level);

    osg::notify(osg::NOTICE)<<"compress2()"<<result<<", destLen="<<destLen<<std::endl;
    
    output.resize(destLen);
    
    std::ofstream fout(fullFileName.c_str());    
    fout<<output;

    return writeResult;
}

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(GZ, ReaderWriterGZ)
