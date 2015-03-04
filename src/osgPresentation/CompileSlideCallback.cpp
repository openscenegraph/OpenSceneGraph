/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield
 *
 * This software is open source and may be redistributed and/or modified under
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * include LICENSE.txt for more details.
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
