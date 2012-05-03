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
#include <osg/GLObjects>

#include <osg/Texture>
#include <osg/VertexProgram>
#include <osg/FragmentProgram>
#include <osg/Shader>
#include <osg/BufferObject>
#include <osg/FrameBufferObject>
#include <osg/Drawable>
#include <osg/OcclusionQueryNode>

void osg::flushDeletedGLObjects(unsigned int contextID, double currentTime, double& availableTime)
{
#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    osg::Drawable::flushDeletedDisplayLists(contextID,availableTime);
#endif

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    osg::FragmentProgram::flushDeletedFragmentProgramObjects(contextID,currentTime,availableTime);
    osg::VertexProgram::flushDeletedVertexProgramObjects(contextID,currentTime,availableTime);
#endif

    osg::GLBufferObject::flushDeletedBufferObjects(contextID,currentTime,availableTime);
    osg::FrameBufferObject::flushDeletedFrameBufferObjects(contextID,currentTime,availableTime);
    osg::RenderBuffer::flushDeletedRenderBuffers(contextID,currentTime,availableTime);
    osg::Program::flushDeletedGlPrograms(contextID,currentTime,availableTime);
    osg::Shader::flushDeletedGlShaders(contextID,currentTime,availableTime);
    osg::Texture::flushDeletedTextureObjects(contextID,currentTime,availableTime);
    osg::OcclusionQueryNode::flushDeletedQueryObjects(contextID,currentTime,availableTime);
}

void osg::flushAllDeletedGLObjects(unsigned int contextID)
{
    double currentTime = DBL_MAX;
    double availableTime = DBL_MAX;

#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    osg::Drawable::flushAllDeletedDisplayLists(contextID);
#endif

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    osg::FragmentProgram::flushDeletedFragmentProgramObjects(contextID,currentTime,availableTime);
    osg::VertexProgram::flushDeletedVertexProgramObjects(contextID,currentTime,availableTime);
#endif

    osg::GLBufferObject::flushAllDeletedBufferObjects(contextID);
    osg::Texture::flushAllDeletedTextureObjects(contextID);

    osg::FrameBufferObject::flushDeletedFrameBufferObjects(contextID,currentTime,availableTime);
    osg::Program::flushDeletedGlPrograms(contextID,currentTime,availableTime);
    osg::RenderBuffer::flushDeletedRenderBuffers(contextID,currentTime,availableTime);
    osg::Shader::flushDeletedGlShaders(contextID,currentTime,availableTime);
    osg::OcclusionQueryNode::flushDeletedQueryObjects(contextID,currentTime,availableTime);
}


void osg::deleteAllGLObjects(unsigned int contextID)
{
    double currentTime = DBL_MAX;
    double availableTime = DBL_MAX;

#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    osg::Drawable::flushAllDeletedDisplayLists(contextID);
#endif

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    osg::FragmentProgram::flushDeletedFragmentProgramObjects(contextID,currentTime,availableTime);
    osg::VertexProgram::flushDeletedVertexProgramObjects(contextID,currentTime,availableTime);
#endif

    osg::GLBufferObject::deleteAllBufferObjects(contextID);
    osg::Texture::deleteAllTextureObjects(contextID);

    osg::FrameBufferObject::flushDeletedFrameBufferObjects(contextID,currentTime,availableTime);
    osg::Program::flushDeletedGlPrograms(contextID,currentTime,availableTime);
    osg::RenderBuffer::flushDeletedRenderBuffers(contextID,currentTime,availableTime);
    osg::Shader::flushDeletedGlShaders(contextID,currentTime,availableTime);
    osg::OcclusionQueryNode::flushDeletedQueryObjects(contextID,currentTime,availableTime);
}


void osg::discardAllGLObjects(unsigned int contextID)
{
#ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    osg::Drawable::discardAllDeletedDisplayLists(contextID);
#endif

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    osg::FragmentProgram::discardDeletedFragmentProgramObjects(contextID);
    osg::VertexProgram::discardDeletedVertexProgramObjects(contextID);
#endif

    osg::GLBufferObject::discardAllBufferObjects(contextID);
    osg::Texture::discardAllTextureObjects(contextID);

    osg::FrameBufferObject::discardDeletedFrameBufferObjects(contextID);
    osg::Program::discardDeletedGlPrograms(contextID);
    osg::RenderBuffer::discardDeletedRenderBuffers(contextID);
    osg::Shader::discardDeletedGlShaders(contextID);
    osg::OcclusionQueryNode::discardDeletedQueryObjects(contextID);
}
