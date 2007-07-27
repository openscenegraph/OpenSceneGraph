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

#include <osgIntrospection/Reflector>

namespace osgIntrospection
{
    template<>
    void Reflector<void>::init_reference_types()
    {
        // Avoid trying to register void & / const void &, which are
        // illegal types.
    }
    template<>
    void Reflector<void>::init_void_converter()
    {
        // Avoid 
    }

}
