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

        ~Registry();

        static Registry* instance();

        void addPrototype(Record* rec);
        void removePrototype(Record* rec);
        Record* getRecordProto(const int opcode);

        void addTexture(osg::Texture* texture);
        void removeTexture(osg::Texture* texture);
        osg::Texture* getTexture(const std::string name);

        void addFltFile(const std::string& name, FltFile* file);
        FltFile* getFltFile(const std::string& name);

    private:

        typedef std::vector<osg::ref_ptr<Record> >          RecordProtoList;
        typedef std::vector<osg::Texture*> TextureList;
        typedef std::map<std::string,osg::ref_ptr<FltFile> > FltFileMap;

        /** constructor is private, as its a singleton, preventing
            construction other than via the instance() method and
            therefore ensuring only one copy is ever constructed*/
        Registry();

        RecordProtoList::iterator getRecordProtoItr(const int opcode);
        TextureList::iterator getTextureItr(const std::string name);

        RecordProtoList  _recordProtoList;
        TextureList      _textureList;
        FltFileMap       _fltFileMap;
};


/**  Proxy class for automatic registration of reader/writers with the
     Registry.*/
template<class T>
class RegisterRecordProxy
{
    public:
        RegisterRecordProxy()
        {
            _obj = new T;
            _obj->ref();
            Registry::instance()->addPrototype(_obj);
        }
        ~RegisterRecordProxy()
        {
            //commented out to prevent seg fault under Linux 
            //due to the registry being previously destructed.
            //Registry::instance()->removePrototype(_obj);
            _obj->unref();
        }
        
    protected:
        T* _obj;
};



}; // end namespace flt

#endif // __FLT_REGISTRY_H
