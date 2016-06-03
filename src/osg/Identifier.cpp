#include <osg/Identifier>

#include <osg/ref_ptr>
#include <osg/Notify>

#include <OpenThreads/Mutex>

#include <map>

namespace osg
{

struct IdentifierKey
{
    IdentifierKey(const std::string& str, int num, osg::Referenced* f, osg::Referenced* s):
        name(str),
        number(num),
        first(f),
        second(s)
    {
    }

    const std::string       name;
    const int               number;
    const osg::Referenced*  first;
    const osg::Referenced*  second;

    bool operator < (const IdentifierKey& rhs) const
    {
        if (name<rhs.name) return true;
        if (name>rhs.name) return false;

        if (number<rhs.number) return true;
        if (number>rhs.number) return false;

        if (first<rhs.first) return true;
        if (first>rhs.first) return false;

        return (second<rhs.second);
    }
};


typedef std::map<IdentifierKey, osg::ref_ptr<Identifier> > IdentifierMap;

static IdentifierMap  s_IdentifierMap;
static OpenThreads::Mutex s_IdentifierMapMutex;

Identifier::Identifier(const std::string& name, int number, osg::Referenced* f, osg::Referenced* s):
    _name(name),
    _number(number),
    _first(f),
    _second(s)
{
    if (_first) _first->addObserver(this);
    if (_second) _second->addObserver(this);
}

Identifier::~Identifier()
{
    if (_first) _first->removeObserver(this);
    if (_second) _second->removeObserver(this);
}

void Identifier::objectDeleted(void* ptr)
{
    if (_first==ptr || _second==ptr)
    {
        IdentifierKey key(_name, _number, _first, _second);

        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_IdentifierMapMutex);
            IdentifierMap::iterator itr = s_IdentifierMap.find(key);
            if (itr!=s_IdentifierMap.end())
            {
               //  OSG_NOTICE<<"Warning : this = "<<this<<"  Identifier::objectDeleted("<<ptr<<") found and deleted from s_IdentifierMap, s_IdentifierMap.size()="<<s_IdentifierMap.size()<<std::endl;
                s_IdentifierMap.erase(itr);
            }
            else
            {
               //  OSG_NOTICE<<"Warning : this = "<<this<<"  Identifier::objectDeleted("<<ptr<<") not found in s_IdentifierMap."<<s_IdentifierMap.size()<<std::endl;
            }
        }

        if (_first==ptr && _second)
        {
            _second->removeObserver(this);
        }

        if (_second==ptr && _first)
        {
            _first->removeObserver(this);
        }

        _first = 0;
        _second = 0;
    }

}


Identifier* osg::Identifier::get(const std::string& name, int number, osg::Referenced* first, osg::Referenced* second)
{
    IdentifierKey key(name, number, first, second);

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_IdentifierMapMutex);

    IdentifierMap::iterator itr = s_IdentifierMap.find(key);
    if (itr!=s_IdentifierMap.end()) return itr->second.get();

    osg::ref_ptr<Identifier> identifier = new Identifier(name, number, first, second);
    s_IdentifierMap[key] = identifier;
    return identifier.get();
}

Identifier* osg::Identifier::get(int number, osg::Referenced* first, osg::Referenced* second)
{
    return get("", number, first, second);
}

Identifier* osg::Identifier::get(osg::Referenced* first, osg::Referenced* second)
{
    return get("", 0, first, second);
}

}
