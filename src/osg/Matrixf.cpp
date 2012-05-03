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

#include <osg/Matrixf>
#include <osg/Matrixd>

// specialise Matrix_implementaiton to be Matrixf
#define  Matrix_implementation Matrixf

osg::Matrixf::Matrixf( const osg::Matrixd& mat )
{
    set(mat.ptr());
}

osg::Matrixf& osg::Matrixf::operator = (const osg::Matrixd& rhs)
{
    set(rhs.ptr());
    return *this;
}

void osg::Matrixf::set(const osg::Matrixd& rhs)
{
    set(rhs.ptr());
}

// now compile up Matrix via Matrix_implementation
#include "Matrix_implementation.cpp"
