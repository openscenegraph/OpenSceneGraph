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

#include <osgIntrospection/Type>
#include <osgIntrospection/Value>
#include <osgIntrospection/Reflection>
#include <osgIntrospection/PropertyInfo>
#include <osgIntrospection/MethodInfo>
#include <osgIntrospection/ReaderWriter>
#include <osgIntrospection/Utility>
#include <osgIntrospection/ConstructorInfo>
#include <osgIntrospection/Comparator>

#include <iterator>
#include <algorithm>

using namespace osgIntrospection;

namespace
{

    template<typename T>
    struct ObjectMatch
    {
        int list_pos;
        float match;
        const T *object;

        bool operator < (const ObjectMatch &m) const
        {
            if (match > m.match) return true;
            if (match < m.match) return false;
            if (list_pos < m.list_pos) return true;
            return false;
        }
    };

    typedef ObjectMatch<MethodInfo> MethodMatch;
    typedef ObjectMatch<ConstructorInfo> ConstructorMatch;

}

void Type::reset()
{
    for (PropertyInfoList::const_iterator i=_props.begin(); i!=_props.end(); ++i)
        delete *i;
    for (MethodInfoList::const_iterator i=_methods.begin(); i!=_methods.end(); ++i)
        delete *i;
    for (MethodInfoList::const_iterator i=_protected_methods.begin(); i!=_protected_methods.end(); ++i)
        delete *i;
    for (ConstructorInfoList::const_iterator i=_cons.begin(); i!=_cons.end(); ++i)
        delete *i;
    for (ConstructorInfoList::const_iterator i=_protected_cons.begin(); i!=_protected_cons.end(); ++i)
        delete *i;

    _props.clear();
    _methods.clear();
    _protected_methods.clear();
    _cons.clear();
    _protected_cons.clear();

    delete _rw;
    delete _cmp;
}

Type::~Type() 
{ 
    reset();
}

bool Type::isSubclassOf(const Type& type) const
{
    check_defined();
    for (TypeList::const_iterator i=_base.begin(); i!=_base.end(); ++i)
    {
        if ((*i)->getExtendedTypeInfo() == type.getExtendedTypeInfo())
            return true;
        if ((*i)->isSubclassOf(type))
            return true;
    }
    return false;
}

const MethodInfo *Type::getCompatibleMethod(const std::string& name, const ValueList& values, bool inherit) const
{
    check_defined();

    MethodInfoList allmethods;
    const MethodInfoList *methods;
    if (inherit)
    {        
        getAllMethods(allmethods);
        methods = &allmethods;
    }
    else
        methods = &_methods;

    typedef std::vector<MethodMatch> MatchList;
    MatchList matches;

    int pos = 0;
    for (MethodInfoList::const_iterator j=methods->begin(); j!=methods->end(); ++j, ++pos)
    {
        const MethodInfo *mi = *j;
        if (mi->getName().compare(name) == 0)
        {
            float match;
            if (areArgumentsCompatible(values, mi->getParameters(), match))
            {
                MethodMatch mm;
                mm.list_pos = pos;
                mm.match = match;
                mm.object = mi;
                matches.push_back(mm);
            }
        }
    }

    if (!matches.empty())
    {
        std::sort(matches.begin(), matches.end());
        return matches.front().object;
    }

    return 0;
}

const MethodInfo *Type::getMethod(const std::string& name, const ParameterInfoList& params, bool inherit) const
{
    check_defined();
    for (MethodInfoList::const_iterator j=_methods.begin(); j!=_methods.end(); ++j)
    {
        const MethodInfo *mi = *j;
        if (mi->getName().compare(name) == 0)
        {
            if (areParametersCompatible(params, mi->getParameters()))
            {
                return mi;
            }
        }
    }

    if (inherit)
    {
        for (TypeList::const_iterator i=_base.begin(); i!=_base.end(); ++i)
        {
            const MethodInfo *mi = (*i)->getMethod(name, params, true);
            if (mi) return mi;
        }
    }

    return 0;
}

void Type::getInheritedProviders(CustomAttributeProviderList& providers) const
{
    check_defined();
    providers.assign(_base.begin(), _base.end());
}

