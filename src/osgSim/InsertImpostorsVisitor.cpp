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
#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <osgSim/InsertImpostorsVisitor>
#include <osgSim/Impostor>

#include <algorithm>

using namespace osg;
using namespace osgSim;

InsertImpostorsVisitor::InsertImpostorsVisitor()
{
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    _impostorThresholdRatio = 30.0f;
    _maximumNumNestedImpostors = 3;
    _numNestedImpostors = 0;
}

void InsertImpostorsVisitor::reset()
{
    _groupList.clear();
    _lodList.clear();
    _numNestedImpostors = 0;
}

void InsertImpostorsVisitor::apply(Node& node)
{
    traverse(node);
}

void InsertImpostorsVisitor::apply(Group& node)
{
    _groupList.push_back(&node);
    
    ++_numNestedImpostors;
    if (_numNestedImpostors<_maximumNumNestedImpostors)
    {
        traverse(node);
    }
    --_numNestedImpostors;
}

void InsertImpostorsVisitor::apply(LOD& node)
{
    if (dynamic_cast<osgSim::Impostor*>(&node)==0)
    {
        _lodList.push_back(&node);
    }
    
    ++_numNestedImpostors;
    if (_numNestedImpostors<_maximumNumNestedImpostors)
    {
        traverse(node);
    }
    --_numNestedImpostors;
}

/* insert the required impostors into the scene graph.*/
void InsertImpostorsVisitor::insertImpostors()
{

    bool _insertImpostorsAboveGroups = true;
    bool _replaceLODsByImpostors = true;

    // handle group's
    if (_insertImpostorsAboveGroups)
    {
        std::sort(_groupList.begin(),_groupList.end());

        Group* previousGroup = NULL;
        for(GroupList::iterator itr=_groupList.begin();
            itr!=_groupList.end();
            ++itr)
        {
            Group* group = (*itr);
            if (group!=previousGroup)
            {
                const BoundingSphere& bs = group->getBound();
                if (bs.valid())
                {

                    // take a copy of the original parent list
                    // before we change it around by adding the group
                    // to an impostor.
                    Node::ParentList parentList = group->getParents();

                    Impostor* impostor = new Impostor;

                    // standard LOD settings
                    impostor->addChild(group);
                    impostor->setRange(0,0.0f,1e7f);
                    
                    // impostor specfic settings.
                    impostor->setImpostorThresholdToBound(_impostorThresholdRatio);

                    // now replace the group by the new impostor in all of the
                    // group's original parent list.
                    for(Node::ParentList::iterator pitr=parentList.begin();
                        pitr!=parentList.end();
                        ++pitr)
                    {
                        (*pitr)->replaceChild(group,impostor);
                    }

                }
            }
        }
    
    }    
    

    // handle LOD's
    if (_replaceLODsByImpostors)
    {
        std::sort(_lodList.begin(),_lodList.end());

        LOD* previousLOD = NULL;
        for(LODList::iterator itr=_lodList.begin();
            itr!=_lodList.end();
            ++itr)
        {
            osg::LOD* lod = (*itr);
            if (lod!=previousLOD)
            {
                const osg::BoundingSphere& bs = lod->getBound();
                if (bs.valid())
                {

                    // take a copy of the original parent list
                    // before we change it around by adding the lod
                    // to an impostor.
                    osg::Node::ParentList parentList = lod->getParents();

                    Impostor* impostor = new Impostor;

                    // standard LOD settings
                    for(unsigned int ci=0;ci<lod->getNumChildren();++ci)
                    {
                        impostor->addChild(lod->getChild(ci));
                        impostor->setRange(ci,lod->getMinRange(ci),lod->getMaxRange(ci));
                    }
                    
                    impostor->setCenter(lod->getCenter());
                    impostor->setCenterMode(lod->getCenterMode());

                    // impostor specfic settings.
                    impostor->setImpostorThresholdToBound(_impostorThresholdRatio);

                    // now replace the lod by the new impostor in all of the
                    // lod's original parent list.
                    for(Node::ParentList::iterator pitr=parentList.begin();
                        pitr!=parentList.end();
                        ++pitr)
                    {
                        (*pitr)->replaceChild(lod,impostor);
                    }

                }
            }
        }
    
    }    
}
