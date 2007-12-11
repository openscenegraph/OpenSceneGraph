#ifndef _READERWRITERDAE_H_
#define _READERWRITERDAE_H_


#include <OpenThreads/ReentrantMutex>

///////////////////////////////////////////////////////////////////////////
// OSG reader/writer plugin for the COLLADA 1.4.x ".dae" format.
// See http://collada.org/ and http://khronos.org/collada/

#define EXTENSION_NAME "dae"

class ReaderWriterDAE : public osgDB::ReaderWriter
{
public:
    ReaderWriterDAE()
    {
    }

    const char* className() const { return "COLLADA 1.4.x DAE reader/writer"; }

    bool acceptsExtension(const std::string& extension) const
    { 
        return osgDB::equalCaseInsensitive( extension, EXTENSION_NAME );
    }

    ReadResult readNode(const std::string&, const Options*) const;

    WriteResult writeNode(const osg::Node&, const std::string&, const Options*) const;

    static std::string ConvertFilePathToColladaCompatibleURI(const std::string& FilePath);
  
private:
    mutable OpenThreads::ReentrantMutex _serializerMutex;
};

///////////////////////////////////////////////////////////////////////////

#endif

