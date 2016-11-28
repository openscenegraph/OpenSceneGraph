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
#include "GPUScene"

//using namespace osgShadow;

GPUScene::GPUScene(GPUDrawTechnique* st)
{
    setNumChildrenRequiringUpdateTraversal(1);

   // setShadowSettings(new ShadowSettings);

    if (st) setGPUDrawTechnique(st);
}

GPUScene::GPUScene(const GPUScene& ss, const osg::CopyOp& copyop):
    osg::Group(ss,copyop)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);

    if (ss._GPUDrawTechnique.valid())
    {
        setGPUDrawTechnique( dynamic_cast<GPUDrawTechnique*>(ss._GPUDrawTechnique->clone(copyop)) );
    }

   /* if (ss._shadowSettings)
    {
        setShadowSettings(ss._shadowSettings.get());
    }
    else
    {
        setShadowSettings(new ShadowSettings);
    }*/

}

GPUScene::~GPUScene()
{
    setGPUDrawTechnique(0);
}

void GPUScene::traverse(osg::NodeVisitor& nv)
{
    if (_GPUDrawTechnique.valid())
    {
        _GPUDrawTechnique->traverse(nv);
    }
    else
    {
        osg::Group::traverse(nv);
    }
}
/*
void GPUScene::setShadowSettings(ShadowSettings* ss)
{
    _shadowSettings = ss;
}*/

void GPUScene::setGPUDrawTechnique(GPUDrawTechnique* technique)
{
    if (_GPUDrawTechnique == technique) return;

    if (_GPUDrawTechnique.valid())
    {
        _GPUDrawTechnique->cleanSceneGraph();
        _GPUDrawTechnique->_GPUScene = 0;
    }

    _GPUDrawTechnique = technique;

    if (_GPUDrawTechnique.valid())
    {
        _GPUDrawTechnique->_GPUScene = this;
        _GPUDrawTechnique->dirty();
    }
}

void GPUScene::cleanSceneGraph()
{
    if (_GPUDrawTechnique.valid())
    {
        _GPUDrawTechnique->cleanSceneGraph();
    }
}


void GPUScene::dirty()
{
    if (_GPUDrawTechnique.valid())
    {
        _GPUDrawTechnique->dirty();
    }
}

void GPUScene::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_GPUDrawTechnique.valid()) _GPUDrawTechnique->resizeGLObjectBuffers(maxSize);
    Group::resizeGLObjectBuffers(maxSize);
}

void GPUScene::releaseGLObjects(osg::State* state) const
{
    if (_GPUDrawTechnique.valid()) _GPUDrawTechnique->releaseGLObjects(state);
    Group::releaseGLObjects(state);
}
