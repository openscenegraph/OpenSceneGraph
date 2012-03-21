#ifndef __READERWRITER_VBSP_H_
#define __READERWRITER_VBSP_H_


#include <osgDB/Registry>
#include <osgDB/FileNameUtils>


namespace bsp
{


class ReaderWriterBSP : public osgDB::ReaderWriter
{
public:

    virtual const char*   className() const;

    virtual bool   acceptsExtension(const std::string& extension) const;

    virtual ReadResult   readNode(const std::string& file,
                                  const Options* options) const;
};


}

#endif
