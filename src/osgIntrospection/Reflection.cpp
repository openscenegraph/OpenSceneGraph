/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 */
//osgIntrospection - Copyright (C) 2005 Marco Jez

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
        std::auto_ptr<Type> tvoid(new Type(extended_typeid<void>()));
        _static_data->typemap.insert(std::make_pair(extended_typeid<void>(), tvoid.get()));
        _static_data->type_void = tvoid.release();
    }
    return *_static_data;
}

const Type& Reflection::getType(const ExtendedTypeInfo &ti)
{
    const TypeMap& types = getTypes();

    TypeMap::const_iterator i = types.find(ti);
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

Type* Reflection::registerType(const ExtendedTypeInfo &ti)
{
    std::auto_ptr<Type> type(new Type(ti));
    getOrCreateStaticData().typemap.insert(std::make_pair(ti, type.get()));
    return type.release();
}

Type* Reflection::getOrRegisterType(const ExtendedTypeInfo &ti, bool replace_if_defined)
{
    TypeMap& tm = getOrCreateStaticData().typemap;
    TypeMap::iterator i = tm.find(ti);

    if (i != tm.end())
    {
        if (replace_if_defined && i->second->isDefined())
        {
            std::string old_name = i->second->getName();
            std::string old_namespace = i->second->getNamespace();
            std::vector<std::string> old_aliases = i->second->_aliases;

            i->second->reset();
            i->second->_name = old_name;
            i->second->_namespace = old_namespace;
            i->second->_aliases.swap(old_aliases);
        }
        return i->second;
    }

    return registerType(ti);
}

void Reflection::registerConverter(const Type& source, const Type& dest, const Converter* cvt)
{
    const Converter* old = NULL;
    StaticData::ConverterMap::iterator it = getOrCreateStaticData().convmap[&source].find(&dest);

    if(it != getOrCreateStaticData().convmap[&source].end())
        old = it->second;

    getOrCreateStaticData().convmap[&source][&dest] = cvt;
    
    if(old)
        delete old;
}

const Converter* Reflection::getConverter(const Type& source, const Type& dest)
{
    return getOrCreateStaticData().convmap[&source][&dest];
}

bool Reflection::getConversionPath(const Type& source, const Type& dest, ConverterList& conv)
{
    ConverterList temp;
    std::vector<const Type* > chain;

    if (accum_conv_path(source, dest, temp, chain, STATIC_CAST))
    {
        conv.swap(temp);
        return true;
    }

    if (source.isPointer() && dest.isPointer())
    {
        chain.clear();
        temp.clear();
        if (accum_conv_path(source, dest, temp, chain, DYNAMIC_CAST))
        {
            conv.swap(temp);
            return true;
        }
    }

    return false;
}

bool Reflection::accum_conv_path(const Type& source, const Type& dest, ConverterList& conv, std::vector<const Type* > &chain, CastType castType)
{
    // break unwanted loops
    if (std::find(chain.begin(), chain.end(), &source) != chain.end())
        return false;

    // store the type being processed to avoid loops
    chain.push_back(&source);

    // search a converter from "source"
    StaticData::ConverterMapMap::const_iterator i = getOrCreateStaticData().convmap.find(&source);
    if (i == getOrCreateStaticData().convmap.end())
        return false;

    // search a converter to "dest"
    const StaticData::ConverterMap& cmap = i->second;
    StaticData::ConverterMap::const_iterator j = cmap.find(&dest);
    if (j != cmap.end() && (j->second->getCastType() == castType))
    {
        conv.push_back(j->second);
        return true;
    }

    // search a undirect converter from "source" to ... to "dest"
    for (j=cmap.begin(); j!=cmap.end(); ++j)
    {
        if ((j->second->getCastType() == castType) && accum_conv_path(*j->first, dest, conv, chain, castType))
        {
            conv.push_front(j->second);
            return true;
        }
    }

    return false;
}

void Reflection::uninitialize() {
    if(_static_data)
        delete _static_data;
    _static_data = 0;
}
