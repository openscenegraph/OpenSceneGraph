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

#ifndef OSGTERRAIN_DISPLACEMENTMAPPINGTECHNIQUE
#define OSGTERRAIN_DISPLACEMENTMAPPINGTECHNIQUE

#include <OpenThreads/Mutex>
#include <osg/MatrixTransform>
#include <osg/Program>
#include <osgTerrain/GeometryTechnique>

namespace osgTerrain
{

class OSGTERRAIN_EXPORT DisplacementMappingTechnique : public osgTerrain::TerrainTechnique
{
    public:

        DisplacementMappingTechnique();

        DisplacementMappingTechnique(const DisplacementMappingTechnique&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgTerrain, DisplacementMappingTechnique);

        virtual void init(int dirtyMask, bool assumeMultiThreaded);
        virtual void update(osgUtil::UpdateVisitor* uv);
        virtual void cull(osgUtil::CullVisitor* cv);
        virtual void traverse(osg::NodeVisitor& nv);
        virtual void cleanSceneGraph();
        virtual void releaseGLObjects(osg::State* state) const;

    protected:

        virtual ~DisplacementMappingTechnique();

        mutable OpenThreads::Mutex              _traversalMutex;

        mutable OpenThreads::Mutex              _transformMutex;
        osg::ref_ptr<osg::MatrixTransform>      _transform;

        OpenThreads::Atomic                     _currentTraversalCount;

};

}

#endif
