#ifndef __READERWRITER_TXP_H_
#define __READERWRITER_TXP_H_

#include "trpage_sys.h"

#include <osg/Object>
#include <osg/Node>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osg/Referenced>
#include <map>

namespace txp
{
    class TXPArchive;
    class ReaderWriterTXP : public osgDB::ReaderWriter
    {
        public:
            virtual const char* className() { return "TXP Reader/Writer"; }

            virtual bool acceptsExtension(const std::string& extension)
            {
                return osgDB::equalCaseInsensitive(extension,"txp");
            }

            virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*);

    protected:
        TXPArchive *getArchive(int id, const std::string&);
        std::map< int,osg::ref_ptr<TXPArchive> >    _archives;

        static int _archiveId;
    };

} // namespace

#endif // __READERWRITER_TXP_H_
