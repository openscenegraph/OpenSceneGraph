#ifndef __FLT_REGISTRY_H
#define __FLT_REGISTRY_H

#ifdef WIN32
#pragma warning( disable : 4786 )
#endif

#include <osg/Referenced>
#include <osg/ref_ptr>
#include <osg/Texture>

#include "FltFile.h"
 
#include <map>
#include <vector>
#include <string>

namespace flt {

// forward declare referenced classes to help avoid mutiple includes
class Record;
class Input;

/**
    Registry is a singleton factory which stores
    the Objects types available at runtime for loading,
    and any Object reader or writers which are linked in
    at runtime for reading non-native file formats.

    The RegisterObjectProxy defined in Object.h can be
    used to automatically register at runtime a Object
    with the Registry.

    The RegisterReaderWriterProxy defined in ReaderWriter.h can
    be used to automatically register at runtime a reader/writer
    with the Registry.
*/

class Registry
{
    public:

        ~Registry() {}

        static Registry* instance();

        void addPrototype(Record* rec);
        Record* getPrototype(const int opcode);

        void addTexture(const std::string& name, osg::Texture* texture);
        osg::Texture* getTexture(const std::string name);

        void addFltFile(const std::string& name, FltFile* file);
        FltFile* getFltFile(const std::string& name);

        inline void setVersion(int ver) { _fltFileVersion = ver; }
        inline int  getVersion() const  { return _fltFileVersion; }

    private:

        typedef std::map<int, osg::ref_ptr<Record> > RecordProtoMap;
        typedef std::map<std::string, osg::ref_ptr<osg::Texture> > TextureMap;
        typedef std::map<std::string, osg::ref_ptr<FltFile> > FltFileMap;

        /** constructor is private, as its a singleton, preventing
            construction other than via the instance() method and
            therefore ensuring only one copy is ever constructed*/
        Registry() {}

        RecordProtoMap  _recordProtoMap;
        TextureMap      _textureMap;
        FltFileMap      _fltFileMap;
        int             _fltFileVersion;

};


/**  Proxy class for automatic registration of reader/writers with the
     Registry.*/
template<class T>
class RegisterRecordProxy
{
    public:

        RegisterRecordProxy()
        {
            if (Registry::instance())
            {
                _obj = new T;
                Registry::instance()->addPrototype(_obj.get());
            }
        }

        ~RegisterRecordProxy() {}
        
    protected:

        osg::ref_ptr<T> _obj;

};



}; // end namespace flt

#endif // __FLT_REGISTRY_H

