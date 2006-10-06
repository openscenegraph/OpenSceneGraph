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
using namespace osg;

ShadowedScene::ShadowedScene()
{
    setNumChildrenRequiringUpdateTraversal(1);
}

ShadowedScene::ShadowedScene(const ShadowedScene& copy, const osg::CopyOp& copyop):
    osg::Group(copy,copyop)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);            
}

void ShadowedScene::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {
        Group::traverse(nv);
        return;
    }

    if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
        Group::traverse(nv);
        return;
    }

    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (!cv)
    {
        Group::traverse(nv);
        return;
    }
    
    Group::traverse(nv);
}
