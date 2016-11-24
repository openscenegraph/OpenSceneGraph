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
#include <osg/RenderInfo>
#include <osg/Drawable>

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
        osg::Node* node = object ? object->asNode() : 0;
        osg::NodeVisitor* nv = data ? data->asNodeVisitor() : 0;
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

bool CallbackObject::run(osg::Object* object, osg::Parameters& /*inputParameters*/, osg::Parameters& /*outputParameters*/) const
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
    osg::Node* node = object ? object->asNode() : 0;
    osg::NodeVisitor* nv = data ? data->asNodeVisitor() : 0;

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
    osg::StateAttribute* sa = object ? object->asStateAttribute() : 0;
    osg::NodeVisitor* nv = data ? data->asNodeVisitor() : 0;
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UniformCallback
//
bool UniformCallback::run(osg::Object* object, osg::Object* data)
{
    osg::Uniform* uniform = object ? object->asUniform() : 0;
    osg::NodeVisitor* nv = data ? data->asNodeVisitor() : 0;
    if (uniform && nv)
    {
        operator()(uniform, nv);
        return true;
    }
    else
    {
        return traverse(object, data);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
bool DrawableUpdateCallback::run(osg::Object* object, osg::Object* data)
{
    osg::Drawable* drawable = object->asDrawable();
    osg::NodeVisitor* nv = data->asNodeVisitor();
    if (drawable && nv)
    {
        update(nv, drawable);
        return true;
    }
    else
    {
        return traverse(object, data);
    }
}

bool DrawableEventCallback::run(osg::Object* object, osg::Object* data)
{
    osg::Drawable* drawable = object->asDrawable();
    osg::NodeVisitor* nv = data->asNodeVisitor();
    if (drawable && nv)
    {
        event(nv, drawable);
        return true;
    }
    else
    {
        return traverse(object, data);
    }
}

bool DrawableCullCallback::cull(osg::NodeVisitor* nv, osg::Drawable* drawable, osg::RenderInfo* renderInfo) const
{
    return cull(nv, drawable, renderInfo? renderInfo->getState():0);
}
