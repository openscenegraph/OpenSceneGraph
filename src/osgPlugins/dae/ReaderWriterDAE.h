#ifndef _READERWRITERDAE_H_
#define _READERWRITERDAE_H_


#include <OpenThreads/ReentrantMutex>

///////////////////////////////////////////////////////////////////////////
// OSG reader/writer plugin for the COLLADA 1.4.x ".dae" format.
// See http://collada.org/ and http://khronos.org/collada/

class ReaderWriterDAE : public osgDB::ReaderWriter
{
public:
    ReaderWriterDAE()
    {
        // Collada document
        supportsExtension("dae","COLLADA 1.4.x DAE format");
        // Collada zip archive (contains one or more dae files and a manifest.xml)
        supportsExtension("zae","COLLADA 1.4.x ZAE format");

        supportsOption("polygon", "Use polygons instead of polylists for element (Write option)");
        supportsOption("GoogleMode", "Write files suitable for use by Google products");
        supportsOption("NoExtras", "Write option (Undocumented)");
        supportsOption("DaeEarthTex", "DAE settings for writing earth textures");
        supportsOption("ZUpAxis", "indicates if the up axis is on Z axis");
        supportsOption("ForceTexture", "force the use an image for a texture, even if the file is not found");

        supportsOption("StrictTransparency", "Read option (Undocumented)");
    }

    const char* className() const { return "COLLADA 1.4.x DAE reader/writer"; }

    ReadResult readNode(const std::string&, const Options*) const;

    WriteResult writeNode(const osg::Node&, const std::string&, const Options*) const;

    static std::string ConvertFilePathToColladaCompatibleURI(const std::string& FilePath);
  
private:
    mutable OpenThreads::ReentrantMutex _serializerMutex;
};

///////////////////////////////////////////////////////////////////////////

#endif

