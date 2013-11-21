/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

#include <osgVolume/VolumeScene>
#include <OpenThreads/ScopedLock>

using namespace osgVolume;

VolumeScene::VolumeScene()
{
}

VolumeScene::VolumeScene(const VolumeScene& vs, const osg::CopyOp& copyop):
    osg::Group(vs,copyop)
{
}


VolumeScene::~VolumeScene()
{
}

void VolumeScene::traverse(osg::NodeVisitor& nv)
{
    Group::traverse(nv);
}
