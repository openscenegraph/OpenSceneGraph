/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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


#include <osgUtil/PickVisitor>
#include <osgUtil/SceneView>
#include <osg/Projection>

using namespace osg;
using namespace osgUtil;

osgUtil::IntersectVisitor::HitList & PickIntersectVisitor::getHits(osgUtil::SceneView *scv, int x, int y)
{ // High level get intersection with sceneview using a ray from x,y on the screen
    int x0,y0,width,height;
    scv->getViewport(x0, y0, width, height);
//    setxy(-1+2*(float)(x-x0)/(float)width,    1-2*(float)(y-y0)/(float)height);
    // sets xp,yp as pixels scaled to a mapping of (-1,1, -1,1); needed for Projection from x,y pixels
    osg::Vec3 near_point,far_point;
     // get ends of line segment perpendicular to screen:
    if (!scv->projectWindowXYIntoObject(x,height-y,near_point,far_point))
    {
        osg::notify(osg::NOTICE) << "PickIntersect failed to calculate intersection ray."<< std::endl;
        return getHitList(NULL); // empty;
    }

    return getIntersections(scv->getSceneData(),near_point,far_point);
}

osgUtil::IntersectVisitor::HitList & PickIntersectVisitor::getIntersections(osg::Node *scene,
    osg::Vec3 near_point,osg::Vec3 far_point)
{ 
    // option for non-sceneView users: you need to get the screen perp line and call getIntersections
    // if you are using Projection nodes you should also call setxy to define the xp,yp positions for use with
    // the ray transformed by Projection
    _lineSegment = new osg::LineSegment;
    _lineSegment->set(near_point,far_point); // make a line segment
    addLineSegment(_lineSegment.get());
    
    scene->accept(*this);
    return getHitList(_lineSegment.get());
}

// pickvisitor - top level; test main scenegraph than traverse to lower Projections
osgUtil::IntersectVisitor::HitList & PickVisitor::getHits(osg::Node *nd, const osg::Vec3 near_point, const osg::Vec3 far_point)
{
    // High level get intersection with sceneview using a ray from x,y on the screen
    // sets xp,yp as pixels scaled to a mapping of (-1,1, -1,1); needed for Projection from x,y pixels
    
    // first get the standard hits in un-projected nodes
    _PIVsegHitList=_piv.getIntersections(nd,near_point,far_point); // fill hitlist

    // then get hits in projection nodes
    traverse(*(nd)); // check for projection nodes
    return _PIVsegHitList;
}

osgUtil::IntersectVisitor::HitList & PickVisitor::getHits(osgUtil::SceneView *scv, const double x, const double y)
{
     // High level get intersection with sceneview using a ray from x,y on the screen
    int x0,y0,width,height;
    scv->getViewport(x0, y0, width, height);
    setxy(-1+2*(float)(x-x0)/(float)width,    1-2*(float)(y-y0)/(float)height);
    // sets xp,yp as pixels scaled to a mapping of (-1,1, -1,1); needed for Projection from x,y pixels
    osg::Vec3 near_point,far_point;
     // get ends of line segment perpendicular to screen:
    if (!scv->projectWindowXYIntoObject(x,height-y,near_point,far_point))
    {
        osg::notify(osg::NOTICE) << "PickIntersect failed to calculate intersection ray."<< std::endl;
        return _piv.getHitList(NULL); // empty;
    }
    osg::Node *nd=scv->getSceneData();
    getHits(nd, near_point,far_point);
    return _PIVsegHitList;
}

osgUtil::IntersectVisitor::HitList & PickVisitor::getHits(void)
{
    // High level return current intersections
    return _PIVsegHitList;
}

osgUtil::IntersectVisitor::HitList& PickVisitor::getHits(osg::Node *scene, 
        const osg::Matrix &projm, const float x, const float y)
{ 
    // utility for non=sceneview viewers
    // x,y are values returned by 
    osg::Matrix inverseMVPW;
    inverseMVPW.invert(projm);
//    float ix=0.5f+0.5f*x, iy=0.5f+0.5f*y; // for this purpose, range from 0-1
    osg::Vec3 near_point = osg::Vec3(x,y,1.0f)*inverseMVPW;
    osg::Vec3 far_point = osg::Vec3(x,y,-1.0f)*inverseMVPW;
    setxy(x,y);    
    getHits(scene,near_point,far_point);
    return _PIVsegHitList;
}

void PickVisitor::apply(osg::Projection& pr)
{ // stack the intersect rays, transform to new projection, traverse
    // Assumes that the Projection is an absolute projection
    osg::Matrix mt=osg::Matrix::inverse(pr.getMatrix());
    osg::Vec3 npt=osg::Vec3(xp,yp,1.0f) * mt, farpt=osg::Vec3(xp,yp,-1.0f) * mt;

    // traversing the nodes children, using the projection direction
    for (unsigned int i=0; i<pr.getNumChildren(); i++) 
    {
        osg::Node *nodech=pr.getChild(i);
        osgUtil::IntersectVisitor::HitList &hli=_piv.getIntersections(nodech,npt, farpt);
        for(osgUtil::IntersectVisitor::HitList::iterator hitr=hli.begin();
            hitr!=hli.end();
            ++hitr)
        { // add the projection hits to the scene hits.
                // This is why _lineSegment is retained as a member of PickIntersectVisitor
            _PIVsegHitList.push_back(*hitr);
        }
        traverse(*nodech);
    }
}
