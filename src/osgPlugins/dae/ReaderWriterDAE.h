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
        _specversion = "1.4.1";

        // Collada document
        supportsExtension("dae","COLLADA 1.4.x DAE format");
        // Collada zip archive (contains one or more dae files and a manifest.xml)
        supportsExtension("zae","COLLADA 1.4.x ZAE format");

        supportsOption("polygon",         "(Write option) Use polygons instead of polylists for element");
        supportsOption("GoogleMode",      "(Write option) Write files suitable for use by Google products");
        supportsOption("NoExtras",        "(Write option) Undocumented");
        supportsOption("daeEarthTex",     "(Write option) DAE settings for writing earth textures");
        supportsOption("daeZUpAxis",      "(Write option) Indicates the up axis is Z instead of Y");
        supportsOption("daeLinkOriginalTexturesNoForce", "(Write option) Writes reference to the original image if found, instead of writing the image in memory");
        supportsOption("daeLinkOriginalTexturesForce",   "(Write option) Writes reference to the original image even if not found, instead of writing the image in memory");
        supportsOption("daeNamesUseCodepage",            "(Write option) All names except filenames (materials, animation, geometries...) should be considered as encoded using current codepage (UTF8 if not). Filenames follow OSG_USE_UTF8_FILENAME.");
        supportsOption("daeRenameIds",                   "(Write option) Rename all IDs (geometries, materials, etc.) to remove characters which may be interpreted as an URI. Useful if you want to ensure names having spaces or slashes behave correctly. This may be undesired if original naming must be somewhat kept.");

        supportsOption("StrictTransparency", "(Read option) Undocumented");
        supportsOption("daeTessellateNone",              "(Read option) Do not tessellate at all (Polygons are stored as GL_POLYGON - not suitable for concave polygons)");
        supportsOption("daeTessellatePolygonsAsTriFans", "(Read option) Tessellate the old way (default), interpreting polygons as triangle fans (faster, but does not work for concave polygons)");
        supportsOption("daeTessellatePolygons",          "(Read option) Use full tessellation of polygons (slower, works for concave polygons)");
        supportsOption("daeUsePredefinedTextureUnits",   "(Read option) Texture units have fixed uses (0: ambient occlusion, 1: main texture...). May create non contiguous units (default).");
        supportsOption("daeUseSequencedTextureUnits",    "(Read option) Texture units are created in sequence (contiguous units).");
    }

    const char* className() const { return "COLLADA 1.4.x DAE reader/writer"; }

    virtual ReadResult readObject(std::istream& fin, const osgDB::ReaderWriter::Options* options) const
    {
        return readNode(fin, options);
    }

    ReadResult readNode(std::istream&, const Options* = NULL) const;

    virtual ReadResult readObject(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        return readNode(fileName, options);
    }

    ReadResult readNode(const std::string&, const Options* = NULL) const;

    WriteResult writeNode(const osg::Node&, const std::string&, const Options* = NULL) const;

    static std::string ConvertFilePathToColladaCompatibleURI(const std::string& FilePath);
    static std::string ConvertColladaCompatibleURIToFilePath(const std::string& uri);

private:
    mutable OpenThreads::ReentrantMutex _serializerMutex;
    const char* _specversion;
};

///////////////////////////////////////////////////////////////////////////

#endif

