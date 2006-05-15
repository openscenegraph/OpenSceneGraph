//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#ifndef FLT_REGISTRY_H
#define FLT_REGISTRY_H 1

#include <queue>
#include <map>
#include <osg/ref_ptr>
#include "opcodes.h"
#include "Record.h"

namespace flt {

class Registry : public osg::Referenced
{
    public:
        
        ~Registry();
        static Registry* instance();

        // Record prototypes
        void addPrototype(int opcode, Record* prototype);
        Record* getPrototype(int opcode);

        // External read queue
        typedef std::pair<std::string, osg::Group*> FilenameParentPair; // ExtNameNodePair;
        typedef std::queue<FilenameParentPair> ExternalQueue;

        inline ExternalQueue& getExternalReadQueue() { return _externalReadQueue; }
        void addToExternalReadQueue(const std::string& filename, osg::Group* parent);

        // Local cache
        void addToLocalCache(const std::string& filename, osg::Node* node);
        osg::Node* getFromLocalCache(const std::string& filename);
        void clearLocalCache();

    protected:

        Registry();

        typedef std::map<int, osg::ref_ptr<Record> > RecordProtoMap;
        RecordProtoMap     _recordProtoMap;

        ExternalQueue      _externalReadQueue;

        typedef std::map<std::string, osg::ref_ptr<osg::Node> > ExternalCacheMap;
        ExternalCacheMap   _externalCacheMap;
};

inline void Registry::addToExternalReadQueue(const std::string& filename, osg::Group* parent)
{
    _externalReadQueue.push( FilenameParentPair(filename,parent) );
}

inline void Registry::addToLocalCache(const std::string& filename, osg::Node* node)
{
    _externalCacheMap[filename] = node;
}

inline osg::Node* Registry::getFromLocalCache(const std::string& filename)
{
    ExternalCacheMap::iterator itr = _externalCacheMap.find(filename);
    if (itr != _externalCacheMap.end())
        return (*itr).second.get();
    return NULL;
}

inline void Registry::clearLocalCache()
{
    _externalCacheMap.clear();
}

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
