#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include "FreeTypeLibrary.h"

class ReaderWriterFreeType : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() const { return "FreeType Font Reader/Writer"; }
        
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"ttf") ||  // true type
                   osgDB::equalCaseInsensitive(extension,"ttc") ||  // true type
                   osgDB::equalCaseInsensitive(extension,"pfb") ||  // type1 binary
                   osgDB::equalCaseInsensitive(extension,"pfa") ||  // type2 ascii
                   osgDB::equalCaseInsensitive(extension,"cid") ||  // Postscript CID-Fonts
                   osgDB::equalCaseInsensitive(extension,"cff") ||  // OpenType
                   osgDB::equalCaseInsensitive(extension,"cef") ||  // OpenType
                   osgDB::equalCaseInsensitive(extension,"fon") ||  // Windows bitmap fonts
                   osgDB::equalCaseInsensitive(extension,"fnt");    // Windows bitmap fonts
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;
            
            osgText::Font* font = FreeTypeLibrary::instance()->getFont(fileName,0);
            
            return font;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterFreeType> g_readerWriter_FreeType_Proxy;
