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

#include <osgPresentation/Volume>
#include <osg/ValueObject>
#include <osgVolume/VolumeTile>
#include <osgDB/ReadFile>

using namespace osgPresentation;


bool Volume::load()
{
    OSG_NOTICE<<"Volume::load() Not implemented yet"<<std::endl;

    std::string filename;
    getPropertyValue("filename", filename);

    double scale = 1.0;
    getPropertyValue("scale", scale);

    std::string technique;
    getPropertyValue("technique", technique);

    OSG_NOTICE<<"Volume : filename = "<<filename<<std::endl;
    OSG_NOTICE<<"         technique = "<<technique<<std::endl;
    OSG_NOTICE<<"         scale = "<<scale<<std::endl;

    osg::ref_ptr<osg::Object> object = osgDB::readObjectFile(filename);
    if (object.valid())
    {
        osg::ref_ptr<osgVolume::VolumeTile> volume = dynamic_cast<osgVolume::VolumeTile*>(object.get());
        if (volume.valid()) addChild(volume.get());
    }

    return false;
}

bool Volume::getSupportedProperties(PropertyList& pl)
{
    pl.push_back(ObjectDescription(new osg::StringValueObject("filename",""), std::string("Volume filename to load.")));
    pl.push_back(ObjectDescription(new osg::StringValueObject("technique",""), std::string("Volume technique to use when rendering.")));
    pl.push_back(ObjectDescription(new osg::DoubleValueObject("scale",1.0), std::string("Volume scale.")));
    return true;
}
