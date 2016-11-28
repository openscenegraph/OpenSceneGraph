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


class DrawArraysIndirect : public osg::DrawArrays
{
public:
  DrawArraysIndirect(GLenum mode=0,  unsigned int indirect=0)
   : osg::DrawArrays(mode), _indirect(indirect)
  {
  }
  virtual osg::Object* cloneType() const { return new DrawArraysIndirect(); }
  virtual osg::Object* clone(const osg::CopyOp& /*copyop*/) const { return NULL; }
  virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const DrawArraysIndirect*>(obj)!=NULL; }
  virtual const char* libraryName() const { return "osg"; }
  virtual const char* className() const { return "DrawArraysIndirect"; }

  virtual void draw(osg::State& state, bool useVertexBufferObjects) const;
protected:
  unsigned int _indirect;
};

class MultiDrawArraysIndirect : public osg::DrawArrays
{
public:
  MultiDrawArraysIndirect(GLenum mode=0, unsigned int indirect=0, GLsizei drawcount=0, GLsizei stride=0)
   : osg::DrawArrays(mode),   _indirect(indirect), _drawcount(drawcount), _stride(stride)
  {
  }
  virtual osg::Object* cloneType() const { return new MultiDrawArraysIndirect(); }
  virtual osg::Object* clone(const osg::CopyOp& /*copyop*/) const { return NULL; }
  virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const MultiDrawArraysIndirect*>(obj)!=NULL; }
  virtual const char* libraryName() const { return "osg"; }
  virtual const char* className() const { return "MultiDrawArraysIndirect"; }

  virtual void draw(osg::State& state, bool useVertexBufferObjects) const;
protected:
  unsigned int _indirect;
  GLsizei _drawcount;
  GLsizei _stride;
};


#endif
