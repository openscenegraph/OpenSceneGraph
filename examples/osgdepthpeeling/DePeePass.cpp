/*
  Steffen Frey
  Fachpraktikum Graphik-Programmierung 2007
  Institut fuer Visualisierung und Interaktive Systeme
  Universitaet Stuttgart
 */

#include "DePeePass.h"

#include <iostream>
#include <assert.h>

DePeePass::DePeePass()
{
  root = new osg::Group;
}

DePeePass::~DePeePass()
{
  root->releaseGLObjects();
  assert(Cameras.size() == settingNodes.size());
  while(!Cameras.empty())
    {
      remRenderPass((*Cameras.begin()).first);
    }
}
  
void DePeePass::newRenderPass(MapMode mapMode)
{
  Cameras[mapMode] = new osg::Camera;
  settingNodes[mapMode] = new osg::Group;
  root->addChild(Cameras[mapMode].get());
  Cameras[mapMode]->addChild(settingNodes[mapMode].get());
}
  
void DePeePass::remRenderPass(MapMode mapMode)
{
  assert(Cameras.find(mapMode) != Cameras.end());
  Cameras[mapMode]->releaseGLObjects();
  settingNodes[mapMode]->releaseGLObjects();
  
  Cameras[mapMode]->removeChild(settingNodes[mapMode].get());
  //setting Nodes have exactly one child
  assert(settingNodes[mapMode]->getNumChildren() == 1);
  settingNodes[mapMode]->removeChild(0,1);
  
  Cameras.erase(Cameras.find(mapMode));
  settingNodes.erase(settingNodes.find(mapMode));
}
