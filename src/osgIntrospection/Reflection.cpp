#include <osgIntrospection/Reflection>
#include <osgIntrospection/Exceptions>
#include <osgIntrospection/Type>

#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#include <memory>

using namespace osgIntrospection;

Reflection::StaticData *Reflection::staticdata__ = 0;

const TypeMap &Reflection::getTypes()
{
    return getOrCreateStaticData().typemap;
}

Reflection::StaticData &Reflection::getOrCreateStaticData()
{
    static OpenThreads::Mutex access_mtx;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(access_mtx);

    if (!staticdata__) 
    {                
        staticdata__ = new StaticData;
        std::auto_ptr<Type> tvoid(new Type(typeid(void)));
        staticdata__->typemap.insert(std::make_pair(&typeid(void), tvoid.get()));
        staticdata__->type_void = tvoid.release();            
    }
    return *staticdata__;
}

const Type &Reflection::getType(const std::type_info &ti)
{
    const TypeMap &types = getTypes();

    TypeMap::const_iterator i = types.find(&ti);
    if (i == types.end())
    {
        return *registerType(ti);
    }
    return *i->second;
}

const Type &Reflection::getType(const std::string &qname)
{
    const TypeMap &types = getTypes();

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

const Type &Reflection::type_void()
{
    return *getOrCreateStaticData().type_void;
}

Type *Reflection::registerType(const std::type_info &ti)
{
    std::auto_ptr<Type> type(new Type(ti));
    getOrCreateStaticData().typemap.insert(std::make_pair(&ti, type.get()));
    return type.release();
}

Type *Reflection::getOrRegisterType(const std::type_info &ti, bool replace_if_defined)
{
    TypeMap &tm = getOrCreateStaticData().typemap;
    TypeMap::iterator i = tm.find(&ti);

    if (i != tm.end())
	{
		if (replace_if_defined && i->second->isDefined())
		{
			std::string old_name = i->second->getName();
			std::string old_namespace = i->second->getNamespace();
			std::vector<std::string> old_aliases = i->second->aliases_;

			Type *newtype = new (i->second) Type(ti);
			newtype->name_ = old_name;
			newtype->namespace_ = old_namespace;
			newtype->aliases_.swap(old_aliases);

			return newtype;
		}
		return i->second;
	}

    return registerType(ti);
}
