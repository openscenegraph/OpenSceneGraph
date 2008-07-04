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

#include <osg/KdTree>
#include <osg/Geode>
#include <osg/io_utils>

using namespace osg;


////////////////////////////////////////////////////////////////////////////////
//
// KdTree

bool KdTree::build(osg::Geometry* geometry)
{
    osg::notify(osg::NOTICE)<<"KdTree::build("<<geometry<<")"<<std::endl;
    return true;
}

bool KdTree::intersect(const osg::Vec3& start, const osg::Vec3& end, LineSegmentIntersections& intersections)
{
    osg::notify(osg::NOTICE)<<"KdTree::intersect("<<start<<","<<end<<")"<<std::endl;
    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
// KdTreeBuilder
KdTreeBuilder::KdTreeBuilder():
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _maxNumLevels(16),
    _targetNumTrianglesPerLeaf(4),
    _numVerticesProcessed(0)
{            
    _kdTreePrototype = new osg::KdTree;
}


void KdTreeBuilder::apply(osg::Geode& geode)
{
    for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
    {            

        osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
        if (geom)
        {
            osg::KdTree* previous = dynamic_cast<osg::KdTree*>(geom->getShape());
            if (previous) continue;

            osg::ref_ptr<osg::KdTree> kdTree = dynamic_cast<osg::KdTree*>(_kdTreePrototype->cloneType());

            if (kdTree->build(geom))
            {
                geom->setShape(kdTree.get());
            }
        }   
    }
}
