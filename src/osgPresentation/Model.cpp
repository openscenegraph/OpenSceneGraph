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

#include <osgPresentation/Model>
#include <osg/ValueObject>
#include <osgDB/ReadFile>

using namespace osgPresentation;


bool Model::load()
{
    OSG_NOTICE<<"Model::load() Not implemented yet"<<std::endl;

    std::string filename;
    getPropertyValue("filename", filename);

    double scale = 1.0;
    getPropertyValue("scale", scale);

    double character_size = 0.06;
    getPropertyValue("character_size", character_size);

    OSG_NOTICE<<"Model : filename = "<<filename<<std::endl;
    OSG_NOTICE<<"       scale = "<<scale<<std::endl;

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(filename);
    if (model.valid())
    {
        addChild(model.get());
    }

    return false;
}

bool Model::getSupportedProperties(PropertyList& pl)
{
    pl.push_back(ObjectDescription(new osg::StringValueObject("filename",""), std::string("Model filename to load.")));
    pl.push_back(ObjectDescription(new osg::DoubleValueObject("scale",1.0), std::string("Model scale.")));
    return true;
}
