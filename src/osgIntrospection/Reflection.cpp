#include <osgIntrospection/Reflection>
#include <osgIntrospection/Exceptions>
#include <osgIntrospection/Type>
#include <osgIntrospection/Converter>

#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#include <memory>

using namespace osgIntrospection;

Reflection::StaticData* Reflection::_static_data = 0;

Reflection::StaticData::~StaticData()
{
    for (TypeMap::iterator i=typemap.begin(); i!=typemap.end(); ++i)
        delete i->second;

    for (ConverterMapMap::iterator i=convmap.begin(); i!=convmap.end(); ++i)
    {
        for (ConverterMap::iterator j=i->second.begin(); j!=i->second.end(); ++j)
        {
            delete j->second;
        }
    }
}

const TypeMap& Reflection::getTypes()
{
    return getOrCreateStaticData().typemap;
}

Reflection::StaticData& Reflection::getOrCreateStaticData()
{
    static OpenThreads::Mutex access_mtx;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(access_mtx);

    if (!_static_data) 
    {                
        _static_data = new StaticData;
        std::auto_ptr<Type> tvoid(new Type(typeid(void)));
        _static_data->typemap.insert(std::make_pair(&typeid(void), tvoid.get()));
        _static_data->type_void = tvoid.release();            
    }
    return *_static_data;
}

const Type& Reflection::getType(const std::type_info& ti)
{
    const TypeMap& types = getTypes();

    TypeMap::const_iterator i = types.find(&ti);
    if (i == types.end())
    {
        return *registerType(ti);
    }
    return *i->second;
}

const Type& Reflection::getType(const std::string& qname)
{
    const TypeMap& types = getTypes();

    for (TypeMap::const_iterator i=types.begin(); i!=types.end(); ++i)
    {
        if (i->second->isDefined() && i->second->getQualifiedName().compare(qname) == 0)
            return *i->second;
        for (int j=0; j<i->second->getNumAliases(); ++j)
            if (i->second->getAlias(j).compare(qname) == 0)
                return *i->second;
    }

    throw TypeNotFoundException(qname);
}

const Type& Reflection::type_void()
{
    return *getOrCreateStaticData().type_void;
}

Type* Reflection::registerType(const std::type_info& ti)
{
    std::auto_ptr<Type> type(new Type(ti));
    getOrCreateStaticData().typemap.insert(std::make_pair(&ti, type.get()));
    return type.release();
}

Type* Reflection::getOrRegisterType(const std::type_info& ti, bool replace_if_defined)
{
    TypeMap& tm = getOrCreateStaticData().typemap;
    TypeMap::iterator i = tm.find(&ti);

    if (i != tm.end())
    {
        if (replace_if_defined && i->second->isDefined())
        {
            std::string old_name = i->second->getName();
            std::string old_namespace = i->second->getNamespace();
            std::vector<std::string> old_aliases = i->second->_aliases;

            Type* newtype = new (i->second) Type(ti);
            newtype->_name = old_name;
            newtype->_namespace = old_namespace;
            newtype->_aliases.swap(old_aliases);

            return newtype;
        }
        return i->second;
    }

    return registerType(ti);
}

void Reflection::registerConverter(const Type& source, const Type& dest, const Converter* cvt)
{
    getOrCreateStaticData().convmap[&source][&dest] = cvt;
}

const Converter* Reflection::getConverter(const Type& source, const Type& dest)
{
    return getOrCreateStaticData().convmap[&source][&dest];
}

bool Reflection::getConversionPath(const Type& source, const Type& dest, ConverterList& conv)
{
    ConverterList temp;
    std::vector<const Type* > chain;
    if (accum_conv_path(source, dest, temp, chain))
    {
        conv.swap(temp);
        return true;
    }
    return false;
}

bool Reflection::accum_conv_path(const Type& source, const Type& dest, ConverterList& conv, std::vector<const Type* > &chain)
{
    // break unwanted loops
    if (std::find(chain.begin(), chain.end(), &source) != chain.end())
        return false;

    // store the type being processed to avoid loops
    chain.push_back(&source);

    StaticData::ConverterMapMap::const_iterator i = getOrCreateStaticData().convmap.find(&source);
    if (i == getOrCreateStaticData().convmap.end()) 
        return false;

    const StaticData::ConverterMap& cmap = i->second;
    StaticData::ConverterMap::const_iterator j = cmap.find(&dest);
    if (j != cmap.end())
    {
        conv.push_back(j->second);
        return true;
    }

    for (j=cmap.begin(); j!=cmap.end(); ++j)
    {
        if (accum_conv_path(*j->first, dest, conv, chain))
            return true;
    }

    return false;
}
