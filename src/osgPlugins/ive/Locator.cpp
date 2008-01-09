/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield 
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


#include "Exception.h"
#include "Locator.h"
#include "Object.h"
#include "EllipsoidModel.h"

using namespace ive;

void Locator::write(DataOutputStream* out)
{
    // Write Locator's identification.
    out->writeInt(IVELOCATOR);

    // If the osg class is inherited by any other class we should also write this to file.
    osg::Object*  object = dynamic_cast<osg::Object*>(this);
    if (object)
        ((ive::Object*)(object))->write(out);
    else
        throw Exception("Layer::write(): Could not cast this osgLayer::Layer to an osg::Object.");
        
    out->writeInt(getCoordinateSystemType());
    out->writeString(getFormat());
    out->writeString(getCoordinateSystem());
    
    out->writeBool(getEllipsoidModel()!=0);
    if(getEllipsoidModel())
    {
        ((ive::EllipsoidModel*)(getEllipsoidModel()))->write(out);
    }
    
    out->writeBool(getDefinedInFile());
    out->writeBool(getTransformScaledByResolution());
    out->writeMatrixd(getTransform());
}

void Locator::read(DataInputStream* in)
{
    // Peek on Locator's identification.
    int id = in->peekInt();
    if(id != IVELOCATOR)
    {
        throw Exception("Locator::read(): Expected Locator identification.");
    }
    
    // Read Locator's identification.
    id = in->readInt();

    // If the osg class is inherited by any other class we should also read this from file.
    osg::Object*  object = dynamic_cast<osg::Object*>(this);
    if(object)
        ((ive::Object*)(object))->read(in);
    else
        throw Exception("Locator::read(): Could not cast this osgLocator::Locator to an osg::Group.");

    setCoordinateSystemType(static_cast<osgTerrain::Locator::CoordinateSystemType>(in->readInt()));
    setFormat(in->readString());
    setCoordinateSystem(in->readString());
    
    bool readEllipsoidModel = in->readBool();
    if (readEllipsoidModel)
    {
        osg::EllipsoidModel* em = new osg::EllipsoidModel();
        ((ive::EllipsoidModel*)(em))->read(in);
        setEllipsoidModel(em);
    }
    
    setDefinedInFile(in->readBool());
    setTransformScaledByResolution(in->readBool());
    setTransform(in->readMatrixd());

}
