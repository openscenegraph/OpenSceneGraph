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
// CollectPropertiesVisitor
//
CollectPropertiesVisitor::CollectPropertiesVisitor() {}

void CollectPropertiesVisitor::apply(Property&) {}

void CollectPropertiesVisitor::apply(CompositeProperty& cp)
{ 
    for(unsigned int i=0; i<cp.getNumProperties(); ++i)
    {
        cp.getProperty(i)->accept(*this);
    }
}

void CollectPropertiesVisitor::apply(TransferFunctionProperty& tf) { _tfProperty = &tf; }
void CollectPropertiesVisitor::apply(ScalarProperty&) {}
void CollectPropertiesVisitor::apply(IsoSurfaceProperty& iso) { _isoProperty = &iso; }
void CollectPropertiesVisitor::apply(AlphaFuncProperty& af) { _afProperty = &af; }
void CollectPropertiesVisitor::apply(MaximumIntensityProjectionProperty& mip) { _mipProperty = &mip; }
void CollectPropertiesVisitor::apply(LightingProperty& lp) { _lightingProperty = &lp; }
void CollectPropertiesVisitor::apply(SampleDensityProperty& sdp) { _sampleDensityProperty = &sdp; }
void CollectPropertiesVisitor::apply(TransparencyProperty& tp) { _transparencyProperty = &tp; }


/////////////////////////////////////////////////////////////////////////////
//
// PropertyAdjustmentCallback
//
PropertyAdjustmentCallback::PropertyAdjustmentCallback()
{
    _transparencyKey = 't';
    _alphaFuncKey = 'a';
    _sampleDensityKey = 'd';

    _updateTransparency = false;
    _updateAlphaCutOff = false;
    _updateSampleDensity = false;
}

bool PropertyAdjustmentCallback::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&, osg::Object* object, osg::NodeVisitor*)
{
    osgVolume::VolumeTile* tile = dynamic_cast<osgVolume::VolumeTile*>(object);
    osgVolume::Layer* layer = tile ? tile->getLayer() : 0;
    osgVolume::Property* property = layer ? layer->getProperty() : 0;
    if (!property) return false;

    osgVolume::CollectPropertiesVisitor cpv;
    property->accept(cpv);

    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::MOVE):
        case(osgGA::GUIEventAdapter::DRAG):
        {
            float v = (ea.getY()-ea.getYmin())/(ea.getYmax()-ea.getYmin());
            float v2 = v*v;
            float v4 = v2*v2;

            if (_updateAlphaCutOff && cpv._isoProperty.valid())
            {
                osg::notify(osg::NOTICE)<<"Setting isoProperty to "<<v<<std::endl;
                cpv._isoProperty->setValue(v);
            }

            if (_updateAlphaCutOff && cpv._afProperty.valid())
            {
                osg::notify(osg::NOTICE)<<"Setting afProperty to "<<v2<<std::endl;
                cpv._afProperty->setValue(v2);
            }

            if (_updateTransparency && cpv._transparencyProperty.valid())
            {
                osg::notify(osg::NOTICE)<<"Setting transparency to "<<v2<<std::endl;
                cpv._transparencyProperty->setValue(v2);
            }

            if (_updateSampleDensity && cpv._sampleDensityProperty.valid())
            {
                osg::notify(osg::NOTICE)<<"Setting sample density to "<<v4<<std::endl;
                cpv._sampleDensityProperty->setValue(v4);
            }
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='t') _updateTransparency = true;
            if (ea.getKey()=='a') _updateAlphaCutOff = true;
            if (ea.getKey()=='d') _updateSampleDensity = true;
            break;
        }
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (ea.getKey()=='t') _updateTransparency = false;
            if (ea.getKey()=='a') _updateAlphaCutOff = false;
            if (ea.getKey()=='d') _updateSampleDensity = false;
            break;
        }
        default:
            break;
    }
    return false;
}

