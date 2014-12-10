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

#include <osg/PrimitiveRestartIndex>
#include <osg/State>
#include <osg/GLExtensions>
#include <osg/Notify>

using namespace osg;

PrimitiveRestartIndex::PrimitiveRestartIndex()
{
    _restartIndex = 0;
}

PrimitiveRestartIndex::PrimitiveRestartIndex(unsigned int restartIndex)
{
    _restartIndex = restartIndex;
}

PrimitiveRestartIndex::PrimitiveRestartIndex(const PrimitiveRestartIndex& primitiveRestartIndex,const CopyOp& copyop):
    StateAttribute(primitiveRestartIndex,copyop)
{
    _restartIndex = primitiveRestartIndex._restartIndex;
}

PrimitiveRestartIndex::~PrimitiveRestartIndex()
{
}

int PrimitiveRestartIndex::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types(PrimitiveRestartIndex,sa)

    COMPARE_StateAttribute_Parameter(_restartIndex)

    return 0; // passed all the above comparison macros, must be equal.
}

void PrimitiveRestartIndex::apply(State& state) const
{
    // get "per-context" extensions
    const GLExtensions* extensions = state.get<GLExtensions>();
    if (extensions->glPrimitiveRestartIndex)
    {
        extensions->glPrimitiveRestartIndex( _restartIndex );
        return;
    }

    OSG_WARN << "PrimitiveRestartIndex failed as the required graphics capabilities were not found\n"
                "   OpenGL 3.1 or GL_NV_primitive_restart extension is required." << std::endl;
}
