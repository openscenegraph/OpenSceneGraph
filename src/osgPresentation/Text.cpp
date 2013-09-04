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

#include <osgPresentation/Text>
#include <osg/ValueObject>
#include <osgText/Text>

using namespace osgPresentation;


bool Text::load()
{
    OSG_NOTICE<<"Text::load() Not implemented yet"<<std::endl;

    std::string value;
    getPropertyValue("string", value);

    std::string font("arial.ttf");
    getPropertyValue("font", font);

    double width = 1.0;
    getPropertyValue("width", width);

    double character_size = 0.06;
    getPropertyValue("character_size", character_size);

    OSG_NOTICE<<"Text : string = "<<value<<std::endl;
    OSG_NOTICE<<"       font = "<<font<<std::endl;
    OSG_NOTICE<<"       width = "<<width<<std::endl;
    OSG_NOTICE<<"       character_size = "<<character_size<<std::endl;

    return false;
}

bool Text::getSupportedProperties(PropertyList& pl)
{
    pl.push_back(ObjectDescription(new osg::StringValueObject("string",""), std::string("Text to render.")));
    pl.push_back(ObjectDescription(new osg::StringValueObject("font",""), std::string("Font name.")));
    pl.push_back(ObjectDescription(new osg::DoubleValueObject("width",1.0), std::string("Maximum width of the text.")));
    pl.push_back(ObjectDescription(new osg::DoubleValueObject("character_size",0.06), std::string("Character size.")));
    return true;
}
