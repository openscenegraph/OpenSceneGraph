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
#include <osg/GLExtensions>
#include <osg/TexEnvFilter>
#include <osg/State>

using namespace osg;

TexEnvFilter::TexEnvFilter(float lodBias) :
        _lodBias(lodBias)
{
}


TexEnvFilter::~TexEnvFilter()
{
}

void TexEnvFilter::apply(State& state) const
{
    // note from RO, need to adapt to do testing per graphics context.
    static float glVersion = atof( (const char *)glGetString( GL_VERSION ) );
    static bool s_isTexLodBias = glVersion>=1.4 || isGLExtensionSupported(state.getContextID(),"GL_EXT_texture_lod_bias");

    if (s_isTexLodBias)
    {
        glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, _lodBias);
    }
}
