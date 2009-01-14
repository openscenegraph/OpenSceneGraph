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

CompositeProperty::CompositeProperty(const CompositeProperty& compositeProperty,const osg::CopyOp& copyop):
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

TransferFunctionProperty::TransferFunctionProperty(const TransferFunctionProperty& tfp,const osg::CopyOp& copyop):
    Property(tfp,copyop),
    _tf(tfp._tf)
{
}
