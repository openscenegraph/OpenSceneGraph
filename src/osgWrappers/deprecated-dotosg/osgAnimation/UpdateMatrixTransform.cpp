/*  -*-c++-*- 
 *  Copyright (C) 2009 Cedric Pinson <cedric.pinson@plopbyte.net>
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


#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/Registry>
#include <osgDB/ReaderWriter>
#include <osg/io_utils>
#include <osgAnimation/StackedTransformElement>
#include <osgAnimation/UpdateMatrixTransform>


using namespace osg;
using namespace osgDB;

bool UpdateMatrixTransform_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    osgAnimation::UpdateMatrixTransform& updateCallback = dynamic_cast<osgAnimation::UpdateMatrixTransform&>(obj);
    osgAnimation::StackedTransform& stackedTransform = updateCallback.getStackedTransforms();

    int entry = fr[0].getNoNestedBrackets();
    while (!fr.eof() && fr[0].getNoNestedBrackets() == entry && fr.matchSequence("%w {"))
    {
        osgAnimation::StackedTransformElement* element = dynamic_cast<osgAnimation::StackedTransformElement*>(fr.readObject());
        if (element)
            stackedTransform.push_back(element);
    }
    return iteratorAdvanced;
}



bool UpdateMatrixTransform_writeLocalData(const Object& obj, Output& fw)
{
    const osgAnimation::UpdateMatrixTransform* uc = dynamic_cast<const osgAnimation::UpdateMatrixTransform*>(&obj);
    const osgAnimation::StackedTransform& transforms = uc->getStackedTransforms();
    for (osgAnimation::StackedTransform::const_iterator it = transforms.begin(); it != transforms.end(); ++it)
    {
        osgAnimation::StackedTransformElement* element = it->get();
        if (element)
            fw.writeObject(*element);
    }
    return true;
}


RegisterDotOsgWrapperProxy g_UpdateMatrixTransformProxy
(
    new osgAnimation::UpdateMatrixTransform,
    "osgAnimation::UpdateMatrixTransform",
    "Object NodeCallback osgAnimation::UpdateMatrixTransform",
    &UpdateMatrixTransform_readLocalData,
    &UpdateMatrixTransform_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
    );

