/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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
#include <osg/Array>

using namespace osg;

static const char* s_ArrayNames[] =
{
    "Array",        // 0
    "ByteArray",    // 1
    "ShortArray",   // 2
    "IntArray",     // 3

    "UByteArray",   // 4
    "UShortArray",  // 5
    "UIntArray",    // 6

    "FloatArray",   // 7
    "DoubleArray",  // 8

    "Vec2bArray",   // 9
    "Vec3bArray",   //10
    "Vec4bArray",   //11

    "Vec2sArray",   //12
    "Vec3sArray",   //13
    "Vec4sArray",   //14

    "Vec2iArray",   //15
    "Vec3iArray",   //16
    "Vec4iArray",   //17

    "Vec2ubArray",  //18
    "Vec3ubArray",  //19
    "Vec4ubArray",  //20

    "Vec2usArray",  //21
    "Vec3usArray",  //22
    "Vec4usArray",  //23

    "Vec2uiArray",  //24
    "Vec3uiArray",  //25
    "Vec4uiArray",  //26

    "Vec2Array",    //27
    "Vec3Array",    //28
    "Vec4Array",    //29

    "Vec2dArray",   //30
    "Vec3dArray",   //31
    "Vec4dArray",   //32

    "MatrixArray",  //33
    "MatrixdArray", //34
};

const char* Array::className() const
{
    if (_arrayType>=ArrayType && _arrayType<=Vec4dArrayType)
        return s_ArrayNames[_arrayType];
    else
        return "UnknownArray";
}

