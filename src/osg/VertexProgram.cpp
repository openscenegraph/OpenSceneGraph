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
#include <osg/VertexProgram>
#include <osg/State>
#include <osg/Timer>

#include <list>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

using namespace osg;

// static cache of deleted vertex programs which can only
// by completely deleted once the appropriate OpenGL context
// is set.
typedef std::list<GLuint> VertexProgramObjectList;
typedef osg::buffered_object<VertexProgramObjectList> DeletedVertexProgramObjectCache;

static OpenThreads::Mutex              s_mutex_deletedVertexProgramObjectCache;
static DeletedVertexProgramObjectCache s_deletedVertexProgramObjectCache;

void VertexProgram::deleteVertexProgramObject(unsigned int contextID,GLuint handle)
{
    if (handle!=0)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedVertexProgramObjectCache);

        // insert the handle into the cache for the appropriate context.
        s_deletedVertexProgramObjectCache[contextID].push_back(handle);
    }
}


void VertexProgram::flushDeletedVertexProgramObjects(unsigned int contextID,double /*currentTime*/, double& availableTime)
{
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedVertexProgramObjectCache);

        const GLExtensions* extensions = GLExtensions::Get(contextID,true);

        VertexProgramObjectList& vpol = s_deletedVertexProgramObjectCache[contextID];

        for(VertexProgramObjectList::iterator titr=vpol.begin();
            titr!=vpol.end() && elapsedTime<availableTime;
            )
        {
            extensions->glDeletePrograms( 1L, &(*titr ) );
            titr = vpol.erase(titr);
            elapsedTime = timer.delta_s(start_tick,timer.tick());
        }
    }

    availableTime -= elapsedTime;
}

void VertexProgram::discardDeletedVertexProgramObjects(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedVertexProgramObjectCache);
    VertexProgramObjectList& vpol = s_deletedVertexProgramObjectCache[contextID];
    vpol.clear();
}


VertexProgram::VertexProgram()
{
}


VertexProgram::VertexProgram(const VertexProgram& vp,const CopyOp& copyop):
    osg::StateAttribute(vp,copyop)
{
    _vertexProgram = vp._vertexProgram;

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
VertexProgram::~VertexProgram()
{
    dirtyVertexProgramObject();
}

void VertexProgram::dirtyVertexProgramObject()
{
    for(unsigned int i=0;i<_vertexProgramIDList.size();++i)
    {
        if (_vertexProgramIDList[i] != 0)
        {
            VertexProgram::deleteVertexProgramObject(i,_vertexProgramIDList[i]);
            _vertexProgramIDList[i] = 0;
        }
    }
}

void VertexProgram::apply(State& state) const
{
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE

    const GLExtensions* extensions = state.get<GLExtensions>();

    if (!extensions->isVertexProgramSupported)
        return;


    GLuint& vertexProgramId=getVertexProgramID(state.getContextID());

    // Vertex Program
    if (vertexProgramId != 0)
    {
        extensions->glBindProgram( GL_VERTEX_PROGRAM_ARB, vertexProgramId );
    }
    else if (!_vertexProgram.empty())
    {
        glGetError(); // Reset Error flags.
        extensions->glGenPrograms( 1, &vertexProgramId );
        extensions->glBindProgram( GL_VERTEX_PROGRAM_ARB, vertexProgramId );
        extensions->glProgramString( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                                     _vertexProgram.length(), _vertexProgram.c_str());

        // Check for errors
        GLint errorposition = 0;
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorposition);
        if (errorposition != -1)
        {
            OSG_FATAL << "VertexProgram: " << glGetString(GL_PROGRAM_ERROR_STRING_ARB) << std::endl;

            std::string::size_type start = _vertexProgram.rfind('\n', errorposition);
            std::string::size_type stop  = _vertexProgram.find('\n', errorposition);
            if (start!=std::string::npos && stop!=std::string::npos)
            {
                OSG_FATAL << "             : " << _vertexProgram.substr(start+1, stop-start-2) << std::endl;
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
            glMatrixMode((*itr).first);
            glLoadMatrix((*itr).second.ptr());
        }
        glMatrixMode(GL_MODELVIEW); // restore matrix mode
    }
#else
    OSG_NOTICE<<"Warning: VertexProgram::apply(State&) - not supported."<<std::endl;
#endif
}

void VertexProgram::resizeGLObjectBuffers(unsigned int maxSize)
{
    _vertexProgramIDList.resize(maxSize);
}

void VertexProgram::releaseGLObjects(State* state) const
{
    if (!state) const_cast<VertexProgram*>(this)->dirtyVertexProgramObject();
    else
    {
        unsigned int contextID = state->getContextID();
        if (_vertexProgramIDList[contextID] != 0)
        {
            VertexProgram::deleteVertexProgramObject(contextID,_vertexProgramIDList[contextID]);
            _vertexProgramIDList[contextID] = 0;
        }
    }
}
