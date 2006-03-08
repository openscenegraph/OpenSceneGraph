//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#ifndef FLT_REGISTRY_H
#define FLT_REGISTRY_H 1

#include <queue>
#include <osg/ref_ptr>
#include "opcodes.h"
#include "Record.h"

namespace flt {

class Registry : public osg::Referenced
{
    public:
        
        ~Registry();
        static Registry* instance();

        // Prototypes
        void addPrototype(int opcode, Record* prototype);
        Record* getPrototype(int opcode);

        // Externals
        typedef std::pair<std::string, osg::Group*> ExtNameNodePair;
	typedef std::queue<ExtNameNodePair> ExternalQueue;

        inline ExternalQueue& getExternalQueue() { return _externalQueue; }

        inline void addExternal(const std::string& name, osg::Group* node)
        {
            _externalQueue.push( ExtNameNodePair(name,node) );
        }

    protected:

        Registry();

        typedef std::map<int, osg::ref_ptr<Record> > RecordProtoMap;
        RecordProtoMap _recordProtoMap;

        ExternalQueue   _externalQueue;
};


/** Proxy class for automatic registration of reader/writers with the Registry.*/
template<class T>
class RegisterRecordProxy
{
    public:

        explicit RegisterRecordProxy(int opcode)
        {
            Registry::instance()->addPrototype(opcode,new T);
        }

        ~RegisterRecordProxy() {}
};

} // end namespace

#endif
