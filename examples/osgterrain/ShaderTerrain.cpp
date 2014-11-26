
#include <osgTerrain/Layer>
#include <osgTerrain/TerrainTile>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Program>
#include <osg/Uniform>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include "ShaderTerrain.h"

using namespace osgTerrain;

#if 0
#define LOCK(mutex)  OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);
#else
#define LOCK(mutex) /* OpenThreads::Thread::microSleep(1);*/
#endif



/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ShaderTerrain
//
ShaderTerrain::ShaderTerrain()
{
    // OSG_NOTICE<<"ShaderTerrain::ShaderTerrain()"<<std::endl;
    _geometryPool = new GeometryPool;
}

ShaderTerrain::ShaderTerrain(const ShaderTerrain& st,const osg::CopyOp& copyop):
    osgTerrain::TerrainTechnique(st, copyop),
    _geometryPool(st._geometryPool)
{
    // OSG_NOTICE<<"ShaderTerrain::ShaderTerrain(ShaderTerrain&, CopyOp&) "<<_geometryPool.get()<<std::endl;
}

ShaderTerrain::~ShaderTerrain()
{
}

void ShaderTerrain::init(int dirtyMask, bool assumeMultiThreaded)
{
    if (!_terrainTile) return;

    LOCK(_transformMutex);

    ++_currentTraversalCount;

#if 0
    if (_currentTraversalCount>1)
    {
        unsigned int val = _currentTraversalCount;
        printf("Has a concurrent traversal %i\n",val);
        //throw "have concurrent traversal happening";
        OpenThreads::Thread::YieldCurrentThread();
    }
    else
    {
        printf("Single threaded traversal\n");
    }
#endif

    //OSG_NOTICE<<"ShaderTerrain::init("<<dirtyMask<<", "<<assumeMultiThreaded<<")"<<std::endl;

    _transform = _geometryPool->getTileSubgraph(_terrainTile);

    // set tile as no longer dirty.
    _terrainTile->setDirtyMask(0);

    --_currentTraversalCount;
}

void ShaderTerrain::update(osgUtil::UpdateVisitor* uv)
{
    LOCK(_transformMutex);

    if (_terrainTile) _terrainTile->osg::Group::traverse(*uv);

    if (_transform.valid()) _transform->accept(*uv);
}


void ShaderTerrain::cull(osgUtil::CullVisitor* cv)
{
    LOCK(_transformMutex);

    if (_transform.valid()) _transform->accept(*cv);
}


void ShaderTerrain::traverse(osg::NodeVisitor& nv)
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
        LOCK(_transformMutex);
        if (_transform.valid())
        {
            _transform->accept(nv);
        }
    }
}


void ShaderTerrain::cleanSceneGraph()
{
}

void ShaderTerrain::releaseGLObjects(osg::State* state) const
{
//    LOCK(_transformMutex);
    if (_transform.valid())
    {
//      OSG_NOTICE<<"ShaderTerrain::releaseGLObjects()"<<std::endl;
        _transform->releaseGLObjects(state);
    }
}
