/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2018 Robert Osfield
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

#include <osgPresentation/CompileSlideCallback>

#include <osgUtil/GLObjectsVisitor>

using namespace osgPresentation;

void CompileSlideCallback::operator()(const osg::Camera & camera) const
{
    osg::GraphicsContext* context = const_cast<osg::GraphicsContext*>(camera.getGraphicsContext());
    if (!context) return;

    osg::State* state = context->getState();
    if (!state) return;

    const osg::FrameStamp* fs = state->getFrameStamp();
    if (!fs) return;

    if (_needCompile)
    {
        _frameNumber = fs->getFrameNumber();
        _needCompile = false;
    }

    if (_frameNumber!=fs->getFrameNumber()) return;

    osgUtil::GLObjectsVisitor globjVisitor(osgUtil::GLObjectsVisitor::COMPILE_DISPLAY_LISTS|
                                  osgUtil::GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES);

    globjVisitor.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

    globjVisitor.setNodeMaskOverride(0xffffffff);

    globjVisitor.setState(state);

    _sceneToCompile->accept(globjVisitor);
}
