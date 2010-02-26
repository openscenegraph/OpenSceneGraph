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

