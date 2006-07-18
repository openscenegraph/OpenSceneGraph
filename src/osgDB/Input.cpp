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
#include <osg/Object>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/Input>

using namespace osgDB;

Input::Input()
{
}


Input::~Input()
{
}


osg::Object* Input::getObjectForUniqueID(const std::string& uniqueID)
{
    UniqueIDToObjectMapping::iterator fitr = _uniqueIDToObjectMap.find(uniqueID);
    if (fitr != _uniqueIDToObjectMap.end()) return (*fitr).second.get();
    else return NULL;
}


void Input::regisiterUniqueIDForObject(const std::string& uniqueID,osg::Object* obj)
{
    _uniqueIDToObjectMap[uniqueID] = obj;
}


osg::Object* Input::readObjectOfType(const osg::Object& compObj)
{
    return Registry::instance()->readObjectOfType(compObj,*this);
}

osg::Object* Input::readObjectOfType(const basic_type_wrapper &btw)
{
    return Registry::instance()->readObjectOfType(btw,*this);
}

osg::Object* Input::readObject()
{
    return Registry::instance()->readObject(*this);
}


osg::Image*  Input::readImage()
{
    return Registry::instance()->readImage(*this);
}

osg::Drawable* Input::readDrawable()
{
    return Registry::instance()->readDrawable(*this);
}

osg::StateAttribute* Input::readStateAttribute()
{
    return Registry::instance()->readStateAttribute(*this);
}
osg::Uniform* Input::readUniform()
{
    return Registry::instance()->readUniform(*this);
}

osg::Node* Input::readNode()
{
    return Registry::instance()->readNode(*this);
}

osg::Object* Input::readObject(const std::string& fileName)
{
    return readObjectFile(fileName,_options.get());
}

osg::Image*  Input::readImage(const std::string& fileName)
{

    return readImageFile(fileName,_options.get());
}

osg::Node* Input::readNode(const std::string& fileName)
{
    return readNodeFile(fileName,_options.get());
}
