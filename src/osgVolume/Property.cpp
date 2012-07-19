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

#include <osgVolume/Property>
#include <osgVolume/VolumeTile>

using namespace osgVolume;

Property::Property()
{
}

Property::Property(const Property& property,const osg::CopyOp& copyop):
    osg::Object(property,copyop)
{
}

Property::~Property()
{
}

/////////////////////////////////////////////////////////////////////////////
//
// CompositeProperty
//
CompositeProperty::CompositeProperty()
{
}

CompositeProperty::CompositeProperty(const CompositeProperty& compositeProperty, const osg::CopyOp& copyop):
    Property(compositeProperty,copyop)
{
}


void CompositeProperty::clear()
{
    _properties.clear();
}

/////////////////////////////////////////////////////////////////////////////
//
// SwitchProperty
//
SwitchProperty::SwitchProperty()
{
}

SwitchProperty::SwitchProperty(const SwitchProperty& switchProperty, const osg::CopyOp& copyop):
    CompositeProperty(switchProperty,copyop),
    _activeProperty(switchProperty._activeProperty)
{
}


/////////////////////////////////////////////////////////////////////////////
//
// TransferFunctionProperty
//
TransferFunctionProperty::TransferFunctionProperty(osg::TransferFunction* tf):
    _tf(tf)
{
}

TransferFunctionProperty::TransferFunctionProperty(const TransferFunctionProperty& tfp, const osg::CopyOp& copyop):
    Property(tfp,copyop),
    _tf(tfp._tf)
{
}

/////////////////////////////////////////////////////////////////////////////
//
// ScalarProperty
//
ScalarProperty::ScalarProperty()
{
    _uniform = new osg::Uniform;
}

ScalarProperty::ScalarProperty(const std::string& scalarName, float value)
{
    setName(scalarName);
    _uniform = new osg::Uniform(scalarName.c_str(), value);
}

ScalarProperty::ScalarProperty(const ScalarProperty& sp, const osg::CopyOp& copyop):
    Property(sp,copyop)
{
    _uniform = new osg::Uniform(sp._uniform->getName().c_str(), getValue());

}

/////////////////////////////////////////////////////////////////////////////
//
// IsoSurfaceProperty
//
IsoSurfaceProperty::IsoSurfaceProperty(float value):
    ScalarProperty("IsoSurfaceValue",value)
{
}

IsoSurfaceProperty::IsoSurfaceProperty(const IsoSurfaceProperty& isp,const osg::CopyOp& copyop):
    ScalarProperty(isp, copyop)
{
}


/////////////////////////////////////////////////////////////////////////////
//
// AlphaFuncProperty
//
AlphaFuncProperty::AlphaFuncProperty(float value):
    ScalarProperty("AlphaFuncValue",value)
{
    _alphaFunc = new osg::AlphaFunc(osg::AlphaFunc::GREATER, value);
}

AlphaFuncProperty::AlphaFuncProperty(const AlphaFuncProperty& afp,const osg::CopyOp& copyop):
    ScalarProperty(afp, copyop)
{
    _alphaFunc = new osg::AlphaFunc(osg::AlphaFunc::GREATER, getValue());
}

void AlphaFuncProperty::setValue(float v)
{
    _uniform->set(v);
    _alphaFunc->setReferenceValue(v);
}

/////////////////////////////////////////////////////////////////////////////
//
// MaximumIntensityProjectionProperty
//
MaximumIntensityProjectionProperty::MaximumIntensityProjectionProperty()
{
}

MaximumIntensityProjectionProperty::MaximumIntensityProjectionProperty(const MaximumIntensityProjectionProperty& isp,const osg::CopyOp& copyop):
    Property(isp, copyop)
{
}

/////////////////////////////////////////////////////////////////////////////
//
// LightingProperty
//
LightingProperty::LightingProperty()
{
}

LightingProperty::LightingProperty(const LightingProperty& isp,const osg::CopyOp& copyop):
    Property(isp, copyop)
{
}


/////////////////////////////////////////////////////////////////////////////
//
// SampleDensityProperty
//
SampleDensityProperty::SampleDensityProperty(float value):
    ScalarProperty("SampleDensityValue",value)
{
}

