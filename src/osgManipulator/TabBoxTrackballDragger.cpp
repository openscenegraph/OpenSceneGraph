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
//osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/TabBoxTrackballDragger>

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/PolygonMode>
#include <osg/CullFace>
#include <osg/Quat>
#include <osg/AutoTransform>

using namespace osgManipulator;

TabBoxTrackballDragger::TabBoxTrackballDragger()
{
    _trackballDragger = new TrackballDragger(true);
    addChild(_trackballDragger.get());
    addDragger(_trackballDragger.get());

    _tabBoxDragger = new TabBoxDragger();
    addChild(_tabBoxDragger.get());
    addDragger(_tabBoxDragger.get());

    setParentDragger(getParentDragger());
}

TabBoxTrackballDragger::~TabBoxTrackballDragger()
{
}

void TabBoxTrackballDragger::setupDefaultGeometry()
{
    _trackballDragger->setupDefaultGeometry();
    _tabBoxDragger->setupDefaultGeometry();
}