const PropertyInfo *Type::getProperty(const std::string& name, const Type& ptype, const ParameterInfoList& indices, bool inherit) const
{
    check_defined();
    for (PropertyInfoList::const_iterator i=_props.begin(); i!=_props.end(); ++i)
    {
        const PropertyInfo *pi = *i;
        if (pi->getName() == name && pi->getPropertyType() == ptype)
        {
            if (areParametersCompatible(indices, pi->getIndexParameters()))
            {
                return pi;
            }
        }
    }

    if (inherit)
    {
        for (TypeList::const_iterator i=_base.begin(); i!=_base.end(); ++i)
        {
            const PropertyInfo *pi = (*i)->getProperty(name, ptype, indices, true);
            if (pi) return pi;
        }
    }

    return 0;
}

Value Type::invokeMethod(const std::string& name, const Value& instance, ValueList& args, bool inherit) const
{
    check_defined();
    const MethodInfo *mi = getCompatibleMethod(name, args, inherit);
    if (!mi) throw MethodNotFoundException(name, _name);
    return mi->invoke(instance, args);
}

Value Type::invokeMethod(const std::string& name, Value& instance, ValueList& args, bool inherit) const
{
    check_defined();
    const MethodInfo *mi = getCompatibleMethod(name, args, inherit);
    if (!mi) throw MethodNotFoundException(name, _name);
    return mi->invoke(instance, args);
}

void Type::getAllProperties(PropertyInfoList& props) const
{
    check_defined();
    std::copy(_props.begin(), _props.end(),    std::back_inserter(props));
    for (TypeList::const_iterator i=_base.begin(); i!=_base.end(); ++i)
    {
        (*i)->getAllProperties(props);
    }
}

void Type::getPropertiesMap(PropertyInfoMap& props) const
{
    check_defined();
    props[this] = _props;
    for (TypeList::const_iterator i=_base.begin(); i!=_base.end(); ++i)
    {
        (*i)->getPropertiesMap(props);
    }
}

void Type::getAllMethods(MethodInfoList& methods, FunctionCategory category) const
{
    check_defined();
    const MethodInfoList& input_methods = (category == PUBLIC_FUNCTIONS ? _methods : _protected_methods);
    std::copy(input_methods.begin(), input_methods.end(), std::back_inserter(methods));
    for (TypeList::const_iterator i=_base.begin(); i!=_base.end(); ++i)
    {
        (*i)->getAllMethods(methods, category);
    }
}

void Type::getMethodsMap(MethodInfoMap& methods, FunctionCategory category) const
{
    check_defined();
    methods[this] = (category == PUBLIC_FUNCTIONS ? _methods : _protected_methods);
    for (TypeList::const_iterator i=_base.begin(); i!=_base.end(); ++i)
    {
        (*i)->getMethodsMap(methods, category);
    }
}

Value Type::createInstance(ValueList& args) const
{
    if (isAbstract())
        throw TypeIsAbstractException(_ti);

    const ConstructorInfo *ci = getCompatibleConstructor(args);
    if (!ci)
        throw ConstructorNotFoundException(_ti);

    return ci->createInstance(args);
}

const ConstructorInfo *Type::getCompatibleConstructor(const ValueList& values) const
{
    check_defined();

    typedef std::vector<ConstructorMatch> MatchList;
    MatchList matches;

    int pos = 0;
    for (ConstructorInfoList::const_iterator j=_cons.begin(); j!=_cons.end(); ++j, ++pos)
    {
        float match;
        if (areArgumentsCompatible(values, (*j)->getParameters(), match))
        {
            ConstructorMatch mm;
            mm.list_pos = pos;
            mm.match = match;
            mm.object = *j;
            matches.push_back(mm);
        }
    }

    if (!matches.empty())
    {
        std::sort(matches.begin(), matches.end());
        return matches.front().object;
    }

    return 0;
}

const ConstructorInfo *Type::getConstructor(const ParameterInfoList& params) const
{
    check_defined();

    for (ConstructorInfoList::const_iterator j=_cons.begin(); j!=_cons.end(); ++j)
    {
        if (areParametersCompatible(params, (*j)->getParameters()))
            return *j;
    }

    return 0;
}
