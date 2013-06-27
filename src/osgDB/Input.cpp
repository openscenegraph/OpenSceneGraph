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


void Input::registerUniqueIDForObject(const std::string& uniqueID,osg::Object* obj)
{
    _uniqueIDToObjectMap[uniqueID] = obj;
}


osg::Object* Input::readObjectOfType(const osg::Object& compObj)
{
    return Registry::instance()->getDeprecatedDotOsgObjectWrapperManager()->readObjectOfType(compObj,*this);
}

osg::Object* Input::readObjectOfType(const basic_type_wrapper &btw)
{
    return Registry::instance()->getDeprecatedDotOsgObjectWrapperManager()->readObjectOfType(btw,*this);
}

osg::Object* Input::readObject()
{
    return Registry::instance()->getDeprecatedDotOsgObjectWrapperManager()->readObject(*this);
}


osg::Image*  Input::readImage()
{
    return Registry::instance()->getDeprecatedDotOsgObjectWrapperManager()->readImage(*this);
}

osg::Drawable* Input::readDrawable()
{
    osg::Drawable* drawable = Registry::instance()->getDeprecatedDotOsgObjectWrapperManager()->readDrawable(*this);
    osg::Geometry* geometry = drawable ? drawable->asGeometry() : 0;
    if (geometry && geometry->containsDeprecatedData()) geometry->fixDeprecatedData();
    return drawable;
}

osg::StateAttribute* Input::readStateAttribute()
{
    return Registry::instance()->getDeprecatedDotOsgObjectWrapperManager()->readStateAttribute(*this);
}
osg::Uniform* Input::readUniform()
{
    return Registry::instance()->getDeprecatedDotOsgObjectWrapperManager()->readUniform(*this);
}

osg::Node* Input::readNode()
{
    return Registry::instance()->getDeprecatedDotOsgObjectWrapperManager()->readNode(*this);
}

osg::Object* Input::readObject(const std::string& fileName)
{
    return readObjectFile(fileName,_options.get());
}

osg::Shader*  Input::readShader()
{
    return Registry::instance()->getDeprecatedDotOsgObjectWrapperManager()->readShader(*this);
}

osg::Image*  Input::readImage(const std::string& fileName)
{
    return readImageFile(fileName,_options.get());
}

osg::Node* Input::readNode(const std::string& fileName)
{
    return readNodeFile(fileName,_options.get());
}

osg::Shader*  Input::readShader(const std::string& fileName)
{
    return readShaderFile(fileName,_options.get());
}

bool Input::read(Parameter value1)
{
    if (value1.valid((*this)[0].getStr()))
    {
        value1.assign((*this)[0].getStr());
        (*this) += 1;
        return true;
    }
    else return false;
}

bool Input::read(Parameter value1, Parameter value2)
{
    if (value1.valid((*this)[0].getStr()) &&
        value2.valid((*this)[1].getStr()))
    {
        value1.assign((*this)[0].getStr());
        value2.assign((*this)[1].getStr());
        (*this) += 2;
        return true;
    }
    else return false;
}

bool Input::read(Parameter value1, Parameter value2, Parameter value3)
{
    if (value1.valid((*this)[0].getStr()) &&
        value2.valid((*this)[1].getStr()) &&
        value3.valid((*this)[2].getStr()))
    {
        value1.assign((*this)[0].getStr());
        value2.assign((*this)[1].getStr());
        value3.assign((*this)[2].getStr());
        (*this) += 3;
        return true;
    }
    else return false;
}

bool Input::read(Parameter value1, Parameter value2, Parameter value3, Parameter value4)
{
    if (value1.valid((*this)[0].getStr()) &&
        value2.valid((*this)[1].getStr()) &&
        value3.valid((*this)[2].getStr()) &&
        value4.valid((*this)[3].getStr()))
    {
        value1.assign((*this)[0].getStr());
        value2.assign((*this)[1].getStr());
        value3.assign((*this)[2].getStr());
        value4.assign((*this)[3].getStr());
        (*this) += 4;
        return true;
    }
    else return false;
}

