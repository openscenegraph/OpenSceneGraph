#ifndef _READERWRITERIV_H_
#define _READERWRITERIV_H_

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

class ReaderWriterIV : public osgDB::ReaderWriter
{
    public:
        ReaderWriterIV();
        
        virtual const char* className() const
        { 
            return "Inventor Reader"; 
        }
        
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension, "iv") ? true :
                   osgDB::equalCaseInsensitive(extension, "wrl") ? true : false;
        }
        
        virtual ReadResult readNode(const std::string& filename, 
                                    const osgDB::ReaderWriter::Options *) const;

};

#endif
