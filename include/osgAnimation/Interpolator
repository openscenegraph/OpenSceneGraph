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
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 *         Michael Platings <mplatings@pixelpower.com>
 */

#ifndef OSGANIMATION_INTERPOLATOR
#define OSGANIMATION_INTERPOLATOR 1

#include <osg/Notify>
#include <osgAnimation/Keyframe>

namespace osgAnimation
{

    template <class TYPE, class KEY>
    class TemplateInterpolatorBase
    {
    public:
        typedef KEY KeyframeType;
        typedef TYPE UsingType;

    public:
        TemplateInterpolatorBase() {}

        int getKeyIndexFromTime(const TemplateKeyframeContainer<KEY>& keys, double time) const
        {
            int key_size = keys.size();
            if (!key_size) {
                osg::notify(osg::WARN) << "TemplateInterpolatorBase::getKeyIndexFromTime the container is empty, impossible to get key index from time" << std::endl;;
                return -1;
            }
            const TemplateKeyframe<KeyframeType>* keysVector = &keys.front();
            int k = 0;
            int l = key_size;
            int mid = key_size/2;
            while(mid != k){
                double time1 = keysVector[mid].getTime();
                if(time1 < time){
                    k = mid;
                } else {
                    l = mid;
                }
                mid = (l+k)/2;
            }
            return k;
        }
    };


    template <class TYPE, class KEY=TYPE>
    class TemplateStepInterpolator : public TemplateInterpolatorBase<TYPE,KEY>
    {
    public:

        TemplateStepInterpolator() {}
        void getValue(const TemplateKeyframeContainer<KEY>& keyframes, double time, TYPE& result) const
        {

            if (time >= keyframes.back().getTime())
            {
                result = keyframes.back().getValue();
                return;
            }
            else if (time <= keyframes.front().getTime())
            {
                result = keyframes.front().getValue();
                return;
            }

            int i = this->getKeyIndexFromTime(keyframes,time);
            result = keyframes[i].getValue();
        }
    };


    template <class TYPE, class KEY=TYPE>
    class TemplateLinearInterpolator : public TemplateInterpolatorBase<TYPE,KEY>
    {
    public:

        TemplateLinearInterpolator() {}
        void getValue(const TemplateKeyframeContainer<KEY>& keyframes, double time, TYPE& result) const
        {

            if (time >= keyframes.back().getTime())
            {
                result = keyframes.back().getValue();
                return;
            }
            else if (time <= keyframes.front().getTime())
            {
                result = keyframes.front().getValue();
                return;
            }

            int i = this->getKeyIndexFromTime(keyframes,time);
            float blend = (time - keyframes[i].getTime()) / ( keyframes[i+1].getTime() -  keyframes[i].getTime());
            const TYPE& v1 =  keyframes[i].getValue();
            const TYPE& v2 =  keyframes[i+1].getValue();
            result = v1*(1-blend) + v2*blend;
        }
    };


    template <class TYPE, class KEY=TYPE>
    class TemplateSphericalLinearInterpolator : public TemplateInterpolatorBase<TYPE,KEY>
    {
    public:
        TemplateSphericalLinearInterpolator() {}
        void getValue(const TemplateKeyframeContainer<KEY>& keyframes, double time, TYPE& result) const
        {
            if (time >= keyframes.back().getTime())
            {
                result = keyframes.back().getValue();
                return;
            }
            else if (time <= keyframes.front().getTime())
            {
                result = keyframes.front().getValue();
                return;
            }

            int i = this->getKeyIndexFromTime(keyframes,time);
            float blend = (time -  keyframes[i].getTime()) / ( keyframes[i+1].getTime() -  keyframes[i].getTime());
            const TYPE& q1 =  keyframes[i].getValue();
            const TYPE& q2 =  keyframes[i+1].getValue();
            result.slerp(blend,q1,q2);
        }
    };


    template <class TYPE, class KEY>
    class TemplateLinearPackedInterpolator : public TemplateInterpolatorBase<TYPE,KEY>
    {
    public:

