/*  -*-c++-*-
 *  Copyright (C) 2008 Cedric Pinson <cedric.pinson@plopbyte.net>
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

#include <osg/Geode>
#include <osgAnimation/MorphGeometry>
#include <osgAnimation/RigGeometry>

#include <sstream>

using namespace osgAnimation;

MorphGeometry::MorphGeometry() :
    _dirty(false),
    _method(NORMALIZED),
    _morphNormals(true)
{
    setUseDisplayList(false);
    setUpdateCallback(new UpdateMorphGeometry);
    setDataVariance(osg::Object::DYNAMIC);
    setUseVertexBufferObjects(true);
}

MorphGeometry::MorphGeometry(const osg::Geometry& b) :
    osg::Geometry(b, osg::CopyOp::DEEP_COPY_ARRAYS),
    _dirty(false),
    _method(NORMALIZED),
    _morphNormals(true)
{
    setUseDisplayList(false);
    setUpdateCallback(new UpdateMorphGeometry);
    setDataVariance(osg::Object::DYNAMIC);
    setUseVertexBufferObjects(true);
}

MorphGeometry::MorphGeometry(const MorphGeometry& b, const osg::CopyOp& copyop) :
    osg::Geometry(b,copyop),
    _dirty(b._dirty),
    _method(b._method),
    _morphTargets(b._morphTargets),
    _positionSource(b._positionSource),
    _normalSource(b._normalSource),
    _morphNormals(b._morphNormals)
{
    setUseDisplayList(false);
    setUseVertexBufferObjects(true);
}


void MorphGeometry::transformSoftwareMethod()
{
    if (_dirty)
    {
        // See if we have an internal optimized geometry
        osg::Geometry* morphGeometry = this;

        osg::Vec3Array* pos = dynamic_cast<osg::Vec3Array*>(morphGeometry->getVertexArray());

        if(pos)
        {
            if ( _positionSource.size() != pos->size())
            {
                _positionSource = std::vector<osg::Vec3>(pos->begin(),pos->end());
                pos->setDataVariance(osg::Object::DYNAMIC);
            }

            osg::Vec3Array* normal = dynamic_cast<osg::Vec3Array*>(morphGeometry->getNormalArray());
            bool normalmorphable = _morphNormals && normal;
            if (normal && _normalSource.size() != normal->size())
            {
                _normalSource = std::vector<osg::Vec3>(normal->begin(),normal->end());
                normal->setDataVariance(osg::Object::DYNAMIC);
            }


            if (!_positionSource.empty())
            {
                bool initialized = false;
                if (_method == NORMALIZED)
                {
                    // base * 1 - (sum of weights) + sum of (weight * target)
                    float baseWeight = 0;
                    for (unsigned int i=0; i < _morphTargets.size(); i++)
                    {
                        baseWeight += _morphTargets[i].getWeight();
                    }
                    baseWeight = 1 - baseWeight;

                    if (baseWeight != 0)
                    {
                        initialized = true;
                        for (unsigned int i=0; i < pos->size(); i++)
                        {
                            (*pos)[i] = _positionSource[i] * baseWeight;
                        }
                        if (normalmorphable)
                        {
                            for (unsigned int i=0; i < normal->size(); i++)
                            {
                                (*normal)[i] = _normalSource[i] * baseWeight;
                            }
                        }
                    }
                }
                else //if (_method == RELATIVE)
                {
                    // base + sum of (weight * target)
                    initialized = true;
                    for (unsigned int i=0; i < pos->size(); i++)
                    {
                        (*pos)[i] = _positionSource[i];
                    }
                    if (normalmorphable)
                    {
                        for (unsigned int i=0; i < normal->size(); i++)
                        {
                            (*normal)[i] = _normalSource[i];
                        }
                    }
                }

                for (unsigned int i=0; i < _morphTargets.size(); i++)
                {
                    if (_morphTargets[i].getWeight() > 0)
                    {
                        // See if any the targets use the internal optimized geometry
                        osg::Geometry* targetGeometry = _morphTargets[i].getGeometry();

                        osg::Vec3Array* targetPos = dynamic_cast<osg::Vec3Array*>(targetGeometry->getVertexArray());
                        osg::Vec3Array* targetNormals = dynamic_cast<osg::Vec3Array*>(targetGeometry->getNormalArray());
                        normalmorphable = normalmorphable && targetNormals;
                        if(targetPos)
                        {
                            if (initialized)
                            {
                                // If vertices are initialized, add the morphtargets
                                for (unsigned int j=0; j < pos->size(); j++)
                                {
                                    (*pos)[j] += (*targetPos)[j] * _morphTargets[i].getWeight();
                                }

                                if (normalmorphable)
                                {
                                    for (unsigned int j=0; j < normal->size(); j++)
                                    {
                                        (*normal)[j] += (*targetNormals)[j] * _morphTargets[i].getWeight();
                                    }
                                }
                            }
                            else
                            {
                                // If not initialized, initialize with this morph target
                                initialized = true;
                                for (unsigned int j=0; j < pos->size(); j++)
                                {
                                    (*pos)[j] = (*targetPos)[j] * _morphTargets[i].getWeight();
                                }

                                if (normalmorphable)
                                {
                                    for (unsigned int j=0; j < normal->size(); j++)
                                    {
                                        (*normal)[j] = (*targetNormals)[j] * _morphTargets[i].getWeight();
                                    }
                                }
                            }
                        }
                    }
                }

                pos->dirty();
                if (normalmorphable)
                {
                    for (unsigned int j=0; j < normal->size(); j++)
                    {
                        (*normal)[j].normalize();
                    }
                    normal->dirty();
                }
            }

            dirtyBound();

        }
        _dirty = false;
    }
}

UpdateMorph::UpdateMorph(const UpdateMorph& apc,const osg::CopyOp& copyop) :
    osg::Object(apc, copyop),
    osg::Callback(apc, copyop),
    AnimationUpdateCallback<osg::NodeCallback>(apc, copyop)
{
}

UpdateMorph::UpdateMorph(const std::string& name) : AnimationUpdateCallback<osg::NodeCallback>(name)
{
}

/** Callback method called by the NodeVisitor when visiting a node.*/
void UpdateMorph::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    if (nv && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {
        osg::Geode* geode = dynamic_cast<osg::Geode*>(node);
        if (geode)
        {
            unsigned int numDrawables = geode->getNumDrawables();
            for (unsigned int i = 0; i != numDrawables; ++i)
            {
                osg::Drawable *drw = geode->getDrawable(i);
                osgAnimation::RigGeometry *rig = dynamic_cast<osgAnimation::RigGeometry*>(drw);
                if(rig && rig->getSourceGeometry())
                    drw = rig->getSourceGeometry();

                osgAnimation::MorphGeometry* morph = dynamic_cast<osgAnimation::MorphGeometry*>(drw);
                if (morph)
                {
                    // Update morph weights
                    std::map<int, osg::ref_ptr<osgAnimation::FloatTarget> >::iterator iter = _weightTargets.begin();
                    while (iter != _weightTargets.end())
                    {
                        if (iter->second->getValue() >= 0)
                        {
                            morph->setWeight(iter->first, iter->second->getValue());
                        }
                        ++iter;
                    }
                }
            }
        }
    }
    traverse(node,nv);
}



