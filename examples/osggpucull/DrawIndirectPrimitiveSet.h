/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
 *  Copyright (C) 2014 Pawel Ksiezopolski
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
 *
*/

#ifndef OSG_DRAWINDIRECTPRIMITIVESET
#define OSG_DRAWINDIRECTPRIMITIVESET 1

#include <osg/PrimitiveSet>
#include <osg/BufferObject>
#include <osg/TextureBuffer>

#ifndef GL_ARB_draw_indirect
    #define GL_DRAW_INDIRECT_BUFFER           0x8F3F
    #define GL_DRAW_INDIRECT_BUFFER_BINDING   0x8F43
#endif

#ifndef GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
    #define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT    0x00000020
#endif


class DrawArraysIndirect : public osg::DrawArrays
{
public:
  DrawArraysIndirect(GLenum mode=0, osg::TextureBuffer* buffer=NULL, unsigned int indirect=0)
   : osg::DrawArrays(mode), _buffer(buffer), _indirect(indirect)
  {
  }
  virtual osg::Object* cloneType() const { return new DrawArraysIndirect(); }
  virtual osg::Object* clone(const osg::CopyOp& copyop) const { return NULL; }
  virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const DrawArraysIndirect*>(obj)!=NULL; }
  virtual const char* libraryName() const { return "osg"; }
  virtual const char* className() const { return "DrawArraysIndirect"; }

  virtual void draw(osg::State& state, bool useVertexBufferObjects) const;
protected:
  osg::ref_ptr<osg::TextureBuffer> _buffer;
  unsigned int _indirect;
};

class MultiDrawArraysIndirect : public osg::DrawArrays
{
public:
  MultiDrawArraysIndirect(GLenum mode=0, osg::TextureBuffer* buffer=NULL, unsigned int indirect=0, GLsizei drawcount=0, GLsizei stride=0)
   : osg::DrawArrays(mode), _buffer(buffer), _indirect(indirect), _drawcount(drawcount), _stride(stride)
  {
  }
  virtual osg::Object* cloneType() const { return new MultiDrawArraysIndirect(); }
  virtual osg::Object* clone(const osg::CopyOp& copyop) const { return NULL; }
  virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const MultiDrawArraysIndirect*>(obj)!=NULL; }
  virtual const char* libraryName() const { return "osg"; }
  virtual const char* className() const { return "MultiDrawArraysIndirect"; }

  virtual void draw(osg::State& state, bool useVertexBufferObjects) const;
protected:
  osg::ref_ptr<osg::TextureBuffer> _buffer;
  unsigned int _indirect;
  GLsizei _drawcount;
  GLsizei _stride;
};

class DrawIndirectGLExtensions : public osg::Referenced
{
public:
    DrawIndirectGLExtensions( unsigned int contextID );
    DrawIndirectGLExtensions( const DrawIndirectGLExtensions &rhs );
    void lowestCommonDenominator( const DrawIndirectGLExtensions &rhs );
    void setupGLExtensions( unsigned int contextID );

    void glDrawArraysIndirect(GLenum  mode,  const void * indirect) const;
    void glMultiDrawArraysIndirect(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
    void glMemoryBarrier(GLbitfield barriers);

    static DrawIndirectGLExtensions* getExtensions( unsigned int contextID,bool createIfNotInitalized );
protected:

    typedef void ( GL_APIENTRY *DrawArraysIndirectProc ) (GLenum  mode,  const void * indirect);
    typedef void ( GL_APIENTRY *MultiDrawArraysIndirectProc ) (GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
    typedef void ( GL_APIENTRY *MemoryBarrierProc ) (GLbitfield barriers);

    DrawArraysIndirectProc _glDrawArraysIndirect;
    MultiDrawArraysIndirectProc _glMultiDrawArraysIndirect;
    MemoryBarrierProc _glMemoryBarrier;

};



#endif
