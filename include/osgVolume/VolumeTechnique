/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

#ifndef OSGVOLUME_VOLUMETECHNIQUE
#define OSGVOLUME_VOLUMETECHNIQUE 1

#include <osg/Object>

#include <osgUtil/UpdateVisitor>
#include <osgUtil/CullVisitor>

#include <osgVolume/Export>

namespace osgVolume {

class VolumeTile;

/** Container for render to texture objects used when doing multi-pass volume rendering techniques.*/
struct TileData : public osg::Referenced
{
    TileData() : active(false) {}

    virtual void update(osgUtil::CullVisitor* cv) = 0;

    bool active;

    osg::NodePath nodePath;
    osg::ref_ptr<osg::RefMatrix> projectionMatrix;
    osg::ref_ptr<osg::RefMatrix> modelviewMatrix;

    osg::ref_ptr<osg::StateSet>  stateset;
};


class OSGVOLUME_EXPORT VolumeTechnique : public osg::Object
{
    public:

        VolumeTechnique();

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        VolumeTechnique(const VolumeTechnique&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgVolume, VolumeTechnique);

        VolumeTile* getVolumeTile() { return _volumeTile; }
        const VolumeTile* getVolumeTile() const { return _volumeTile; }

        virtual void init();

        virtual void update(osgUtil::UpdateVisitor* nv);

        virtual bool isMoving(osgUtil::CullVisitor* nv);

        virtual void cull(osgUtil::CullVisitor* nv);

        /** Clean scene graph from any terrain technique specific nodes.*/
        virtual void cleanSceneGraph();

        /** Traverse the terrain subgraph.*/
        virtual void traverse(osg::NodeVisitor& nv);

        /** Called from VolumeScene to create the TileData container when a multi-pass technique is being used.
         *  The TileData container caches any render to texture objects that are required. */
        virtual TileData* createTileData(osgUtil::CullVisitor* /*cv*/) { return 0; }

    protected:

        void setDirty(bool dirty);

        virtual ~VolumeTechnique();

        friend class osgVolume::VolumeTile;

        VolumeTile*    _volumeTile;

        typedef std::map<osgUtil::CullVisitor::Identifier*, osg::Matrix> ModelViewMatrixMap;
        OpenThreads::Mutex _mutex;
        ModelViewMatrixMap _modelViewMatrixMap;
};

}

#endif