        TemplateLinearPackedInterpolator() {}
        void getValue(const TemplateKeyframeContainer<KEY>& keyframes, double time, TYPE& result) const
        {
            if (time >= keyframes.back().getTime())
            {
                keyframes.back().getValue().uncompress(keyframes.mScale, keyframes.mMin, result);
                return;
            }
            else if (time <= keyframes.front().getTime())
            {
                keyframes.front().getValue().uncompress(keyframes.mScale, keyframes.mMin, result);
                return;
            }

            int i = this->getKeyIndexFromTime(keyframes,time);
            float blend = (time - keyframes[i].getTime()) / ( keyframes[i+1].getTime() -  keyframes[i].getTime());
            TYPE v1,v2;
            keyframes[i].getValue().uncompress(keyframes.mScale, keyframes.mMin, v1);
            keyframes[i+1].getValue().uncompress(keyframes.mScale, keyframes.mMin, v2);
            result = v1*(1-blend) + v2*blend;
        }
    };


    // http://en.wikipedia.org/wiki/B%C3%A9zier_curve
    template <class TYPE, class KEY=TYPE>
    class TemplateCubicBezierInterpolator : public TemplateInterpolatorBase<TYPE,KEY>
    {
    public:

        TemplateCubicBezierInterpolator() {}
        void getValue(const TemplateKeyframeContainer<KEY>& keyframes, double time, TYPE& result) const
        {

            if (time >= keyframes.back().getTime())
            {
                result = keyframes.back().getValue().getPosition();
                return;
            }
            else if (time <= keyframes.front().getTime())
            {
                result = keyframes.front().getValue().getPosition();
                return;
            }

            int i = this->getKeyIndexFromTime(keyframes,time);

            float t = (time - keyframes[i].getTime()) / ( keyframes[i+1].getTime() -  keyframes[i].getTime());
            float one_minus_t = 1.0-t;
            float one_minus_t2 = one_minus_t * one_minus_t;
            float one_minus_t3 = one_minus_t2 * one_minus_t;
            float t2 = t * t;

            TYPE v0 = keyframes[i].getValue().getPosition() * one_minus_t3;
            TYPE v1 = keyframes[i].getValue().getControlPointIn() * (3.0 * t * one_minus_t2);
            TYPE v2 = keyframes[i].getValue().getControlPointOut() * (3.0 * t2 * one_minus_t);
            TYPE v3 = keyframes[i+1].getValue().getPosition() * (t2 * t);

            result = v0 + v1 + v2 + v3;
        }
    };

    typedef TemplateStepInterpolator<double, double> DoubleStepInterpolator;
    typedef TemplateStepInterpolator<float, float> FloatStepInterpolator;
    typedef TemplateStepInterpolator<osg::Vec2, osg::Vec2> Vec2StepInterpolator;
    typedef TemplateStepInterpolator<osg::Vec3, osg::Vec3> Vec3StepInterpolator;
    typedef TemplateStepInterpolator<osg::Vec3, Vec3Packed> Vec3PackedStepInterpolator;
    typedef TemplateStepInterpolator<osg::Vec4, osg::Vec4> Vec4StepInterpolator;
    typedef TemplateStepInterpolator<osg::Quat, osg::Quat> QuatStepInterpolator;

    typedef TemplateLinearInterpolator<double, double> DoubleLinearInterpolator;
    typedef TemplateLinearInterpolator<float, float> FloatLinearInterpolator;
    typedef TemplateLinearInterpolator<osg::Vec2, osg::Vec2> Vec2LinearInterpolator;
    typedef TemplateLinearInterpolator<osg::Vec3, osg::Vec3> Vec3LinearInterpolator;
    typedef TemplateLinearInterpolator<osg::Vec3, Vec3Packed> Vec3PackedLinearInterpolator;
    typedef TemplateLinearInterpolator<osg::Vec4, osg::Vec4> Vec4LinearInterpolator;
    typedef TemplateSphericalLinearInterpolator<osg::Quat, osg::Quat> QuatSphericalLinearInterpolator;
    typedef TemplateLinearInterpolator<osg::Matrixf, osg::Matrixf> MatrixLinearInterpolator;

    typedef TemplateCubicBezierInterpolator<float, FloatCubicBezier > FloatCubicBezierInterpolator;
    typedef TemplateCubicBezierInterpolator<double, DoubleCubicBezier> DoubleCubicBezierInterpolator;
    typedef TemplateCubicBezierInterpolator<osg::Vec2, Vec2CubicBezier> Vec2CubicBezierInterpolator;
    typedef TemplateCubicBezierInterpolator<osg::Vec3, Vec3CubicBezier> Vec3CubicBezierInterpolator;
    typedef TemplateCubicBezierInterpolator<osg::Vec4, Vec4CubicBezier> Vec4CubicBezierInterpolator;

}
#endif
