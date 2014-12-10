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

#include <osg/SampleMaski>
#include <osg/State>
#include <osg/GLExtensions>
#include <osg/Notify>

using namespace osg;


SampleMaski::SampleMaski()
{
    _sampleMask[0u] = ~0u;
    _sampleMask[1u] = ~0u;
}

SampleMaski::SampleMaski(const SampleMaski& sampleMaski,const CopyOp& copyop):
    StateAttribute(sampleMaski,copyop)
{
    _sampleMask[0u] = sampleMaski._sampleMask[0u];
    _sampleMask[1u] = sampleMaski._sampleMask[1u];
}

SampleMaski::~SampleMaski()
{
}

int SampleMaski::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types(SampleMaski,sa)

    COMPARE_StateAttribute_Parameter(_sampleMask[0u])
    COMPARE_StateAttribute_Parameter(_sampleMask[1u])

    return 0; // passed all the above comparison macros, must be equal.
}

void SampleMaski::apply(State& state) const
{
    // get "per-context" extensions
    const GLExtensions* extensions = state.get<GLExtensions>();

    if ( (extensions->isTextureMultisampleSupported) || (extensions->isOpenGL32upported) || (extensions->isSampleMaskiSupported)  )
    {
        extensions->glSampleMaski(0u, _sampleMask[0u]);
        //For now we use only 32-bit Sample mask
        //        extensions->glSampleMaski(1u, _sampleMask[1u]);
        return;
    }

    OSG_WARN << "SampleMaski failed as the required graphics capabilities were not found. \n"
                "OpenGL 3.2 or  ARB_texture_multisample extension is required." << std::endl;
}
