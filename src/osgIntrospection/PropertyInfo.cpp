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

#include <osgIntrospection/PropertyInfo>
#include <osgIntrospection/Attributes>
#include <osgIntrospection/variant_cast>

using namespace osgIntrospection;

void PropertyInfo::getInheritedProviders(CustomAttributeProviderList& providers) const
{
    for (int i=0; i<_decltype.getNumBaseTypes(); ++i)
    {
        const PropertyInfo* pi = _decltype.getBaseType(i).getProperty(_name, _ptype, getIndexParameters(), false);
        if (pi)
        {
            providers.push_back(pi);
        }
    }
}

Value PropertyInfo::getValue(const Value& instance) const
{
    const PropertyTypeAttribute *pta = getAttribute<PropertyTypeAttribute>(false);
    const CustomPropertyGetAttribute *cget = getAttribute<CustomPropertyGetAttribute>(false);

    if (cget) 
    {
        if (pta)
            return cget->getGetter()->get(instance).convertTo(pta->getPropertyType());
        return cget->getGetter()->get(instance);
    }

    if (!_getm)
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::GET);

    if (pta)
        return _getm->invoke(instance).convertTo(pta->getPropertyType());
    return _getm->invoke(instance);
}

Value PropertyInfo::getValue(Value& instance) const
{
    const PropertyTypeAttribute *pta = getAttribute<PropertyTypeAttribute>(false);
    const CustomPropertyGetAttribute *cget = getAttribute<CustomPropertyGetAttribute>(false);

    if (cget) 
    {
        if (pta)
            return cget->getGetter()->get(instance).convertTo(pta->getPropertyType());
        return cget->getGetter()->get(instance);
    }

    if (!_getm)
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::GET);

    if (pta)
        return _getm->invoke(instance).convertTo(pta->getPropertyType());
    return _getm->invoke(instance);
}

void PropertyInfo::setValue(Value& instance, const Value& value) const
{
    const CustomPropertySetAttribute *cset = getAttribute<CustomPropertySetAttribute>(false);

    if (cset) 
    {
        cset->getSetter()->set(instance, value);
        return;
    }

    if (!_setm)
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::SET);

    ValueList args;
    args.push_back(value);
    _setm->invoke(instance, args);
}

Value PropertyInfo::getIndexedValue(const Value& instance, ValueList& args) const
{
    const PropertyTypeAttribute *pta = getAttribute<PropertyTypeAttribute>(false);
    const CustomPropertyGetAttribute *cget = getAttribute<CustomPropertyGetAttribute>(false);

    if (cget) 
    {
        if (pta)
            return cget->getGetter()->get(instance, args).convertTo(pta->getPropertyType());
        return cget->getGetter()->get(instance, args);
    }

    if (!_getm) 
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::IGET);

    if (pta)
        return _getm->invoke(instance, args).convertTo(pta->getPropertyType());
    return _getm->invoke(instance, args);
}

Value PropertyInfo::getIndexedValue(Value& instance, ValueList& args) const
{
    const PropertyTypeAttribute *pta = getAttribute<PropertyTypeAttribute>(false);
    const CustomPropertyGetAttribute *cget = getAttribute<CustomPropertyGetAttribute>(false);

    if (cget) 
    {
        if (pta)
            return cget->getGetter()->get(instance, args).convertTo(pta->getPropertyType());
        return cget->getGetter()->get(instance, args);
    }

    if (!_getm) 
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::IGET);

    if (pta)
        return _getm->invoke(instance, args).convertTo(pta->getPropertyType());
    return _getm->invoke(instance, args);
}

void PropertyInfo::setIndexedValue(Value& instance, ValueList& args, const Value& value) const
{
    const CustomPropertySetAttribute *cset = getAttribute<CustomPropertySetAttribute>(false);
    if (cset) 
    {
        cset->getSetter()->set(instance, args, value);
        return;
    }

    if (!_setm) 
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::ISET);

    args.push_back(value);
    _setm->invoke(instance, args);
    args.pop_back();
}

int PropertyInfo::getNumArrayItems(const Value& instance) const
{
    const CustomPropertyCountAttribute *ccount = getAttribute<CustomPropertyCountAttribute>(false);
    if (ccount) return ccount->getCounter()->count(instance);

    if (!_numm)
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::COUNT);

    return variant_cast<int>(_numm->invoke(instance));
}

Value PropertyInfo::getArrayItem(const Value& instance, int i) const
{
    const PropertyTypeAttribute *pta = getAttribute<PropertyTypeAttribute>(false);
    const CustomPropertyGetAttribute *cget = getAttribute<CustomPropertyGetAttribute>(false);

    if (cget) 
    {
        if (pta)
            return cget->getGetter()->get(instance, i).convertTo(pta->getPropertyType());
        return cget->getGetter()->get(instance, i);
    }

    if (!_getm) 
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::AGET);

    ValueList args;
    args.push_back(i);

    if (pta)
        return _getm->invoke(instance, args).convertTo(pta->getPropertyType());
    return _getm->invoke(instance, args);
}