SampleDensityProperty::SampleDensityProperty(const SampleDensityProperty& isp,const osg::CopyOp& copyop):
    ScalarProperty(isp, copyop)
{
}


/////////////////////////////////////////////////////////////////////////////
//
// SampleDensityWhenMovingProperty
//
SampleDensityWhenMovingProperty::SampleDensityWhenMovingProperty(float value):
    ScalarProperty("SampleDensityValue",value)
{
}

SampleDensityWhenMovingProperty::SampleDensityWhenMovingProperty(const SampleDensityWhenMovingProperty& isp,const osg::CopyOp& copyop):
    ScalarProperty(isp, copyop)
{
}


/////////////////////////////////////////////////////////////////////////////
//
// TransparencyProperty
//
TransparencyProperty::TransparencyProperty(float value):
    ScalarProperty("TransparencyValue",value)
{
}

TransparencyProperty::TransparencyProperty(const TransparencyProperty& isp,const osg::CopyOp& copyop):
    ScalarProperty(isp, copyop)
{
}

/////////////////////////////////////////////////////////////////////////////
//
// PropertyVisitor
//
PropertyVisitor::PropertyVisitor(bool traverseOnlyActiveChildren):
    _traverseOnlyActiveChildren(traverseOnlyActiveChildren)
{
}

void PropertyVisitor::apply(CompositeProperty& cp)
{
    for(unsigned int i=0; i<cp.getNumProperties(); ++i)
    {
        cp.getProperty(i)->accept(*this);
    }
}

