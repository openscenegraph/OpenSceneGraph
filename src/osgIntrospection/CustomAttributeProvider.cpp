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

#include <osgIntrospection/CustomAttributeProvider>
#include <osgIntrospection/Type>

using namespace osgIntrospection;

bool CustomAttributeProvider::isDefined(const Type& type, bool inherit) const
{
    for (CustomAttributeList::const_iterator i=attribs_.begin(); i!=attribs_.end(); ++i)
        if (typeid(**i) == type.getStdTypeInfo()) return true;

    if (inherit)
    {
        CustomAttributeProviderList providers;
        getInheritedProviders(providers);

        for (CustomAttributeProviderList::const_iterator i=providers.begin(); i!=providers.end(); ++i)
        {
            if ((*i)->isDefined(type, true)) return true;
        }
    }

    return false;
}

const CustomAttribute* CustomAttributeProvider::getAttribute(const Type& type, bool inherit) const
{
    for (CustomAttributeList::const_iterator i=attribs_.begin(); i!=attribs_.end(); ++i)
        if (typeid(**i) == type.getStdTypeInfo()) return *i;

    if (inherit)
    {
        CustomAttributeProviderList providers;
        getInheritedProviders(providers);

        for (CustomAttributeProviderList::const_iterator i=providers.begin(); i!=providers.end(); ++i)
        {
            const CustomAttribute* ca = (*i)->getAttribute(type, true);
            if (ca) return ca;
        }
    }

    return 0;
}
