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

#include <osgIntrospection/MethodInfo>

using namespace osgIntrospection;

void MethodInfo::getInheritedProviders(CustomAttributeProviderList& providers) const
{
    for (int i=0; i<_declarationType.getNumBaseTypes(); ++i)
    {
        const MethodInfo* mi = _declarationType.getBaseType(i).getMethod(_name, _params, false);
        if (mi)
        {
            providers.push_back(mi);
        }
    }
}

bool MethodInfo::overrides(const MethodInfo* other) const
{
    if (isConst() != other->isConst()) return false;
    if (_declarationType != other->_declarationType) return false;
    if (_rtype != other->_rtype) return false;
    if (_name != other->_name) return false;
    if (_params.size() != other->_params.size()) return false;

    ParameterInfoList::const_iterator i=_params.begin();
    ParameterInfoList::const_iterator j=other->_params.begin();
    for (; i!=_params.end(); ++i, ++j)
    {
        if (&(*i)->getParameterType() != &(*j)->getParameterType())
            return false;
    }

    return true;

/*
    std::size_t num_fixed_1 = 0;
    std::size_t num_optional_1 = 0;
    for (ParameterInfoList::const_iterator i=_params.begin(); i!=_params.end(); ++i)
    {
        if ((*i)->getDefaultValue().isEmpty())
            ++num_fixed_1;
        else
            ++num_optional_1;
    }

    std::size_t num_fixed_2 = 0;
    std::size_t num_optional_2 = 0;
    for (ParameterInfoList::const_iterator i=other->_params.begin(); i!=other->_params.end(); ++i)
    {
        if ((*i)->getDefaultValue().isEmpty())
            ++num_fixed_2;
        else
            ++num_optional_2;
    }

    if (num_fixed_1 > num_fixed_2)
    {
    }
*/
}