void PropertyVisitor::apply(SwitchProperty& sp)
{
    if (_traverseOnlyActiveChildren)
    {
        if (sp.getActiveProperty()>=0 && sp.getActiveProperty()<static_cast<int>(sp.getNumProperties()))
        {
            sp.getProperty(sp.getActiveProperty())->accept(*this);
        }
    }
    else
    {
        for(unsigned int i=0; i<sp.getNumProperties(); ++i)
        {
            sp.getProperty(i)->accept(*this);
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
//
// CollectPropertiesVisitor
//
CollectPropertiesVisitor::CollectPropertiesVisitor(bool traverseOnlyActiveChildren):
    PropertyVisitor(traverseOnlyActiveChildren)
{
}

void CollectPropertiesVisitor::apply(Property&) {}
void CollectPropertiesVisitor::apply(TransferFunctionProperty& tf) { _tfProperty = &tf; }
void CollectPropertiesVisitor::apply(ScalarProperty&) {}
void CollectPropertiesVisitor::apply(IsoSurfaceProperty& iso) { _isoProperty = &iso; }
void CollectPropertiesVisitor::apply(AlphaFuncProperty& af) { _afProperty = &af; }
void CollectPropertiesVisitor::apply(MaximumIntensityProjectionProperty& mip) { _mipProperty = &mip; }
void CollectPropertiesVisitor::apply(LightingProperty& lp) { _lightingProperty = &lp; }
void CollectPropertiesVisitor::apply(SampleDensityProperty& sdp) { _sampleDensityProperty = &sdp; }
void CollectPropertiesVisitor::apply(SampleDensityWhenMovingProperty& sdp) { _sampleDensityWhenMovingProperty = &sdp; }
void CollectPropertiesVisitor::apply(TransparencyProperty& tp) { _transparencyProperty = &tp; }


class CycleSwitchVisitor : public osgVolume::PropertyVisitor
{
    public:

        CycleSwitchVisitor(int delta):
            PropertyVisitor(false),
            _delta(delta),
            _switchModified(true) {}

        virtual void apply(SwitchProperty& sp)
        {
            if (sp.getNumProperties()>=2)
            {
                if (_delta>0)
                {
                    int newValue = sp.getActiveProperty()+_delta;
                    if (newValue<static_cast<int>(sp.getNumProperties()))
                    {
                        sp.setActiveProperty(newValue);
                    }
                    else
                    {
                        sp.setActiveProperty(0);
                    }

                    _switchModified = true;
                }
                else // _delta<0
                {
                    int newValue = sp.getActiveProperty()+_delta;
                    if (newValue>=0)
                    {
                        sp.setActiveProperty(newValue);
                    }
                    else
                    {
                        sp.setActiveProperty(sp.getNumProperties()-1);
                    }

                    _switchModified = true;
                }
            }

            PropertyVisitor::apply(sp);
        }

        int     _delta;
        bool    _switchModified;
};

/////////////////////////////////////////////////////////////////////////////
//
// PropertyAdjustmentCallback
//
PropertyAdjustmentCallback::PropertyAdjustmentCallback():
    _cyleForwardKey('v'),
    _cyleBackwardKey('V'),
    _transparencyKey('t'),
    _alphaFuncKey('a'),
    _sampleDensityKey('d'),
    _updateTransparency(false),
    _updateAlphaCutOff(false),
    _updateSampleDensity(false)
{
}

PropertyAdjustmentCallback::PropertyAdjustmentCallback(const PropertyAdjustmentCallback& pac,const osg::CopyOp&):
    _cyleForwardKey(pac._cyleForwardKey),
    _cyleBackwardKey(pac._cyleBackwardKey),
    _transparencyKey(pac._transparencyKey),
    _alphaFuncKey(pac._alphaFuncKey),
    _sampleDensityKey(pac._sampleDensityKey),
    _updateTransparency(false),
    _updateAlphaCutOff(false),
    _updateSampleDensity(false)
{
}

bool PropertyAdjustmentCallback::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&, osg::Object* object, osg::NodeVisitor*)
{
    osgVolume::VolumeTile* tile = dynamic_cast<osgVolume::VolumeTile*>(object);
    osgVolume::Layer* layer = tile ? tile->getLayer() : 0;
    osgVolume::Property* property = layer ? layer->getProperty() : 0;
    if (!property) return false;

    osgVolume::CollectPropertiesVisitor cpv;
    property->accept(cpv);

    bool passOnUpdates = false;

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::MOVE):
        case(osgGA::GUIEventAdapter::DRAG):
        {
            passOnUpdates = true;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()==_cyleForwardKey || ea.getKey()==_cyleBackwardKey)
            {
                CycleSwitchVisitor csv( (ea.getKey()==_cyleForwardKey) ? 1 : -1);
                property->accept(csv);
                if (csv._switchModified)
                {
                    tile->setDirty(true);
                    tile->init();
                }
            }
            else if (ea.getKey()==_transparencyKey) _updateTransparency = passOnUpdates = true;
            else if (ea.getKey()==_alphaFuncKey) _updateAlphaCutOff = passOnUpdates = true;
            else if (ea.getKey()==_sampleDensityKey) _updateSampleDensity = passOnUpdates = true;
            break;
        }
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (ea.getKey()==_transparencyKey) _updateTransparency = false;
            else if (ea.getKey()==_alphaFuncKey) _updateAlphaCutOff = false;
            else if (ea.getKey()==_sampleDensityKey) _updateSampleDensity = false;
            break;
        }
        default:
            break;
    }

    if (passOnUpdates)
    {
        float v = (ea.getY()-ea.getYmin())/(ea.getYmax()-ea.getYmin());
        if (ea.getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS) v = 1.0f-v;

        float v2 = v*v;
        float v4 = v2*v2;

        if (_updateAlphaCutOff && cpv._isoProperty.valid())
        {
            OSG_INFO<<"Setting isoProperty to "<<v<<std::endl;
            cpv._isoProperty->setValue(v);
        }

        if (_updateAlphaCutOff && cpv._afProperty.valid())
        {
            OSG_INFO<<"Setting afProperty to "<<v2<<std::endl;
            cpv._afProperty->setValue(v2);
        }

        if (_updateTransparency && cpv._transparencyProperty.valid())
        {
            OSG_INFO<<"Setting transparency to "<<v2<<std::endl;
            cpv._transparencyProperty->setValue(1.0f-v2);
        }

        if (_updateSampleDensity && cpv._sampleDensityProperty.valid())
        {
            OSG_INFO<<"Setting sample density to "<<v4<<std::endl;
            cpv._sampleDensityProperty->setValue(v4);
        }
        if (_updateSampleDensity && cpv._sampleDensityWhenMovingProperty.valid())
        {
            OSG_INFO<<"Setting sample density when moving to "<<v4<<std::endl;
            cpv._sampleDensityWhenMovingProperty->setValue(v4);
        }
    }


    return false;
}

