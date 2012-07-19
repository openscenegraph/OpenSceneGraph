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

#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedMatrixElement>
#include <osgAnimation/StackedScaleElement>
#include <osgAnimation/StackedRotateAxisElement>
#include <osgAnimation/StackedQuaternionElement>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/Registry>
#include <osgDB/ReaderWriter>
#include <osg/io_utils>

#include "Matrix.h"

using namespace osgDB;
using namespace osg;

bool readStackedTranslateElement(Object& obj, Input& fr)
{
    osgAnimation::StackedTranslateElement& element = dynamic_cast<osgAnimation::StackedTranslateElement&>(obj);
    bool iteratorAdvanced = false;
    if (fr.matchSequence("translate %f %f %f"))
    {
        ++fr;
        osg::Vec3 translate;
        fr[0].getFloat(translate.x());
        fr[1].getFloat(translate.y());
        fr[2].getFloat(translate.z());
        element.setTranslate(translate);
        fr += 3;
        iteratorAdvanced = true;
    }
    return iteratorAdvanced;
}

bool writeStackedTranslateElement(const Object& obj, Output& fw)
{
    const osgAnimation::StackedTranslateElement& element = dynamic_cast<const osgAnimation::StackedTranslateElement&>(obj);
    fw.indent() << "translate " << element.getTranslate() << std::endl;
    return true;
}

RegisterDotOsgWrapperProxy g_StackedTranslateElementProxy
(
    new osgAnimation::StackedTranslateElement,
    "osgAnimation::StackedTranslateElement",
    "Object osgAnimation::StackedTranslateElement",
    &readStackedTranslateElement,
    &writeStackedTranslateElement,
    DotOsgWrapper::READ_AND_WRITE
    );


bool readStackedScaleElement(Object& obj, Input& fr)
{
    osgAnimation::StackedScaleElement& element = dynamic_cast<osgAnimation::StackedScaleElement&>(obj);
    bool iteratorAdvanced = false;
    if (fr.matchSequence("scale %f %f %f"))
    {
        ++fr;
        osg::Vec3 scale;
        fr[0].getFloat(scale.x());
        fr[1].getFloat(scale.y());
        fr[2].getFloat(scale.z());
        element.setScale(scale);
        fr += 3;
        iteratorAdvanced = true;
    }
    return iteratorAdvanced;
}

bool writeStackedScaleElement(const Object& obj, Output& fw)
{
    const osgAnimation::StackedScaleElement& element = dynamic_cast<const osgAnimation::StackedScaleElement&>(obj);
    fw.indent() << "scale " << element.getScale() << std::endl;
    return true;
}


RegisterDotOsgWrapperProxy g_StackedScaleElementProxy
(
    new osgAnimation::StackedScaleElement,
    "osgAnimation::StackedScaleElement",
    "Object osgAnimation::StackedScaleElement",
    &readStackedScaleElement,
    &writeStackedScaleElement,
    DotOsgWrapper::READ_AND_WRITE
    );







bool readStackedMatrixElement(Object& obj, Input& fr)
{
    osgAnimation::StackedMatrixElement& element = dynamic_cast<osgAnimation::StackedMatrixElement&>(obj);
    bool iteratorAdvanced = false;
    if (fr[0].matchWord("Matrix"))
    {
        osg::Matrix matrix;
        matrix.makeIdentity();
        iteratorAdvanced = readMatrix(matrix, fr);
        element.setMatrix(matrix);
    }

    return iteratorAdvanced;
}

bool writeStackedMatrixElement(const Object& obj, Output& fw)
{
    const osgAnimation::StackedMatrixElement& element = dynamic_cast<const osgAnimation::StackedMatrixElement&>(obj);
    writeMatrix(element.getMatrix(), fw);
    return true;
}

RegisterDotOsgWrapperProxy g_StackedMatrixElementProxy
(
    new osgAnimation::StackedMatrixElement,
    "osgAnimation::StackedMatrixElement",
    "Object osgAnimation::StackedMatrixElement",
    &readStackedMatrixElement,
    &writeStackedMatrixElement,
    DotOsgWrapper::READ_AND_WRITE
    );




bool writeStackedRotateAxisElement(const Object& obj, Output& fw)
{
    const osgAnimation::StackedRotateAxisElement& element = dynamic_cast<const osgAnimation::StackedRotateAxisElement&>(obj);
    fw.indent() << "axis " << element.getAxis() << std::endl;
    fw.indent() << "angle " << element.getAngle() << std::endl;
    return true;
}

bool readStackedRotateAxisElement(Object& obj, Input& fr)
{
    osgAnimation::StackedRotateAxisElement& element = dynamic_cast<osgAnimation::StackedRotateAxisElement&>(obj);
    bool iteratorAdvanced = false;
    if (fr.matchSequence("axis %f %f %f"))
    {
        ++fr;
        osg::Vec3 axis;
        fr[0].getFloat(axis.x());
        fr[1].getFloat(axis.y());
        fr[2].getFloat(axis.z());
        element.setAxis(axis);
        fr += 3;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("angle %f"))
    {
        ++fr;
        double angle = 0;
        fr[0].getFloat(angle);
        ++fr;
        element.setAngle(angle);
        iteratorAdvanced = true;
    }
    return iteratorAdvanced;
}

RegisterDotOsgWrapperProxy g_StackedRotateAxisElementProxy
(
    new osgAnimation::StackedRotateAxisElement,
    "osgAnimation::StackedRotateAxisElement",
    "Object osgAnimation::StackedRotateAxisElement",
    &readStackedRotateAxisElement,
    &writeStackedRotateAxisElement,
    DotOsgWrapper::READ_AND_WRITE
    );





bool readStackedQuaternionElement(Object& obj, Input& fr)
{
    osgAnimation::StackedQuaternionElement& element = dynamic_cast<osgAnimation::StackedQuaternionElement&>(obj);
    bool iteratorAdvanced = false;
    if (fr.matchSequence("quaternion %f %f %f %f"))
    {
        ++fr;
        osg::Quat quaternion;
        fr[0].getFloat(quaternion[0]);
        fr[1].getFloat(quaternion[1]);
        fr[2].getFloat(quaternion[2]);
        fr[3].getFloat(quaternion[3]);
        element.setQuaternion(quaternion);
        fr += 4;
        iteratorAdvanced = true;
    }
    return iteratorAdvanced;
}

bool writeStackedQuaternionElement(const Object& obj, Output& fw)
{
    const osgAnimation::StackedQuaternionElement& element = dynamic_cast<const osgAnimation::StackedQuaternionElement&>(obj);
    fw.indent() << "quaternion " << element.getQuaternion() << std::endl;
    return true;
}

RegisterDotOsgWrapperProxy g_StackedQuaternionElementProxy
(
    new osgAnimation::StackedQuaternionElement,
    "osgAnimation::StackedQuaternionElement",
    "Object osgAnimation::StackedQuaternionElement",
    &readStackedQuaternionElement,
    &writeStackedQuaternionElement,
    DotOsgWrapper::READ_AND_WRITE
    );
