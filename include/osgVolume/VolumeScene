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

#ifndef OSGVOLUMESCENE
#define OSGVOLUMESCENE 1

#include <osg/Group>
#include <osg/Texture2D>
#include <osgUtil/CullVisitor>

#include <osgVolume/VolumeTile>

namespace osgVolume {

/** VolumeScene provides high level support for doing multi-pass rendering of volumes where the main scene to rendered to color and depth textures and then re-rendered for the purposes of volume rendering.*/
class OSGVOLUME_EXPORT VolumeScene : public osg::Group
{
    public:

        VolumeScene();

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        VolumeScene(const VolumeScene&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Node(osgVolume, VolumeScene);

        virtual void traverse(osg::NodeVisitor& nv);

        TileData* tileVisited(osgUtil::CullVisitor* cv, VolumeTile* tile);
        TileData* getTileData(osgUtil::CullVisitor* cv, VolumeTile* tile);

    protected:

        virtual ~VolumeScene();

        typedef std::map< VolumeTile*, osg::ref_ptr<TileData> > Tiles;

        class ViewData : public osg::Referenced
        {
            public:
                ViewData();

                void clearTiles();
                void visitTile(VolumeTile* tile);

                osg::ref_ptr<osg::Texture2D>    _depthTexture;
                osg::ref_ptr<osg::Texture2D>    _colorTexture;
                osg::ref_ptr<osg::Camera>       _rttCamera;
                osg::ref_ptr<osg::Node>         _backdropSubgraph;
                osg::ref_ptr<osg::Geometry>     _geometry;
                osg::ref_ptr<osg::Vec3Array>    _vertices;
                osg::ref_ptr<osg::StateSet>     _stateset;
                osg::ref_ptr<osg::Uniform>      _viewportDimensionsUniform;

                Tiles                           _tiles;

            protected:
                virtual ~ViewData() {}
        };

        typedef std::map< osgUtil::CullVisitor*, osg::ref_ptr<ViewData> >  ViewDataMap;
        OpenThreads::Mutex _viewDataMapMutex;
        ViewDataMap _viewDataMap;
};

}

#endif
