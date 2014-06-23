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
#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/ScriptEngine>

using namespace osg;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Callback
//
bool Callback::traverse(Object* object, Object* data)
{
    if (_nestedCallback.valid()) return _nestedCallback->run(object, data);
    else
    {
#if 1
        osg::Node* node = dynamic_cast<osg::Node*>(object);
        osg::NodeVisitor* nv = dynamic_cast<osg::NodeVisitor*>(data);
#else
        osg::Node* node = object ? data->asNode() : 0;
        osg::NodeVisitor* nv = data ? data->asNodeVisitor() : 0;
#endif
        if (node && nv)
        {
            nv->traverse(*node);
            return true;
        }
        else return false;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CallbackObject
//
bool CallbackObject::run(osg::Object* object, osg::Object* data)
{
    osg::Parameters inputParameters, outputParameters;

    if (data && data->referenceCount()>=1)
    {
        inputParameters.push_back(data);
    }

    return run(object,inputParameters, outputParameters);
}

bool CallbackObject::run(osg::Object* object, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
{
    OSG_NOTICE<<"CallbackObject::run(object="<<object<<")"<<std::endl;
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// NodeCallback
//
bool NodeCallback::run(osg::Object* object, osg::Object* data)
{
    osg::Node* node = dynamic_cast<osg::Node*>(object);
    osg::NodeVisitor* nv = dynamic_cast<osg::NodeVisitor*>(data);
    if (node && nv)
    {
        operator()(node, nv);
        return true;
    }
    else
    {
        return traverse(object, data);
    }
}
void NodeCallback::operator()(Node* node, NodeVisitor* nv)
{
    // note, callback is responsible for scenegraph traversal so
    // they must call traverse(node,nv) to ensure that the
    // scene graph subtree (and associated callbacks) are traversed.
    traverse(node,nv);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// StateAttributeCallback
//
bool StateAttributeCallback::run(osg::Object* object, osg::Object* data)
{
    osg::StateAttribute* sa = dynamic_cast<osg::StateAttribute*>(object);
    osg::NodeVisitor* nv = dynamic_cast<osg::NodeVisitor*>(data);
    if (sa && nv)
    {
        operator()(sa, nv);
        return true;
    }
    else
    {
        return traverse(object, data);
    }
}
