#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include "FreeTypeLibrary.h"

class ReaderWriterFreeType : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "FreeType Font Reader/Writer"; }
        
        virtual bool acceptsExtension(const std::string& extension)
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

        virtual ReadResult readObject(const std::string& fileName, const osgDB::ReaderWriter::Options*)
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;
            
            osgText::Font* font = FreeTypeLibrary::instance()->getFont(fileName,0);
            
            return font;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterFreeType> g_readerWriter_FreeType_Proxy;
