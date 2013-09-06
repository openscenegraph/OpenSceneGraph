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

#include <osgPresentation/Image>
#include <osg/ValueObject>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

using namespace osgPresentation;


bool Image::load()
{
    OSG_NOTICE<<"Image::load() Not implemented yet"<<std::endl;

    std::string filename;
    getPropertyValue("filename", filename);

    double scale = 1.0;
    getPropertyValue("scale", scale);

    OSG_NOTICE<<"Image : filename = "<<filename<<std::endl;
    OSG_NOTICE<<"        scale = "<<scale<<std::endl;

    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
    if (image.valid())
    {
        osg::Vec3d position(0.0,0.0,0.0);
        osg::Vec3d widthVec(1.0,0.0,0.0);
        osg::Vec3d heightVec(0.0,0.0,1.0);

        osg::ref_ptr<osg::Geometry> geometry = osg::createTexturedQuadGeometry(position, widthVec, heightVec);
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image.get());
        texture->setResizeNonPowerOfTwoHint(false);
        texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
        texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        geometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable(geometry.get());

        addChild(geode.get());
    }

    return false;
}

bool Image::getSupportedProperties(PropertyList& pl)
{
    pl.push_back(ObjectDescription(new osg::StringValueObject("filename",""), std::string("Image filename to load.")));
    pl.push_back(ObjectDescription(new osg::DoubleValueObject("scale",1.0), std::string("Image scale.")));
    return true;
}
