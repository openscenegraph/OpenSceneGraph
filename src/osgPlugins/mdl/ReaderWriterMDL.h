#ifndef __READERWRITER_MDL_H_
#define __READERWRITER_MDL_H_


#include <osgDB/Registry>
#include <osgDB/FileNameUtils>


namespace mdl
{

class ReaderWriterMDL : public osgDB::ReaderWriter
{
public:

    virtual const char*   className() const;

    virtual bool   acceptsExtension(const std::string& extension) const;

    virtual ReadResult readObject(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
    {
        return readNode(filename, options);
    }

    virtual ReadResult   readNode(const std::string& file,
                                  const Options* options) const;
};


}

#endif