Value PropertyInfo::getArrayItem(Value& instance, int i) const
{
    const PropertyTypeAttribute *pta = getAttribute<PropertyTypeAttribute>(false);
    const CustomPropertyGetAttribute *cget = getAttribute<CustomPropertyGetAttribute>(false);

    if (cget) 
    {
        if (pta)
            return cget->getGetter()->get(instance, i).convertTo(pta->getPropertyType());
        return cget->getGetter()->get(instance, i);
    }

    if (!_getm) 
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::AGET);

    ValueList args;
    args.push_back(i);

    if (pta)
        return _getm->invoke(instance, args).convertTo(pta->getPropertyType());
    return _getm->invoke(instance, args);
}

void PropertyInfo::setArrayItem(Value& instance, int i, const Value& value) const
{
    const CustomPropertySetAttribute *cset = getAttribute<CustomPropertySetAttribute>(false);
    if (cset) 
    {
        cset->getSetter()->set(instance, i, value);
        return;
    }
    
    if (!_setm) 
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::ASET);

    ValueList args;
    args.push_back(i);
    args.push_back(value);
    _setm->invoke(instance, args);
}

void PropertyInfo::addArrayItem(Value& instance, const Value& value) const
{
    const CustomPropertyAddAttribute *cadd = getAttribute<CustomPropertyAddAttribute>(false);
    if (cadd) 
    {
        cadd->getAdder()->add(instance, value);
        return;
    }

    if (!_addm) 
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::ADD);

    ValueList args;
    args.push_back(value);
    _addm->invoke(instance, args);
}

void PropertyInfo::removeArrayItem(Value& instance, int i) const
{
    const CustomPropertyRemoveAttribute *crem = getAttribute<CustomPropertyRemoveAttribute>(false);
    if (crem) 
    {
        crem->getRemover()->remove(instance, i);
        return;
    }

    if (!_remm) 
        throw PropertyAccessException(_decltype.getQualifiedName() + "::" + _name, PropertyAccessException::REMOVE);

    ValueList args;
    args.push_back(i);
    _remm->invoke(instance, args);
}

Value PropertyInfo::getDefaultValue() const
{
    if (isArray() || isIndexed()) return Value();

    const CustomAttributeList& cal = getCustomAttributes();
    for (CustomAttributeList::const_iterator i=cal.begin(); i!=cal.end(); ++i)
    {
        if (dynamic_cast<const NoDefaultValueAttribute *>(*i) != 0) 
            return Value();

        const DefaultValueAttribute *dv = dynamic_cast<const DefaultValueAttribute *>(*i);
        if (dv)
        {
            return dv->getDefaultValue();
        }
    }

    if (_decltype.isAbstract()) 
    {
        if (_ptype.isAbstract() || !_ptype.isDefined())
            return Value();
        return _ptype.createInstance();
    }

    // auto default value
    Value instance = _decltype.createInstance();
    return getValue(instance);
}

void PropertyInfo::getIndexValueSet(int whichindex, const Value& instance, ValueList& values) const
{
    const CustomIndexAttribute *cia = getAttribute<CustomIndexAttribute>(false);
    if (cia)
    {
        cia->getIndexInfo()->getIndexValueSet(whichindex, instance, values);
    }
    else
    {    
        std::map<int, const IndexTypeAttribute *> ita_map;
        const CustomAttributeList& cal = getCustomAttributes();
        for (CustomAttributeList::const_iterator i=cal.begin(); i!=cal.end(); ++i)
        {
            const IndexTypeAttribute *ita = dynamic_cast<const IndexTypeAttribute *>(*i);
            if (ita)
                ita_map[ita->getWhichIndex()] = ita;
        }

        const EnumLabelMap& elm = getIndexParameters().at(whichindex)->getParameterType().getEnumLabels();
        if (elm.empty())
            throw IndexValuesNotDefinedException(_name, getIndexParameters().at(whichindex)->getName());

        for (EnumLabelMap::const_iterator i=elm.begin(); i!=elm.end(); ++i)
        {
            if (ita_map[whichindex])
                values.push_back(Value(i->first).convertTo(ita_map[whichindex]->getIndexType()));
            else
                values.push_back(Value(i->first).convertTo(_indices[whichindex]->getParameterType()));
        }
    }
}

const ParameterInfoList& PropertyInfo::getIndexParameters() const
{
    const CustomIndexAttribute *cia = getAttribute<CustomIndexAttribute>(false);
    if (cia)
    {
        return cia->getIndexInfo()->getIndexParameters();
    }

    return _indices;
}
