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

#include "MethodObject.h"
#include <osg/Notify>

using namespace osgDB;


void MethodsObject::add(const std::string& methodName, MethodObject* mo)
{
    methodMap.insert(MethodMap::value_type(methodName, mo));
}

bool MethodsObject::run(void* objectPtr, const std::string& methodName, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
{
    MethodMap::const_iterator itr = methodMap.find(methodName);
    while (itr != methodMap.end())
    {
        if (itr->first==methodName)
        {
            OSG_NOTICE<<"Calling methodObject for "<<methodName<<std::endl;
            if (itr->second->run(objectPtr, inputParameters, outputParameters)) return true;
        }
        ++itr;
    }
    OSG_NOTICE<<"No matching methodObject found for "<<methodName<<std::endl;
    return false;
}

bool MethodsObject::hasMethod(const std::string& methodName) const
{
    return (methodMap.find(methodName)!=methodMap.end());
}



osg::ref_ptr<MethodsObjectManager>& MethodsObjectManager::instance()
{
    static osg::ref_ptr<MethodsObjectManager> s_MethodsObjectManager = new MethodsObjectManager;
    return s_MethodsObjectManager;
}

void MethodsObjectManager::add(const std::string& compoundClassName, const std::string& methodName, MethodObject* mo)
{
    MethodsObjectMap::iterator itr = methodsObjects.find( compoundClassName );
    if (itr==methodsObjects.end())
    {
        methodsObjects[compoundClassName] = new MethodsObject;
        itr = methodsObjects.find( compoundClassName );
    }
    itr->second->add(methodName, mo);
}

bool MethodsObjectManager::run(void* objectPtr, const std::string& compoundClassName, const std::string& methodName, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
{
    MethodsObjectMap::const_iterator itr = methodsObjects.find( compoundClassName );
    if (itr!=methodsObjects.end())
    {
        return itr->second->run(objectPtr, methodName, inputParameters, outputParameters);
    }
    OSG_NOTICE<<"No matching methodObject found for class "<<compoundClassName<<", method "<<methodName<<std::endl;
    return false;
}

bool MethodsObjectManager::hasMethod(const std::string& compoundClassName, const std::string& methodName) const
{
    MethodsObjectMap::const_iterator itr = methodsObjects.find( compoundClassName );
    if (itr!=methodsObjects.end())
    {
        return itr->second->hasMethod(methodName);
    }
    return false;

}
