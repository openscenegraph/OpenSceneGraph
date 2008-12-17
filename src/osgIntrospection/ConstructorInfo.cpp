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

#include <osgIntrospection/ConstructorInfo>

using namespace osgIntrospection;

void ConstructorInfo::getInheritedProviders(CustomAttributeProviderList& providers) const
{
    for (int i=0; i<_declarationType.getNumBaseTypes(); ++i)
    {
        const ConstructorInfo* ci = _declarationType.getBaseType(i).getConstructor(_params);
        if (ci)
        {
            providers.push_back(ci);
        }
    }
}
