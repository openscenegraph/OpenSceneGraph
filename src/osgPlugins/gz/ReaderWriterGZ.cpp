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


        bool read(std::istream& fin, std::string& destination) const;
        bool write(std::ostream& fout, const std::string& source) const;


};

ReaderWriterGZ::ReaderWriterGZ()
{
    // OSG_NOTICE<<"ReaderWriterGZ::ReaderWriterGZ()"<<std::endl;

    supportsExtension("osgz","Compressed .osg file extension.");
    supportsExtension("ivez","Compressed .ive file extension.");
    supportsExtension("gz","Compressed file extension.");
}

ReaderWriterGZ::~ReaderWriterGZ()
{
    // OSG_NOTICE<<"ReaderWriterGZ::~ReaderWriterGZ()"<<std::endl;
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

    if (osgDB::containsServerAddress(fullFileName)) return ReadResult::FILE_NOT_HANDLED;

    osgDB::ReaderWriter* rw = 0;

    if (osgDB::equalCaseInsensitive(ext,"osgz"))
    {
        rw = osgDB::Registry::instance()->getReaderWriterForExtension("osg");
        OSG_INFO<<"osgz ReaderWriter "<<rw<<std::endl;
    }
    else if (osgDB::equalCaseInsensitive(ext,"ivez"))
    {
        rw = osgDB::Registry::instance()->getReaderWriterForExtension("ive");
        OSG_INFO<<"ivez ReaderWriter "<<rw<<std::endl;
    }
    else
    {
        std::string baseFileName = osgDB::getNameLessExtension(fullFileName);
        std::string baseExt = osgDB::getLowerCaseFileExtension(baseFileName);
        rw = osgDB::Registry::instance()->getReaderWriterForExtension(baseExt);
        OSG_INFO<<baseExt<<" ReaderWriter "<<rw<<std::endl;
    }


    std::string fileName = osgDB::findDataFile( fullFileName, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    // code for setting up the database path so that internally referenced file are searched for on relative paths.
    osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
    local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

    osgDB::ifstream fin(fileName.c_str(), std::ios::binary|std::ios::in);
    if (!fin) return ReadResult::ERROR_IN_READING_FILE;


    std::string dest;
    read(fin, dest);

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
        OSG_NOTICE<<"osgz ReaderWriter "<<rw<<std::endl;
    }
    else if (osgDB::equalCaseInsensitive(ext,"ivez"))
    {
        rw = osgDB::Registry::instance()->getReaderWriterForExtension("ive");
        OSG_NOTICE<<"ivez ReaderWriter "<<rw<<std::endl;
    }
    else
    {
        std::string baseFileName = osgDB::getNameLessExtension(fullFileName);
        std::string baseExt = osgDB::getLowerCaseFileExtension(baseFileName);
        rw = osgDB::Registry::instance()->getReaderWriterForExtension(baseExt);
        OSG_NOTICE<<baseExt<<" ReaderWriter "<<rw<<std::endl;
    }

    std::stringstream strstream;
    osgDB::ReaderWriter::WriteResult writeResult = writeFile(objectType, object, rw, strstream, options);

    osgDB::ofstream fout(fullFileName.c_str(), std::ios::binary|std::ios::out);

    write(fout,strstream.str());

    return writeResult;
}

#define CHUNK 16384

bool ReaderWriterGZ::read(std::istream& fin, std::string& destination) const
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm,
                       15 + 32 // autodetected zlib or gzip header
                       );
    if (ret != Z_OK)
        return false;

    /* decompress until deflate stream ends or end of file */
    do {

        fin.read((char*)in, CHUNK);
        strm.avail_in = fin.gcount();

        if (fin.bad())
        {
            (void)inflateEnd(&strm);
            return false;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);

            switch (ret) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return false;
            }
            have = CHUNK - strm.avail_out;

            destination.append((char*)out, have);

        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? true : false;
}

bool ReaderWriterGZ::write(std::ostream& fout, const std::string& source) const
{
    int ret, flush = Z_FINISH;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    int level = 6;
    int stategy = Z_DEFAULT_STRATEGY; // looks to be the best for .osg/.ive files
    //int stategy = Z_FILTERED;
    //int stategy = Z_HUFFMAN_ONLY;
    //int stategy = Z_RLE;

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit2(&strm,
                       level,
                       Z_DEFLATED,
                       15+16, // +16 to use gzip encoding
                       8, // default
                       stategy);
    if (ret != Z_OK)
    return false;

    strm.avail_in = source.size();
    strm.next_in = (Bytef*)(&(*source.begin()));

    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do {
        strm.avail_out = CHUNK;
        strm.next_out = out;
        ret = deflate(&strm, flush);    /* no bad return value */

        if (ret == Z_STREAM_ERROR)
        {
            OSG_NOTICE<<"Z_STREAM_ERROR"<<std::endl;
            return false;
        }

        have = CHUNK - strm.avail_out;

        if (have>0) fout.write((const char*)out, have);

        if (fout.fail())
        {
            (void)deflateEnd(&strm);
            return false;
        }
    } while (strm.avail_out == 0);

    /* clean up and return */
    (void)deflateEnd(&strm);
    return true;
}


// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(GZ, ReaderWriterGZ)
