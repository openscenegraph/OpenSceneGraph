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
#include <osg/NodeVisitor>

#include <osg/Billboard>
#include <osg/ClearNode>
#include <osg/ClipNode>
#include <osg/CoordinateSystemNode>
#include <osg/Geode>
#include <osg/Group>
#include <osg/LightSource>
#include <osg/LOD>
#include <osg/MatrixTransform>
#include <osg/OccluderNode>
#include <osg/OcclusionQueryNode>
#include <osg/PagedLOD>
#include <osg/PositionAttitudeTransform>
#include <osg/Projection>
#include <osg/ProxyNode>
#include <osg/Sequence>
#include <osg/Switch>
#include <osg/TexGenNode>
#include <osg/Transform>
#include <osg/Camera>
#include <osg/CameraView>

#include <stdlib.h>

using namespace osg;

NodeVisitor::NodeVisitor(TraversalMode tm):
        Referenced(true)
{
    _visitorType = NODE_VISITOR;
    _traversalNumber = osg::UNINITIALIZED_FRAME_NUMBER;

    _traversalMode = tm;
    _traversalMask = 0xffffffff;
    _nodeMaskOverride = 0x0;
}

NodeVisitor::NodeVisitor(VisitorType type,TraversalMode tm):
    Referenced(true)
{
    _visitorType = type;
    _traversalNumber = osg::UNINITIALIZED_FRAME_NUMBER;

    _traversalMode = tm;
    _traversalMask = 0xffffffff;
    _nodeMaskOverride = 0x0;
}


NodeVisitor::~NodeVisitor()
{
    // if (_traversalVisitor) detach from _traversalVisitor;
}

void NodeVisitor::apply(Node& node)
{
    traverse(node);
}

void NodeVisitor::apply(Geode& node)
{
    apply(static_cast<Node&>(node));
}

void NodeVisitor::apply(Billboard& node)
{
    apply(static_cast<Geode&>(node));
}

void NodeVisitor::apply(Group& node)
{
    apply(static_cast<Node&>(node));
}

void NodeVisitor::apply(ProxyNode& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(Projection& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(CoordinateSystemNode& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(ClipNode& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(TexGenNode& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(LightSource& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(Transform& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(Camera& node)
{
    apply(static_cast<Transform&>(node));
}

void NodeVisitor::apply(CameraView& node)
{
    apply(static_cast<Transform&>(node));
}

void NodeVisitor::apply(MatrixTransform& node)
{
    apply(static_cast<Transform&>(node));
}

void NodeVisitor::apply(PositionAttitudeTransform& node)
{
    apply(static_cast<Transform&>(node));
}

void NodeVisitor::apply(Switch& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(Sequence& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(LOD& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(PagedLOD& node)
{
    apply(static_cast<LOD&>(node));
}

void NodeVisitor::apply(ClearNode& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(OccluderNode& node)
{
    apply(static_cast<Group&>(node));
}

void NodeVisitor::apply(OcclusionQueryNode& node)
{
    apply(static_cast<Group&>(node));
}
