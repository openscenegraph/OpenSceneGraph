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
#include <osg/CollectOccludersVisitor>
#include <osg/Transform>
#include <osg/Switch>
#include <osg/LOD>
#include <osg/OccluderNode>
#include <osg/Projection>

#include <algorithm>

using namespace osg;

CollectOccludersVisitor::CollectOccludersVisitor():
    NodeVisitor(COLLECT_OCCLUDER_VISITOR,TRAVERSE_ACTIVE_CHILDREN)
{

    setCullingMode(VIEW_FRUSTUM_CULLING|
                   NEAR_PLANE_CULLING|
                   FAR_PLANE_CULLING|
                   SMALL_FEATURE_CULLING);

    _minimumShadowOccluderVolume = 0.005f;
    _maximumNumberOfActiveOccluders = 10;
    _createDrawables = false;

}

CollectOccludersVisitor::~CollectOccludersVisitor()
{
}

void CollectOccludersVisitor::reset()
{
    CullStack::reset();
    _occluderSet.clear();
}

float CollectOccludersVisitor::getDistanceToEyePoint(const Vec3& pos, bool withLODScale) const
{
    if (withLODScale) return (pos-getEyeLocal()).length()*getLODScale();
    else return (pos-getEyeLocal()).length();
}

float CollectOccludersVisitor::getDistanceToViewPoint(const Vec3& pos, bool withLODScale) const
{
    if (withLODScale) return (pos-getViewPointLocal()).length()*getLODScale();
    else return (pos-getViewPointLocal()).length();
}

float CollectOccludersVisitor::getDistanceFromEyePoint(const Vec3& pos, bool withLODScale) const
{
    const Matrix& matrix = *_modelviewStack.back();
    float dist = -(pos[0]*matrix(0,2)+pos[1]*matrix(1,2)+pos[2]*matrix(2,2)+matrix(3,2));

    if (withLODScale) return dist*getLODScale();
    else return dist*getLODScale();
}

void CollectOccludersVisitor::apply(osg::Node& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    handle_cull_callbacks_and_traverse(node);

    // pop the culling mode.
    popCurrentMask();
}

void CollectOccludersVisitor::apply(osg::Transform& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    ref_ptr<osg::RefMatrix> matrix = createOrReuseMatrix(*getModelViewMatrix());
    node.computeLocalToWorldMatrix(*matrix,this);
    pushModelViewMatrix(matrix.get(), node.getReferenceFrame());

    handle_cull_callbacks_and_traverse(node);

    popModelViewMatrix();

    // pop the culling mode.
    popCurrentMask();
}

void CollectOccludersVisitor::apply(osg::Projection& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    ref_ptr<osg::RefMatrix> matrix = createOrReuseMatrix(node.getMatrix());
    pushProjectionMatrix(matrix.get());

    handle_cull_callbacks_and_traverse(node);

    popProjectionMatrix();

    // pop the culling mode.
    popCurrentMask();
}

void CollectOccludersVisitor::apply(osg::Switch& node)
{
    apply((Group&)node);
}

void CollectOccludersVisitor::apply(osg::LOD& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    handle_cull_callbacks_and_traverse(node);

    // pop the culling mode.
    popCurrentMask();
}

void CollectOccludersVisitor::apply(osg::OccluderNode& node)
{
    // need to check if occlusion node is in the occluder
    // list, if so disable the appropriate ShadowOccluderVolume
    disableAndPushOccludersCurrentMask(_nodePath);


    if (isCulled(node))
    {
        popOccludersCurrentMask(_nodePath);
        return;
    }

//    std::cout<<"CollectOccludersVisitor:: We have found an Occlusion node in frustum"<<&node<<std::endl;

    // push the culling mode.
    pushCurrentMask();


    if (node.getOccluder())
    {
        // computeOccluder will check if the occluder is the view frustum,
        // if it ins't then the it will return false, when in it will
        // clip the occluder's polygons in clip space, then create occluder
        // planes, all with their normals facing inward towards the volume,
        // and then transform them back into projection space.
        ShadowVolumeOccluder svo;
        if (svo.computeOccluder(_nodePath, *node.getOccluder(), *this,_createDrawables))
        {

            if (svo.getVolume()>_minimumShadowOccluderVolume)
            {
                // need to test occluder against view frustum.
                //std::cout << "    adding in Occluder"<<std::endl;
                _occluderSet.insert(svo);
            }
            else
            {
                //std::cout << "    rejecting Occluder as its volume is too small "<<svo.getVolume()<<std::endl;
            }
        }
    }

    handle_cull_callbacks_and_traverse(node);

    // pop the culling mode.
    popCurrentMask();

    // pop the current mask for the disabled occluder
    popOccludersCurrentMask(_nodePath);
}

void CollectOccludersVisitor::removeOccludedOccluders()
{
    if (_occluderSet.empty()) return;

    ShadowVolumeOccluderSet::iterator occludeeItr=_occluderSet.begin();

    // skip the first element as this can't be occluded by anything else.
    occludeeItr++;

    // step through the rest of the occluders, remove occluders which are themselves occluded.
    for(;
        occludeeItr!=_occluderSet.end();
        ++occludeeItr)
    {

        // search for any occluders that occlude the current occluder,
        // we only need to test any occluder near the front of the set since
        // you can't be occluder by something smaller than you.
        ShadowVolumeOccluder& occludee = const_cast<ShadowVolumeOccluder&>(*occludeeItr);
        ShadowVolumeOccluder::HoleList& holeList = occludee.getHoleList();

        for(ShadowVolumeOccluderSet::iterator occluderItr=_occluderSet.begin();
            occluderItr!=occludeeItr;
            ++occluderItr)
        {
            // cast away constness of the std::set element since ShadowVolumeOccluder::contains() is non const,
            // and the std::set is a const, just for the invariance of the operator <!! Ahhhhh. oh well the below
            // should be robust since contains won't change the getVolume which is used by the operator <.  Honest,  :-)
            ShadowVolumeOccluder* occluder = const_cast<ShadowVolumeOccluder*>(&(*occluderItr));
            if (occluder->contains(occludee.getOccluder().getReferenceVertexList()))
            {
                // erase occluder from set.
                // take a copy of the iterator then rewind it one element so to prevent invalidating the occludeeItr.
                ShadowVolumeOccluderSet::iterator eraseItr = occludeeItr--;
                _occluderSet.erase(eraseItr);
                break;
            }

            // now check all the holes in the occludee against the occluder,
            // do so in reverse order so that the iterators remain valid.
            for(ShadowVolumeOccluder::HoleList::reverse_iterator holeItr=holeList.rbegin();
                holeItr!=holeList.rend();
                )
            {
                if (occluder->contains((*holeItr).getReferenceVertexList()))
                {
                    holeList.erase((++holeItr).base());
                }
                else
                {
                    ++holeItr;
                }

            }

        }
    }


    if (_occluderSet.size()<=_maximumNumberOfActiveOccluders) return;

    // move the iterator to the _maximumNumberOfActiveOccluders th occluder.
    occludeeItr = _occluderSet.begin();
    for(unsigned int i=0;i<_maximumNumberOfActiveOccluders;++i)
        ++occludeeItr;

    // discard last occluders.
    _occluderSet.erase(occludeeItr,_occluderSet.end());

}
