/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2018 Robert Osfield
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

#include <osgPresentation/AnimationMaterial>

#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Notify>
#include <osg/io_utils>

using namespace osgPresentation;

void AnimationMaterial::insert(double time,osg::Material* material)
{
    _timeControlPointMap[time] = material;
}

bool AnimationMaterial::getMaterial(double time,osg::Material& material) const
{
    if (_timeControlPointMap.empty()) return false;

    switch(_loopMode)
    {
        case(SWING):
        {
            double modulated_time = (time - getFirstTime())/(getPeriod()*2.0);
            double fraction_part = modulated_time - floor(modulated_time);
            if (fraction_part>0.5) fraction_part = 1.0-fraction_part;

            time = getFirstTime()+(fraction_part*2.0) * getPeriod();
            break;
        }
        case(LOOP):
        {
            double modulated_time = (time - getFirstTime())/getPeriod();
            double fraction_part = modulated_time - floor(modulated_time);
            time = getFirstTime()+fraction_part * getPeriod();
            break;
        }
        case(NO_LOOPING):
            // no need to modulate the time.
            break;
    }



    TimeControlPointMap::const_iterator second = _timeControlPointMap.lower_bound(time);
    if (second==_timeControlPointMap.begin())
    {
        material = *(second->second);
    }
    else if (second!=_timeControlPointMap.end())
    {
        TimeControlPointMap::const_iterator first = second;
        --first;

        // we have both a lower bound and the next item.

        // deta_time = second.time - first.time
        double delta_time = second->first - first->first;

        if (delta_time==0.0)
            material = *(first->second);
        else
        {
            interpolate(material,(time - first->first)/delta_time, *first->second, *second->second);
        }
    }
    else // (second==_timeControlPointMap.end())
    {
        material = *(_timeControlPointMap.rbegin()->second);
    }
    return true;
}

template<class T>
T interp(float r, const T& lhs, const T& rhs)
{
    return lhs*(1.0f-r)+rhs*r;
}


void AnimationMaterial::interpolate(osg::Material& material, float r, const osg::Material& lhs,const osg::Material& rhs) const
{
    material.setColorMode(lhs.getColorMode());

    material.setAmbient(osg::Material::FRONT_AND_BACK,interp(r, lhs.getAmbient(osg::Material::FRONT),rhs.getAmbient(osg::Material::FRONT)));
    if (!material.getAmbientFrontAndBack())
        material.setAmbient(osg::Material::BACK,interp(r, lhs.getAmbient(osg::Material::BACK),rhs.getAmbient(osg::Material::BACK)));

    material.setDiffuse(osg::Material::FRONT_AND_BACK,interp(r, lhs.getDiffuse(osg::Material::FRONT),rhs.getDiffuse(osg::Material::FRONT)));
    if (!material.getDiffuseFrontAndBack())
        material.setDiffuse(osg::Material::BACK,interp(r, lhs.getDiffuse(osg::Material::BACK),rhs.getDiffuse(osg::Material::BACK)));

    material.setSpecular(osg::Material::FRONT_AND_BACK,interp(r, lhs.getSpecular(osg::Material::FRONT),rhs.getSpecular(osg::Material::FRONT)));
    if (!material.getSpecularFrontAndBack())
        material.setSpecular(osg::Material::BACK,interp(r, lhs.getSpecular(osg::Material::BACK),rhs.getSpecular(osg::Material::BACK)));

    material.setEmission(osg::Material::FRONT_AND_BACK,interp(r, lhs.getEmission(osg::Material::FRONT),rhs.getEmission(osg::Material::FRONT)));
    if (!material.getEmissionFrontAndBack())
        material.setEmission(osg::Material::BACK,interp(r, lhs.getEmission(osg::Material::BACK),rhs.getEmission(osg::Material::BACK)));

    material.setShininess(osg::Material::FRONT_AND_BACK,interp(r, lhs.getShininess(osg::Material::FRONT),rhs.getShininess(osg::Material::FRONT)));
    if (!material.getShininessFrontAndBack())
        material.setShininess(osg::Material::BACK,interp(r, lhs.getShininess(osg::Material::BACK),rhs.getShininess(osg::Material::BACK)));
}

void AnimationMaterial::read(std::istream& in)
{
    while (!in.eof())
    {
        double time;
        osg::Vec4 color;
        in >> time >> color[0] >> color[1] >> color[2] >> color[3];
        if(!in.eof())
        {
            osg::Material* material = new osg::Material;
            material->setAmbient(osg::Material::FRONT_AND_BACK,color);
            material->setDiffuse(osg::Material::FRONT_AND_BACK,color);
            insert(time,material);
        }
    }
}

void AnimationMaterial::write(std::ostream& fout) const
{
    const TimeControlPointMap& tcpm = getTimeControlPointMap();
    for(TimeControlPointMap::const_iterator tcpmitr=tcpm.begin();
        tcpmitr!=tcpm.end();
        ++tcpmitr)
    {
        fout<<tcpmitr->first<<" "<<tcpmitr->second->getDiffuse(osg::Material::FRONT)<<std::endl;
    }
}

bool AnimationMaterial::requiresBlending() const
{
    const TimeControlPointMap& tcpm = getTimeControlPointMap();
    for(TimeControlPointMap::const_iterator tcpmitr=tcpm.begin();
        tcpmitr!=tcpm.end();
        ++tcpmitr)
    {
         if ((tcpmitr->second->getDiffuse(osg::Material::FRONT))[3]!=1.0f) return true;
    }
    return false;
}


void AnimationMaterialCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    if (_animationMaterial.valid() &&
        nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR &&
        nv->getFrameStamp())
    {
        double time = nv->getFrameStamp()->getReferenceTime();
        _latestTime = time;

        if (!_pause)
        {
            // Only update _firstTime the first time, when its value is still DBL_MAX
            if (_firstTime==DBL_MAX)
            {
                OSG_INFO<<"AnimationMaterialCallback::operator() resetting _firstTime to "<<time<<std::endl;
                _firstTime = time;
            }
            update(*node);

        }
    }

    // must call any nested node callbacks and continue subgraph traversal.
    NodeCallback::traverse(node,nv);
}

double AnimationMaterialCallback::getAnimationTime() const
{
    if (_firstTime==DBL_MAX) return 0.0f;
    else return ((_latestTime-_firstTime)-_timeOffset)*_timeMultiplier;
}

void AnimationMaterialCallback::update(osg::Node& node)
{
    osg::StateSet* stateset = node.getOrCreateStateSet();
    osg::Material* material =
        dynamic_cast<osg::Material*>(stateset->getAttribute(osg::StateAttribute::MATERIAL));

    if (!material)
    {
        material = new osg::Material;
        stateset->setAttribute(material,osg::StateAttribute::OVERRIDE);
    }

    _animationMaterial->getMaterial(getAnimationTime(),*material);
}


void AnimationMaterialCallback::reset()
{
#if 1
    _firstTime = DBL_MAX;
    _pauseTime = DBL_MAX;
#else
    _firstTime = _latestTime;
    _pauseTime = _latestTime;
#endif
}

void AnimationMaterialCallback::setPause(bool pause)
{
    if (_pause==pause)
    {
        return;
    }

    _pause = pause;

    if (_firstTime==DBL_MAX) return;

    if (_pause)
    {
        _pauseTime = _latestTime;
    }
    else
    {
        _firstTime += (_latestTime-_pauseTime);
    }
}
