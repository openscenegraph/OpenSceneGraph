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

using namespace osg;


VertexProgram::VertexProgram() :
    _vertexProgramId(0)
{
}


VertexProgram::VertexProgram(const VertexProgram& vp,const CopyOp& copyop):
    osg::StateAttribute(vp,copyop),
    _vertexProgramId(vp._vertexProgramId)
{}


// virtual
VertexProgram::~VertexProgram()
{}


typedef void (APIENTRY * BindProgramProc) (GLenum target, GLuint id);
typedef void (APIENTRY * GenProgramsProc) (GLsizei n, GLuint *programs);
typedef void (APIENTRY * ProgramStringProc) (GLenum target, GLenum format, GLsizei len, const void *string); 
typedef void (APIENTRY * ProgramLocalParameter4fvProc) (GLenum target, GLuint index, const GLfloat *params);

void VertexProgram::apply(State& state) const
{
    static bool supported = osg::isGLExtensionSupported("GL_ARB_vertex_program");
    if (!supported) return;

    static BindProgramProc s_glBindProgram =
        (BindProgramProc)osg::getGLExtensionFuncPtr("glBindProgramARB");
    static GenProgramsProc s_glGenPrograms =
        (GenProgramsProc)osg::getGLExtensionFuncPtr("glGenProgramsARB");
    static ProgramStringProc s_glProgramString =
        (ProgramStringProc)osg::getGLExtensionFuncPtr("glProgramStringARB");
    static ProgramLocalParameter4fvProc s_glProgramLocalParameter4fv =
        (ProgramLocalParameter4fvProc)osg::getGLExtensionFuncPtr("glProgramLocalParameter4fvARB");

    // Vertex Program
    if (_vertexProgramId != 0)
    {
        s_glBindProgram( GL_VERTEX_PROGRAM_ARB, _vertexProgramId );
    }
    else if (!_vertexProgram.empty())
    {
        ::glGetError(); // Reset Error flags.
        s_glGenPrograms( 1, &_vertexProgramId );
        s_glBindProgram( GL_VERTEX_PROGRAM_ARB, _vertexProgramId );
        s_glProgramString( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
            _vertexProgram.length(), _vertexProgram.c_str());

        // Check for errors
        int errorposition;
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
            s_glProgramLocalParameter4fv(GL_VERTEX_PROGRAM_ARB, (*itr).first, (*itr).second.ptr());
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


