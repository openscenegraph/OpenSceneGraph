#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include "ESRIShape.h"
#include "ESRIShapeParser.h"

class ESRIShapeReaderWriter : public osgDB::ReaderWriter
{
    public:
        ESRIShapeReaderWriter() {}

        virtual const char* className() { return "ESRI Shape ReaderWriter"; }
        
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"shp");
        }

        virtual ReadResult readObject(const std::string& fileName, const Options* opt) const
        { return readNode(fileName,opt); }

        virtual ReadResult readNode(const std::string& file, const Options* options) const
        {
            std::string ext = osgDB::getFileExtension(file);
            if (!acceptsExtension(ext)) 
                return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile(file, options);
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            ESRIShape::ESRIShapeParser sp(fileName);
            return sp.getGeode();
        }

    private:
};

osgDB::RegisterReaderWriterProxy<ESRIShapeReaderWriter> g_esriShapeReaderWriter_Proxy;

