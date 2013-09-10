/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2013 Robert Osfield
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

#include <osgPresentation/Presentation>

using namespace osgPresentation;

bool Presentation::getSupportedProperties(PropertyList& pl)
{
    pl.push_back(ObjectDescription(new osg::Vec4dValueObject("background_colour",osg::Vec4d(0.0,0.0,0.0,0.0)), std::string("Background colour.")));
    pl.push_back(ObjectDescription(new osg::Vec4dValueObject("text_colour",osg::Vec4d(1.0,1.0,1.0,1.0)), std::string("Text colour.")));
    return true;
}
