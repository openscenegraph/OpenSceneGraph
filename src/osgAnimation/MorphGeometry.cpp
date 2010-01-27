/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <cedric.pinson@plopbyte.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *
 * Roland Smeenk
 * Cedric Pinson <cedric.pinson@plopbyte.net>
 *
 */
#include <osg/Geode>
#include <osgAnimation/MorphGeometry>

#include <stdlib.h>

using namespace osgAnimation;

MorphGeometry::MorphGeometry() :
    _dirty(false),
    _method(NORMALIZED),
    _morphNormals(true)
{
    setUseDisplayList(false);
    setUpdateCallback(new UpdateVertex);
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
    setUpdateCallback(new UpdateVertex);
    setDataVariance(osg::Object::DYNAMIC);
    setUseVertexBufferObjects(true);
    if (b.getInternalOptimizedGeometry())
        computeInternalOptimizedGeometry();
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
    if (b.getInternalOptimizedGeometry())
        computeInternalOptimizedGeometry();
}

void MorphGeometry::transformSoftwareMethod()
{
    if (_dirty)
    {
        // See if we have an internal optimized geometry
        osg::Geometry* morphGeometry = this;
        if (_internalOptimizedGeometry.valid())
            morphGeometry = _internalOptimizedGeometry.get();

        osg::Vec3Array* pos = dynamic_cast<osg::Vec3Array*>(morphGeometry->getVertexArray());
        if (pos && _positionSource.size() != pos->size())
        {
            _positionSource = std::vector<osg::Vec3>(pos->begin(),pos->end());
            pos->setDataVariance(osg::Object::DYNAMIC);
        }

        osg::Vec3Array* normal = dynamic_cast<osg::Vec3Array*>(morphGeometry->getNormalArray());
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
                    if (_morphNormals)
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
                if (_morphNormals)
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
                    osg::Geometry* targetGeometry = _morphTargets[i].getGeometry()->getInternalOptimizedGeometry();
                    if (!targetGeometry)
                        targetGeometry = _morphTargets[i].getGeometry();

                    osg::Vec3Array* targetPos = dynamic_cast<osg::Vec3Array*>(targetGeometry->getVertexArray());
                    osg::Vec3Array* targetNormals = dynamic_cast<osg::Vec3Array*>(targetGeometry->getNormalArray());

                    if (initialized)
                    {
                        // If vertices are initialized, add the morphtargets
                        for (unsigned int j=0; j < pos->size(); j++)
                        {
                            (*pos)[j] += (*targetPos)[j] * _morphTargets[i].getWeight();
                        }

                        if (_morphNormals)
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

                        if (_morphNormals)
                        {
                            for (unsigned int j=0; j < normal->size(); j++)
                            {
                                (*normal)[j] = (*targetNormals)[j] * _morphTargets[i].getWeight();
                            }
                        }
                    }
                }
            }

            pos->dirty();
            if (_morphNormals)
            {
                for (unsigned int j=0; j < normal->size(); j++)
                {
                    (*normal)[j].normalize();
                }
                normal->dirty();
            }
        }

        dirtyBound();
        _dirty = false;
    }
}

UpdateMorph::UpdateMorph(const UpdateMorph& apc,const osg::CopyOp& copyop) : 
    osg::Object(apc, copyop),
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
                osgAnimation::MorphGeometry* morph = dynamic_cast<osgAnimation::MorphGeometry*>(geode->getDrawable(i));
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

    // Expect a weight value
    // TODO Should we make this more generic to handle other things than single values?
    int weightIndex = atoi(channel->getName().c_str());

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
        osg::notify(osg::WARN) << "Channel " << channel->getName() << " does not contain a valid symbolic name for this class" << std::endl;
    }
    return false;
}
