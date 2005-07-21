/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield
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


#include <osg/GraphicsContext>

using namespace osg;

static ref_ptr<GraphicsContext::CreateGraphicContexCallback> s_createGraphicsContextCallback;

void GraphicsContext::setCreateGraphicsContextCallback(CreateGraphicContexCallback* callback)
{
    s_createGraphicsContextCallback = callback;
}

GraphicsContext::CreateGraphicContexCallback* GraphicsContext::getCreateGraphicsContextCallback()
{
    return s_createGraphicsContextCallback.get();
}

GraphicsContext* GraphicsContext::createGraphicsContext(Traits* traits)
{
    if (s_createGraphicsContextCallback.valid())
        return s_createGraphicsContextCallback->createGraphicsContext(traits);
    else
        return 0;    
}

GraphicsContext::GraphicsContext():
    _threadOfLastMakeCurrent(0)
{
}

