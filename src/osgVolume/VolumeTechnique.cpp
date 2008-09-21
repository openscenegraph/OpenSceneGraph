/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield 
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

#include <osgVolume/VolumeTechnique>
#include <osgVolume/Brick>

using namespace osgVolume;

VolumeTechnique::VolumeTechnique():
    _brick(0)
{
    setThreadSafeRefUnref(true);
}

VolumeTechnique::VolumeTechnique(const VolumeTechnique& rhs,const osg::CopyOp& copyop):
    osg::Object(rhs,copyop),
    _brick(0)
{
}

VolumeTechnique::~VolumeTechnique()
{
}

void VolumeTechnique::init()
{
    osg::notify(osg::NOTICE)<<className()<<"::initialize(..) not implementated yet"<<std::endl;
}

void VolumeTechnique::update(osgUtil::UpdateVisitor* uv)
{
    osg::notify(osg::NOTICE)<<className()<<"::update(..) not implementated yet"<<std::endl;
    if (_brick) _brick->osg::Group::traverse(*uv);
}

void VolumeTechnique::cull(osgUtil::CullVisitor* cv)
{
    osg::notify(osg::NOTICE)<<className()<<"::cull(..) not implementated yet"<<std::endl;
    if (_brick) _brick->osg::Group::traverse(*cv);
}

void VolumeTechnique::cleanSceneGraph()
{
    osg::notify(osg::NOTICE)<<className()<<"::cleanSceneGraph(..) not implementated yet"<<std::endl;
}

void VolumeTechnique::traverse(osg::NodeVisitor& nv)
{
    if (!_brick) return;

    // if app traversal update the frame count.
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (_brick->getDirty()) _brick->init();

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

    if (_brick->getDirty()) _brick->init();

    // otherwise fallback to the Group::traverse()
    _brick->osg::Group::traverse(nv);
}
