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

#include <osg/Texture2D>
#include <osg/CoordinateSystemNode>
#include <osg/TexEnv>
#include <osg/io_utils>

#include <osgUtil/CullVisitor>
#include <osgShadow/ShadowedScene>

using namespace osgShadow;

ShadowedScene::ShadowedScene(ShadowTechnique* st)
{
    setNumChildrenRequiringUpdateTraversal(1);

    setShadowSettings(new ShadowSettings);

    if (st) setShadowTechnique(st);
}

ShadowedScene::ShadowedScene(const ShadowedScene& ss, const osg::CopyOp& copyop):
    osg::Group(ss,copyop)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);

    if (ss._shadowTechnique.valid())
    {
        setShadowTechnique( dynamic_cast<osgShadow::ShadowTechnique*>(ss._shadowTechnique->clone(copyop)) );
    }

    if (ss._shadowSettings)
    {
        setShadowSettings(ss._shadowSettings.get());
    }
    else
    {
        setShadowSettings(new ShadowSettings);
    }

}

ShadowedScene::~ShadowedScene()
{
    setShadowTechnique(0);
}

void ShadowedScene::traverse(osg::NodeVisitor& nv)
{
    if (_shadowTechnique.valid())
    {
        _shadowTechnique->traverse(nv);
    }
    else
    {
        osg::Group::traverse(nv);
    }
}

void ShadowedScene::setShadowSettings(ShadowSettings* ss)
{
    _shadowSettings = ss;
}

void ShadowedScene::setShadowTechnique(ShadowTechnique* technique)
{
    if (_shadowTechnique == technique) return;

    if (_shadowTechnique.valid())
    {
        _shadowTechnique->cleanSceneGraph();
        _shadowTechnique->_shadowedScene = 0;
    }

    _shadowTechnique = technique;

    if (_shadowTechnique.valid())
    {
        _shadowTechnique->_shadowedScene = this;
        _shadowTechnique->dirty();
    }
}

void ShadowedScene::cleanSceneGraph()
{
    if (_shadowTechnique.valid())
    {
        _shadowTechnique->cleanSceneGraph();
    }
}


void ShadowedScene::dirty()
{
    if (_shadowTechnique.valid())
    {
        _shadowTechnique->dirty();
    }
}

void ShadowedScene::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_shadowTechnique.valid()) _shadowTechnique->resizeGLObjectBuffers(maxSize);
    Group::resizeGLObjectBuffers(maxSize);
}

void ShadowedScene::releaseGLObjects(osg::State* state) const
{
    if (_shadowTechnique.valid()) _shadowTechnique->releaseGLObjects(state);
    Group::releaseGLObjects(state);
}