bool Input::read(Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5)
{
    if (value1.valid((*this)[0].getStr()) &&
        value2.valid((*this)[1].getStr()) &&
        value3.valid((*this)[2].getStr()) &&
        value4.valid((*this)[3].getStr()) &&
        value5.valid((*this)[4].getStr()))
    {
        value1.assign((*this)[0].getStr());
        value2.assign((*this)[1].getStr());
        value3.assign((*this)[2].getStr());
        value4.assign((*this)[3].getStr());
        value5.assign((*this)[4].getStr());
        (*this) += 5;
        return true;
    }
    else return false;
}

bool Input::read(Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5, Parameter value6)
{
    if (value1.valid((*this)[0].getStr()) &&
        value2.valid((*this)[1].getStr()) &&
        value3.valid((*this)[2].getStr()) &&
        value4.valid((*this)[3].getStr()) &&
        value5.valid((*this)[4].getStr()) &&
        value6.valid((*this)[5].getStr()))
    {
        value1.assign((*this)[0].getStr());
        value2.assign((*this)[1].getStr());
        value3.assign((*this)[2].getStr());
        value4.assign((*this)[3].getStr());
        value5.assign((*this)[4].getStr());
        value6.assign((*this)[5].getStr());
        (*this) += 6;
        return true;
    }
    else return false;
}

bool Input::read(Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5, Parameter value6, Parameter value7)
{
    if (value1.valid((*this)[0].getStr()) &&
        value2.valid((*this)[1].getStr()) &&
        value3.valid((*this)[2].getStr()) &&
        value4.valid((*this)[3].getStr()) &&
        value5.valid((*this)[4].getStr()) &&
        value6.valid((*this)[5].getStr()) &&
        value7.valid((*this)[6].getStr()))
    {
        value1.assign((*this)[0].getStr());
        value2.assign((*this)[1].getStr());
        value3.assign((*this)[2].getStr());
        value4.assign((*this)[3].getStr());
        value5.assign((*this)[4].getStr());
        value6.assign((*this)[5].getStr());
        value7.assign((*this)[6].getStr());
        (*this) += 7;
        return true;
    }
    else return false;
}

bool Input::read(Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5, Parameter value6, Parameter value7, Parameter value8)
{
    if (value1.valid((*this)[0].getStr()) &&
        value2.valid((*this)[1].getStr()) &&
        value3.valid((*this)[2].getStr()) &&
        value4.valid((*this)[3].getStr()) &&
        value5.valid((*this)[4].getStr()) &&
        value6.valid((*this)[5].getStr()) &&
        value7.valid((*this)[6].getStr()) &&
        value8.valid((*this)[7].getStr()))
    {
        value1.assign((*this)[0].getStr());
        value2.assign((*this)[1].getStr());
        value3.assign((*this)[2].getStr());
        value4.assign((*this)[3].getStr());
        value5.assign((*this)[4].getStr());
        value6.assign((*this)[5].getStr());
        value7.assign((*this)[6].getStr());
        value8.assign((*this)[7].getStr());
        (*this) += 8;
        return true;
    }
    else return false;
}

bool Input::read(const char* str)
{
    if ((*this)[0].matchWord(str))
    {
        (*this) += 1;
        return true;
    }
    else return false;
}

bool Input::read(const char* str, Parameter value1)
{
    if ((*this)[0].matchWord(str) && value1.valid((*this)[1].getStr()))
    {
        value1.assign((*this)[1].getStr());
        (*this) += 2;
        return true;
    }
    else return false;
}

bool Input::read(const char* str, Parameter value1, Parameter value2)
{
    if ((*this)[0].matchWord(str) &&
        value1.valid((*this)[1].getStr()) &&
        value2.valid((*this)[2].getStr()))
    {
        value1.assign((*this)[1].getStr());
        value2.assign((*this)[2].getStr());
        (*this) += 3;
        return true;
    }
    else return false;
}

bool Input::read(const char* str, Parameter value1, Parameter value2, Parameter value3)
{
    if ((*this)[0].matchWord(str) &&
        value1.valid((*this)[1].getStr()) &&
        value2.valid((*this)[2].getStr()) &&
        value3.valid((*this)[3].getStr()))
    {
        value1.assign((*this)[1].getStr());
        value2.assign((*this)[2].getStr());
        value3.assign((*this)[3].getStr());
        (*this) += 4;
        return true;
    }
    else return false;
}

