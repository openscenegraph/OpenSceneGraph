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
    osg::BufferObject::flushDeletedBufferObjects(contextID,currentTime,availableTime);
    osg::Drawable::flushDeletedDisplayLists(contextID,availableTime);
    osg::FragmentProgram::flushDeletedFragmentProgramObjects(contextID,currentTime,availableTime);
    osg::FrameBufferObject::flushDeletedFrameBufferObjects(contextID,currentTime,availableTime);
    osg::Program::flushDeletedGlPrograms(contextID,currentTime,availableTime);
    osg::RenderBuffer::flushDeletedRenderBuffers(contextID,currentTime,availableTime);
    osg::Shader::flushDeletedGlShaders(contextID,currentTime,availableTime);
    osg::Texture::flushDeletedTextureObjects(contextID,currentTime,availableTime);
    osg::VertexProgram::flushDeletedVertexProgramObjects(contextID,currentTime,availableTime);
    osg::OcclusionQueryNode::flushDeletedQueryObjects(contextID,currentTime,availableTime);
}

void osg::flushAllDeletedGLObjects(unsigned int contextID)
{
    double currentTime = DBL_MAX;
    double availableTime = DBL_MAX;
    osg::BufferObject::flushDeletedBufferObjects(contextID,currentTime,availableTime);
    osg::Drawable::flushAllDeletedDisplayLists(contextID);
    osg::FragmentProgram::flushDeletedFragmentProgramObjects(contextID,currentTime,availableTime);
    osg::FrameBufferObject::flushDeletedFrameBufferObjects(contextID,currentTime,availableTime);
    osg::Program::flushDeletedGlPrograms(contextID,currentTime,availableTime);
    osg::RenderBuffer::flushDeletedRenderBuffers(contextID,currentTime,availableTime);
    osg::Shader::flushDeletedGlShaders(contextID,currentTime,availableTime);
    osg::Texture::flushAllDeletedTextureObjects(contextID);
    osg::VertexProgram::flushDeletedVertexProgramObjects(contextID,currentTime,availableTime);
    osg::OcclusionQueryNode::flushDeletedQueryObjects(contextID,currentTime,availableTime);
}

void osg::discardAllDeletedGLObjects(unsigned int contextID)
{
    osg::BufferObject::discardDeletedBufferObjects(contextID);
    osg::Drawable::discardAllDeletedDisplayLists(contextID);
    osg::FragmentProgram::discardDeletedFragmentProgramObjects(contextID);
    osg::FrameBufferObject::discardDeletedFrameBufferObjects(contextID);
    osg::Program::discardDeletedGlPrograms(contextID);
    osg::RenderBuffer::discardDeletedRenderBuffers(contextID);
    osg::Shader::discardDeletedGlShaders(contextID);
    osg::Texture::discardAllDeletedTextureObjects(contextID);
    osg::VertexProgram::discardDeletedVertexProgramObjects(contextID);
    osg::OcclusionQueryNode::discardDeletedQueryObjects(contextID);
}
