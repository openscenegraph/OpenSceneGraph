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

#include <osg/Geode>
#include <osg/ValueObject>
#include "MethodObject.h"

struct GeodeGetNumDrawables : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        osg::Geode* geode = reinterpret_cast<osg::Geode*>(objectPtr);
        outputParameters.push_back(new osg::UIntValueObject("return", geode->getNumDrawables()));
        return true;
    }
};


struct GeodeGetDrawable : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Object* indexObject = inputParameters[0].get();
        osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
        if (!uivo) return false;

        osg::Geode* geode = reinterpret_cast<osg::Geode*>(objectPtr);
        outputParameters.push_back(geode->getDrawable(uivo->getValue()));

        return true;
    }
};

struct GeodeAddDrawable : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Drawable* child = dynamic_cast<osg::Drawable*>(inputParameters[0].get());
        if (!child) return false;

        osg::Geode* geode = reinterpret_cast<osg::Geode*>(objectPtr);
        geode->addDrawable(child);

        return true;
    }
};


struct GeodeRemoveDrawable : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Drawable* child = dynamic_cast<osg::Drawable*>(inputParameters[0].get());
        if (!child) return false;

        osg::Geode* geode = reinterpret_cast<osg::Geode*>(objectPtr);
        geode->removeDrawable(child);

        return true;
    }
};

static osgDB::RegisterMethodObjectProxy s_getNumDrawables("osg::Geode","getNumDrawables",new GeodeGetNumDrawables());
static osgDB::RegisterMethodObjectProxy s_getNumDrawable("osg::Geode","getDrawable",new GeodeGetDrawable());
static osgDB::RegisterMethodObjectProxy s_getAddDrawable("osg::Geode","addDrawable",new GeodeAddDrawable());
static osgDB::RegisterMethodObjectProxy s_getRemoveDrawable("osg::Geode","removeDrawable",new GeodeRemoveDrawable());
