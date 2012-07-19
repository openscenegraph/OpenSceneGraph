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

#include <osgTerrain/Layer>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool SwitchLayer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool SwitchLayer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(SwitchLayer_Proxy)
(
    new osgTerrain::SwitchLayer,
    "SwitchLayer",
    "Object SwitchLayer CompositeLayer Layer",
    SwitchLayer_readLocalData,
    SwitchLayer_writeLocalData
);

bool SwitchLayer_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgTerrain::SwitchLayer& layer = static_cast<osgTerrain::SwitchLayer&>(obj);

    bool itrAdvanced = false;

    int i;
    if (fr.read("ActiveLayer",i))
    {
        layer.setActiveLayer(i);
        itrAdvanced = true;
    };

    return itrAdvanced;
}

bool SwitchLayer_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::SwitchLayer& layer = static_cast<const osgTerrain::SwitchLayer&>(obj);

    fw.indent()<<"ActiveLayer "<<layer.getActiveLayer()<<std::endl;

    return true;
}
