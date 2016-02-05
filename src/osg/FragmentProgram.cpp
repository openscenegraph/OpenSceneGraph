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
#include <osg/Notify>
#include <osg/GLExtensions>
#include <osg/FragmentProgram>
#include <osg/State>
#include <osg/Timer>
#include <osg/ContextData>

using namespace osg;

class GLFragmentProgramManager : public GLObjectManager
{
public:
    GLFragmentProgramManager(unsigned int contextID) : GLObjectManager("GLFragmentProgramManager",contextID) {}

    virtual void deleteGLObject(GLuint globj)
    {
        const GLExtensions* extensions = GLExtensions::Get(_contextID,true);
        if (extensions->isGlslSupported) extensions->glDeletePrograms(1, &globj );
    }
};

FragmentProgram::FragmentProgram()
{
}


FragmentProgram::FragmentProgram(const FragmentProgram& vp,const CopyOp& copyop):
    osg::StateAttribute(vp,copyop)
{
    _fragmentProgram = vp._fragmentProgram;

    for( LocalParamList::const_iterator itr = vp._programLocalParameters.begin();
        itr != vp._programLocalParameters.end(); ++itr )
    {
        _programLocalParameters[itr->first] = itr->second;
    }

    for( MatrixList::const_iterator mitr = vp._matrixList.begin();
        mitr != vp._matrixList.end(); ++mitr )
    {
        _matrixList[mitr->first] = mitr->second;
    }
}


// virtual
FragmentProgram::~FragmentProgram()
{
    dirtyFragmentProgramObject();
}

void FragmentProgram::dirtyFragmentProgramObject()
{
    for(unsigned int i=0;i<_fragmentProgramIDList.size();++i)
    {
        if (_fragmentProgramIDList[i] != 0)
        {
            osg::get<GLFragmentProgramManager>(i)->scheduleGLObjectForDeletion(_fragmentProgramIDList[i]);
            _fragmentProgramIDList[i] = 0;
        }
    }
}

void FragmentProgram::apply(State& state) const
{
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE

    const GLExtensions* extensions = state.get<GLExtensions>();

    if (!extensions->isFragmentProgramSupported)
        return;


    GLuint& fragmentProgramId=getFragmentProgramID(state.getContextID());

    // Fragment Program
    if (fragmentProgramId != 0)
    {
        extensions->glBindProgram( GL_FRAGMENT_PROGRAM_ARB, fragmentProgramId );
    }
    else if (!_fragmentProgram.empty())
    {
        glGetError(); // Reset Error flags.
        extensions->glGenPrograms( 1, &fragmentProgramId );
        extensions->glBindProgram( GL_FRAGMENT_PROGRAM_ARB, fragmentProgramId );
        extensions->glProgramString( GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                                     _fragmentProgram.length(), _fragmentProgram.c_str());

        // Check for errors
        GLint errorposition = 0;
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorposition);
        if (errorposition != -1)
        {
            OSG_FATAL << "FragmentProgram: " << glGetString(GL_PROGRAM_ERROR_STRING_ARB) << std::endl;

            std::string::size_type start = _fragmentProgram.rfind('\n', errorposition);
            std::string::size_type stop  = _fragmentProgram.find('\n', errorposition);
            if (start!=std::string::npos && stop!=std::string::npos)
            {
                OSG_FATAL << "             : " << _fragmentProgram.substr(start+1, stop-start-2) << std::endl;
                std::string pointAtproblem(errorposition-(start+1), ' ');
                OSG_FATAL << "             : " << pointAtproblem << '^' << std::endl;
            }
            return;
        }
    }

    // Update local program parameters
    {
        for(LocalParamList::const_iterator itr=_programLocalParameters.begin();
            itr!=_programLocalParameters.end();
            ++itr)
        {
            extensions->glProgramLocalParameter4fv(GL_FRAGMENT_PROGRAM_ARB, (*itr).first, (*itr).second.ptr());
        }
    }

    // Update matrix
    if (!_matrixList.empty())
    {
        for(MatrixList::const_iterator itr = _matrixList.begin();
            itr!=_matrixList.end();
            ++itr)
        {
            glMatrixMode((*itr).first);
            glLoadMatrix((*itr).second.ptr());
        }
        glMatrixMode(GL_MODELVIEW); // restore matrix mode
    }
#else
    OSG_NOTICE<<"Warning: FragmentProgram::apply(State&) - not supported."<<std::endl;
#endif
}

void FragmentProgram::resizeGLObjectBuffers(unsigned int maxSize)
{
    _fragmentProgramIDList.resize(maxSize);
}

void FragmentProgram::releaseGLObjects(State* state) const
{
    if (!state) const_cast<FragmentProgram*>(this)->dirtyFragmentProgramObject();
    else
    {
        unsigned int contextID = state->getContextID();
        if (_fragmentProgramIDList[contextID] != 0)
        {
            osg::get<GLFragmentProgramManager>(contextID)->scheduleGLObjectForDeletion(_fragmentProgramIDList[contextID]);
            _fragmentProgramIDList[contextID] = 0;
        }
    }
}
