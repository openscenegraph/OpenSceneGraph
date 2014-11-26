#ifndef OSGTERRAIN_SHADERTERRAIN
#define OSGTERRAIN_SHADERTERRAIN

#include <OpenThreads/Mutex>
#include <osg/MatrixTransform>
#include <osg/Program>
#include <osgTerrain/GeometryTechnique>
#include <osgTerrain/GeometryPool>

namespace osgTerrain
{

class ShaderTerrain : public osgTerrain::TerrainTechnique
{
    public:

        ShaderTerrain();

        ShaderTerrain(const ShaderTerrain&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgTerrain, ShaderTerrain);

        virtual void init(int dirtyMask, bool assumeMultiThreaded);
        virtual void update(osgUtil::UpdateVisitor* uv);
        virtual void cull(osgUtil::CullVisitor* cv);
        virtual void traverse(osg::NodeVisitor& nv);
        virtual void cleanSceneGraph();
        virtual void releaseGLObjects(osg::State* state) const;

    protected:

        virtual ~ShaderTerrain();

        osg::ref_ptr<GeometryPool> _geometryPool;

        mutable OpenThreads::Mutex              _traversalMutex;

        mutable OpenThreads::Mutex              _transformMutex;
        osg::ref_ptr<osg::MatrixTransform>      _transform;

        OpenThreads::Atomic                     _currentTraversalCount;

};

}

#endif
