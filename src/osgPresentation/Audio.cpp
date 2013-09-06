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

#include <osgPresentation/Audio>
#include <osg/ValueObject>
#include <osgDB/ReadFile>

using namespace osgPresentation;


bool Audio::load()
{
    OSG_NOTICE<<"Audio::load() Not implemented yet"<<std::endl;

    std::string filename;
    getPropertyValue("filename", filename);

    double volume = 1.0;
    getPropertyValue("volume", volume);

    OSG_NOTICE<<"Audio : filename = "<<filename<<std::endl;
    OSG_NOTICE<<"        volume = "<<volume<<std::endl;

    return false;
}

bool Audio::getSupportedProperties(PropertyList& pl)
{
    pl.push_back(ObjectDescription(new osg::StringValueObject("filename",""), std::string("Audio filename to load.")));
    pl.push_back(ObjectDescription(new osg::DoubleValueObject("volume",1.0), std::string("Audio volume.")));
    return true;
}
