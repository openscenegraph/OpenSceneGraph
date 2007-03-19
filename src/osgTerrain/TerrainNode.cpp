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

#include <osgTerrain/TerrainNode>

using namespace osg;
using namespace osgTerrain;

TerrainNode::TerrainNode()
{
    setNumChildrenRequiringUpdateTraversal(1);
}

TerrainNode::TerrainNode(const TerrainNode& terrain,const osg::CopyOp& copyop):
    Group(terrain,copyop),
    _heightLayer(terrain._heightLayer)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);
    
    if (terrain.getTerrainTechnique()) setTerrainTechnique(dynamic_cast<TerrainTechnique*>(terrain.getTerrainTechnique()->cloneType()));
}

TerrainNode::~TerrainNode()
{
}

void TerrainNode::traverse(osg::NodeVisitor& nv)
{
    // if app traversal update the frame count.
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        osgUtil::UpdateVisitor* uv = dynamic_cast<osgUtil::UpdateVisitor*>(&nv);
        if (getTerrainTechnique() && uv)
        {
            getTerrainTechnique()->update(uv);
            return;
        }        
        
    }
    else if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (getTerrainTechnique() && cv)
        {
            getTerrainTechnique()->cull(cv);
            return;
        }
    }

    // otherwise fallback to the Group::traverse()
    Group::traverse(nv);
}

void TerrainNode::setHeightLayer(osgTerrain::Layer* layer)
{
    _heightLayer = layer;
}

osgTerrain::Layer* TerrainNode::getHeightLayer()
{
    return _heightLayer.get();
}

void TerrainNode::addColorLayer(osgTerrain::Layer* layer)
{
}

void TerrainNode::removeColorLayer(osgTerrain::Layer* layer)
{
}
