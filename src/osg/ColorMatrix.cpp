/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osg/GLExtensions>
#include <osg/ColorMatrix>
#include <osg/GL>

using namespace osg;

ColorMatrix::ColorMatrix()
{
}


ColorMatrix::~ColorMatrix()
{
}

void ColorMatrix::apply(State&) const
{
//    std::cout<<"applying matrix"<<_matrix<<std::endl;
    static bool s_ARB_imaging = isGLExtensionSupported("GL_ARB_imaging");
    if (s_ARB_imaging)
    {
        glMatrixMode( GL_COLOR );
        glLoadMatrixf( _matrix.ptr() );
        glMatrixMode( GL_MODELVIEW );
    }
}
