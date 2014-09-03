/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

#ifndef OSGVOLUMESETTINGS
#define OSGVOLUMESETTINGS 1

#include <osg/Object>
#include <osgVolume/Property>

namespace osgVolume {

class OSGVOLUME_EXPORT VolumeSettings : public Property
{
    public:

        VolumeSettings();

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        VolumeSettings(const VolumeSettings&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgVolume, VolumeSettings);

        virtual void accept(PropertyVisitor& pv);
        virtual void traverse(PropertyVisitor& pv);

        void setFilename(const std::string& str) { _filename = str; dirty(); }
        const std::string& getFilename() const { return _filename; }

        enum Technique
        {
            FixedFunction,
            RayTraced,
            MultiPass
        };

        void setTechnique(Technique technique) { _technique = technique; dirty(); }
        Technique getTechnique() const { return _technique; }

        enum ShadingModel
        {
            Standard,
            Light,
            Isosurface,
            MaximumIntensityProjection
        };

        void setShadingModel(ShadingModel sm) { _shadingModel = sm; dirty(); }
        ShadingModel getShadingModel() const { return _shadingModel; }

        void setSampleRatio(float sr) { _sampleRatioProperty->setValue(sr); dirty(); }
        float getSampleRatio() const { return _sampleRatioProperty->getValue(); }

        void setSampleRatioWhenMoving(float sr) { _sampleRatioWhenMovingProperty->setValue(sr); dirty(); }
        float getSampleRatioWhenMoving() const { return _sampleRatioWhenMovingProperty->getValue(); }

        void setCutoff(float co);
        float getCutoff() const { return _cutoffProperty->getValue(); }

        void setTransparency(float t) { _transparencyProperty->setValue(t); dirty(); }
        float getTransparency() const { return _transparencyProperty->getValue(); }


        SampleRatioProperty* getSampleRatioProperty() { return _sampleRatioProperty.get(); }
        const SampleRatioProperty* getSampleRatioProperty() const { return _sampleRatioProperty.get(); }

        SampleRatioWhenMovingProperty* getSampleRatioWhenMovingProperty() { return _sampleRatioWhenMovingProperty.get(); }
        const SampleRatioWhenMovingProperty* getSampleRatioWhenMovingProperty() const { return _sampleRatioWhenMovingProperty.get(); }

        AlphaFuncProperty* getCutoffProperty() { return _cutoffProperty.get(); }
        const AlphaFuncProperty* getCutoffProperty() const { return _cutoffProperty.get(); }

        TransparencyProperty* getTransparencyProperty() { return _transparencyProperty.get(); }
        const TransparencyProperty* getTransparencyProperty() const { return _transparencyProperty.get(); }

        IsoSurfaceProperty* getIsoSurfaceProperty() { return _isoSurfaceProperty.get(); }
        const IsoSurfaceProperty* getIsoSurfaceProperty() const { return _isoSurfaceProperty.get(); }

protected:

        virtual ~VolumeSettings() {}

        std::string     _filename;

        Technique       _technique;
        ShadingModel    _shadingModel;

        osg::ref_ptr<SampleRatioProperty>               _sampleRatioProperty;
        osg::ref_ptr<SampleRatioWhenMovingProperty>     _sampleRatioWhenMovingProperty;
        osg::ref_ptr<AlphaFuncProperty>                 _cutoffProperty;
        osg::ref_ptr<TransparencyProperty>              _transparencyProperty;
        osg::ref_ptr<IsoSurfaceProperty>                _isoSurfaceProperty;
};

}

#endif
