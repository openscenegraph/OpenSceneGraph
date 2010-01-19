// -*-c++-*-

/*
 * OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

/*
 * osgFX::Outline - Copyright (C) 2004,2009 Ulrich Hertlein
 */

#include <osgFX/Outline>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osg/io_utils>


bool Outline_readLocalData(osg::Object& obj, osgDB::Input& fr);
bool Outline_writeLocalData(const osg::Object& obj, osgDB::Output& fw);


REGISTER_DOTOSGWRAPPER(Outline_Proxy)
(
    new osgFX::Outline,
    "osgFX::Outline",
    "Object Node Group osgFX::Effect osgFX::Outline",
    Outline_readLocalData,
    Outline_writeLocalData
);


bool Outline_readLocalData(osg::Object& obj, osgDB::Input& fr)
{
    osgFX::Outline& myobj = static_cast<osgFX::Outline&>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("outlineWidth")) {
        float w;
        if (fr[1].getFloat(w)) {
            myobj.setWidth(w);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("outlineColor")) {
        osg::Vec4 col;
        if (fr[1].getFloat(col.x()) && fr[2].getFloat(col.y()) &&
            fr[3].getFloat(col.z()) && fr[4].getFloat(col.w())) {
            myobj.setColor(col);
            fr += 5;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool Outline_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
   const osgFX::Outline& myobj = static_cast<const osgFX::Outline&>(obj);

   fw.indent() << "outlineWidth " << myobj.getWidth() << std::endl;
   fw.indent() << "outlineColor " << myobj.getColor() << std::endl;

   return true;
}
