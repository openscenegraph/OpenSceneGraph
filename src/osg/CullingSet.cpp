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
#include <osg/CullingSet>

using namespace osg;

CullingSet::CullingSet()
{
    _mask = ALL_CULLING;
    _pixelSizeVector.set(0,0,0,1);
    _smallFeatureCullingPixelSize=1.0f;
}

CullingSet::~CullingSet()
{
}

void PrintNodePath(const NodePath& nodePath)
{
    for(NodePath::const_iterator itr=nodePath.begin();
        itr!=nodePath.end();
        ++itr)
    {
        std::cout<<*itr<<"  ";
    }
}


void CullingSet::disableAndPushOccludersCurrentMask(NodePath& nodePath)
{
    //std::cout<<"  trying to disable occluder ";PrintNodePath(nodePath);std::cout<<std::endl;
    for(OccluderList::iterator itr=_occluderList.begin();
        itr!=_occluderList.end();
        ++itr)
    {
        //std::cout<<"    checking against ";PrintNodePath(itr->getNodePath());std::cout<<std::endl;
        if (itr->getNodePath()==nodePath)
        {
            //std::cout<<"  ++ disabling occluder "<<itr<<std::endl;
            // we have trapped for the case an occlude potentially occluding itself,
            // to prevent this we disable the results mask so that no subsequnt 
            // when the next pushCurrentMask calls happens this occluder is switched off.
            itr->disableResultMasks();
            itr->pushCurrentMask();
        }
    }
}


void CullingSet::popOccludersCurrentMask(NodePath& nodePath)
{
    //std::cout<<"  trying to pop occluder ";PrintNodePath(nodePath);std::cout<<std::endl;
    for(OccluderList::iterator itr=_occluderList.begin();
        itr!=_occluderList.end();
        ++itr)
    {
        //std::cout<<"    checking against ";PrintNodePath(itr->getNodePath());std::cout<<std::endl;
        if (itr->getNodePath()==nodePath)
        {
            //std::cout<<"  popping occluder "<<itr<<std::endl;
            // we have trapped for the case an occlude potentially occluding itself,
            // to prevent this we disable the results mask so that no subsequnt 
            // when the next pushCurrentMask calls happens this occluder is switched off.
            itr->popCurrentMask();
        }
    }
}