bool UpdateMorph::needLink() const
{
    // the idea is to return true if nothing is linked
    return (_weightTargets.size() == 0);
}

bool UpdateMorph::link(osgAnimation::Channel* channel)
{
    // Typically morph geometries only have the weights for morph targets animated

    std::istringstream iss(channel->getName());

    int weightIndex;
    iss >> weightIndex;

    if (iss.fail())
    {
        return false;
    }

    if (weightIndex >= 0)
    {
        osgAnimation::FloatTarget* ft = _weightTargets[weightIndex].get();
        if (!ft)
        {
            ft = new osgAnimation::FloatTarget;
            _weightTargets[weightIndex] = ft;
        }
        return channel->setTarget(ft);
    }
    else
    {
        OSG_WARN << "Channel " << channel->getName() << " does not contain a valid symbolic name for this class" << std::endl;
    }
    return false;
}

int UpdateMorph::link(Animation* animation)
{
    if (getNumTarget() == 0)
    {
        osg::notify(osg::WARN) << "An update callback has no name, it means it could link only with \"\" named Target, often an error, discard" << std::endl;
        return 0;
    }

    unsigned int nbLinks = 0;
    for (ChannelList::iterator channel = animation->getChannels().begin();
         channel != animation->getChannels().end();
         ++channel)
    {
        std::string targetName = (*channel)->getTargetName();
        for(int i = 0, num = getNumTarget(); i < num; ++i) {
            if (targetName == getTargetName(i))
            {
                AnimationUpdateCallbackBase* a = this;
                a->link((*channel).get());
                nbLinks++;
            }
        }
    }
    return nbLinks;
}
