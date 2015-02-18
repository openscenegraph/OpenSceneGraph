/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
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

#include <osgTerrain/Terrain>
#include <osgTerrain/DisplacementMappingTechnique>


using namespace osgTerrain;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DisplacementMappingTechnique
//
DisplacementMappingTechnique::DisplacementMappingTechnique()
{
    // OSG_NOTICE<<"DisplacementMappingTechnique::DisplacementMappingTechnique()"<<std::endl;
}

DisplacementMappingTechnique::DisplacementMappingTechnique(const DisplacementMappingTechnique& st,const osg::CopyOp& copyop):
    osgTerrain::TerrainTechnique(st, copyop)
{
}

DisplacementMappingTechnique::~DisplacementMappingTechnique()
{
}

void DisplacementMappingTechnique::init(int dirtyMask, bool assumeMultiThreaded)
{
    if (!_terrainTile) return;
    if (!_terrainTile->getTerrain()) return;

    //OSG_NOTICE<<"DisplacementMappingTechnique::init("<<dirtyMask<<", "<<assumeMultiThreaded<<")"<<std::endl;

    GeometryPool* geometryPool = _terrainTile->getTerrain()->getGeometryPool();
    _transform = geometryPool->getTileSubgraph(_terrainTile);

    // set tile as no longer dirty.
    _terrainTile->setDirtyMask(0);
}

void DisplacementMappingTechnique::update(osgUtil::UpdateVisitor* uv)
{
    if (_terrainTile) _terrainTile->osg::Group::traverse(*uv);

    if (_transform.valid()) _transform->accept(*uv);
}


void DisplacementMappingTechnique::cull(osgUtil::CullVisitor* cv)
{
    if (_transform.valid()) _transform->accept(*cv);
}


void DisplacementMappingTechnique::traverse(osg::NodeVisitor& nv)
{
    if (!_terrainTile) return;

    // if app traversal update the frame count.
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        // if (_terrainTile->getDirty()) _terrainTile->init(_terrainTile->getDirtyMask(), false);

        osgUtil::UpdateVisitor* uv = dynamic_cast<osgUtil::UpdateVisitor*>(&nv);
        if (uv)
        {
            update(uv);
            return;
        }
    }
    else if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (cv)
        {
            cull(cv);
            return;
        }
    }

    {
        if (_transform.valid())
        {
            _transform->accept(nv);
        }
    }
}


void DisplacementMappingTechnique::cleanSceneGraph()
{
}

void DisplacementMappingTechnique::releaseGLObjects(osg::State* state) const
{
    if (_transform.valid())
    {
//      OSG_NOTICE<<"DisplacementMappingTechnique::releaseGLObjects()"<<std::endl;
        _transform->releaseGLObjects(state);
    }
}
