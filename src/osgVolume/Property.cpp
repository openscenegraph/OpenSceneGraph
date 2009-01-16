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
}

AlphaFuncProperty::AlphaFuncProperty(const AlphaFuncProperty& afp,const osg::CopyOp& copyop):
    ScalarProperty(afp, copyop)
{
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