bool Input::read(const char* str, Parameter value1, Parameter value2, Parameter value3, Parameter value4)
{
    if ((*this)[0].matchWord(str) &&
        value1.valid((*this)[1].getStr()) &&
        value2.valid((*this)[2].getStr()) &&
        value3.valid((*this)[3].getStr()) &&
        value4.valid((*this)[4].getStr()))
    {
        value1.assign((*this)[1].getStr());
        value2.assign((*this)[2].getStr());
        value3.assign((*this)[3].getStr());
        value4.assign((*this)[4].getStr());
        (*this) += 5;
        return true;
    }
    else return false;
}

bool Input::read(const char* str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5)
{
    if ((*this)[0].matchWord(str) &&
        value1.valid((*this)[1].getStr()) &&
        value2.valid((*this)[2].getStr()) &&
        value3.valid((*this)[3].getStr()) &&
        value4.valid((*this)[4].getStr()) &&
        value5.valid((*this)[5].getStr()))
    {
        value1.assign((*this)[1].getStr());
        value2.assign((*this)[2].getStr());
        value3.assign((*this)[3].getStr());
        value4.assign((*this)[4].getStr());
        value5.assign((*this)[5].getStr());
        (*this) += 6;
        return true;
    }
    else return false;
}

bool Input::read(const char* str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5, Parameter value6)
{
    if ((*this)[0].matchWord(str) &&
        value1.valid((*this)[1].getStr()) &&
        value2.valid((*this)[2].getStr()) &&
        value3.valid((*this)[3].getStr()) &&
        value4.valid((*this)[4].getStr()) &&
        value5.valid((*this)[5].getStr()) &&
        value6.valid((*this)[6].getStr()))
    {
        value1.assign((*this)[1].getStr());
        value2.assign((*this)[2].getStr());
        value3.assign((*this)[3].getStr());
        value4.assign((*this)[4].getStr());
        value5.assign((*this)[5].getStr());
        value6.assign((*this)[6].getStr());
        (*this) += 7;
        return true;
    }
    else return false;
}

bool Input::read(const char* str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5, Parameter value6, Parameter value7)
{
    if ((*this)[0].matchWord(str) &&
        value1.valid((*this)[1].getStr()) &&
        value2.valid((*this)[2].getStr()) &&
        value3.valid((*this)[3].getStr()) &&
        value4.valid((*this)[4].getStr()) &&
        value5.valid((*this)[5].getStr()) &&
        value6.valid((*this)[6].getStr()) &&
        value7.valid((*this)[7].getStr()))
    {
        value1.assign((*this)[1].getStr());
        value2.assign((*this)[2].getStr());
        value3.assign((*this)[3].getStr());
        value4.assign((*this)[4].getStr());
        value5.assign((*this)[5].getStr());
        value6.assign((*this)[6].getStr());
        value7.assign((*this)[7].getStr());
        (*this) += 8;
        return true;
    }
    else return false;
}

bool Input::read(const char* str, Parameter value1, Parameter value2, Parameter value3, Parameter value4, Parameter value5, Parameter value6, Parameter value7, Parameter value8)
{
    if ((*this)[0].matchWord(str) &&
        value1.valid((*this)[1].getStr()) &&
        value2.valid((*this)[2].getStr()) &&
        value3.valid((*this)[3].getStr()) &&
        value4.valid((*this)[4].getStr()) &&
        value5.valid((*this)[5].getStr()) &&
        value6.valid((*this)[6].getStr()) &&
        value7.valid((*this)[7].getStr()) &&
        value8.valid((*this)[8].getStr()))
    {
        value1.assign((*this)[1].getStr());
        value2.assign((*this)[2].getStr());
        value3.assign((*this)[3].getStr());
        value4.assign((*this)[4].getStr());
        value5.assign((*this)[5].getStr());
        value6.assign((*this)[6].getStr());
        value7.assign((*this)[7].getStr());
        value8.assign((*this)[8].getStr());
        (*this) += 9;
        return true;
    }
    else return false;
}
