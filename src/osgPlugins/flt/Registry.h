#ifndef __FLT_REGISTRY_H
#define __FLT_REGISTRY_H

#include <vector>
#include <string>

#include "osg/Referenced"

namespace osg {
class Node;
};

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
//      bool apply(Record& rec);
//      Record* readRecord(Input& fr);
//        osg::Node* readNode(Input& fr);

    private:

        typedef std::vector<osg::ref_ptr<Record> >          RecordProtoList;

        /** constructor is private, as its a singleton, preventing
            construction other than via the instance() method and
            therefore ensuring only one copy is ever constructed*/
        Registry();

        RecordProtoList::iterator getRecordProtoItr(const int opcode);

        RecordProtoList          _recordProtoList;
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
