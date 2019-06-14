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
            return "Inventor reader/writer";
        }

        bool isInventorExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension, "iv") ? true : false;
        }

        virtual ReadResult readObject(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
        {
            return readNode(filename, options);
        }

        virtual ReadResult readNode(const std::string& filename,
                                    const osgDB::ReaderWriter::Options*) const;

        virtual ReadResult readObject(std::istream& fin, const osgDB::ReaderWriter::Options* options) const
        {
            return readNode(fin, options);
        }

        virtual ReadResult readNode(std::istream& fin,
                                    const osgDB::ReaderWriter::Options* = NULL) const;


        virtual WriteResult writeNode(const osg::Node& node, const std::string& filename,
                                      const osgDB::ReaderWriter::Options* options = NULL) const;

    protected:
        void initInventor() const;
        ReadResult readNodeFromSoInput(class SoInput&,
                  std::string &fileName, const osgDB::ReaderWriter::Options*) const;
};

#endif

