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

#include <osgVolume/MultipassTechnique>
#include <osgVolume/VolumeTile>
#include <osgVolume/VolumeScene>

#include <osg/Geometry>
#include <osg/io_utils>

#include <osg/Program>
#include <osg/TexGen>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TransferFunction>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

namespace osgVolume
{


MultipassTechnique::MultipassTechnique()
{
}

MultipassTechnique::MultipassTechnique(const MultipassTechnique& fft,const osg::CopyOp& copyop):
    VolumeTechnique(fft,copyop)
{
}

MultipassTechnique::~MultipassTechnique()
{
}

void MultipassTechnique::init()
{
    OSG_INFO<<"MultipassTechnique::init()"<<std::endl;

    if (!_volumeTile)
    {
        OSG_NOTICE<<"MultipassTechnique::init(), error no volume tile assigned."<<std::endl;
        return;
    }

    if (_volumeTile->getLayer()==0)
    {
        OSG_NOTICE<<"MultipassTechnique::init(), error no layer assigend to volume tile."<<std::endl;
        return;
    }

    if (_volumeTile->getLayer()->getImage()==0)
    {
        OSG_NOTICE<<"MultipassTechnique::init(), error no image assigned to layer."<<std::endl;
        return;
    }

    OSG_NOTICE<<"MultipassTechnique::init() Need to set up"<<std::endl;
}

void MultipassTechnique::update(osgUtil::UpdateVisitor* /*uv*/)
{
//    OSG_NOTICE<<"MultipassTechnique:update(osgUtil::UpdateVisitor* nv):"<<std::endl;
}

void MultipassTechnique::cull(osgUtil::CullVisitor* cv)
{
    OSG_NOTICE<<"MultipassTechnique::cull() Need to set up"<<std::endl;
    osg::NodePath& nodePath = cv->getNodePath();
    for(osg::NodePath::reverse_iterator itr = nodePath.rbegin();
        itr != nodePath.rend();
        ++itr)
    {
        OSG_NOTICE<<"  parent path node "<<*itr<<" "<<(*itr)->className()<<std::endl;
        osgVolume::VolumeScene* vs = dynamic_cast<osgVolume::VolumeScene*>(*itr);
        if (vs)
        {
            OSG_NOTICE<<"  HAVE VolumeScene"<<std::endl;
            vs->tileVisited(cv, getVolumeTile());
            break;
        }
    }
}

void MultipassTechnique::cleanSceneGraph()
{
    OSG_NOTICE<<"MultipassTechnique::cleanSceneGraph()"<<std::endl;
}

void MultipassTechnique::traverse(osg::NodeVisitor& nv)
{
    // OSG_NOTICE<<"MultipassTechnique::traverse(osg::NodeVisitor& nv)"<<std::endl;
    if (!_volumeTile) return;

    // if app traversal update the frame count.
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (_volumeTile->getDirty()) _volumeTile->init();

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


    if (_volumeTile->getDirty())
    {
        OSG_INFO<<"******* Doing init ***********"<<std::endl;
        _volumeTile->init();
    }
}


} // end of osgVolume namespace
