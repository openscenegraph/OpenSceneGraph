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

        supportsOption("polygon",         "(Write option) Use polygons instead of polylists for element");
        supportsOption("GoogleMode",      "(Write option) Write files suitable for use by Google products");
        supportsOption("NoExtras",        "(Write option) Undocumented");
        supportsOption("daeEarthTex",     "(Write option) DAE settings for writing earth textures");
        supportsOption("daeZUpAxis",      "(Write option) Indicates the up axis is Z instead of Y");
        supportsOption("daeForceTexture", "(Write option) Force writing references to an image for a texture, even if the file is not found");

        supportsOption("StrictTransparency", "(Read option) Undocumented");
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

