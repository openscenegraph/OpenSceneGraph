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
#include <osg/Notify>
#include <osg/GLExtensions>
#include <osg/VertexProgram>
#include <osg/State>

using namespace osg;


VertexProgram::VertexProgram()
{
}


VertexProgram::VertexProgram(const VertexProgram& vp,const CopyOp& copyop):
    osg::StateAttribute(vp,copyop)
{}


// virtual
VertexProgram::~VertexProgram()
{}

void VertexProgram::apply(State& state) const
{
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    if (!extensions->isVertexProgramSupported())
        return;


    GLuint& vertexProgramId=getVertexProgramID(state.getContextID());

    // Vertex Program
    if (vertexProgramId != 0)
    {
        extensions->glBindProgram( GL_VERTEX_PROGRAM_ARB, vertexProgramId );
    }
    else if (!_vertexProgram.empty())
    {
        ::glGetError(); // Reset Error flags.
        extensions->glGenPrograms( 1, &vertexProgramId );
        extensions->glBindProgram( GL_VERTEX_PROGRAM_ARB, vertexProgramId );
        extensions->glProgramString( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                                     _vertexProgram.length(), _vertexProgram.c_str());

        // Check for errors
        GLint errorposition;
        ::glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorposition);
        if (errorposition != -1)
        {
            notify(osg::FATAL) << "VertexProgram: " << ::glGetString(GL_PROGRAM_ERROR_STRING_ARB) << std::endl;

            std::string::size_type start = _vertexProgram.rfind('\n', errorposition);
            std::string::size_type stop  = _vertexProgram.find('\n', errorposition);
            if (start!=std::string::npos && stop!=std::string::npos)
            {
                notify(osg::FATAL) << "             : " << _vertexProgram.substr(start+1, stop-start-2) << std::endl;
                std::string pointAtproblem(errorposition-(start+1), ' ');
                notify(osg::FATAL) << "             : " << pointAtproblem << '^' << std::endl;
            }
            exit(1);
        }
    }

    // Update local program parameters
    {
        for(LocalParamList::const_iterator itr=_programLocalParameters.begin();
            itr!=_programLocalParameters.end();
            ++itr)
        {
            extensions->glProgramLocalParameter4fv(GL_VERTEX_PROGRAM_ARB, (*itr).first, (*itr).second.ptr());
        }
    }

    // Update matrix
    if (!_matrixList.empty())
    {
        for(MatrixList::const_iterator itr = _matrixList.begin();
            itr!=_matrixList.end();
            ++itr)
        {
            ::glMatrixMode((*itr).first);
            ::glLoadMatrixf((*itr).second.ptr());
        }
        ::glMatrixMode(GL_MODELVIEW); // restore matrix mode
    }
}


typedef buffered_value< ref_ptr<VertexProgram::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

VertexProgram::Extensions* VertexProgram::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions;
    return s_extensions[contextID].get();
}

void VertexProgram::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

VertexProgram::Extensions::Extensions()
{
    setupGLExtenions();
}

VertexProgram::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isVertexProgramSupported = rhs._isVertexProgramSupported;
    _glBindProgram = rhs._glBindProgram;
    _glGenPrograms = rhs._glGenPrograms;
    _glProgramString = rhs._glProgramString;
    _glProgramLocalParameter4fv = rhs._glProgramLocalParameter4fv;
}


void VertexProgram::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isVertexProgramSupported) _isVertexProgramSupported = false;

    if (!rhs._glBindProgram) _glBindProgram = 0;
    if (!rhs._glGenPrograms) _glGenPrograms = 0;
    if (!rhs._glProgramString) _glProgramString = 0;
    if (!rhs._glProgramLocalParameter4fv) _glProgramLocalParameter4fv = 0;

}

void VertexProgram::Extensions::setupGLExtenions()
{
    _isVertexProgramSupported = isGLExtensionSupported("GL_ARB_vertex_program");

    _glBindProgram = osg::getGLExtensionFuncPtr("glBindProgramARB");
    _glGenPrograms = osg::getGLExtensionFuncPtr("glGenProgramsARB");
    _glProgramString = osg::getGLExtensionFuncPtr("glProgramStringARB");
    _glProgramLocalParameter4fv = osg::getGLExtensionFuncPtr("glProgramLocalParameter4fvARB");
}

void VertexProgram::Extensions::glBindProgram(GLenum target, GLuint id) const
{
    if (_glBindProgram)
    {
        typedef void (APIENTRY * BindProgramProc) (GLenum target, GLuint id);
        ((BindProgramProc)_glBindProgram)(target,id);
    }
    else
    {
        notify(WARN)<<"Error: glBindProgram not supported by OpenGL driver"<<std::endl;
    }    
}

void VertexProgram::Extensions::glGenPrograms(GLsizei n, GLuint *programs) const
{
    if (_glGenPrograms)
    {
        typedef void (APIENTRY * GenProgramsProc) (GLsizei n, GLuint *programs);
        ((GenProgramsProc)_glGenPrograms)(n,programs);
    }
    else
    {
        notify(WARN)<<"Error: glGenPrograms not supported by OpenGL driver"<<std::endl;
    }
}

void VertexProgram::Extensions::glProgramString(GLenum target, GLenum format, GLsizei len, const void *string) const
{
    if (_glProgramString)
    {
        typedef void (APIENTRY * ProgramStringProc) (GLenum target, GLenum format, GLsizei len, const void *string); 
        ((ProgramStringProc)_glProgramString)(target,format, len, string); 
    }
    else
    {
        notify(WARN)<<"Error: glProgramString not supported by OpenGL driver"<<std::endl;
    }
}

void VertexProgram::Extensions::glProgramLocalParameter4fv(GLenum target, GLuint index, const GLfloat *params) const
{
    if (_glProgramLocalParameter4fv)
    {
        typedef void (APIENTRY * ProgramLocalParameter4fvProc) (GLenum target, GLuint index, const GLfloat *params);
        ((ProgramLocalParameter4fvProc)_glProgramLocalParameter4fv)(target, index, params);
    }
    else
    {
        notify(WARN)<<"Error: glProgramLocalParameter4fv not supported by OpenGL driver"<<std::endl;
    }
}
