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

#ifndef METHODSOBJECT_H
#define METHODSOBJECT_H

#include <osg/ScriptEngine>

namespace osgDB
{

struct MethodObject : public osg::Referenced
{
    typedef std::vector< osg::ref_ptr<osg::Object> > Parameters;

    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const = 0;
    virtual ~MethodObject() {}
};

struct MethodsObject  : public osg::Referenced
{
    MethodsObject() {}
    virtual ~MethodsObject() {}

    void add(const std::string& methodName, MethodObject* mo);

    bool run(void* objectPtr, const std::string& methodName, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const;

    bool hasMethod(const std::string& methodName) const;

    typedef std::multimap< std::string, osg::ref_ptr<MethodObject> > MethodMap;
    MethodMap methodMap;
};

struct MethodsObjectManager : public osg::Referenced
{
    MethodsObjectManager() {}
    virtual ~MethodsObjectManager() {}

    static osg::ref_ptr<MethodsObjectManager>& instance();

    void add(const std::string& compoundClassName, const std::string& methodName, MethodObject* mo);

    bool run(void* objectPtr, const std::string& compoundClassName, const std::string& methodName, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const;

    bool hasMethod(const std::string& compoundClassName, const std::string& methodName) const;

    typedef std::map< std::string, osg::ref_ptr<MethodsObject> > MethodsObjectMap;
    MethodsObjectMap methodsObjects;
};

struct RegisterMethodObjectProxy
{
    RegisterMethodObjectProxy(const std::string& compoundClassName, const std::string& methodName, MethodObject* mo)
    {
        MethodsObjectManager::instance()->add(compoundClassName, methodName, mo);
    }
};

}

#endif
