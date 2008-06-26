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


#include <osgUtil/IntersectionVisitor>

#include <osg/PagedLOD>
#include <osg/Transform>
#include <osg/Projection>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/io_utils>

using namespace osgUtil;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  IntersectorGroup
//

IntersectorGroup::IntersectorGroup()
{
}

void IntersectorGroup::addIntersector(Intersector* intersector)
{
    _intersectors.push_back(intersector);
}

void IntersectorGroup::clear()
{
    _intersectors.clear();
}

Intersector* IntersectorGroup::clone(osgUtil::IntersectionVisitor& iv)
{
    IntersectorGroup* ig = new IntersectorGroup;
    
    // now copy across all intersectors that arn't disabled.
    for(Intersectors::iterator itr = _intersectors.begin();
        itr != _intersectors.end();
        ++itr)
    {
        if (!(*itr)->disabled())
        {
            ig->addIntersector( (*itr)->clone(iv) );
        }
    }

    return ig;
}

bool IntersectorGroup::enter(const osg::Node& node)
{
    if (disabled()) return false;
    
    bool foundIntersections = false;
    
    for(Intersectors::iterator itr = _intersectors.begin();
        itr != _intersectors.end();
        ++itr)
    {
        if ((*itr)->disabled()) (*itr)->incrementDisabledCount();
        else if ((*itr)->enter(node)) foundIntersections = true;
        else (*itr)->incrementDisabledCount();
    }
    
    if (!foundIntersections) 
    {
        // need to call leave to clean up the DisabledCount's.
        leave();
        return false;
    }
    
    // we have found at least one suitable intersector, so return true
    return true;
}

void IntersectorGroup::leave()
{
    for(Intersectors::iterator itr = _intersectors.begin();
        itr != _intersectors.end();
        ++itr)
    {
        if ((*itr)->disabled()) (*itr)->decrementDisabledCount();
    }
}

void IntersectorGroup::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)
{
    if (disabled()) return;

    unsigned int numTested = 0;
    for(Intersectors::iterator itr = _intersectors.begin();
        itr != _intersectors.end();
        ++itr)
    {
        if (!(*itr)->disabled())
        {
            (*itr)->intersect(iv, drawable);
            
            ++numTested;
        }
    }
    
    // osg::notify(osg::NOTICE)<<"Number testing "<<numTested<<std::endl;

}

void IntersectorGroup::reset()
{
    Intersector::reset();
    
    for(Intersectors::iterator itr = _intersectors.begin();
        itr != _intersectors.end();
        ++itr)
    {
        (*itr)->reset();
    }
}

bool IntersectorGroup::containsIntersections()
{
    for(Intersectors::iterator itr = _intersectors.begin();
        itr != _intersectors.end();
        ++itr)
    {
        if ((*itr)->containsIntersections()) return true;
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  IntersectionVisitor
//

IntersectionVisitor::IntersectionVisitor(Intersector* intersector, ReadCallback* readCallback)
{
    // override the default node visitor mode.
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
    
    setIntersector(intersector);
    
    setReadCallback(readCallback);
}

void IntersectionVisitor::setIntersector(Intersector* intersector)
{
    // keep reference around just in case intersector is already in the _intersectorStack, otherwise the clear could delete it.
    osg::ref_ptr<Intersector> temp = intersector;

    _intersectorStack.clear();

    if (intersector) _intersectorStack.push_back(intersector);
}

void IntersectionVisitor::reset()
{
    if (!_intersectorStack.empty())
    {
        osg::ref_ptr<Intersector> intersector = _intersectorStack.front();
        intersector->reset();
        
        _intersectorStack.clear();
        _intersectorStack.push_back(intersector);
    }
}

void IntersectionVisitor::apply(osg::Node& node)
{
    // osg::notify(osg::NOTICE)<<"apply(Node&)"<<std::endl;

    if (!enter(node)) return;

    // osg::notify(osg::NOTICE)<<"inside apply(Node&)"<<std::endl;

    traverse(node);

    leave();
}

void IntersectionVisitor::apply(osg::Group& group)
{
    if (!enter(group)) return;

    traverse(group);

    leave();
}

void IntersectionVisitor::apply(osg::Geode& geode)
{
    // osg::notify(osg::NOTICE)<<"apply(Geode&)"<<std::endl;

    if (!enter(geode)) return;

    // osg::notify(osg::NOTICE)<<"inside apply(Geode&)"<<std::endl;

    for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
    {
        intersect( geode.getDrawable(i) );
    }

    leave();
}

void IntersectionVisitor::apply(osg::Billboard& billboard)
{
    if (!enter(billboard)) return;

    for(unsigned int i=0; i<billboard.getNumDrawables(); ++i)
    {
        intersect( billboard.getDrawable(i) );
    }

    leave();
}

void IntersectionVisitor::apply(osg::LOD& lod)
{
    if (!enter(lod)) return;

    traverse(lod);

    leave();
}


void IntersectionVisitor::apply(osg::PagedLOD& plod)
{
    if (!enter(plod)) return;

    if (plod.getNumFileNames()>0)
    {
        osg::ref_ptr<osg::Node> highestResChild;

        if (plod.getNumFileNames() != plod.getNumChildren() && _readCallback.valid())
        {
            highestResChild = _readCallback->readNodeFile( plod.getDatabasePath() + plod.getFileName(plod.getNumFileNames()-1) );
        }
        
        if ( !highestResChild.valid() && plod.getNumChildren()>0)
        {
            highestResChild = plod.getChild( plod.getNumChildren()-1 );
        }

        if (highestResChild.valid())
        {
            highestResChild->accept(*this);
        }
    }

    leave();
}


void IntersectionVisitor::apply(osg::Transform& transform)
{
    if (!enter(transform)) return;

    osg::ref_ptr<osg::RefMatrix> matrix = _modelStack.empty() ? new osg::RefMatrix() : new osg::RefMatrix(*_modelStack.back());
    transform.computeLocalToWorldMatrix(*matrix,this);

    pushModelMatrix(matrix.get());

    // now push an new intersector clone transform to the new local coordinates
    push_clone();

    traverse(transform);
    
    // pop the clone.
    pop_clone();
    
    popModelMatrix();

    // tidy up an cached cull variables in the current intersector.
    leave();
}


void IntersectionVisitor::apply(osg::Projection& projection)
{
    if (!enter(projection)) return;

    pushProjectionMatrix(new osg::RefMatrix(projection.getMatrix()) );

    // now push an new intersector clone transform to the new local coordinates
    push_clone();

    traverse(projection);
    
    // pop the clone.
    pop_clone();
    
    popProjectionMatrix();

    leave();
}


void IntersectionVisitor::apply(osg::Camera& camera)
{
    // osg::notify(osg::NOTICE)<<"apply(Camera&)"<<std::endl;

    // note, commenting out right now because default Camera setup is with the culling active.  Should this be changed?
    // if (!enter(camera)) return;
    
    // osg::notify(osg::NOTICE)<<"inside apply(Camera&)"<<std::endl;

    if (camera.getViewport()) pushWindowMatrix( camera.getViewport() );
    pushProjectionMatrix( new osg::RefMatrix(camera.getProjectionMatrix()) );
    pushViewMatrix( new osg::RefMatrix(camera.getViewMatrix()) );
    pushModelMatrix( new osg::RefMatrix() );

    // now push an new intersector clone transform to the new local coordinates
    push_clone();

    traverse(camera);
    
    // pop the clone.
    pop_clone();
    
    popModelMatrix();
    popViewMatrix();
    popProjectionMatrix();
    if (camera.getViewport()) popWindowMatrix();

    // leave();
}
