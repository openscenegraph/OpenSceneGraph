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

#ifndef OSGSIM_LIGHTPOINTDRAWABLE
#define OSGSIM_LIGHTPOINTDRAWABLE 1

#include <osgSim/Export>

#include <osg/Drawable>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/ColorMask>
#include <osg/Point>
#include <osg/Endian>

#include <vector>

namespace osgSim {


class OSGSIM_EXPORT LightPointDrawable : public osg::Drawable
{
    public :

        LightPointDrawable();

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        LightPointDrawable(const LightPointDrawable&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        virtual osg::Object* cloneType() const { return new LightPointDrawable(); }
        virtual osg::Object* clone(const osg::CopyOp&) const { return new LightPointDrawable(); }
        virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const LightPointDrawable*>(obj)!=NULL; }
        virtual const char* className() const { return "LightPointDrawable"; }


        //typedef std::pair<unsigned int,osg::Vec3> ColorPosition;
        struct ColorPosition
        {
            unsigned int first;
            osg::Vec3 second;
            ColorPosition() {}
            ColorPosition(unsigned int f,const osg::Vec3& s):first(f),second(s) {}
        };

        void reset();

        inline unsigned int asRGBA(const osg::Vec4& color) const
        {
            return _endian==osg::BigEndian?color.asABGR():color.asRGBA();
        }

        inline void addOpaqueLightPoint(unsigned int pointSize,const osg::Vec3& position,const osg::Vec4& color)
        {
            if (pointSize>=_sizedOpaqueLightPointList.size()) _sizedOpaqueLightPointList.resize(pointSize+1);
            _sizedOpaqueLightPointList[pointSize].push_back(ColorPosition(asRGBA(color),position));
        }

        inline void addAdditiveLightPoint(unsigned int pointSize,const osg::Vec3& position,const osg::Vec4& color)
        {
            if (pointSize>=_sizedAdditiveLightPointList.size()) _sizedAdditiveLightPointList.resize(pointSize+1);
            _sizedAdditiveLightPointList[pointSize].push_back(ColorPosition(asRGBA(color),position));
        }

        inline void addBlendedLightPoint(unsigned int pointSize,const osg::Vec3& position,const osg::Vec4& color)
        {
            if (pointSize>=_sizedBlendedLightPointList.size()) _sizedBlendedLightPointList.resize(pointSize+1);
            _sizedBlendedLightPointList[pointSize].push_back(ColorPosition(asRGBA(color),position));
        }

        /** draw LightPoints. */
        virtual void drawImplementation(osg::RenderInfo& renderInfo) const;


        void setSimulationTime(double time)
        {
            _simulationTime = time;
            _simulationTimeInterval = 0.0;
        }

        void updateSimulationTime(double time)
        {
            _simulationTimeInterval = osg::clampAbove(time-_simulationTime,0.0);
            _simulationTime = time;
        }

        double getSimulationTime() const { return _simulationTime; }
        double getSimulationTimeInterval() const { return _simulationTimeInterval; }

        virtual osg::BoundingBox computeBoundingBox() const;

    protected:

        virtual ~LightPointDrawable();

        osg::Endian                     _endian;

        double                          _simulationTime;
        double                          _simulationTimeInterval;

        typedef std::vector<ColorPosition>  LightPointList;
        typedef std::vector<LightPointList> SizedLightPointList;

        SizedLightPointList             _sizedOpaqueLightPointList;
        SizedLightPointList             _sizedAdditiveLightPointList;
        SizedLightPointList             _sizedBlendedLightPointList;

        osg::ref_ptr<osg::Depth>        _depthOff;
        osg::ref_ptr<osg::Depth>        _depthOn;
        osg::ref_ptr<osg::BlendFunc>    _blendOne;
        osg::ref_ptr<osg::BlendFunc>    _blendOneMinusSrcAlpha;
        osg::ref_ptr<osg::ColorMask>    _colorMaskOff;


};

}

#endif
