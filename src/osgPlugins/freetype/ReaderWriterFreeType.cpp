#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osg/Notify>

#include "FreeTypeLibrary.h"

class ReaderWriterFreeType : public osgDB::ReaderWriter
{
    public:
        ReaderWriterFreeType()
        {
            supportsExtension("ttf","true type font format");
            supportsExtension("ttc","true type collection format");
            supportsExtension("pfb","type1 binary format");
            supportsExtension("pfa","type2 ascii format");
            supportsExtension("cid","Postscript CID-Fonts format");
            supportsExtension("cff","OpenType format");
            supportsExtension("cef","OpenType format");
            supportsExtension("fon","Windows bitmap fonts format");
            supportsExtension("fnt","Windows bitmap fonts format");
            supportsExtension("text3d","use 3D Font instead of 2D Font");
            supportsExtension("woff","web open font format");

            supportsOption("monochrome","Select monochrome font.");
            supportsOption("index=<uint>", "Select index of font within ttc collection. Defaults to 0.");
        }

        virtual const char* className() const { return "FreeType Font Reader/Writer"; }

        static unsigned int getFlags(const osgDB::ReaderWriter::Options* options)
        {
            unsigned int flags = 0;
            if (options && options->getOptionString().find("monochrome") != std::string::npos)
            {
                flags |= FT_LOAD_MONOCHROME;
            }

            return flags;
        }

        static unsigned int getIndex(const osgDB::ReaderWriter::Options* options)
        {
            if(!options) return 0;

            std::string indexstr = options->getPluginStringData("index");
            int index = std::atoi(indexstr.c_str());
            if(index < 0)
            {
                OSG_WARN<< "Warning: invalid index string (" << indexstr << ") when loading freetype font. Attempting to use default index 0." << std::endl;
                return 0;
            }
            else return (unsigned int)index;
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            FreeTypeLibrary* freeTypeLibrary = FreeTypeLibrary::instance();
            if (!freeTypeLibrary)
            {
                OSG_WARN<<"Warning:: cannot create freetype font after freetype library has been deleted."<<std::endl;
                return ReadResult::ERROR_IN_READING_FILE;
            }

            return freeTypeLibrary->getFont(fileName, getIndex(options), getFlags(options));
        }

        virtual ReadResult readObject(std::istream& stream, const osgDB::ReaderWriter::Options* options) const
        {
            FreeTypeLibrary* freeTypeLibrary = FreeTypeLibrary::instance();
            if (!freeTypeLibrary)
            {
                OSG_WARN<<"Warning:: cannot create freetype font after freetype library has been deleted."<<std::endl;
                return ReadResult::ERROR_IN_READING_FILE;
            }

            return freeTypeLibrary->getFont(stream, getIndex(options), getFlags(options));
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(freetype, ReaderWriterFreeType)
