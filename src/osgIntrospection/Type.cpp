/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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

Type::~Type() 
{ 
    for (PropertyInfoList::const_iterator i=props_.begin(); i!=props_.end(); ++i)
        delete *i;
    for (MethodInfoList::const_iterator i=methods_.begin(); i!=methods_.end(); ++i)
        delete *i;
	for (ConstructorInfoList::const_iterator i=cons_.begin(); i!=cons_.end(); ++i)
		delete *i;

    delete rw_;
	delete cmp_;
}

bool Type::isSubclassOf(const Type &type) const
{
    check_defined();
    for (TypeList::const_iterator i=base_.begin(); i!=base_.end(); ++i)
    {
        if (**i == type.getStdTypeInfo())
            return true;
        if ((*i)->isSubclassOf(type))
            return true;
    }
    return false;
}

const MethodInfo *Type::getCompatibleMethod(const std::string &name, const ValueList &values, bool inherit) const
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
        methods = &methods_;

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

const MethodInfo *Type::getMethod(const std::string &name, const ParameterInfoList &params, bool inherit) const
{
    check_defined();
    for (MethodInfoList::const_iterator j=methods_.begin(); j!=methods_.end(); ++j)
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
        for (TypeList::const_iterator i=base_.begin(); i!=base_.end(); ++i)
        {
            const MethodInfo *mi = (*i)->getMethod(name, params, true);
            if (mi) return mi;
        }
    }

    return 0;
}

void Type::getInheritedProviders(CustomAttributeProviderList &providers) const
{
    check_defined();
    providers.assign(base_.begin(), base_.end());
}

const PropertyInfo *Type::getProperty(const std::string &name, const Type &ptype, const ParameterInfoList &indices, bool inherit) const
{
    check_defined();
    for (PropertyInfoList::const_iterator i=props_.begin(); i!=props_.end(); ++i)
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
        for (TypeList::const_iterator i=base_.begin(); i!=base_.end(); ++i)
        {
            const PropertyInfo *pi = (*i)->getProperty(name, ptype, indices, true);
            if (pi) return pi;
        }
    }

    return 0;
}

Value Type::invokeMethod(const std::string &name, const Value &instance, ValueList &args, bool inherit) const
{
    check_defined();
    const MethodInfo *mi = getCompatibleMethod(name, args, inherit);
    if (!mi) throw MethodNotFoundException(name, name_);
    return mi->invoke(instance, args);
}

Value Type::invokeMethod(const std::string &name, Value &instance, ValueList &args, bool inherit) const
{
    check_defined();
    const MethodInfo *mi = getCompatibleMethod(name, args, inherit);
    if (!mi) throw MethodNotFoundException(name, name_);
    return mi->invoke(instance, args);
}

void Type::getAllProperties(PropertyInfoList &props) const
{
    check_defined();
    std::copy(props_.begin(), props_.end(),    std::back_inserter(props));
    for (TypeList::const_iterator i=base_.begin(); i!=base_.end(); ++i)
    {
        (*i)->getAllProperties(props);
    }
}

void Type::getAllMethods(MethodInfoList &methods) const
{
    check_defined();
    std::copy(methods_.begin(), methods_.end(), std::back_inserter(methods));
    for (TypeList::const_iterator i=base_.begin(); i!=base_.end(); ++i)
    {
        (*i)->getAllMethods(methods);
    }
}

Value Type::createInstance(ValueList &args) const
{
	if (isAbstract())
		throw TypeIsAbstractException(ti_);

	const ConstructorInfo *ci = getCompatibleConstructor(args);
	if (!ci)
		throw ConstructorNotFoundException(ti_);

	return ci->createInstance(args);
}

const ConstructorInfo *Type::getCompatibleConstructor(const ValueList &values) const
{
    check_defined();

    typedef std::vector<ConstructorMatch> MatchList;
    MatchList matches;

    int pos = 0;
    for (ConstructorInfoList::const_iterator j=cons_.begin(); j!=cons_.end(); ++j, ++pos)
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

const ConstructorInfo *Type::getConstructor(const ParameterInfoList &params) const
{
    check_defined();

	for (ConstructorInfoList::const_iterator j=cons_.begin(); j!=cons_.end(); ++j)
    {
		if (areParametersCompatible(params, (*j)->getParameters()))
			return *j;
    }

    return 0;
}
