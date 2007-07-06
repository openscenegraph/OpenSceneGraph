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

void osg::flushDeletedGLObjects(unsigned int contextID, double currentTime, double& availableTime)
{
    osg::FrameBufferObject::flushDeletedFrameBufferObjects(contextID,currentTime,availableTime);
    osg::RenderBuffer::flushDeletedRenderBuffers(contextID,currentTime,availableTime);
    osg::Texture::flushDeletedTextureObjects(contextID,currentTime,availableTime);
    osg::Drawable::flushDeletedDisplayLists(contextID,availableTime);
    osg::Drawable::flushDeletedVertexBufferObjects(contextID,currentTime,availableTime);
    osg::VertexProgram::flushDeletedVertexProgramObjects(contextID,currentTime,availableTime);
    osg::FragmentProgram::flushDeletedFragmentProgramObjects(contextID,currentTime,availableTime);
    osg::Program::flushDeletedGlPrograms(contextID,currentTime,availableTime);
    osg::Shader::flushDeletedGlShaders(contextID,currentTime,availableTime);
    osg::BufferObject::flushDeletedBufferObjects(contextID,currentTime,availableTime);
}

void osg::flushAllDeletedGLObjects(unsigned int contextID)
{
    double currentTime = DBL_MAX;
    double availableTime = DBL_MAX;
    osg::FrameBufferObject::flushDeletedFrameBufferObjects(contextID,currentTime,availableTime);
    osg::RenderBuffer::flushDeletedRenderBuffers(contextID,currentTime,availableTime);
    osg::Texture::flushAllDeletedTextureObjects(contextID);
    osg::Drawable::flushAllDeletedDisplayLists(contextID);
    osg::Drawable::flushDeletedVertexBufferObjects(contextID,currentTime,availableTime);
    osg::VertexProgram::flushDeletedVertexProgramObjects(contextID,currentTime,availableTime);
    osg::FragmentProgram::flushDeletedFragmentProgramObjects(contextID,currentTime,availableTime);
    osg::Program::flushDeletedGlPrograms(contextID,currentTime,availableTime);
    osg::Shader::flushDeletedGlShaders(contextID,currentTime,availableTime);
    osg::BufferObject::flushDeletedBufferObjects(contextID,currentTime,availableTime);
}

