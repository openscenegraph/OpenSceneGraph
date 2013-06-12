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
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);
    
    if ( (extensions->isOpenGL31Supported()) || (extensions->isPrimitiveRestartIndexNVSupported())  )
    {
        extensions->glPrimitiveRestartIndex( _restartIndex );
        return;
    }

    OSG_WARN << "PrimitiveRestartIndex failed as the required graphics capabilities were\n"
                "   not found (contextID " << contextID << "). OpenGL 3.1 or \n"
                "   GL_NV_primitive_restart extension is required." << std::endl;
}


typedef buffered_value< ref_ptr<PrimitiveRestartIndex::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

PrimitiveRestartIndex::Extensions* PrimitiveRestartIndex::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions(contextID);
    return s_extensions[contextID].get();
}

void PrimitiveRestartIndex::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

PrimitiveRestartIndex::Extensions::Extensions(unsigned int contextID)
{
    setupGLExtensions(contextID);
}

PrimitiveRestartIndex::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isOpenGL31Supported = rhs._isOpenGL31Supported;
    _isPrimitiveRestartIndexNVSupported = rhs._isPrimitiveRestartIndexNVSupported;
    _glPrimitiveRestartIndex = rhs._glPrimitiveRestartIndex;
}

void PrimitiveRestartIndex::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isOpenGL31Supported) _isOpenGL31Supported = false;
    if (!rhs._isPrimitiveRestartIndexNVSupported) _isPrimitiveRestartIndexNVSupported = false;
    if (!rhs._glPrimitiveRestartIndex) _glPrimitiveRestartIndex = NULL;
}

void PrimitiveRestartIndex::Extensions::setupGLExtensions(unsigned int contextID)
{
    // extension support
    _isPrimitiveRestartIndexNVSupported = isGLExtensionSupported(contextID, "GL_NV_primitive_restart");
    _isOpenGL31Supported = getGLVersionNumber() >= 3.1;
    _glPrimitiveRestartIndex = NULL;

    // function pointers
    if (_isOpenGL31Supported)
        setGLExtensionFuncPtr(_glPrimitiveRestartIndex, "glPrimitiveRestartIndex");
    else if (_isPrimitiveRestartIndexNVSupported)
        setGLExtensionFuncPtr(_glPrimitiveRestartIndex, "glPrimitiveRestartIndexNV");

    // protect against buggy drivers (maybe not necessary)
    if (!_glPrimitiveRestartIndex) _isPrimitiveRestartIndexNVSupported = false;

    // notify
    if( _isOpenGL31Supported )
    {
       OSG_INFO << "PrimitiveRestartIndex is going to use OpenGL 3.1 API (contextID " << contextID << ")." << std::endl;
    }
    else if( _isPrimitiveRestartIndexNVSupported )
    {
       OSG_INFO << "PrimitiveRestartIndex is going to use GL_NV_primitive_restart extension (contextID " << contextID << ")." << std::endl;
    }
    else
    {
       OSG_INFO << "PrimitiveRestartIndex did not found required graphics capabilities\n"
                   "   (contextID " << contextID << "). OpenGL 3.1 or \n"
                   "   GL_NV_primitive_restart extension is required." << std::endl;
    }
}

void PrimitiveRestartIndex::Extensions::glPrimitiveRestartIndex( GLuint index ) const
{
    _glPrimitiveRestartIndex( index );
}

