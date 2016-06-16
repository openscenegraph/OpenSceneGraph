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
#include <osgAnimation/UpdateMaterial>


using namespace osg;
using namespace osgDB;


bool UpdateMaterial_readLocalData(Object& /*obj*/, Input& /*fr*/)
{
    bool iteratorAdvanced = false;
    return iteratorAdvanced;
}

bool UpdateMaterial_writeLocalData(const Object& /*obj*/, Output& /*fw*/)
{
    return true;
}

RegisterDotOsgWrapperProxy g_UpdateMaterialProxy
(
    new osgAnimation::UpdateMaterial,
    "osgAnimation::UpdateMaterial",
    "Object StateAttribute::Callback osgAnimation::UpdateMaterial",
    &UpdateMaterial_readLocalData,
    &UpdateMaterial_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);
