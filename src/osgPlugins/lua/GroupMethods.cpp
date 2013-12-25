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

#include <osg/Group>
#include <osg/ValueObject>

#include "MethodObject.h"

using namespace osgDB;

struct GroupGetNumChildren : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        osg::Group* group = reinterpret_cast<osg::Group*>(objectPtr);
        outputParameters.push_back(new osg::UIntValueObject("return", group->getNumChildren()));
        return true;
    }
};

struct GroupGetChild : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Object* indexObject = inputParameters[0].get();
        OSG_NOTICE<<"GroupGetChild "<<indexObject->className()<<std::endl;

        unsigned int index = 0;
        osg::DoubleValueObject* dvo = dynamic_cast<osg::DoubleValueObject*>(indexObject);
        if (dvo) index = static_cast<unsigned int>(dvo->getValue());
        else
        {
            osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
            if (uivo) index = uivo->getValue();
        }
        osg::Group* group = reinterpret_cast<osg::Group*>(objectPtr);
        outputParameters.push_back(group->getChild(index));

        return true;
    }
};

struct GroupAddChild : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Node* child = dynamic_cast<osg::Node*>(inputParameters[0].get());
        if (!child) return false;

        osg::Group* group = reinterpret_cast<osg::Group*>(objectPtr);
        group->addChild(child);

        return true;
    }
};


struct GroupRemoveChild : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Node* child = dynamic_cast<osg::Node*>(inputParameters[0].get());
        if (!child) return false;

        osg::Group* group = reinterpret_cast<osg::Group*>(objectPtr);
        group->removeChild(child);

        return true;
    }
};

static osgDB::RegisterMethodObjectProxy s_getNumChildren("osg::Group","getNumChildren",new GroupGetNumChildren());
static osgDB::RegisterMethodObjectProxy s_getNumChild("osg::Group","getChild",new GroupGetChild());
static osgDB::RegisterMethodObjectProxy s_getAddChild("osg::Group","addChild",new GroupAddChild());
static osgDB::RegisterMethodObjectProxy s_getRemoveChild("osg::Group","removeChild",new GroupRemoveChild());
